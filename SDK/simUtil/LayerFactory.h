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
#ifndef SIMUTIL_LAYERFACTORY_H
#define SIMUTIL_LAYERFACTORY_H

#include <memory>
#include <string>
#include "osg/Vec4f"
#include "simCore/Common/Common.h"
#include "osgEarth/Version"
#include "osgEarth/FeatureModelLayer"

namespace osgEarth
{
  class CachePolicy;
  class ElevationLayer;
  class ImageLayer;
  class Profile;
  class Style;
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
  /**
   * Factory method for creating a new image layer.
   * @param layerName Name of the layer.  Used to identify the layer in GUI.
   * @param options Configuration options for the Tile Source.  Typically this is either a
   *   directly allocated driver options like GDALOptions or DBOptions, but it could be set
   *   up using an osgEarth::Config passed into an osgEarth::ConfigOptions constructor.
   * @param mapProfile Contents of the osgEarth::Map->getProfile(), required for preventing
   *   crashes when loading MBTiles.  See also SIM-4171.
   * @param cachePolicy When non-NULL, sets the cache policy on the layer.
   * @return Image layer on success; NULL on failure.  Caller responsible for memory.
   *   (put in ref_ptr)
   */
  static osgEarth::ImageLayer* newImageLayer(
    const std::string& layerName,
    const osgEarth::ConfigOptions& options,
    const osgEarth::Profile* mapProfile,
    const osgEarth::CachePolicy* cachePolicy = NULL);

  /**
   * Factory method for creating a new elevation layer.
   * @param layerName Name of the layer.  Used to identify the layer in GUI.
   * @param options Configuration options for the Tile Source.  Typically this is either a
   *   directly allocated driver options like GDALOptions or DBOptions, but it could be set
   *   up using an osgEarth::Config passed into an osgEarth::ConfigOptions constructor.
   * @param cachePolicy When non-NULL, sets the cache policy on the layer.
   * @param extraOptions Additional elevation layer options to merge in, such as noDataValue()
   * @return Elevation layer on success; NULL on failure.  Caller responsible for memory.
   *   (put in ref_ptr)
   */
  static osgEarth::ElevationLayer* newElevationLayer(
    const std::string& layerName,
    const osgEarth::ConfigOptions& options,
    const osgEarth::CachePolicy* cachePolicy = NULL,
    const osgEarth::ConfigOptions* extraOptions = NULL);

  /**
   * Factory method for creating a new feature model layer.
   * @param options Configuration options for the layer.
   * @return Feature model layer on success; NULL on failure.  Caller responsible for memory.
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
