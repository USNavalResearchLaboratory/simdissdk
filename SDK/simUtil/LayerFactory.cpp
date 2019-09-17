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
#include "osgEarthDrivers/feature_ogr/OGRFeatureOptions"
#include "osgEarthFeatures/FeatureModelLayer"
#include "simCore/Common/Exception.h"
#include "simVis/Constants.h"
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

/////////////////////////////////////////////////////////////////

ShapeFileLayerFactory::ShapeFileLayerFactory()
  : style_(new osgEarth::Symbology::Style)
{
  // Configure some defaults
  setLineColor(osgEarth::Symbology::Color::Cyan);
  setLineWidth(1.5f);

  // Configure the render symbol to render line shapes
  osgEarth::Symbology::RenderSymbol* rs = style_->getOrCreateSymbol<osgEarth::Symbology::RenderSymbol>();
  rs->depthTest() = false;
  rs->clipPlane() = simVis::CLIPPLANE_VISIBLE_HORIZON;
  rs->order()->setLiteral(simVis::BIN_GOG_FLAT);
  rs->renderBin() = simVis::BIN_GLOBAL_SIMSDK;
}

ShapeFileLayerFactory::~ShapeFileLayerFactory()
{
}

osgEarth::Features::FeatureModelLayer* ShapeFileLayerFactory::load(const std::string& url) const
{
  osgEarth::Features::FeatureModelLayerOptions layerOptions;
  configureOptions(url, layerOptions);
  return LayerFactory::newFeatureLayer(layerOptions);
}

void ShapeFileLayerFactory::configureOptions(const std::string& url, osgEarth::Features::FeatureModelLayerOptions& driver) const
{
  osgEarth::Drivers::OGRFeatureOptions ogr;
  ogr.url() = url;

  // Configure the stylesheet that will be associated with the layer
  osgEarth::Symbology::StyleSheet* stylesheet = new osgEarth::Symbology::StyleSheet;
  stylesheet->addStyle(*style_);

  driver.featureSource() = ogr;
  driver.styles() = stylesheet;
  driver.alphaBlending() = true;
  driver.enableLighting() = false;
}

void ShapeFileLayerFactory::setLineColor(const osg::Vec4f& color)
{
  osgEarth::Symbology::LineSymbol* ls = style_->getOrCreateSymbol<osgEarth::Symbology::LineSymbol>();
  ls->stroke()->color() = color;
}

void ShapeFileLayerFactory::setLineWidth(float width)
{
  osgEarth::Symbology::LineSymbol* ls = style_->getOrCreateSymbol<osgEarth::Symbology::LineSymbol>();
  ls->stroke()->width() = width;
}

void ShapeFileLayerFactory::setStipple(unsigned short pattern, unsigned int factor)
{
  osgEarth::Symbology::LineSymbol* ls = style_->getOrCreateSymbol<osgEarth::Symbology::LineSymbol>();
  ls->stroke()->stipplePattern() = pattern;
  ls->stroke()->stippleFactor() = factor;
}

}
