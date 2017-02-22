/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <iostream>
#include "osg/ArgumentParser"
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simUtil/ExampleResources.h"
#include "ViewerApp.h"

namespace
{

/** Command line argument usage */
int usage(char** argv)
{
  std::cout << "\n" << argv[0] << "\n\n"
    "Command line arguments:\n"
    " --help                    Show this help and exit\n"
    " --simple                  Use Simple ocean model\n"
    " --triton                  Use Triton ocean model\n"
    " --tritonuser <user>       User for Triton license\n"
    " --tritonlicense <lic>     Triton license key\n"
    " --tritonpath <path>       Override default resource path to Triton\n"
    " --bathymetryoffset <val>  Set the bathymetry offset in meters for Ocean\n"
    " --nosky                   Do not use a sky model\n"
    << std::endl;

  return 0;
}

}

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  if (arguments.read("--help"))
    return usage(argv);

  SimpleServer::ViewerApp app(arguments);
  return app.run();
}
