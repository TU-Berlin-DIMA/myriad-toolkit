Getting Started - Configuring And Running The Data Generator
============================================================

This document contains a brief description of the project layout and instructions for running the *${{dgen_name}}* data generator. Users that are not familiar with the specifics of running Myriad-based data generators should read the following text in order to understand how the parallel data generation process can be configured and initiated.

We assume that the reader has already followed the *Build and Install* instructions from the `README.md` file located in the root of the source project and has installed the *${{dgen_name}}* data generator under the install path `<${{dgen_name}}-install>`.

In the following discussion, we refer to the set of core components comprising the Myriad toolkit as *toolkit* and to the set of toolkit extensions comprising the *${{dgen_name}}* data generator as *${{dgen_name}} extensions* or *generator extensions*.


Manually Starting Generator Nodes
=================================

If you look at the contents of the `<${{dgen_name}}-install>/bin` folder, you will find two files -- `${{dgen_name}}-frontend` and `${{dgen_name}}-node`. 

The first file is the CLI frontend to the simple parallel deployment script that comes bundled with the generator build (to be discussed in the next section). 

The second file is the C++ executable that represents a single data generator node. If you don't want to use the Python frontend script, you have to start all your generator nodes manually or use some other parallel deployment container (e.g. a MapReduce engine like Hadoop). To start a parallel generation process manually, you have to specify concrete values for the following parameters:

    Parameter:      Meaning:

    -s<SF>          Scaling factor for the generated dataset.

    -N<N>           The total number of parallel instances (required by the 
                    partitioning logic of each node).

    -i<i>           The index ID of the current node. Index numbers are 
                    zero-based, i.e. for <N> nodes you have to start instances  
                    with IDs from 0 to <N> - 1.

    -o<PATH>        Base path for writing the output. The path should be 
                    accessible from each node. For optimal I/O throughput, 
                    we recommend to create the same path on the local storage  
                    of each host that will run a generator node.

    -m<DATASET_ID>  ID of the generated dataset. This affects the last part of 
                    the path where the generated data is stored.

Once the values for these parameters are specified, you can start node `<i>` of your `<N>`-node parallel generation setup manually by typing:

    <${{dgen_name}}-install>$ bin/${{dgen_name}}-node -m<DATASET_ID> -s<SF> -i<i> -N<N> -o<PATH>

The data generator instance will store its output into the `<PATH>/<DATASET_ID>/node<i>` folder. The folder will contain a horizontal partition of each table in a separate text file. The logical concatenation of all `node<i>/table.txt` files represents the entire table.


Parallel Generation With The Python Frontend
============================================

If you want to use the Python parallel deployment frontend, you have to setup your parallel environment. For this purpose, you need to create a `${{dgen_name}}-frontend.xml` configuration inside the `<${{dgen_name}}-install>/config` folder (you can use `${{dgen_name}}-config.template.xml` bundled during the build process as a basis). The Python frontend config files has the following structure:

    <?xml version="1.0" encoding="utf-8"?>
    <myriad xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
            xmlns="http://www.dima.tu-berlin.de/myriad" 
            xsi:schemaLocation="http://www.dima.tu-berlin.de/myriad myriad-frontend.xsd">
  
      <!-- a set of cloud environments -->
      <cloud_environments>
        ...
      </cloud_environments>
  
      <!-- a set of data generator environments -->
      <dgen_configurations>
        ...
      </dgen_configurations>
    
    </myriad>


Cloud Environments Configuration
--------------------------------

The `<cloud_enviroments>` section consists of a list of `<cloud_env>` elements, each one representing a single distributed computation environment. Each `<cloud_env>` defines of a list of *slave hosts* and a single *master host*. This is very similar to the contents of the `masters` and `slaves` files in Hadoop.

For instance, consider a scenario where you have a 10 node cluster but only want to use the first five nodes for the data generation process. This will translate into the following `<cloud_env>` entry:

    <cloud-env id="mycloud-05" dgen="<${{dgen_name}}-install>">
      <master>
        <host name="myhost01.example.com" />
      </master>
      <slaves>
        <host name="myhost01.example.com" />
        <host name="myhost02.example.com" />
        <host name="myhost03.example.com" />
        <host name="myhost04.example.com" />
        <host name="myhost05.example.com" />
      </slaves>
    </cloud-env>

Now suppose that at a later point you want to use all 10 nodes. You can add a second `<cloud-env>` to reflect this: 

    <cloud-env id="mycloud-10" dgen="<${{dgen_name}}-install>">
      <master>
        <host name="myhost01.example.com" />
      </master>
      <slaves>
        <host name="myhost01.example.com" />
        <host name="myhost02.example.com" />
        <host name="myhost03.example.com" />
        <host name="myhost04.example.com" />
        <host name="myhost05.example.com" />
        <host name="myhost06.example.com" />
        <host name="myhost07.example.com" />
        <host name="myhost08.example.com" />
        <host name="myhost09.example.com" />
        <host name="myhost10.example.com" />
      </slaves>
    </cloud-env>

In other words, you can use different `<cloud_env>` elements to configure multiple parallelization environments operating on the same network. The only requirements for the configured environments are:

* The master node should be able to communicate with all the slave nodes through a password-less ssh connection (similar to a Hadoop setup).
* The `<${{dgen_name}}-install>` should be is accessible from all nodes (this is best achieved through NFS exported install path).


Deployment Configurations
-------------------------

The `<dgen_configurations>` section defines a set of `<dgen_config>` configuration entries that can be used with the Python frontend. Each `<dgen_config>` represents a concrete way to use a specific cloud environment for parallel data generation. 

So if, for instance, we want to use the `mycloud-05` environment from above and run two node instances per host, and we want to store the data generated by each instance inside the `/data/myriad` path of the corresponding node, we have to add the following entry:

    <dgen_config id="mycloud-10x4" cloud="mycloud-10">
      <nodes_per_host>4</nodes-per-host>
      <output_base>/data/myriad</output_base>
    </dgen_config>

We recommend choosing a `nodes-per-host` value in the range [X,2X] where X is the number of processor cores in each host in the referenced environment. You can start the parallel deployment script for a particular <DATASET_ID> and scaling factor <SF> with this command:

    <${{dgen_name}}-install>$ bin/${{dgen_name}}-frontend -m<DATASET_ID> -s<SF> <DGEN-CONFIG-ID>
