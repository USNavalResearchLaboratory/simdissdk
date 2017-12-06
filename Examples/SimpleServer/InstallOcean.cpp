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
#include "osg/ArgumentParser"
#include "osgEarth/MapNode"
#include "osgEarth/TerrainEngineNode"
#include "osgEarthUtil/Ocean"

// Potentially defines HAVE_TRITON_NODEKIT, include first
#include "simVis/osgEarthVersion.h"

#ifdef HAVE_TRITON_NODEKIT
#include "osgEarthTriton/TritonOptions"
#else
#include "osgEarthDrivers/ocean_triton/TritonOptions"
#endif
#include "osgEarthDrivers/ocean_simple/SimpleOceanOptions"
#include "simVis/BathymetryGenerator.h"
#include "simVis/Constants.h"
#include "simVis/SceneManager.h"
#include "simUtil/ExampleResources.h"
#include "InstallOcean.h"

namespace osgEarth {
  // 7/5/2016: osgEarth::Drivers::SimpleOcean became osgEarth::SimpleOcean; this
  // "using" statement allows for compilation both before and after this change.
  using namespace Drivers;
}

namespace SimpleServer {

InstallOcean::InstallOcean()
  : type_(NONE),
    bathymetryOffset_(0.0),
    resourcePath_(simExamples::getTritonResourcesPath())
{
}

void InstallOcean::set(osg::ArgumentParser& args)
{
  args.read("--bathymetryoffset", bathymetryOffset_);
  bathymetryOffset_ = fabs(bathymetryOffset_);
  args.read("--tritonuser", user_);
  args.read("--tritonlicense", license_);
  args.read("--tritonpath", resourcePath_);
  if (args.read("--triton"))
    type_ = TRITON;
  else if (args.read("--simple"))
    type_ = SIMPLE;
  else
    type_ = NONE;
}

void InstallOcean::setNone()
{
  type_ = NONE;
  bathymetryOffset_ = 0.0;
}

void InstallOcean::setSimple(double bathymetryOffset)
{
  type_ = SIMPLE;
  bathymetryOffset_ = fabs(bathymetryOffset);
}

void InstallOcean::setTriton(double bathymetryOffset, const std::string& user, const std::string& license, const std::string& resourcePath)
{
  type_ = TRITON;
  bathymetryOffset_ = fabs(bathymetryOffset);
  user_ = user;
  license_ = license;
  resourcePath_ = resourcePath;
  if (resourcePath_.empty())
    resourcePath_ = simExamples::getTritonResourcesPath();
}

void InstallOcean::install(simVis::SceneManager& scene)
{
  if (type_ == NONE)
    return;

  // Install the bathymetry offset
  if (bathymetryOffset_ != 0.0)
  {
    simVis::BathymetryGenerator* bathGen = new simVis::BathymetryGenerator();
    bathGen->setOffset(-bathymetryOffset_);
    scene.getMapNode()->getTerrainEngine()->addEffect(bathGen);
  }

  // Install the driver for the ocean
  osg::ref_ptr<osgEarth::Util::OceanNode> oceanNode;
  if (type_ == TRITON)
  {
    osgEarth::Triton::TritonOptions triton;
    if (!user_.empty())
      triton.user() = user_;
    if (!license_.empty())
      triton.licenseCode() = license_;
    if (!resourcePath_.empty())
      triton.resourcePath() = resourcePath_;

    triton.useHeightMap() = false;
    triton.maxAltitude() = 30000.0f;
    triton.renderBinNumber() = simVis::BIN_OCEAN;
    oceanNode = osgEarth::Util::OceanNode::create(triton, scene.getMapNode());
  }
  else // type_ == SIMPLE
  {
    osgEarth::SimpleOcean::SimpleOceanOptions ocean;
    ocean.maxAltitude() = 30000.0f;
    ocean.renderBinNumber() = simVis::BIN_OCEAN;
    oceanNode = osgEarth::Util::OceanNode::create(ocean, scene.getMapNode());
  }

  if (oceanNode.valid())
    scene.setOceanNode(oceanNode.get());
}

}
