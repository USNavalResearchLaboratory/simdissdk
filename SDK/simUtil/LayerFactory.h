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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMUTIL_LAYERFACTORY_H
#define SIMUTIL_LAYERFACTORY_H

#include <memory>
#include <string>
#include "osg/Vec4f"
#include "simCore/Common/Common.h"
#include "osgEarth/FeatureModelLayer"

namespace osgEarth
{
  class CachePolicy;
  class ElevationLayer;
  class GDALElevationLayer;
  class GDALImageLayer;
  class MBTilesElevationLayer;
  class MBTilesImageLayer;
  class Profile;
  class Style;
}
namespace simVis
{
  class DBElevationLayer;
  class DBImageLayer;
}

namespace simUtil {

/**
 * Responsible for creating layers.  Generally layers are easy to create, but there are
 * some features and options that may not be frequently set that can cause problems if
 * they are not set.  The factory takes care of this for you.
 *
 * For example, cache policy needs to be set before layer creation.  And in some cases
 * the map profile needs to be hinted to the layer driver on creation to avoid crashes.
 */
class SDKUTIL_EXPORT LayerFactory
{
public:
  /** Returns an image layer properly configured for DB layer. May return nullptr if not configured with DB support. */
  simVis::DBImageLayer* newDbImageLayer(const std::string& fullPath) const;
  /** Returns an image layer properly configured for MBTiles layer. */
  osgEarth::MBTilesImageLayer* newMbTilesImageLayer(const std::string& fullPath) const;
  /** Returns an image layer properly configured for GDAL layer. */
  osgEarth::GDALImageLayer* newGdalImageLayer(const std::string& fullPath) const;

  /** Returns an elevation layer properly configured for DB layer. May return nullptr if not configured with DB support. */
  simVis::DBElevationLayer* newDbElevationLayer(const std::string& fullPath) const;
  /** Returns an elevation layer properly configured for MBTiles layer. */
  osgEarth::MBTilesElevationLayer* newMbTilesElevationLayer(const std::string& fullPath) const;
  /** Returns an elevation layer properly configured for GDAL layer. */
  osgEarth::GDALElevationLayer* newGdalElevationLayer(const std::string& fullPath) const;

  /** Retrieves the complete base name (e.g. "filename" for "c:/tmp/filename.db") of a URL */
  static std::string completeBaseName(const std::string& fullPath);

  /**
   * Factory method for creating a new feature model layer.
   * @param options Configuration options for the layer.
   * @return Feature model layer on success; nullptr on failure.  Caller responsible for memory.
   *   (put in ref_ptr)
   */
  static osgEarth::FeatureModelLayer* newFeatureLayer(const osgEarth::FeatureModelLayer::Options& options);
};

/** Simplified factory interface to load line-based shape files. */
class SDKUTIL_EXPORT ShapeFileLayerFactory
{
public:
  ShapeFileLayerFactory();
  virtual ~ShapeFileLayerFactory();

  /** Creates a new layer given the URL provided. */
  osgEarth::FeatureModelLayer* load(const std::string& url) const;

  /** Helper method that fills out the model layer options based on URL and current configuration. */
  void configureOptions(const std::string& url, osgEarth::FeatureModelLayer* layer) const;

  /** Changes the line color for the next loaded layer. */
  void setLineColor(const osg::Vec4f& color);
  /** Changes the line width for the next loaded layer. */
  void setLineWidth(float width);
  /** Changes the stipple pattern and factor for the next loaded layer. */
  void setStipple(unsigned short pattern, unsigned int factor);

private:
  std::unique_ptr<osgEarth::Style> style_;
};

}

#endif /* SIMUTIL_LAYERFACTORY_H */
