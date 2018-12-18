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

#include <string>
#include "simCore/Common/Common.h"

namespace osgEarth
{
  class TileSourceOptions;
  class ImageLayer;
  class ElevationLayer;
  class CachePolicy;
  class Profile;

  namespace Features
  {
    class FeatureModelLayer;
    class FeatureModelLayerOptions;
  }
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
   *   up using an osgEarth::Config passed into an osgEarth::TileSourceOptions constructor.
   * @param mapProfile Contents of the osgEarth::Map->getProfile(), required for preventing
   *   crashes when loading MBTiles.  See also SIM-4171.
   * @param cachePolicy When non-NULL, sets the cache policy on the layer.
   * @return Image layer on success; NULL on failure.  Caller responsible for memory.
   *   (put in ref_ptr)
   */
  static osgEarth::ImageLayer* newImageLayer(
    const std::string& layerName,
    const osgEarth::TileSourceOptions& options,
    const osgEarth::Profile* mapProfile,
    const osgEarth::CachePolicy* cachePolicy=NULL);

  /**
   * Factory method for creating a new elevation layer.
   * @param layerName Name of the layer.  Used to identify the layer in GUI.
   * @param options Configuration options for the Tile Source.  Typically this is either a
   *   directly allocated driver options like GDALOptions or DBOptions, but it could be set
   *   up using an osgEarth::Config passed into an osgEarth::TileSourceOptions constructor.
   * @param cachePolicy When non-NULL, sets the cache policy on the layer.
   * @param extraOptions Additional elevation layer options to merge in, such as noDataValue()
   * @return Elevation layer on success; NULL on failure.  Caller responsible for memory.
   *   (put in ref_ptr)
   */
  static osgEarth::ElevationLayer* newElevationLayer(
    const std::string& layerName,
    const osgEarth::TileSourceOptions& options,
    const osgEarth::CachePolicy* cachePolicy=NULL,
    const osgEarth::ElevationLayerOptions* extraOptions=NULL);

  /**
   * Factory method for creating a new feature model layer.
   * @param options Configuration options for the layer.
   * @return Feature model layer on success; NULL on failure.  Caller responsible for memory.
   *   (put in ref_ptr)
   */
  static osgEarth::Features::FeatureModelLayer* newFeatureLayer(const osgEarth::Features::FeatureModelLayerOptions& options);
};

}

#endif /* SIMUTIL_LAYERFACTORY_H */
