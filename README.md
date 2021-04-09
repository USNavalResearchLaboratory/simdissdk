About
=====
The SIMDIS Software Development Kit (SDK) is an open source C++ framework
providing functionality to create 3D scenes consisting of objects, whose
position and state change with time, that are placed relative to a geographic
map.  The primary modules provided by the SDK supply functionality to manage
object position/state data, update object position/state data as time
changes, and render 3D representations of objects within a 3D scene.
Secondary modules provide utilities for parsing [SIMDIS] data files, performing
coordinate system conversions, object state vector computations, and data
metric calculations.

The intent of the SIMDIS SDK is to provide a C++ framework to be used by
third party developers to incorporate functionality similar to that provided
by the SIMDIS 3D Visualization and Analysis Toolkit within their own
applications.  The SIMDIS SDK serves as the basis for development of each
new major SIMDIS release.

The SIMDIS SDK was developed by the Visualization Systems Integration branch
of the Tactical Electronic Warfare Division at the U.S. Naval Research
Laboratory.  For more information visit:

  https://simdis.nrl.navy.mil

The SIMDIS SDK is not SIMDIS.  It is a C++ framework that is the underlying
application framework supporting SIMDIS.  SIMDIS is built on top of the SIMDIS
SDK.  You can use the SIMDIS SDK to build your own SIMDIS-like application,
or to build geospatial software using SIMDIS's tested core library of
conversions and calculations.

The SIMDIS SDK is also not the SIMDIS Plug-in API.  The SIMDIS Plug-in API is
a toolkit for writing plug-ins to SIMDIS that interact directly with SIMDIS
inside the memory space of the application, using dynamic load libraries or
shared objects as appropriate for your system.  The SIMDIS Plug-in API is
available on the [SIMDIS] website.  A free SIMDIS account is required to download.
The SIMDIS SDK is used for building standalone applications using the same
framework as the SIMDIS application.


Installation
============
The SIMDIS SDK is provided as a package containing C++ source code for use
with Windows and Linux operating systems.

The [SIMDIS] website also hosts precompiled binaries of the SIMDIS SDK,
precompiled binary third party dependencies ("Third Party Packs"), and a
small sample data repository.  A free SIMDIS account is required to access these
downloads.  See the file [INSTALL.md](INSTALL.md) for detailed information
about supported systems/compilers, third party dependencies, and installation
instructions.

Though the Third Party Packs can be useful, they are not required.  You can
build your own dependencies and pass them to CMake at configuration time.  The
SIMDIS SDK relies on CMake to configure all build options and third party
dependencies.  The CMake configuration will intelligently enable and disable
different parts of the SDK and examples based on the configured third party
libraries that you have specified.


Usage
=====
The SIMDIS SDK source distribution comes with a number of example programs as
well as a sample data set.  The example which illustrates the most common
usage of the SIMDIS SDK is the Platform Symbology example.  Detailed tutorials
describing SIMDIS SDK use will be added at a later date.

HTML based API documentation for the SIMDIS SDK source code can be found in
the `Doc` subdirectory.  This documentation describes the different software
components provided by the SDK.  You can generate the documentation by running
the directory through Doxygen.

The SIMDIS SDK provides support for loading all 3D model formats supported by
[OpenSceneGraph] (OpenFlight, 3D Studio, Wavefront OBJ, etc), loading the
SIMDIS .db Terrain and Imagery files, and streaming terrain and imagery data
from the ReadyMap server product developed by [Pelican Mapping] using [osgEarth].

The 3D models provided with the SIMDIS SDK sample data package, available on
the SIMDIS website, have all been converted to the native OpenScenegraph .ive
binary file format, which embeds the 3D model's geometry data and associated
texture files within a single, easy to distribute file.


Support
=======
Limited free support is available on either the [SIMDIS Help Desk CAC Required] or
[SIMDIS Help Desk Public].  Create a new topic under the project SDK for assistance.
You can search through previously answered tickets to find solutions to common
problems.  An account on the either website is required to access the Help Desk.
Use of the either help desk is the fastest and most reliable way to get free support
for the SIMDIS SDK.

We also accept pull requests.  If there are modifications you wish to share with
the community, please consider submitting a pull request.  All pull requests should
abide by the [Code Style Guide].

The SIMDIS SDK has scheduled releases every six months, coinciding with the release
schedule for the SIMDIS software.  The SDK receives updates on a regular basis
between releases as we add features, fix bugs, and enhance the SDK to support
applications in the SIMDIS toolkit.


  [Code Style Guide]: <Doc/CodeStyleGuide.pdf>
  [Doxygen]: <http://www.doxygen.nl/>
  [OpenSceneGraph]: <https://github.com/OpenSceneGraph/OpenSceneGraph>
  [osgEarth]: <https://github.com/gwaldron/osgearth>
  [SIMDIS]: <https://simdis.nrl.navy.mil>
  [SIMDIS Help Desk CAC Required]: <https://www.trmc.osd.mil/helpdesk/projects/SIMDIS>
  [SIMDIS Help Desk Public]: <https://www.tena-sda.org/helpdesk/projects/SIMDIS>
  [Pelican Mapping]: <http://www.pelicanmapping.com>

