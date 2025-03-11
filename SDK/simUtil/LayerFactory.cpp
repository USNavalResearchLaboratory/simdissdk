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
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cassert>
#include "osgEarth/ArcGISTilePackage"
#include "osgEarth/ElevationLayer"
#include "osgEarth/FeatureModelLayer"
#include "osgEarth/GDAL"
#include "osgEarth/ImageLayer"
#include "osgEarth/MapboxGLImageLayer"
#include "osgEarth/MBTiles"
#include "osgEarth/OGRFeatureSource"
#include "osgEarth/Version"
#include "simCore/Common/Exception.h"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simVis/Constants.h"
#include "simVis/Types.h"
#include "simUtil/LayerFactory.h"

#ifdef SIM_HAVE_DB_SUPPORT
#include "simVis/DBFormat.h"
#endif

#if OSGEARTH_SOVERSION >= 152
#include "osgEarth/SimplifyFilter"
#endif

namespace simUtil {

/** Default cache time of one year */
static const osgEarth::TimeSpan ONE_YEAR(365 * 86400);

simVis::DBImageLayer* LayerFactory::newDbImageLayer(const std::string& fullPath) const
{
#ifndef SIM_HAVE_DB_SUPPORT
  // Likely developer error unintended
  assert(0);
  return nullptr;
#else
  osgEarth::Config config;
  config.setReferrer(fullPath);

  simVis::DBImageLayer::Options opts(config);
  osg::ref_ptr<simVis::DBImageLayer> layer = new simVis::DBImageLayer(opts);
  layer->setURL(fullPath);
  layer->setName(LayerFactory::completeBaseName(fullPath));

  // set max age to 1 year (in secs)
  osgEarth::CachePolicy cachePolicy = osgEarth::CachePolicy::USAGE_READ_WRITE;
  cachePolicy.maxAge() = ONE_YEAR;
  layer->setCachePolicy(cachePolicy);

  return layer.release();
#endif
}

osgEarth::MBTilesImageLayer* LayerFactory::newMbTilesImageLayer(const std::string& fullPath) const
{
  osgEarth::Config config;
  config.setReferrer(fullPath);

  osgEarth::MBTilesImageLayer::Options opts(config);
  osg::ref_ptr<osgEarth::MBTilesImageLayer> layer = new osgEarth::MBTilesImageLayer(opts);
  layer->setName(LayerFactory::completeBaseName(fullPath));
  layer->setURL(fullPath);

  // mbtiles already have preprocessed data, no need to use cache
  layer->setCachePolicy(osgEarth::CachePolicy::USAGE_NO_CACHE);

  return layer.release();
}

osgEarth::GDALImageLayer* LayerFactory::newGdalImageLayer(const std::string& fullPath) const
{
  osgEarth::Config config;
  config.setReferrer(fullPath);

  osgEarth::GDALImageLayer::Options opts(config);
  osg::ref_ptr<osgEarth::GDALImageLayer> layer = new osgEarth::GDALImageLayer(opts);
  layer->setName(LayerFactory::completeBaseName(fullPath));
  layer->setURL(fullPath);

  osgEarth::CachePolicy cachePolicy = osgEarth::CachePolicy::USAGE_READ_WRITE;
  cachePolicy.maxAge() = ONE_YEAR;
  layer->setCachePolicy(cachePolicy);

  // SIM-8582 workaround: MrSID files should have interpolation set to nearest neighbor to prevent crash
  const std::string suffixWithDot = simCore::getExtension(fullPath);
  if (suffixWithDot == ".jp2" || suffixWithDot == ".sid")
    layer->setInterpolation(osgEarth::INTERP_NEAREST);

  return layer.release();
}

osgEarth::MapBoxGLImageLayer* LayerFactory::newMapBoxGlImageLayer(const std::string& fullPath) const
{
  osgEarth::Config config;
  config.setReferrer(fullPath);

  osgEarth::MapBoxGLImageLayer::Options opts(config);
  osg::ref_ptr<osgEarth::MapBoxGLImageLayer> layer = new osgEarth::MapBoxGLImageLayer(opts);
  layer->setName(LayerFactory::completeBaseName(fullPath));
  layer->setURL(fullPath);
  layer->setTileSize(512u);
#if OSGEARTH_SOVERSION >= 128
  // A good rule of thumb is: (tile size / 256) * (FOV / 30). This results in a scale of 4.f
  // for most SIMDIS cases, but this is still a bit small, so boost it up to 6.f.
  layer->setPixelScale(6.f);
#endif

  // Use the same cache policy as GDAL
  osgEarth::CachePolicy cachePolicy = osgEarth::CachePolicy::USAGE_READ_WRITE;
  cachePolicy.maxAge() = ONE_YEAR;
  layer->setCachePolicy(cachePolicy);

  return layer.release();
}

osgEarth::ArcGISTilePackageImageLayer* LayerFactory::newArcGisTilePackageImageLayer(const std::string& confXmlPath) const
{
  osgEarth::Config config;
  config.setReferrer(confXmlPath);

  osgEarth::ArcGISTilePackageImageLayer::Options opts(config);
  osg::ref_ptr<osgEarth::ArcGISTilePackageImageLayer> layer = new osgEarth::ArcGISTilePackageImageLayer(opts);
  layer->setName(LayerFactory::completeBaseName(confXmlPath));
  layer->setURL(confXmlPath);

  // Use the same cache policy as GDAL
  osgEarth::CachePolicy cachePolicy = osgEarth::CachePolicy::USAGE_READ_WRITE;
  cachePolicy.maxAge() = ONE_YEAR;
  layer->setCachePolicy(cachePolicy);

  return layer.release();
}

simVis::DBElevationLayer* LayerFactory::newDbElevationLayer(const std::string& fullPath) const
{
#ifndef SIM_HAVE_DB_SUPPORT
  // Likely developer error unintended
  assert(0);
  return nullptr;
#else
  osgEarth::Config config;
  config.setReferrer(fullPath);

  simVis::DBElevationLayer::Options opts(config);
  osg::ref_ptr<simVis::DBElevationLayer> layer = new simVis::DBElevationLayer(opts);
  layer->setURL(fullPath);
  layer->setName(LayerFactory::completeBaseName(fullPath));

  // set max age to 1 year (in secs)
  osgEarth::CachePolicy cachePolicy = osgEarth::CachePolicy::USAGE_READ_WRITE;
  cachePolicy.maxAge() = ONE_YEAR;
  layer->setCachePolicy(cachePolicy);

  return layer.release();
#endif
}

osgEarth::MBTilesElevationLayer* LayerFactory::newMbTilesElevationLayer(const std::string& fullPath) const
{
  osgEarth::Config config;
  config.setReferrer(fullPath);

  osgEarth::MBTilesElevationLayer::Options opts(config);
  osg::ref_ptr<osgEarth::MBTilesElevationLayer> layer = new osgEarth::MBTilesElevationLayer(opts);
  layer->setName(LayerFactory::completeBaseName(fullPath));
  layer->setURL(fullPath);

  // mbtiles already have preprocessed data, no need to use cache
  layer->setCachePolicy(osgEarth::CachePolicy::USAGE_NO_CACHE);

  return layer.release();
}

osgEarth::GDALElevationLayer* LayerFactory::newGdalElevationLayer(const std::string& fullPath) const
{
  osgEarth::Config config;
  config.setReferrer(fullPath);

  osgEarth::GDALElevationLayer::Options opts(config);
  osg::ref_ptr<osgEarth::GDALElevationLayer> layer = new osgEarth::GDALElevationLayer(opts);
  layer->setName(LayerFactory::completeBaseName(fullPath));
  layer->setURL(fullPath);

  osgEarth::CachePolicy cachePolicy = osgEarth::CachePolicy::USAGE_READ_WRITE;
  cachePolicy.maxAge() = ONE_YEAR;
  layer->setCachePolicy(cachePolicy);

  // SIM-8582 workaround: MrSID files should have interpolation set to nearest neighbor to prevent crash
  const std::string suffixWithDot = simCore::getExtension(fullPath);
  if (suffixWithDot == ".jp2" || suffixWithDot == ".sid")
    layer->setInterpolation(osgEarth::INTERP_NEAREST);

  return layer.release();
}

osgEarth::ArcGISTilePackageElevationLayer* LayerFactory::newArcGisTilePackageElevationLayer(const std::string& confXmlPath) const
{
  osgEarth::Config config;
  config.setReferrer(confXmlPath);

  osgEarth::ArcGISTilePackageElevationLayer::Options opts(config);
  osg::ref_ptr<osgEarth::ArcGISTilePackageElevationLayer> layer = new osgEarth::ArcGISTilePackageElevationLayer(opts);
  layer->setName(LayerFactory::completeBaseName(confXmlPath));
  layer->setURL(confXmlPath);

  // Use the same cache policy as GDAL
  osgEarth::CachePolicy cachePolicy = osgEarth::CachePolicy::USAGE_READ_WRITE;
  cachePolicy.maxAge() = ONE_YEAR;
  layer->setCachePolicy(cachePolicy);

  return layer.release();
}

std::string LayerFactory::completeBaseName(const std::string& fullPath)
{
  // Get the base name -- strip out extension, then strip out everything after last \\ or /
  std::string stripped = simCore::StringUtils::beforeLast(fullPath, '.');
  if (stripped.find('/') != std::string::npos)
    stripped = simCore::StringUtils::afterLast(stripped, '/');
  if (stripped.find('\\') != std::string::npos)
    stripped = simCore::StringUtils::afterLast(stripped, '\\');

  return stripped;
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
  return nullptr;
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
  configureOptions(url, layer.get());

  // check the feature source's status for errors, since the layer's status will be ResourceUnavailable until it is opened
  if (!layer->getFeatureSource())
    return nullptr;
  osgEarth::Status status = layer->getFeatureSource()->getStatus();
  if (status.isError())
  {
    SIM_WARN << "ShapeFileLayerFactory::load(" << url << ") failed : " << status.message() << "\n";
    layer = nullptr;
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

#if OSGEARTH_SOVERSION >= 152
  // Apply simplify tolerance only if it's been set by user
  if (simplifyTolerance_.has_value())
  {
#if OSGEARTH_SOVERSION >= 165
    osgEarth::SimplifyFilter::Options so;
#else
    osgEarth::SimplifyFilterOptions so;
#endif
    so.tolerance() = *simplifyTolerance_;
    ogr->options().filters().push_back(so);
  }
#endif

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
#if OSGEARTH_SOVERSION < 169
  ls->stroke()->width() = width;
#else
  ls->stroke()->width() = osgEarth::Distance(width, osgEarth::Units::PIXELS);
#endif
}

void ShapeFileLayerFactory::setStipple(unsigned short pattern, unsigned int factor)
{
  osgEarth::LineSymbol* ls = style_->getOrCreateSymbol<osgEarth::LineSymbol>();
  ls->stroke()->stipplePattern() = pattern;
  ls->stroke()->stippleFactor() = factor;
}

void ShapeFileLayerFactory::setSimplifyTolerance(double tolerance)
{
  simplifyTolerance_ = tolerance;
}

}
