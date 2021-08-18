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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgDB/FileUtils"
#include "osgEarth/Cube"
#include "osgEarth/ImageToHeightFieldConverter"
#include "simCore/Calc/Math.h"
#include "simCore/Time/TimeClass.h"
#include "simVis/DBOptions.h"
#include "simVis/DBFormat.h"
#include "simVis/DB/QSCommon.h"
#include "simVis/DB/swapbytes.h"
#include "simVis/DB/SQLiteDataBaseReadUtil.h"

using namespace simVis;
using namespace simVis_db;

//...........................................................

namespace
{
  bool convertTileKeyToQsKey(const osgEarth::TileKey& key, FaceIndexType& out_faceIndex, QSNodeId& out_nodeId,
    osg::Vec2d& out_fmin, osg::Vec2d& out_fmax)
  {
    QSNodeId zero(0);
    QSNodeId one(1);

    const unsigned int maxLevel = key.getLevelOfDetail();

    QSNodeId nodeId;

    osgEarth::TileKey pkey = key;

    for (unsigned int i = 0; i < maxLevel; ++i)
    {
      const unsigned int plevel = pkey.getLevelOfDetail();
      const unsigned int level = plevel * 3;
      const QSNodeId bit0 = one << level;
      const QSNodeId bit1 = one << (level + 1);
      const QSNodeId bit2 = one << (level + 2);

      unsigned int tx, ty;
      pkey.getTileXY(tx, ty);

      const int xoff = ((tx % 2) == 0) ? 0 : 1;
      const int yoff = ((ty % 2) == 0) ? 0 : 1;

      if (xoff == 0 && yoff == 0)
      {
        nodeId |= bit1;
      }
      else if (xoff == 1 && yoff == 0)
      {
        nodeId |= bit0;
      }
      else if (xoff == 0 && yoff == 1)
      {
        nodeId |= bit0;
        nodeId |= bit1;
      }
      else if (xoff == 1 && yoff == 1)
      {
        nodeId |= bit2;
      }

      pkey = pkey.createParentKey();
    }

    out_faceIndex = osgEarth::Contrib::UnifiedCubeProfile::getFace(key);
    out_nodeId = nodeId;

    double xMin = key.getExtent().xMin();
    double yMin = key.getExtent().yMin();
    double xMax = key.getExtent().xMax();
    double yMax = key.getExtent().yMax();
    int face;

    osgEarth::Contrib::CubeUtils::cubeToFace(xMin, yMin, xMax, yMax, face);

    out_fmin.set(xMin * QS_MAX_LENGTH_DOUBLE, yMin * QS_MAX_LENGTH_DOUBLE);
    out_fmax.set(xMax * QS_MAX_LENGTH_DOUBLE, yMax * QS_MAX_LENGTH_DOUBLE);

    return true;
  }

  bool decompressZLIB(const char* input, int inputLen, std::string& output)
  {
    osgDB::BaseCompressor* comp = osgDB::Registry::instance()->getObjectWrapperManager()->findCompressor("zlib");
    std::string inString(input, inputLen);
    std::istringstream inStream(inString);
    return comp->decompress(inStream, output);
  }

  // Uses one of OSG's native ReaderWriter's to read image data from a buffer.
  bool readNativeImage(osgDB::ReaderWriter* reader, const char* inBuf, int inBufLen, osg::ref_ptr<osg::Image>& outImage)
  {
    std::string inString(inBuf, inBufLen);
    std::istringstream inStream(inString);
    osgDB::ReaderWriter::ReadResult result = reader->readImage(inStream);
    outImage = result.getImage();
    if (result.error() || !outImage.valid())
    {
      return false;
    }
    else
      return true;
  }

  struct DBContext
  {
    DBContext()
    {
      rasterFormat_ = SPLIT_UNKNOWN;
      pixelLength_ = 128;
      shallowLevel_ = 0;
      deepLevel_ = 32;
      timeSpecified_ = false;
      timeStamp_ = simCore::INFINITE_TIME_STAMP;
      db_ = nullptr;
    }

    int rasterFormat_;
    int pixelLength_;
    int shallowLevel_;
    int deepLevel_;
    bool timeSpecified_;
    simCore::TimeStamp timeStamp_;

    std::string pathname_;
    sqlite3* db_;
    SQLiteDataBaseReadUtil dbUtil_;
    PosXPosYExtents extents_[6];
    std::string source_;
    std::string classification_;
    std::string description_;

