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
#include "osgEarthTriton/TritonLayer"
#include "osgEarthTriton/TritonOptions"
#endif

#if SDK_OSGEARTH_VERSION_LESS_THAN(1,10,0)
#include "osgEarthDrivers/ocean_simple/SimpleOceanOptions"
#else
#include "osgEarthUtil/SimpleOceanLayer"
#endif

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
#ifdef HAVE_TRITON_NODEKIT
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
    osg::ref_ptr<osgEarth::Triton::TritonLayer> layer = new osgEarth::Triton::TritonLayer(triton);
    scene.getMap()->addLayer(layer.get());
  }
  else // type_ == SIMPLE
#endif
  {
#if SDK_OSGEARTH_VERSION_LESS_THAN(1,10,0)
    osgEarth::Drivers::SimpleOcean::SimpleOceanOptions ocean;
    ocean.lowFeatherOffset() = 0.0f;
    ocean.highFeatherOffset() = 1.0f;
    ocean.renderBinNumber() = simVis::BIN_OCEAN;
#else
    osgEarth::Util::SimpleOceanLayerOptions ocean;
    // To get similar behavior as old ocean_simple driver, useBathymetry() == false helps
    ocean.useBathymetry() = false;
#endif
    ocean.maxAltitude() = 30000.0f;
    osg::ref_ptr<osgEarth::Util::OceanNode> oceanNode = osgEarth::Util::OceanNode::create(ocean, scene.getMapNode());
    if (oceanNode.valid())
      scene.setOceanNode(oceanNode.get());
  }
}

}
