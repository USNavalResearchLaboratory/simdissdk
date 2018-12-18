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
#include "osgEarth/ImageLayer"
#include "osgEarth/ElevationLayer"
#include "osgEarthFeatures/FeatureModelLayer"
#include "simCore/Common/Exception.h"
#include "simUtil/LayerFactory.h"

namespace simUtil {

osgEarth::ImageLayer* LayerFactory::newImageLayer(
  const std::string& layerName,
  const osgEarth::TileSourceOptions& options,
  const osgEarth::Profile* mapProfile,
  const osgEarth::CachePolicy* cachePolicy)
{
  SAFETRYBEGIN;
  osgEarth::ImageLayerOptions ilOptions(layerName, options);
  osgEarth::Config config = ilOptions.getConfig();
  config.key() = "image";
  ilOptions.mergeConfig(config);

  if (cachePolicy)
    ilOptions.cachePolicy() = *cachePolicy;

  // Allocate the image layer with the provided options
  osg::ref_ptr<osgEarth::ImageLayer> imageLayer = new osgEarth::ImageLayer(ilOptions);

  // need to set the target profile hint to prevent crash for MBTiles, SIM-4171
  imageLayer->setTargetProfileHint(mapProfile);
  if (imageLayer->open().isError())
    return imageLayer.release();

  if (imageLayer->getTileSource() && imageLayer->getTileSource()->isOK())
  {
    // Only return valid layers that have good tile sources
    return imageLayer.release(); // decrement count, but do not delete
  }

  // Error encountered
  SAFETRYEND("during LayerFactory::newImageLayer()");
  return NULL;
}

osgEarth::ElevationLayer* LayerFactory::newElevationLayer(
  const std::string& layerName,
  const osgEarth::TileSourceOptions& options,
  const osgEarth::CachePolicy* cachePolicy,
  const osgEarth::ElevationLayerOptions* extraOptions)
{
  SAFETRYBEGIN;

  // Now instantiate an ElevationLayerOptions out of the TileSourceOptions
  osgEarth::ElevationLayerOptions elOptions(layerName, options);
  osgEarth::Config config = elOptions.getConfig();
  config.key() = "elevation";
  elOptions.mergeConfig(config);

  // Set the cache policy if there is one
  if (cachePolicy)
    elOptions.cachePolicy() = *cachePolicy;

  // Merge in the extra options if specified
  if (extraOptions)
    elOptions.merge(*extraOptions);

  // Allocate the elevation layer now with the provided options
  osg::ref_ptr<osgEarth::ElevationLayer> elevationLayer = new osgEarth::ElevationLayer(elOptions);
  // Newer osgEarth requires an open() before retrieving tile source
  if (!elevationLayer->open())
    return elevationLayer.release();

  if (elevationLayer->getTileSource() && elevationLayer->getTileSource()->isOK())
  {
    // Only return valid layers that have good tile sources
    return elevationLayer.release(); // decrement count, but do not delete
  }

  // Error encountered
  SAFETRYEND("during LayerFactory::newElevationLayer()");
  return NULL;
}

osgEarth::Features::FeatureModelLayer* LayerFactory::newFeatureLayer(const osgEarth::Features::FeatureModelLayerOptions& options)
{
  SAFETRYBEGIN;
  osg::ref_ptr<osgEarth::Features::FeatureModelLayer> featureLayer = new osgEarth::Features::FeatureModelLayer(options);

  // Return layer regardless of if open() succeeds
  featureLayer->open();
  return featureLayer.release();

  // Error encountered
  SAFETRYEND("during LayerFactory::newFeatureLayer()");
  return NULL;
}

}