    osg::ref_ptr<osgDB::ReaderWriter> pngReader_;
    osg::ref_ptr<osgDB::ReaderWriter> jpgReader_;
    osg::ref_ptr<osgDB::ReaderWriter> tifReader_;
    osg::ref_ptr<osgDB::ReaderWriter> rgbReader_;

    template<typename T>
    void makeImage(int size, GLenum internalFormat, GLenum pixelFormat, GLenum type,
      std::string& buf, osg::ref_ptr<osg::Image>& outImage)
    {
      unsigned char* data = new unsigned char[buf.length()];
      std::copy(buf.begin(), buf.end(), data);

      // Be sure to cast here to get the right swap function:
      makeBigEndian((T*)data, size * size);

      outImage = new osg::Image();
      outImage->setImage(size, size, 1, internalFormat, pixelFormat, type, data, osg::Image::USE_NEW_DELETE);
    }

    bool decodeRaster_(int rasterFormat, const char* inputBuffer, int inputBufferLen, osg::ref_ptr<osg::Image>& outImage)
    {
      switch (rasterFormat)
      {
      case SPLIT_5551_ZLIB_COMPRESS: // TESTED OK
      case SPLIT_5551_GZ:            // UNTESTED
      {
        std::string buf;
        if (decompressZLIB(inputBuffer, inputBufferLen, buf))
        {
          // Three component image (red, green, and blue channels)
          makeImage<uint16_t>(
            pixelLength_, GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, buf, outImage);
          return true;
        }
      }
      break;

      case SPLIT_8BIT_ZLIB_COMPRESS: // TESTED OK
      case SPLIT_8BIT_GZ:            // UNTESTED
      {
        std::string buf;
        if (decompressZLIB(inputBuffer, inputBufferLen, buf))
        {
          // Single component image (grayscale channel)
          makeImage<unsigned char>(
            pixelLength_, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf, outImage);
          return true;
        }
      }
      break;
      case SPLIT_INTA_ZLIB_COMPRESS: // TESTED OK
      {
        std::string buf;
        if (decompressZLIB(inputBuffer, inputBufferLen, buf))
        {
          // Two component image (grayscale w/alpha channel)
          makeImage<unsigned char>(
            pixelLength_, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, buf, outImage);
          return true;
        }
        break;
      }
      case SPLIT_RGBA_ZLIB_COMPRESS: // TESTED OK
      {
        std::string buf;
        if (decompressZLIB(inputBuffer, inputBufferLen, buf))
        {
          // Four component image (red, green, blue and alpha channels)
          makeImage<unsigned char>(
            pixelLength_, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, buf, outImage);
          return true;
        }
        break;
      }
      case SPLIT_SGI_RGBA: // TESTED - OK (earthColorSGI.db)
      {
        if (rgbReader_.valid())
          return readNativeImage(rgbReader_.get(), inputBuffer, inputBufferLen, outImage);
        else
          OE_WARN << "SGI RGBA reader not available" << std::endl;
      }
      break;

      case SPLIT_SGI_RGB: // UNTESTED
      {
        if (rgbReader_.valid())
          return readNativeImage(rgbReader_.get(), inputBuffer, inputBufferLen, outImage);
        else
          OE_WARN << "SGI RGB reader not available" << std::endl;
      }
      break;

      case SPLIT_FLOAT32_ZLIB_COMPRESS: // TESTED OK;
      {
        // Single-channel 32-bit float elevation data
        std::string buf;
        if (decompressZLIB(inputBuffer, inputBufferLen, buf))
        {
          makeImage<float>(pixelLength_, GL_LUMINANCE32F_ARB, GL_LUMINANCE, GL_FLOAT, buf, outImage);
          return true;
        }
      }
      break;

      case SPLIT_JPEG: // TESTED OK
      {
        if (jpgReader_.valid())
          return readNativeImage(jpgReader_.get(), inputBuffer, inputBufferLen, outImage);
        else
          OE_WARN << "JPEG reader not available" << std::endl;
      }
      break;

      case SPLIT_PNG: // UNTESTED
      {
        if (pngReader_.valid())
          return readNativeImage(pngReader_.get(), inputBuffer, inputBufferLen, outImage);
        else
          OE_WARN << "PNG reader not available" << std::endl;
      }
      break;

      case SPLIT_TIFF: // UNTESTED
      {
        if (tifReader_.valid())
          return readNativeImage(tifReader_.get(), inputBuffer, inputBufferLen, outImage);
        else
          OE_WARN << "TIFF reader not available" << std::endl;
      }
      break;

      default:
      {
        OE_WARN << "Support for raster format " << rasterFormat << " not implemented" << std::endl;
      }
      break;
      }
      return false;
    }
  };
}

