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

#include "simCore/Calc/Math.h"
#include "osg/ValueObject"
#include "osgEarth/Registry"
#include "osgEarth/FileUtils"
#include "osgEarth/Cube"
#include "osgEarth/HeightFieldUtils"
#include "osgEarth/ImageUtils"
#include "osgEarth/ImageToHeightFieldConverter"
#include "osgEarth/DateTime"
#include "osgDB/FileNameUtils"
#include "osgDB/ObjectWrapper"
#include "osgDB/WriteFile"
#include "osgDB/FileUtils"
#include "simVis/osgEarthVersion.h"
#include "QSCommon.h"
#include "swapbytes.h"
#include "SQLiteDataBaseReadUtil.h"
#include "DBTileSource.h"

#define LC "[simVis::DBTileSource] "

namespace simVis_db {

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

    out_fmin.set(xMin * gQsDMaxLength, yMin * gQsDMaxLength);
    out_fmax.set(xMax * gQsDMaxLength, yMax * gQsDMaxLength);

    return true;
  }

  bool decompressZLIB(const char* input, int inputLen, std::string& output)
  {
    osgDB::BaseCompressor* comp = osgDB::Registry::instance()->getObjectWrapperManager()->findCompressor("zlib");
    std::string inString(input, inputLen);
    std::istringstream inStream(inString);
    return comp->decompress(inStream, output);
  }
}

// --------------------------------------------------------------------------

DBTileSource::DBTileSource(const osgEarth::Contrib::TileSourceOptions& options)
  : osgEarth::Contrib::TileSource(options),
  options_(options),
  db_(NULL),
  rasterFormat_(SPLIT_UNKNOWN),
  pixelLength_(128),
  shallowLevel_(0),
  deepLevel_(32),
  timeSpecified_(false),
  timeStamp_(simCore::INFINITE_TIME_STAMP)
{
  if (!options_.url().isSet() || options_.url()->empty())
  {
    OE_WARN << LC << "No pathname given" << std::endl;
  }
}

DBTileSource::~DBTileSource()
{
  if (db_)
  {
    sqlite3_close(db_);
  }
}

osgEarth::Status DBTileSource::initialize(const osgDB::Options* dbOptions)
{
  if (options_.url().isSet())
  {
    pathname_ = osgDB::findDataFile(options_.url()->full(), dbOptions);

    if (dbUtil_.OpenDataBaseFile(pathname_, &db_, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX) != QS_IS_OK)
    {
      db_ = NULL;
      return osgEarth::Status::Error(osgEarth::Stringify() << "Failed to open DB file at " << options_.url()->full());
    }
    else
    {
      QsErrorType err = dbUtil_.TsGetSetFromListOfSetsTable(
        db_,
        "default",
        rasterFormat_,
        pixelLength_,
        shallowLevel_,
        deepLevel_,
        extents_,
        source_,
        classification_,
        description_,
        timeSpecified_,
        timeStamp_);

      // Limit the deepLevel_ by the passed-in option
      if (options_.deepestLevel().isSet())
      {
        deepLevel_ = simCore::sdkMin(deepLevel_, static_cast<int>(options_.deepestLevel().get()));
      }

      if (err != QS_IS_OK)
      {
        sqlite3_close(db_);
        db_ = NULL;
        return osgEarth::Status::Error(osgEarth::Stringify() << "Failed to read metadata for " << pathname_);
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
        if (extents_[f].minX < extents_[f].maxX && extents_[f].minY < extents_[f].maxY)
        {
          const double x0 = extents_[f].minX / gQsDMaxLength;
          const double x1 = extents_[f].maxX / gQsDMaxLength;
          const double y0 = extents_[f].minY / gQsDMaxLength;
          const double y1 = extents_[f].maxY / gQsDMaxLength;

          osgEarth::GeoExtent cubeEx(profile->getSRS(), f + x0, y0, f + x1, y1);

          // Transform to lat/long for the debugging msgs
          cubeEx.transform(profile->getSRS()->getGeodeticSRS(), llex[f]);

          getDataExtents().push_back(osgEarth::DataExtent(cubeEx, shallowLevel_, deepLevel_));
        }
      }

      // Set time value of image if a time was found in the db
      if (timeStamp_ != simCore::INFINITE_TIME_STAMP)
      {
        const osgEarth::DateTime osgTime(timeStamp_.secondsSinceRefYear(1970).getSeconds());
        // Set time as a user value since config is not editable from here
        setUserValue("time", osgTime.asISO8601());
      }

      OE_INFO << LC
        << "Table: " << options_.url()->full() << std::endl
        << "  Raster format = " << rasterFormat_ << std::endl
        << "  Tile size     = " << pixelLength_ << std::endl
        << "  Shallow level = " << shallowLevel_ << std::endl
        << "  Deep level    = " << deepLevel_ << std::endl
        << "  QS Extents    = " << std::endl
        << "    0: " << extents_[0].minX << "," << extents_[0].minY << "," << extents_[0].maxX << "," << extents_[0].maxY << "(" << (llex[0].isValid() ? llex[0].toString() : "empty") << ")\n"
        << "    1: " << extents_[1].minX << "," << extents_[1].minY << "," << extents_[1].maxX << "," << extents_[1].maxY << "(" << (llex[1].isValid() ? llex[1].toString() : "empty") << ")\n"
        << "    2: " << extents_[2].minX << "," << extents_[2].minY << "," << extents_[2].maxX << "," << extents_[2].maxY << "(" << (llex[2].isValid() ? llex[2].toString() : "empty") << ")\n"
        << "    3: " << extents_[3].minX << "," << extents_[3].minY << "," << extents_[3].maxX << "," << extents_[3].maxY << "(" << (llex[3].isValid() ? llex[3].toString() : "empty") << ")\n"
        << "    4: " << extents_[4].minX << "," << extents_[4].minY << "," << extents_[4].maxX << "," << extents_[4].maxY << "(" << (llex[4].isValid() ? llex[4].toString() : "empty") << ")\n"
        << "    5: " << extents_[5].minX << "," << extents_[5].minY << "," << extents_[5].maxX << "," << extents_[5].maxY << "(" << (llex[5].isValid() ? llex[5].toString() : "empty") << ")\n";

      // Line up the native format readers:
      pngReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/png");
      jpgReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/jpeg");
      tifReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/tiff");
      rgbReader_ = osgDB::Registry::instance()->getReaderWriterForMimeType("image/x-rgb");
    }
  }

  return osgEarth::STATUS_OK;
}

