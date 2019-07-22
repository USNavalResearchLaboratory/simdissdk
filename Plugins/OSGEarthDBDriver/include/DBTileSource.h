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

#ifndef SIMDIS_PLUGIN_OSGEARTH_DB_DRIVER_H
#define SIMDIS_PLUGIN_OSGEARTH_DB_DRIVER_H 1

#include "osgEarth/TileSource"
#include "simCore/Time/Utils.h"
#include "simVis/DBOptions.h"
#include "simVis/osgEarthVersion.h"
#include "sqlite3.h"
#include "SQLiteDataBaseReadUtil.h"
#include "QSPosXYExtents.h"

namespace simVis_db
{
  class DBTileSource : public osgEarth::TileSource
  {
  public:
    /** Constructs a new driver for reading .DB raster files */
    DBTileSource(const osgEarth::TileSourceOptions& options);

    /// TileSource interface
    virtual osgEarth::Status initialize(const osgDB::Options* dbOptions);
    virtual osg::Image* createImage(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress);
    virtual osg::HeightField* createHeightField(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress);
    virtual std::string getExtension() const;
    virtual int getPixelsPerTile() const;

  protected:
      virtual ~DBTileSource();

  private:
      bool decodeRaster_(
          int          rasterFormat,
          const char*  inputBuffer,
          int          inputBufferLen,
          osg::ref_ptr<osg::Image>& out_image);

      osg::Image* createImage_(const osgEarth::TileKey& key, bool isHeightField);

      const simVis::DBOptions options_;
      std::string pathname_;
      sqlite3* db_;
      SQLiteDataBaseReadUtil dbUtil_;

      // layer metadata
      /** Defined in RasterCommon.h */
      int rasterFormat_;
      /** AKA tile size */
      int pixelLength_;
      int shallowLevel_;
      int deepLevel_;
      PosXPosYExtents extents_[6];
      std::string source_;
      std::string classification_;
      std::string description_;
      bool timeSpecified_;
      simCore::TimeStamp timeStamp_;

      osg::ref_ptr<osgDB::ReaderWriter> pngReader_;
      osg::ref_ptr<osgDB::ReaderWriter> jpgReader_;
      osg::ref_ptr<osgDB::ReaderWriter> tifReader_;
      osg::ref_ptr<osgDB::ReaderWriter> rgbReader_;
  };

} // namespace simVis_db


#endif // SIMDIS_PLUGIN_OSGEARTH_DB_DRIVER_H

