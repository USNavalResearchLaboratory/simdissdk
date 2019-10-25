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
#include "osgEarth/ElevationLayer"
#include "osgEarth/FeatureModelLayer"
#include "osgEarth/ImageLayer"
#include "osgEarth/OGRFeatureSource"
#include "simCore/Common/Exception.h"
#include "simVis/Constants.h"
#include "simVis/Types.h"
#include "simUtil/LayerFactory.h"

namespace simUtil {

  osgEarth::ImageLayer* LayerFactory::newImageLayer(
    const std::string& layerName,
    const osgEarth::ConfigOptions& options,
    const osgEarth::Profile* mapProfile,
    const osgEarth::CachePolicy* cachePolicy)
  {
    SAFETRYBEGIN;
    osg::ref_ptr<osgEarth::ImageLayer> layer = new osgEarth::ImageLayer(options);

    if (cachePolicy)
      layer->setCachePolicy(*cachePolicy);

    layer->open();

    return layer.release();

    // Error encountered
    SAFETRYEND("during LayerFactory::newImageLayer()");
    return NULL;
  }

osgEarth::ElevationLayer* LayerFactory::newElevationLayer(
  const std::string& layerName,
  const osgEarth::ConfigOptions& options,
  const osgEarth::CachePolicy* cachePolicy,
  const osgEarth::ConfigOptions* extraOptions)
{
  SAFETRYBEGIN;
  osgEarth::ConfigOptions combined(options);
  if (extraOptions)
    combined.merge(*extraOptions);

  osg::ref_ptr<osgEarth::ElevationLayer> layer = new osgEarth::ElevationLayer(combined);

  if (cachePolicy)
    layer->setCachePolicy(*cachePolicy);

  layer->open();

  return layer.release();

  // Error encountered
  SAFETRYEND("during LayerFactory::newElevationLayer()");
  return NULL;
}

osgEarth::FeatureModelLayer* LayerFactory::newFeatureLayer(const osgEarth::FeatureModelLayer::Options& options)
{
  SAFETRYBEGIN;
  osg::ref_ptr<osgEarth::FeatureModelLayer> featureLayer = new osgEarth::FeatureModelLayer(options);

  // Return layer regardless of if open() succeeds
  featureLayer->open();
  return featureLayer.release();

  // Error encountered
  SAFETRYEND("during LayerFactory::newFeatureLayer()");
  return NULL;
}

/////////////////////////////////////////////////////////////////

ShapeFileLayerFactory::ShapeFileLayerFactory()
  : style_(new osgEarth::Style)
{
  // Configure some defaults
  setLineColor(simVis::Color::Cyan);
  setLineWidth(1.5f);

  // Configure the render symbol to render line shapes
  osgEarth::RenderSymbol* rs = style_->getOrCreateSymbol<osgEarth::RenderSymbol>();
  rs->depthTest() = false;
  rs->clipPlane() = simVis::CLIPPLANE_VISIBLE_HORIZON;
  rs->order()->setLiteral(simVis::BIN_GOG_FLAT);
  rs->renderBin() = simVis::BIN_GLOBAL_SIMSDK;
}

ShapeFileLayerFactory::~ShapeFileLayerFactory()
{
}

osgEarth::FeatureModelLayer* ShapeFileLayerFactory::load(const std::string& url) const
{
  osg::ref_ptr<osgEarth::FeatureModelLayer> layer = new osgEarth::FeatureModelLayer();
  configureOptions(url, layer);

  if (layer->getStatus().isError())
  {
    SIM_WARN << "ShapeFileLayerFactory::load(" << url << ") failed : " << layer->getStatus().message() << "\n";
    layer = NULL;
  }
  return layer.release();
}

void ShapeFileLayerFactory::configureOptions(const std::string& url, osgEarth::FeatureModelLayer* layer) const
{
  // Configure the stylesheet that will be associated with the layer
  osgEarth::StyleSheet* stylesheet = new osgEarth::StyleSheet;
  stylesheet->addStyle(*style_);
  layer->setStyleSheet(stylesheet);

  osgEarth::OGRFeatureSource* ogr = new osgEarth::OGRFeatureSource();
  ogr->setURL(url);
  ogr->open(); // not error-checking here; caller can do that at the layer level
  layer->setFeatureSource(ogr);

  layer->setAlphaBlending(true);
  layer->setEnableLighting(false);
}

void ShapeFileLayerFactory::setLineColor(const osg::Vec4f& color)
{
  osgEarth::LineSymbol* ls = style_->getOrCreateSymbol<osgEarth::LineSymbol>();
  ls->stroke()->color() = color;
}

void ShapeFileLayerFactory::setLineWidth(float width)
{
  osgEarth::LineSymbol* ls = style_->getOrCreateSymbol<osgEarth::LineSymbol>();
  ls->stroke()->width() = width;
}

void ShapeFileLayerFactory::setStipple(unsigned short pattern, unsigned int factor)
{
  osgEarth::LineSymbol* ls = style_->getOrCreateSymbol<osgEarth::LineSymbol>();
  ls->stroke()->stipplePattern() = pattern;
  ls->stroke()->stippleFactor() = factor;
}

}