int DBTileSource::getPixelsPerTile() const
{
  return pixelLength_;
}

osg::Image* DBTileSource::createImage(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress)
{
  return createImage_(key, false);
}

osg::HeightField* DBTileSource::createHeightField(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress)
{
  if (!db_) return NULL;

  osg::ref_ptr<osg::HeightField> result;

  // Convert osgEarth::TileKey into a QuadKeyID
  FaceIndexType faceId;
  QSNodeId      nodeId;
  osg::Vec2d    tileMin;
  osg::Vec2d    tileMax; // Tile extents in QS units
  convertTileKeyToQsKey(key, faceId, nodeId, tileMin, tileMax);

  if (!extents_[faceId].Valid())
  {
    // If there is no data on that face, return nothing.
    OE_DEBUG << LC << "Face " << (int)faceId << " is invalid; returning empty heightfield" << std::endl;
    return NULL;
  }

  // Query the database
  TextureDataType* buf = NULL;
  uint32_t bufSize = 0;
  uint32_t currentRasterSize = 0;

  QsErrorType err = dbUtil_.TsReadDataBuffer(
    db_,
    pathname_,
    "default",
    faceId,
    nodeId,
    &buf,
    &bufSize,
    &currentRasterSize,
    false);             // AllowLocalDB: no, we created it ourselves

  if (err == QS_IS_OK)
  {
    if (currentRasterSize > 0)
    {
      osg::ref_ptr<osg::Image> image;
      if (decodeRaster_(rasterFormat_, (const char*)buf, currentRasterSize, image))
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
          const double qppx = tileWidth / pixelLength_;
          const double qppy = tileHeight / pixelLength_;

          /**
           * Adjust the reported extents to remove the border.
           * NOTE: This will fail in the (rare?) edge case in which a data extent falls
           * exactly on a cube-face boundary. Ignore that for now.
           */
          const double xMin = extents_[faceId].minX + qppx;
          const double xMax = extents_[faceId].maxX - qppx;
          const double yMin = extents_[faceId].minY + qppy;
          const double yMax = extents_[faceId].maxY - qppy;

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
      result = NULL;
    }
  }
  else
  {
    OE_WARN << "Failed to read heightfield from " << key.str() << std::endl;
  }

  delete[] buf;
  return result.release();
}

