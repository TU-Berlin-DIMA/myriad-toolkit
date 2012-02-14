Getting Started
===================================

This document contains a brief description of the project layout and instructions for building and running the *${{dgen_name}}* data generator. Users that are not familiar with the build and setup specifics of Myriad based data generators should read the following text carefully before they start using the downloaded software.

In the following discussion, we refer to the set of core components comprising the Myriad toolkit as *toolkit* and to the set of toolkit extensions 
comprising the *${{dgen_name}}* data generator as *${{dgen_name}} extensions* or *generator extensions*.

Folder Structure
================

The basic layout of all Myriad-based data generators looks like that:

    build/            target folder for the build process
    
    src/              generator extension sources
      config/         extension specific config files
      cpp/            extension specific C++ sources
    
    vendor/           third party libraries
      myriad/         Myriad toolkit
        src/          Myriad toolkit sources
          config/     common config-related files
          cpp/        toolkit C++ sources
          python/     toolkit Python sources
          script/     script files (e.g. Python parallel frontend)

Please note that if you are cloning the data generator project from a Git repository, the Myriad toolkit package is most likely configured as a submodule pointing to the `vendor/myriad` folder and you have to explicitly initialize the submodule repository to and fetch the contents under `vendor/myriad`. To do this, execute the following commands:

    <project-root>$ git submodule init
    <project-root>$ git submodule update

After that you should be able to see the toolkit contents under `vendor/myriad`.


Prerequisites
=============

The build process for all Myriad-based generators depends on the packages listed in this section. Please make sure you have them all installed before you start the build of your generator.

GNU Build Tools
---------------

The build process is implemented as a set of `makefiles`. Make sure you have 

* the *build-essential* package (Ubuntu users)
* the *Development Tools* package (RedHat users)
* the *GNU C++ compiler (g++)* and the *make* tool (other Linux distributions)

installed on your build system.

POCO C++ Libraries
------------------

The Myriad Toolkit is built on top of the POCO C++ Libraries. Consequently, the build process depends on the following POCO libraries:

    libPocoFoundation.so
    libPocoNet.so
    libPocoUtil.so
    libPocoXML.so

All libraries are part of the "Basic Edition" distribution package of POCO (version >= 1.4.1p1) which can be obtained [here](http://pocoproject.org/download/index.html).

To ease deployment on parallel environments, the toolkit build bundles the shared object POCO libraries in a single distribution package. The build system therefore needs to know the path where the POCO Libraries are installed (configured using the `configure` script -- see the next section).

A brief installation guide for the POCO C++ Libraries, please refer to Appendix A. at the end of this document.

Python
------

The parallel deployment script generated under `bin/${{dgen_name}}-frontend` as well as some other utility scripts are written in Python. For those to work, you have to make sure that the build host and the master node in your distributed build environment (normally the same machine) have Python >= 2.4.3 installed.


Building on UNIX / Linux distributions
======================================

Before you start the build process, you have to configure your build environment. To do this, execute the following command:

    <project-root>$ ./configure --prefix=<install-path> --poco-prefix=<poco-path>

The first option specifies the base directory where data generator package will be installed after the build. We advise to use a path which is exported on a shared file system and is accessible from all nodes in your environment. This is convenient as otherwise you will have to manually copy the contents of the installation folder to all nodes in your distributed build environment.

The second option specifies the installation path of the POCO C++ Libraries and should have the same value as the `--prefix` option configured when installing 
the POCO libraries (see Appendix A).

For a full list of all supported options type:

    <project-root>$ ./configure --help

Upon configuration, go to the `build` folder and issue the following commands:

<project-root>/build> make -s all
<project-root>/build> make -s install

This should build and install the data generator under `<install-path>/${{dgen_name}}`. For the remainder of this document, we will use `<${{dgen_name}}-install>` as an alias for `<install-path>/${{dgen_name}}`.


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
      <cloud-environments>
        ...
      </cloud-environments>
  
      <!-- a set of data generator environments -->
      <dgen-configurations>
        ...
      </dgen-configurations>
    
    </myriad>


Cloud Environments Configuration
--------------------------------

The `<cloud-enviroments>` section consists of a list of <cloud-env> elements, each one representing a single distributed computation environment. Each <cloud-env> defines of a list of *slave hosts* and a single *master host*. This is very similar to the contens of the `masters` and `slaves` files in Hadoop.

For instance, consider a scenario where you have a 10 node cluster but only want to use the first five nodes for the data generation process. This will translate into the following `<cloud-env>` entry:

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

In other words, you can use different `<cloud-env>` elements to configure multiple parallelization environments operating on the same network. The only requirements for the configured environments are:

* The master node should be able to communicate with all the slave nodes through a password-less ssh connection (similar to a Hadoop setup).
* The `<${{dgen_name}}-install>` should be is accessible from all nodes (this is best achieved through NFS exported install path).


Deployment Configurations
-------------------------

The `<dgen-configurations>` section defines a set of `<dgen-config>` configuration entries that can be used with the Python frontend. Each `<dgen-config>` represents a concrete way to use a specific cloud environment for parallel data generation. 

So if, for instance, we want to use the `mycloud-05` environment from above and run two node instances per host, and we want to store the data generated by each instance inside the `/data/myriad` path of the corresponding node, we have to add the following entry:

    <dgen-config id="mycloud-10x4" cloud="mycloud-10">
      <nodes-per-host>4</nodes-per-host>
      <output-base>/data/myriad</output-base>
    </dgen-config>

We recommend choosing a `nodes-per-host` value in the range [X,2X] where X is the number of processor cores in each host in the referenced environment. You can start the parallel deployment script for a particular <DATASET_ID> and scaling factor <SF> with this command:

    <${{dgen_name}}-install>$ bin/${{dgen_name}}-frontend -m<DATASET_ID> -s<SF> <DGEN-CONFIG-ID>


Appendix
========

A. Installing the POCO C++ Libraries
------------------------------ ------

For a straight forward installation of the POCO C++ Libraries issue the following commands in a terminal:

    $ POCO_PATH = "~/etc" # or some other install path, typically "/usr/local"
    $ tar -xzvf poco-<version>.tar.gz
    $ cd poco-<version>
    poco-<version>$ ./configure --no-tests --no-samples --shared --prefix=$POCO_PATH
    poco-<version>$ make -s all install

If the installation is successful, you should see the installed POCO Libraries under `$POCO_PATH/lib`:

    $ ls -la $POCO_PATH/lib/libPoco*
    $POCO_PATH/libPocoFoundationd.so
    $POCO_PATH/libPocoFoundationd.so.11
    $POCO_PATH/libPocoFoundation.so
    $POCO_PATH/libPocoFoundation.so.11
    $POCO_PATH/libPocoNetd.so
    $POCO_PATH/libPocoNetd.so.11
    $POCO_PATH/libPocoNet.so
    $POCO_PATH/libPocoNet.so.11
    $POCO_PATH/libPocoUtild.so
    $POCO_PATH/libPocoUtild.so.11
    $POCO_PATH/libPocoUtil.so
    $POCO_PATH/libPocoUtil.so.11
    $POCO_PATH/libPocoXMLd.so
    $POCO_PATH/libPocoXMLd.so.11
    $POCO_PATH/libPocoXML.so
    $POCO_PATH/libPocoXML.so.11