//...........................................................

#undef LC
#define LC "[DBImageLayer] "

REGISTER_OSGEARTH_LAYER(dbimage, simVis::DBImageLayer);

osgEarth::Config DBImageLayer::Options::getConfig() const
{
  osgEarth::Config conf = osgEarth::ImageLayer::Options::getConfig();
  conf.set("url", url());
  conf.set("deepest_level", deepestLevel());
  return conf;
}

void DBImageLayer::Options::fromConfig(const osgEarth::Config& conf)
{
  conf.get("url", url());
  conf.get("deepest_level", deepestLevel());
}

void DBImageLayer::setURL(const osgEarth::URI& value)
{
  options().url() = value;
}

const osgEarth::URI& DBImageLayer::getURL() const
{
  return options().url().get();
}

void DBImageLayer::setDeepestLevel(unsigned int value)
{
  options().deepestLevel() = value;
}

unsigned int DBImageLayer::getDeepestLevel() const
{
  return options().deepestLevel().get();
}

void DBImageLayer::init()
{
  osgEarth::ImageLayer::init();
  context_ = new DBContext();
}

DBImageLayer::~DBImageLayer()
{
  delete static_cast<DBContext*>(context_);
}

osgEarth::Status DBImageLayer::openImplementation()
{
  osgEarth::Status parent = osgEarth::ImageLayer::openImplementation();
  if (parent.isError())
    return parent;

  DBContext& cx = *static_cast<DBContext*>(context_);

  if (!options().url().isSet())
    return osgEarth::Status(osgEarth::Status::ConfigurationError, "Missing required URL");

  cx.pathname_ = osgDB::findDataFile(options().url()->full(), getReadOptions());

  if (cx.dbUtil_.openDatabaseFile(cx.pathname_, &cx.db_, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX) != simVis_db::QS_IS_OK)
  {
    cx.db_ = nullptr;
    return osgEarth::Status(
      osgEarth::Status::ResourceUnavailable,
      osgEarth::Stringify() << "Failed to open DB file at " << options().url()->full());
  }
  else
  {
    QsErrorType err = cx.dbUtil_.getSetFromListOfSetsTable(
      cx.db_,
      "default",
      cx.rasterFormat_,
      cx.pixelLength_,
      cx.shallowLevel_,
      cx.deepLevel_,
      cx.extents_,
      cx.source_,
      cx.classification_,
      cx.description_,
      cx.timeSpecified_,
      cx.timeStamp_);

    // Limit the deepLevel_ by the passed-in option
    if (options().deepestLevel().isSet())
    {
      cx.deepLevel_ = simCore::sdkMin(cx.deepLevel_, static_cast<int>(options().deepestLevel().get()));
    }

    if (err != simVis_db::QS_IS_OK)
    {
      sqlite3_close(cx.db_);
      cx.db_ = nullptr;
      return osgEarth::Status(
        osgEarth::Status::ResourceUnavailable,
        osgEarth::Stringify() << "Failed to read metadata for " << cx.pathname_);
    }

    // Set up as a unified cube:
    osgEarth::Profile* profile = new osgEarth::Contrib::UnifiedCubeProfile();
    // DB are expected to be wgs84, which Cube defaults to
    setProfile(profile);

    // Lat/long extents (for debugging)
    osgEarth::GeoExtent llex[6];

    // Tell the engine how deep the data actually goes:
    for (unsigned int f = 0; f < 6; ++f)
    {
      if (cx.extents_[f].minX < cx.extents_[f].maxX && cx.extents_[f].minY < cx.extents_[f].maxY)
      {
        const double x0 = cx.extents_[f].minX / QS_MAX_LENGTH_DOUBLE;
        const double x1 = cx.extents_[f].maxX / QS_MAX_LENGTH_DOUBLE;
        const double y0 = cx.extents_[f].minY / QS_MAX_LENGTH_DOUBLE;
        const double y1 = cx.extents_[f].maxY / QS_MAX_LENGTH_DOUBLE;

        osgEarth::GeoExtent cubeEx(profile->getSRS(), f + x0, y0, f + x1, y1);

        // Transform to lat/long for the debugging msgs
        cubeEx.transform(profile->getSRS()->getGeodeticSRS(), llex[f]);

        dataExtents().push_back(osgEarth::DataExtent(cubeEx, cx.shallowLevel_, cx.deepLevel_));
      }
    }

    // Set time value of image if a time was found in the db
    if (cx.timeStamp_ != simCore::INFINITE_TIME_STAMP)
    {
      const osgEarth::DateTime osgTime(cx.timeStamp_.secondsSinceRefYear(1970).getSeconds());
      // Set time as a user value since config is not editable from here
      setUserValue("time", osgTime.asISO8601());
    }

    OE_INFO << LC
      << "Table: " << options().url()->full() << std::endl
      << "  Raster format = " << cx.rasterFormat_ << std::endl
      << "  Tile size     = " << cx.pixelLength_ << std::endl
      << "  Shallow level = " << cx.shallowLevel_ << std::endl
      << "  Deep level    = " << cx.deepLevel_ << std::endl
      << "  QS Extents    = " << std::endl
      << "    0: " << cx.extents_[0].minX << "," << cx.extents_[0].minY << "," << cx.extents_[0].maxX << "," << cx.extents_[0].maxY << "(" << (llex[0].isValid() ? llex[0].toString() : "empty") << ")\n"
      << "    1: " << cx.extents_[1].minX << "," << cx.extents_[1].minY << "," << cx.extents_[1].maxX << "," << cx.extents_[1].maxY << "(" << (llex[1].isValid() ? llex[1].toString() : "empty") << ")\n"
      << "    2: " << cx.extents_[2].minX << "," << cx.extents_[2].minY << "," << cx.extents_[2].maxX << "," << cx.extents_[2].maxY << "(" << (llex[2].isValid() ? llex[2].toString() : "empty") << ")\n"
      << "    3: " << cx.extents_[3].minX << "," << cx.extents_[3].minY << "," << cx.extents_[3].maxX << "," << cx.extents_[3].maxY << "(" << (llex[3].isValid() ? llex[3].toString() : "empty") << ")\n"
      << "    4: " << cx.extents_[4].minX << "," << cx.extents_[4].minY << "," << cx.extents_[4].maxX << "," << cx.extents_[4].maxY << "(" << (llex[4].isValid() ? llex[4].toString() : "empty") << ")\n"
      << "    5: " << cx.extents_[5].minX << "," << cx.extents_[5].minY << "," << cx.extents_[5].maxX << "," << cx.extents_[5].maxY << "(" << (llex[5].isValid() ? llex[5].toString() : "empty") << ")\n";

    // Line up the native format readers:
    cx.pngReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/png");
    cx.jpgReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/jpeg");
    cx.tifReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/tiff");
    cx.rgbReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/x-rgb");
  }
  return osgEarth::Status::OK();
}

