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

#include <string>
#include <cassert>
#include <cstdlib>
#ifndef USE_SIMDIS_SDK
#include "iostreamc"
#include "Raster/RasterCommon.h"
#else // used in SIMDIS SDK
#include <iostream>
#include "Utils/Raster/RasterCommon.h"
#endif

#include "simCore/String/Format.h"

using namespace simCore;
using namespace std;
#ifdef USE_SIMDIS_SDK
using namespace simVis_db;
#endif

//---------------------------------------------------------------------------
const char* GetImageTypeExtension(const RasterFormat& imageType)
{
  switch (imageType)
  {
    case SPLIT_JPEG_2000            : return ".jp2";
    case SPLIT_SGI_RGB              : return ".rgb";
    case SPLIT_SGI_RGBA             : return ".rgba";
    case SPLIT_RGBA_ZLIB_COMPRESS   : return ".rgba";
    case SPLIT_INTA_ZLIB_COMPRESS   : return ".inta";
    case SPLIT_JPEG                 : return ".jpg";
    case SPLIT_PNG                  : return ".png";
    case SPLIT_TIFF                 : return ".tiff";
    case SPLIT_5551_GZ              : return ".5551";
    case SPLIT_5551_ZLIB_COMPRESS   : return ".5zc";
    case SPLIT_FLOAT32_ZLIB_COMPRESS: return ".fzc";
    case SPLIT_8BIT_GZ              : return ".8bz";
    case SPLIT_8BIT_ZLIB_COMPRESS   : return ".8zc";

    default:
      // assert(0);
      return ".none";
  }
}

RasterFormat GetRasterFormatFromExt(const std::string& imageTypeString)
{
  if (imageTypeString.empty())
  {
    std::cerr << "ERROR:  Empty String.\n";
    return SPLIT_UNKNOWN;
  }

  if (simCore::caseCompare(imageTypeString, ".jp2") == 0)
    return SPLIT_JPEG_2000;
  else if (simCore::caseCompare(imageTypeString, ".rgb") == 0)
    return SPLIT_SGI_RGB;
  else if (simCore::caseCompare(imageTypeString, ".rgba") == 0)
    return SPLIT_SGI_RGBA;
  else if (simCore::caseCompare(imageTypeString, ".jpg") == 0)
    return SPLIT_JPEG;
  else if (simCore::caseCompare(imageTypeString, ".png") == 0)
    return SPLIT_PNG;
  else if (simCore::caseCompare(imageTypeString, ".tiff") == 0)
    return SPLIT_TIFF;
  else if (simCore::caseCompare(imageTypeString, ".tif") == 0)
    return SPLIT_TIFF;
  else if (simCore::caseCompare(imageTypeString, ".5551") == 0)
    return SPLIT_5551_GZ;
  else if (simCore::caseCompare(imageTypeString, ".5zc") == 0)
    return SPLIT_5551_ZLIB_COMPRESS;
  else if (simCore::caseCompare(imageTypeString, ".fzc") == 0)
    return SPLIT_FLOAT32_ZLIB_COMPRESS;
  else if (simCore::caseCompare(imageTypeString, ".8bz") == 0)
    return SPLIT_8BIT_GZ;
  else if (simCore::caseCompare(imageTypeString, ".8zc") == 0)
    return SPLIT_8BIT_ZLIB_COMPRESS;
  else if (simCore::caseCompare(imageTypeString, ".inta") == 0)
    return SPLIT_INTA_ZLIB_COMPRESS;
  else
  {
    std::cerr << "ERROR:  Unrecognized image type string (" << imageTypeString.c_str() << ").\n";
    assert(0);
    return SPLIT_UNKNOWN;
  }
}

const char* GetOutputFileTypeString(const RasterFormat& imageType)
{
  switch (imageType)
  {
    case SPLIT_JPEG_2000:
      return "SPLIT_JPEG_2000";
    case SPLIT_SGI_RGB:
      return "SPLIT_SGI_RGB";
    case SPLIT_SGI_RGBA:
      return "SPLIT_SGI_RGBA";
    case SPLIT_JPEG:
      return "SPLIT_JPEG";
    case SPLIT_PNG:
      return "SPLIT_PNG";
    case SPLIT_TIFF:
      return "SPLIT_TIFF";
    case SPLIT_5551_GZ:
      return "SPLIT_5551";
    case SPLIT_5551_ZLIB_COMPRESS:
      return "SPLIT_5551_ZLIB_COMPRESS";
    case SPLIT_RGBA_ZLIB_COMPRESS:
      return "SPLIT_RGBA_ZLIB_COMPRESS";
    case SPLIT_INTA_ZLIB_COMPRESS:
      return "SPLIT_INTA_ZLIB_COMPRESS";
    case SPLIT_FLOAT32_ZLIB_COMPRESS:
      return "SPLIT_FLOAT32_ZLIB_COMPRESS";
    case SPLIT_8BIT_GZ:
      return "SPLIT_8BIT_GZ";
    case SPLIT_8BIT_ZLIB_COMPRESS:
      return "SPLIT_8BIT_ZLIB_COMPRESS";
    case SPLIT_UNKNOWN:
      return "SPLIT_UNKNOWN";
    default:
      std::cerr << "ERROR:  Unrecognized image type (" << int(imageType) << ").\n";
      assert(0);
      return "Unknown";
  }
}

RasterFormat GetOutputFileTypeFromString(const string& imageTypeString)
{
  if (imageTypeString == "SPLIT_JPEG_2000")
    return SPLIT_JPEG_2000;
  else if (imageTypeString == "SPLIT_SGI_RGB")
    return SPLIT_SGI_RGB;
  else if (imageTypeString == "SPLIT_SGI_RGBA")
    return SPLIT_SGI_RGBA;
  else if (imageTypeString == "SPLIT_JPEG")
    return SPLIT_JPEG;
  else if (imageTypeString == "SPLIT_PNG")
    return SPLIT_PNG;
  else if (imageTypeString == "SPLIT_TIFF")
    return SPLIT_TIFF;
  else if (imageTypeString == "SPLIT_5551")
    return SPLIT_5551_GZ;
  else if (imageTypeString == "SPLIT_5551_ZLIB_COMPRESS")
    return SPLIT_5551_ZLIB_COMPRESS;
  else if (imageTypeString == "SPLIT_RGBA_ZLIB_COMPRESS")
    return SPLIT_RGBA_ZLIB_COMPRESS;
  else if (imageTypeString == "SPLIT_INTA_ZLIB_COMPRESS")
    return SPLIT_INTA_ZLIB_COMPRESS;
  else if (imageTypeString == "SPLIT_FLOAT32_ZLIB_COMPRESS")
    return SPLIT_FLOAT32_ZLIB_COMPRESS;
  else if (imageTypeString == "SPLIT_8BIT_GZ")
    return SPLIT_8BIT_GZ;
  else if (imageTypeString == "SPLIT_8BIT_ZLIB_COMPRESS")
    return SPLIT_8BIT_ZLIB_COMPRESS;
  else
  {
    std::cerr << "ERROR:  Unrecognized image type string (" << imageTypeString.c_str() << ").\n";
    assert(0);
    return SPLIT_UNKNOWN;
  }
}

int getDstPixelsIndex(const int& bandIndex,
                      const int& y,
                      const int& x,
                      const int& length,
                      const int& numBytesPerEntireBand,
                      const int& numBytesPerBandValue)
{
  return (bandIndex*numBytesPerEntireBand) + (y*length*numBytesPerBandValue) + (x*numBytesPerBandValue);
}

