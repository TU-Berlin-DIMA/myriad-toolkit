Getting Started - Build & Installation Instructions
===================================================

This document contains a brief description of the project layout and instructions for building the *${{dgen_name}}* data generator. Users that are not familiar with the build and setup specifics of Myriad-based data generators should read the following text carefully before they start using the downloaded software.

In the following discussion, we refer to the set of core components comprising the Myriad toolkit as *toolkit* and to the set of toolkit extensions 
comprising the *${{dgen_name}}* data generator as *${{dgen_name}} extensions* or *generator extensions*.

For detailed instructions on configuring and running the compiled data generator, please refer to the `README.md` located in `src/resources/` (or, equivalently, to the `README.md` in the installation path of the installed package).


Folder Structure
================

The basic layout of all Myriad-based data generators looks like that:

    build/            target folder for the build process
    
    src/              generator extension sources
      config/         extension specific config files
      cpp/            extension specific C++ sources
      resources/      install package resources (e.g. README, LICENSE)
    
    vendor/           third party libraries
      myriad-toolkit/ Myriad toolkit
        bin/          contains the Myriad CLI tools (e.g. assistant)
        src/          Myriad toolkit sources
          config/     common config-related files
          cpp/        toolkit C++ sources
          python/     toolkit Python sources
          script/     script files (e.g. Python parallel frontend)
        tools/        sources for the CLI tools


Initializing Git Submodules
===========================

If you are cloning the data generator project from a Git repository, the Myriad toolkit package is most likely configured as a submodule pointing to the `vendor/myriad-toolkit` folder and you have to explicitly initialize the submodule repository to and fetch the contents under `vendor/myriad-toolkit`. To do this, execute the following commands:

    <project-root>$ git submodule init
    <project-root>$ git submodule update

After that you should be able to see the toolkit contents under `vendor/myriad-toolkit`. 

By default, git submodules are checked out in a so called *detached HEAD* mode. This means that the changeset to which the submodule points is cloned in a detached tree that does not track back the `origin` repository. The output of the `git branch` right after the initial `git submodule update` therefore should look somewhat like that:

    <project-root>/vendor/myriad-toolkit$ git branch
    * (no branch)
      main_branch

Note that in order to push submodule changesets upstream, you first have to explicitly switch the branch in that submodule. For instance, if you want to work directly on the `v0.2.x` branch in the `vendor/myriad-toolkit` submodule, go to the `vendor/myriad-toolkit` folder and issue the following commands:

    <project-root>/vendor/myriad-toolkit$ git checkout v0.2.x
    <project-root>/vendor/myriad-toolkit$ git branch
    * v0.2.x
      main_branch
      
If you don't envision making changes to the submodule you can leave the detached HEAD as it is. In this case, to propagete updates in the submodules to your local repository you need to execute the update command:

    <project-root>$ git submodule update


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

    <project-root>/build> make -s prototype
    <project-root>/build> make -s all
    <project-root>/build> make -s install

This should build and install the data generator under `<install-path>/${{dgen_name}}`. For detailed instructions on configuring and running the compiled data generator, please refer to `<install-path>/${{dgen_name}}/README.md`.


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