osgEarth::GeoImage DBImageLayer::createImageImplementation(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress) const
{
  DBContext& cx = *static_cast<DBContext*>(context_);

  if (!cx.db_)
    return osgEarth::GeoImage::INVALID;

  osg::ref_ptr<osg::Image> result;

  // Convert osgEarth::TileKey into a QuadKeyID
  FaceIndexType faceId;
  QSNodeId      nodeId;
  osg::Vec2d    tileMin;
  osg::Vec2d    tileMax;  // Tile extents in QS units
  convertTileKeyToQsKey(key, faceId, nodeId, tileMin, tileMax);

  if (!cx.extents_[faceId].isValid())
  {
    // no data on this face? return nothing
    return osgEarth::GeoImage::INVALID;
  }

  if (key.getLevelOfDetail() > static_cast<unsigned int>(cx.deepLevel_))
  {
    // Hopefully this doesn't happen since we called setMaxDataLevel, but you never know
    return osgEarth::GeoImage::INVALID;
  }

  // Query the database
  TextureDataType* buf = nullptr;
  uint32_t bufSize = 0;
  uint32_t currentRasterSize = 0;

  QsErrorType err = cx.dbUtil_.readDataBuffer(
    cx.db_,
    cx.pathname_,
    "default",
    faceId,
    nodeId,
    &buf,
    &bufSize,
    &currentRasterSize,
    false, true);             // AllowLocalDB: no, we created it ourselves

  if (err == simVis_db::QS_IS_OK)
  {
    if (currentRasterSize > 0)
    {
      if (cx.decodeRaster_(cx.rasterFormat_, (const char*)buf, currentRasterSize, result))
      {
        // If result is 1x1, skip border processing
        if (result->s() >= 1 && result->t() >= 1)
        {
          const unsigned int resultS = static_cast<unsigned int>(result->s());
          const unsigned int resultT = static_cast<unsigned int>(result->t());

          // Tile width and height in QS units:
          const double tileWidth = tileMax.x() - tileMin.x();
          const double tileHeight = tileMax.y() - tileMin.y();

          const double qppx = 0.0;
          const double qppy = 0.0;
          const double xMin = cx.extents_[faceId].minX + qppx;
          const double xMax = cx.extents_[faceId].maxX - qppx;
          const double yMin = cx.extents_[faceId].minY + qppy;
          const double yMax = cx.extents_[faceId].maxY - qppy;

          osgEarth::ImageUtils::PixelReader read(result.get());
          osgEarth::ImageUtils::PixelWriter write(result.get());

          // Write "no data" to all pixels outside the reported extent.
          const double colw = tileWidth / (resultS - 1);
          const double rowh = tileHeight / (resultT - 1);

          for (unsigned int row = 0; row < resultS; ++row)
          {
            const double y = tileMin.y() + row * rowh;

            for (unsigned int col = 0; col < resultT; ++col)
            {
              const double x = tileMin.x() + col * colw;

              if (x < xMin || x > xMax || y < yMin || y > yMax)
              {
                osg::Vec4f pixel = read(col, row);
                pixel.a() = 0.0f;
                write(pixel, col, row);
              }
            }
          }
        }
      }
      else
      {
        OE_WARN << "Image decode failed for key " << key.str() << std::endl;
      }
    }
    else
    {
      // Raster size of 0 means no tile in the db
      //OE_DEBUG << "No image in the database for key " << key->str() << std::endl;
      result = nullptr;
    }
  }
  else
  {
    std::cerr << simVis_db::getErrorString(err) << std::endl;
    OE_WARN << "Failed to read image from " << key.str() << std::endl;
  }

  delete[] buf;

  return osgEarth::GeoImage(result.release(), key.getExtent());
}

