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
#include "osgDB/FileUtils"
#include "simVis/DBFormat.h"

using namespace simVis;

osgEarth::Config DBImageLayer::Options::getConfig() const
{
  osgEarth::Config conf = osgEarth::Contrib::TileSourceImageLayer::Options::getConfig();
  conf.merge(driver()->getConfig());
  return conf;
}

void DBImageLayer::Options::fromConfig(const osgEarth::Config& conf)
{
  driver() = simVis::DBOptions(conf);
}

void DBImageLayer::setURL(const osgEarth::URI& value)
{
  localCopyOfOptions_.url() = value;
  options().driver() = localCopyOfOptions_;
}

const osgEarth::URI& DBImageLayer::getURL() const
{
  return localCopyOfOptions_.url().get();
}

void DBImageLayer::setDeepestLevel(const unsigned int& value)
{
  localCopyOfOptions_.deepestLevel() = value;
  options().driver() = localCopyOfOptions_;
}

const unsigned int& DBImageLayer::getDeepestLevel() const
{
  return localCopyOfOptions_.deepestLevel().get();
}

void DBImageLayer::init()
{
  osgEarth::Contrib::TileSourceImageLayer::init();
  localCopyOfOptions_ = simVis::DBOptions(options().driver().get());
}

osgEarth::Status DBImageLayer::openImplementation()
{
  return osgEarth::Contrib::TileSourceImageLayer::openImplementation();
}

osgEarth::GeoImage DBImageLayer::createImageImplementation(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress) const
{
  //todo
  return osgEarth::Contrib::TileSourceImageLayer::createImageImplementation(key, progress);
}

//...........................................................

osgEarth::Config DBElevationLayer::Options::getConfig() const
{
  osgEarth::Config conf = osgEarth::ElevationLayer::Options::getConfig();
  conf.merge(driver()->getConfig());
  return conf;
}

void DBElevationLayer::Options::fromConfig(const osgEarth::Config& conf)
{
  driver() = simVis::DBOptions(conf);
}

void DBElevationLayer::setURL(const osgEarth::URI& value)
{
  localCopyOfOptions_.url() = value;
  options().driver() = localCopyOfOptions_;
}

const osgEarth::URI& DBElevationLayer::getURL() const
{
  return localCopyOfOptions_.url().get();
}

void DBElevationLayer::setDeepestLevel(const unsigned int& value)
{
  localCopyOfOptions_.deepestLevel() = value;
  options().driver() = localCopyOfOptions_;
}

const unsigned int& DBElevationLayer::getDeepestLevel() const
{
  return localCopyOfOptions_.deepestLevel().get();
}

void DBElevationLayer::init()
{
  osgEarth::Contrib::ElevationLayer::init();
  localCopyOfOptions_ = simVis::DBOptions(options().driver().get());
}

osgEarth::Status DBElevationLayer::openImplementation()
{
  return osgEarth::ElevationLayer::openImplementation();
}

osgEarth::GeoHeightField DBElevationLayer::createHeightFieldImplementation(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress) const
{
  //todo
  return osgEarth::ElevationLayer::createHeightFieldImplementation(key, progress);
}