osg::Image* DBTileSource::createImage_(const osgEarth::TileKey& key, bool isHeightField)
{
  if (!db_)
    return NULL;

  osg::ref_ptr<osg::Image> result;

  // Convert osgEarth::TileKey into a QuadKeyID
  FaceIndexType faceId;
  QSNodeId      nodeId;
  osg::Vec2d    tileMin;
  osg::Vec2d    tileMax;  // Tile extents in QS units
  convertTileKeyToQsKey(key, faceId, nodeId, tileMin, tileMax);

  if (!extents_[faceId].Valid())
  {
    // If there is no data on that face, return nothing.
    OE_DEBUG << LC << "Face " << static_cast<int>(faceId) << " is invalid; returning empty image" << std::endl;
    return NULL;
  }

  if (key.getLevelOfDetail() > static_cast<unsigned int>(deepLevel_))
  {
    // Hopefully this doesn't happen since we called setMaxDataLevel, but you never know
    return NULL;
  }

  // Query the database
  TextureDataType* buf = NULL;
  uint32_t bufSize = 0;
  uint32_t currentRasterSize = 0;

  QsErrorType err = dbUtil_.TsReadDataBuffer(
    db_,
    pathname_,
    "default",
    faceId,
    nodeId,
    &buf,
    &bufSize,
    &currentRasterSize,
    false, true);             // AllowLocalDB: no, we created it ourselves

  if (err == QS_IS_OK)
  {
    if (currentRasterSize > 0)
    {
      if (decodeRaster_(rasterFormat_, (const char*)buf, currentRasterSize, result))
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
          const double xMin = extents_[faceId].minX + qppx;
          const double xMax = extents_[faceId].maxX - qppx;
          const double yMin = extents_[faceId].minY + qppy;
          const double yMax = extents_[faceId].maxY - qppy;

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
      result = NULL;
    }
  }
  else
  {
    std::cerr << GetErrorString(err) << std::endl;
    OE_WARN << "Failed to read image from " << key.str() << std::endl;
  }

  delete[] buf;
  return result.release();
}

std::string DBTileSource::getExtension() const
{
  // Image formats with an alpha channel:
  if (
    rasterFormat_ == SPLIT_5551_ZLIB_COMPRESS ||
    rasterFormat_ == SPLIT_5551_GZ ||
    rasterFormat_ == SPLIT_RGBA_ZLIB_COMPRESS ||
    rasterFormat_ == SPLIT_INTA_ZLIB_COMPRESS ||
    rasterFormat_ == SPLIT_SGI_RGBA ||
    rasterFormat_ == SPLIT_PNG ||
    rasterFormat_ == SPLIT_TIFF)
  {
    return "png";
  }

  // Image formats with no alpha channel:
  else if (
    rasterFormat_ == SPLIT_SGI_RGB ||
    rasterFormat_ == SPLIT_JPEG)
  {
    return "jpg";
  }

  // Elevation formats:
  else
  {
    return "tif";
  }
}

template<typename T>
void makeImage(int size, GLenum internalFormat, GLenum pixelFormat, GLenum type,
  std::string& buf, osg::ref_ptr<osg::Image>& outImage)
{
  unsigned char* data = new unsigned char[buf.length()];
  std::copy(buf.begin(), buf.end(), data);

  // Be sure to cast here to get the right swap function:
  make_big_endian((T*)data, size * size);

  outImage = new osg::Image();
  outImage->setImage(size, size, 1, internalFormat, pixelFormat, type, data, osg::Image::USE_NEW_DELETE);
}

// Uses one of OSG's native ReaderWriter's to read image data from a buffer.
static bool readNativeImage(osgDB::ReaderWriter* reader, const char* inBuf, int inBufLen, osg::ref_ptr<osg::Image>& outImage)
{
  std::string inString(inBuf, inBufLen);
  std::istringstream inStream(inString);
  osgDB::ReaderWriter::ReadResult result = reader->readImage(inStream);
  outImage = result.getImage();
  if (result.error() || !outImage.valid())
  {
    OE_WARN << LC << "Failed to read JPEG image" << std::endl;
    return false;
  }
  else
    return true;
}

// Decode a raster from an input buffer in one of SIMDIS's .db raster formats.
bool DBTileSource::decodeRaster_(int rasterFormat, const char* inputBuffer, int inputBufferLen, osg::ref_ptr<osg::Image>& outImage)
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
      OE_WARN << LC << "SGI RGBA reader not available" << std::endl;
  }
  break;

  case SPLIT_SGI_RGB: // UNTESTED
  {
    if (rgbReader_.valid())
      return readNativeImage(rgbReader_.get(), inputBuffer, inputBufferLen, outImage);
    else
      OE_WARN << LC << "SGI RGB reader not available" << std::endl;
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
      OE_WARN << LC << "JPEG reader not available" << std::endl;
  }
  break;

  case SPLIT_PNG: // UNTESTED
  {
    if (pngReader_.valid())
      return readNativeImage(pngReader_.get(), inputBuffer, inputBufferLen, outImage);
    else
      OE_WARN << LC << "PNG reader not available" << std::endl;
  }
  break;

  case SPLIT_TIFF: // UNTESTED
  {
    if (tifReader_.valid())
      return readNativeImage(tifReader_.get(), inputBuffer, inputBufferLen, outImage);
    else
      OE_WARN << LC << "TIFF reader not available" << std::endl;
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

}