//...........................................................

#undef LC
#define LC "[DBElevationLayer] "

REGISTER_OSGEARTH_LAYER(dbelevation, simVis::DBElevationLayer);

osgEarth::Config DBElevationLayer::Options::getConfig() const
{
  osgEarth::Config conf = osgEarth::ElevationLayer::Options::getConfig();
  conf.set("url", url());
  conf.set("deepest_level", deepestLevel());
  return conf;
}

void DBElevationLayer::Options::fromConfig(const osgEarth::Config& conf)
{
  conf.get("url", url());
  conf.get("deepest_level", deepestLevel());
}

void DBElevationLayer::setURL(const osgEarth::URI& value)
{
  options().url() = value;
}

const osgEarth::URI& DBElevationLayer::getURL() const
{
  return options().url().get();
}

void DBElevationLayer::setDeepestLevel(unsigned int value)
{
  options().deepestLevel() = value;
}

unsigned int DBElevationLayer::getDeepestLevel() const
{
  return options().deepestLevel().get();
}

void DBElevationLayer::init()
{
  osgEarth::ElevationLayer::init();
  context_ = new DBContext();
}

DBElevationLayer::~DBElevationLayer()
{
  delete static_cast<DBContext*>(context_);
}

osgEarth::Status DBElevationLayer::openImplementation()
{
  osgEarth::Status parent = osgEarth::ElevationLayer::openImplementation();
  if (parent.isError())
    return parent;


  DBContext& cx = *static_cast<DBContext*>(context_);

  if (!options().url().isSet())
    return osgEarth::Status(osgEarth::Status::ConfigurationError, "Missing required URL");

  cx.pathname_ = osgDB::findDataFile(options().url()->full(), getReadOptions());

  if (cx.dbUtil_.openDatabaseFile(cx.pathname_, &cx.db_, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX) != simVis_db::QS_IS_OK)
  {
    cx.db_ = nullptr;
    return osgEarth::Status(
      osgEarth::Status::ResourceUnavailable,
      osgEarth::Stringify() << "Failed to open DB file at " << options().url()->full());
  }
  else
  {
    QsErrorType err = cx.dbUtil_.getSetFromListOfSetsTable(
      cx.db_,
      "default",
      cx.rasterFormat_,
      cx.pixelLength_,
      cx.shallowLevel_,
      cx.deepLevel_,
      cx.extents_,
      cx.source_,
      cx.classification_,
      cx.description_,
      cx.timeSpecified_,
      cx.timeStamp_);

    // Limit the deepLevel_ by the passed-in option
    if (options().deepestLevel().isSet())
    {
      cx.deepLevel_ = simCore::sdkMin(cx.deepLevel_, static_cast<int>(options().deepestLevel().get()));
    }

    if (err != simVis_db::QS_IS_OK)
    {
      sqlite3_close(cx.db_);
      cx.db_ = nullptr;
      return osgEarth::Status(
        osgEarth::Status::ResourceUnavailable,
        osgEarth::Stringify() << "Failed to read metadata for " << cx.pathname_);
    }

    // Set up as a unified cube:
    osg::ref_ptr<osgEarth::Profile> profile = new osgEarth::Contrib::UnifiedCubeProfile();
    // DB are expected to be wgs84, which Cube defaults to
    setProfile(profile.get());

    // Lat/long extents (for debugging)
    osgEarth::GeoExtent llex[6];

    // Tell the engine how deep the data actually goes:
    for (unsigned int f = 0; f < 6; ++f)
    {
      if (cx.extents_[f].minX < cx.extents_[f].maxX && cx.extents_[f].minY < cx.extents_[f].maxY)
      {
        const double x0 = cx.extents_[f].minX / QS_MAX_LENGTH_DOUBLE;
        const double x1 = cx.extents_[f].maxX / QS_MAX_LENGTH_DOUBLE;
        const double y0 = cx.extents_[f].minY / QS_MAX_LENGTH_DOUBLE;
        const double y1 = cx.extents_[f].maxY / QS_MAX_LENGTH_DOUBLE;

        osgEarth::GeoExtent cubeEx(profile->getSRS(), f + x0, y0, f + x1, y1);

        // Transform to lat/long for the debugging msgs
        cubeEx.transform(profile->getSRS()->getGeodeticSRS(), llex[f]);

        dataExtents().push_back(osgEarth::DataExtent(cubeEx, cx.shallowLevel_, cx.deepLevel_));
      }
    }

    // Set time value of image if a time was found in the db
    if (cx.timeStamp_ != simCore::INFINITE_TIME_STAMP)
    {
      const osgEarth::DateTime osgTime(cx.timeStamp_.secondsSinceRefYear(1970).getSeconds());
      // Set time as a user value since config is not editable from here
      setUserValue("time", osgTime.asISO8601());
    }

    OE_INFO << LC
      << "Table: " << options().url()->full() << std::endl
      << "  Raster format = " << cx.rasterFormat_ << std::endl
      << "  Tile size     = " << cx.pixelLength_ << std::endl
      << "  Shallow level = " << cx.shallowLevel_ << std::endl
      << "  Deep level    = " << cx.deepLevel_ << std::endl
      << "  QS Extents    = " << std::endl
      << "    0: " << cx.extents_[0].minX << "," << cx.extents_[0].minY << "," << cx.extents_[0].maxX << "," << cx.extents_[0].maxY << "(" << (llex[0].isValid() ? llex[0].toString() : "empty") << ")\n"
      << "    1: " << cx.extents_[1].minX << "," << cx.extents_[1].minY << "," << cx.extents_[1].maxX << "," << cx.extents_[1].maxY << "(" << (llex[1].isValid() ? llex[1].toString() : "empty") << ")\n"
      << "    2: " << cx.extents_[2].minX << "," << cx.extents_[2].minY << "," << cx.extents_[2].maxX << "," << cx.extents_[2].maxY << "(" << (llex[2].isValid() ? llex[2].toString() : "empty") << ")\n"
      << "    3: " << cx.extents_[3].minX << "," << cx.extents_[3].minY << "," << cx.extents_[3].maxX << "," << cx.extents_[3].maxY << "(" << (llex[3].isValid() ? llex[3].toString() : "empty") << ")\n"
      << "    4: " << cx.extents_[4].minX << "," << cx.extents_[4].minY << "," << cx.extents_[4].maxX << "," << cx.extents_[4].maxY << "(" << (llex[4].isValid() ? llex[4].toString() : "empty") << ")\n"
      << "    5: " << cx.extents_[5].minX << "," << cx.extents_[5].minY << "," << cx.extents_[5].maxX << "," << cx.extents_[5].maxY << "(" << (llex[5].isValid() ? llex[5].toString() : "empty") << ")\n";

    // Line up the native format readers:
    cx.pngReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/png");
    cx.jpgReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/jpeg");
    cx.tifReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/tiff");
    cx.rgbReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/x-rgb");
  }
  return osgEarth::Status::OK();
}

