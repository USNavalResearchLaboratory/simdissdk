Overview
========
The SIMDIS SDK installation instructions will cover the following topics:

* Supported Systems/Compilers
* Third Party Dependencies
* Binary Installation
* Source Installation
* System Environment Setup


Supported Systems/Compilers
===========================
Operating systems officially supported by the SIMDIS SDK:

* Windows 10
* Red Hat Enterprise Linux 7

Compilers officially supported by the SIMDIS SDK:

* Windows compilers:
  - Microsoft Visual C++ 2019 (VC-14.2)
  - Microsoft Visual C++ 2022 (VC-14.3)
* Linux compilers:
  - GCC, minimum 8.3

Other compiler combinations may work, or could work with minimal improvements
to the CMake configuration.  This is only a list of the systems that we internally
are able to support.  We are glad to accept pull requests supporting new
compilers.

C++-11 support is be mandatory for SIMDIS SDK compilers starting as of
September 2019, after the 1.11 release.  This eliminated support for
GCC 4.4.  RHEL 7 users can continue to build the SIMDIS SDK by using the
devtoolset package from the RHEL SCL, in order to use an ABI-compatible
compiler with newer language support.


Third Party Dependencies
========================
The SIMDIS SDK depends on the following third party libraries:

- [GLEW 2.1+](http://glew.sourceforge.net/)
- [OpenSceneGraph 3.6+](http://www.openscenegraph.org)
- [osgEarth 3.2+](http://www.osgearth.org)
- [protobuf 2.6+](http://code.google.com/p/protobuf)
- [Python 3.6+](https://www.python.org/)
- [Qt 5.5+](http://qt-project.org)
- [SQLite 3.8+](http://www.sqlite.org)
- [SWIG 4.0+](http://www.swig.org/)


While other versions may also work, we can only support the configurations
that we build against.

Precompiled Windows binaries for all of the SIMDIS SDK dependencies can be
obtained from the SIMDIS SDK project download page.  Linux users may find that
many of the SIMDIS SDK dependencies are available from the package repository
for their Linux distribution.

To build the dependencies from source, obtain the source packages from the
specified project websites and follow the included build and installation
instructions.

The SIMDIS SDK requires a minimum GLSL version 3.3 to support osgEarth.  We
internally only test OpenSceneGraph and osgEarth against the Core Profile,
since it has wider support across a larger variety of hardware for OpenGL 3.3.
If you build OpenSceneGraph yourself, we recommend
[building OpenSceneGraph with GL3 support](https://github.com/openscenegraph/OpenSceneGraph/blob/master/examples/osgsimplegl3/osgsimplegl3.cpp#L121).

We also maintain a "Third Party Pack" for each supported architecture on the
[Releases page](https://github.com/USNavalResearchLaboratory/simdissdk/releases).


SWIG and Python are only required if you intend to build the Python modules
in the swig/ subdirectory.  This feature is disabled by default.

**NOTE:** Both OpenSceneGraph and osgEarth have additional third party
dependencies.  More information about these dependencies can be found at the
OpenSceneGraph and osgEarth web sites.  Linux users will likely find that all
of these additional dependencies are available in the package repository for
their Linux distribution.  For your convenience, precompiled binaries for these
dependencies have been included with the precompiled binary packages for
Windows.


Binary Installation
===================
**If you are compiling the SIMDIS SDK from source, use the instructions in the
next section. You may skip this section, proceeding to the next section
entitled "Source Installation".**

To install the precompiled SIMDIS SDK binaries available from the SIMDIS SDK
project page, simply extract the zip file containing the SIMDIS SDK files to a
directory of your choosing.  Make note of the location of the SIMDIS SDK
installation.  The documentation will refer to this directory as &lt;simdis-sdk
-bin-dir&gt;.  Whenever you see this value, &lt;simdis-sdk-bin-dir&gt;,
substitute the name of the directory on your system that contains the
installation of the SIMDIS SDK.


Source Installation
===================
**If you do not plan on building the SIMDIS SDK from source, and instead plan
to use one of the precompiled binary versions available from the SIMDIS SDK
project page, you may skip this section and proceed to the next section
entitled "System Environment Setup".**

1.  Installing the source:
You must first extract the SIMDIS SDK from the zip file obtained from the
SIMDIS SDK project page to a directory of your choosing.  Make note of the
location of the SIMDIS SDK.  The documentation will refer to this directory as
&lt;simdis-sdk-src-dir&gt;.  Whenever you see this value, &lt;simdis-sdk-src-dir&gt;,
substitute the name of the directory on your system that contains the SIMDIS
SDK source code.

2.  Configuring the build:
  The SIMDIS SDK makes use of the CMake Cross Platform Make build system for
  generating platform specific build files.  If you do not already have CMake
  installed on your system, you must obtain and install it.  CMake can be
  obtained from the [CMake](http://www.cmake.org) project web page.

  You will use the 'cmake-gui' application to configure the SIMDIS SDK for
  building.  Start 'cmake-gui' and specify the source and build directories.
  You should specify &lt;simdis-sdk-src-dir&gt; as the source directory.  You may
  also specify &lt;simdis-sdk-src-dir&gt; as the build directory, but it is
  recommended that you keep the build directory separate from the source
  directory, using a directory such as &lt;simdis-sdk-src-dir&gt;/build.

  Now click the 'Configure' button and select your target build system.  CMake
  will generate a list of variables describing the build properties, such as
  the locations of the 3rd party dependencies, and display them in a list.
  Many of 3rd party dependency locations will be initially marked as
  'NOTFOUND'.  Specify the correct paths to the 3rd party dependencies and
  click 'Configure' again, then click Generate.  You are now ready to build
  the SIMDIS SDK source code.

  **NOTE: If you have installed all of the third party dependencies in the same
  directory(for example if you download 3rd party dependencies from
  [Releases page](https://github.com/USNavalResearchLaboratory/simdissdk/releases),
  extract it and put the "3rd" folder in the source code directory not the build
  directory), the SIMDIS SDK CMake configuration should locate the files.  You
  do not have to specify the location of each 3rd party library independently.**

  **NOTE: You can specify the installation location for the SIMDIS SDK with
  the "CMAKE\_INSTALL\_PREFIX" variable.**

3.  Compiling the source:
To compile the source, simply start the build using the build system that you
specified in the previous section.  Microsoft Visual Studio users will find
solution files for the SIMDIS SDK in the build directory specified to CMake.
Make users will find a make file in the build directory specified to CMake.

4.  Installing the results:
The CMake project files include support for installing the SIMDIS SDK files
generated by the build process.  Microsoft Visual Studio users can run the install
process by building the INSTALL project that appears in the solution under the
CMakePredefinedTargets folder.  'make' users can run the install process by
invoking the 'make install' command.

Make note of the location of the SIMDIS SDK installation.  The documentation
will refer to this directory as &lt;simdis-sdk-bin-dir&gt;.  Whenever you see
this value, &lt;simdis-sdk-bin-dir&gt;, substitute the name of the directory on
your system that contains the installation of the SIMDIS SDK.


System Environment Setup
========================
1.  Environment setup for the sample data set
The example programs provided with the SIMDIS SDK require a sample data set
for proper operation.  This data set can be obtained from the SIMDIS website;
a free SIMDIS account is required.  Once you have downloaded the zip file containing
the sample data set, you should extract it to a directory of your choosing.
After the files have been extracted, you must create an environment variable
named:

  SIMDIS\_SDK\_FILE\_PATH

  whose value is set to the path to the directory containing the sample data.

2.  Setting the system library search path
  The system library and program search paths must be setup to find the
  libraries and programs installed for the SIMDIS SDK.  Windows users need to
  add &lt;simdis-sdk-bin-dir&gt;/bin to the PATH environment variable, so
  Windows knows where to look for the SIMDIS SDK DLLs and example programs.

  Linux users will need to add &lt;simdis-sdk-bin-dir&gt;/bin to their PATH
  environment variable to run the SIMDIS SDK examples from the command line
  without having to specify the fill path to the example executables.  The
  directory &lt;simdis-sdk-bin-dir&gt;/lib will need to be added to the
  system's library search path so that the system knows where to find the
  SIMDIS SDK shared libraries.  This can be done with RPATH, LD\_LIBRARY\_PATH,
  or ldconfig.  Some notes on shared libraries can be found [here](http://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html)

  **NOTE: Use of LD\_LIBRARY\_PATH is considered bad practice/dangerous.**

  **NOTE: If you only want to run the SIMDIS SDK example programs, you should
  not need to setup the library search path.**

3.  Setting up a project that uses the SIMDIS SDK
Projects using the SIMDIS SDK must add &lt;simdis-sdk-bin-dir&gt;/include to
their include path and &lt;simdis-sdk-bin-dir&gt;/lib to their library path.
Microsoft Visual Studio users can set these values on a project by project
basis by specifying the appropriate values in the project settings, or can set
these values globally by adding them to the "Include files" and "Library files"
lists found in the "VC++ Directories" section of the "Projects and Solutions"
section of the Options dialog.  The Options dialog is accessed through the
Microsoft Visual Studio Tools menu.