osgEarth::GeoHeightField DBElevationLayer::createHeightFieldImplementation(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress) const
{
  DBContext& cx = *static_cast<DBContext*>(context_);

  if (!cx.db_)
    return osgEarth::GeoHeightField::INVALID;

  osg::ref_ptr<osg::HeightField> result;

  // Convert osgEarth::TileKey into a QuadKeyID
  FaceIndexType faceId;
  QSNodeId      nodeId;
  osg::Vec2d    tileMin;
  osg::Vec2d    tileMax; // Tile extents in QS units
  convertTileKeyToQsKey(key, faceId, nodeId, tileMin, tileMax);

  if (!cx.extents_[faceId].isValid())
  {
    // If there is no data on that face, return nothing.
    return osgEarth::GeoHeightField::INVALID;
  }

  // Query the database
  TextureDataType* buf = nullptr;
  uint32_t bufSize = 0;
  uint32_t currentRasterSize = 0;

  QsErrorType err = cx.dbUtil_.readDataBuffer(
    cx.db_,
    cx.pathname_,
    "default",
    faceId,
    nodeId,
    &buf,
    &bufSize,
    &currentRasterSize,
    false);             // AllowLocalDB: no, we created it ourselves

  if (err == simVis_db::QS_IS_OK)
  {
    if (currentRasterSize > 0)
    {
      osg::ref_ptr<osg::Image> image;
      if (cx.decodeRaster_(cx.rasterFormat_, (const char*)buf, currentRasterSize, image))
      {

        // SIMDIS .db elevation data is y-inverted:
        image->flipVertical();

        osgEarth::ImageToHeightFieldConverter i2h;
        result = i2h.convert(image.get());

        // If result is 1x1, skip border processing
        if (result->getNumColumns() >= 1 && result->getNumRows() >= 1)
        {
          // Tile width and height in QS units:
          const double tileWidth = tileMax.x() - tileMin.x();
          const double tileHeight = tileMax.y() - tileMin.y();

          /**
          * DB data contains a one-pixel border with undefined data. That border falls
          * within the reported extents. We have to fill that with "NO DATA".
          * First, calculate the size of a pixel in QS units for this tile:
          */
          const double qppx = tileWidth / cx.pixelLength_;
          const double qppy = tileHeight / cx.pixelLength_;

          /**
          * Adjust the reported extents to remove the border.
          * NOTE: This will fail in the (rare?) edge case in which a data extent falls
          * exactly on a cube-face boundary. Ignore that for now.
          */
          const double xMin = cx.extents_[faceId].minX + qppx;
          const double xMax = cx.extents_[faceId].maxX - qppx;
          const double yMin = cx.extents_[faceId].minY + qppy;
          const double yMax = cx.extents_[faceId].maxY - qppy;

          // Write "no data" to all pixels outside the reported extent.
          const double colWidth = tileWidth / (result->getNumColumns() - 1);
          const double rowHeight = tileHeight / (result->getNumRows() - 1);

          for (unsigned int row = 0; row < result->getNumRows(); ++row)
          {
            const double y = tileMin.y() + row * rowHeight;

            for (unsigned int col = 0; col < result->getNumColumns(); ++col)
            {
              const double x = tileMin.x() + col * colWidth;

              if (x < xMin || x > xMax || y < yMin || y >  yMax)
              {
                result->setHeight(col, row, NO_DATA_VALUE);
              }
            }
          }
        }
      }
      else
      {
        OE_WARN << "Heightfield decode failed for key " << key.str() << std::endl;
      }
    }
    else
    {
      // Raster size of 0 means no tile in the db
      result = nullptr;
    }
  }
  else
  {
    OE_WARN << "Failed to read heightfield from " << key.str() << std::endl;
  }

  delete[] buf;

  return osgEarth::GeoHeightField(result.release(), key.getExtent());
}
