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

#ifndef RASTER_COMMON_H
#define RASTER_COMMON_H

#include <string>
#include <vector>
#ifndef USE_SIMDIS_SDK
#include "inttypesc.h"
#else
#include "simCore/Common/Common.h"
#endif
#include "stdgl.h"

#ifdef USE_SIMDIS_SDK
namespace simVis_db
{
#endif
  typedef uint8_t TextureDataType;

  /** Struct containing red, green, blue, opacity values for a pixel */
  struct TexturePixel
  {
      uint8_t red;
      uint8_t green;
      uint8_t blue;
      uint8_t opacity;

      TexturePixel()
        : red(0),
          green(0),
          blue(0),
          opacity(0)
      {}
  };

  /// Raster formats
  typedef uint8_t RasterFormat;
  static const RasterFormat SPLIT_UNKNOWN            = 0;
  static const RasterFormat SPLIT_SGI_RGB            = 1;
  static const RasterFormat SPLIT_SGI_RGBA           = 2;
  static const RasterFormat SPLIT_5551_GZ            = 3;
  static const RasterFormat SPLIT_5551_ZLIB_COMPRESS = 4;
  static const RasterFormat SPLIT_RGBA_ZLIB_COMPRESS = 5;
  static const RasterFormat SPLIT_INTA_ZLIB_COMPRESS = 6;
  static const RasterFormat SPLIT_JPEG_2000          = 8;
  static const RasterFormat SPLIT_8BIT_GZ            = 9;   // GL_LUMINANCE
  static const RasterFormat SPLIT_8BIT_ZLIB_COMPRESS = 10;
  static const RasterFormat SPLIT_FLOAT32_ZLIB_COMPRESS = 11;
  static const RasterFormat SPLIT_JPEG = 12;
  static const RasterFormat SPLIT_PNG = 13;
  static const RasterFormat SPLIT_TIFF = 14;
  typedef std::vector<RasterFormat> vRasterFormats;

  /** Matches given RasterFormat to a file extension (with a .) */
  const char* GetImageTypeExtension(const RasterFormat& imageType);

  /** Matches given file extension string (with a .) to the appropriate RasterFormat constant */
  RasterFormat GetRasterFormatFromExt(const std::string&);

  /** Matches given RasterFormat to the appropriate RasterFormat indicator string */
  const char* GetOutputFileTypeString(const RasterFormat& imageType);

  /** Matches given RasterFormat indicator string to the appropriate RasterFormat constant */
  RasterFormat GetOutputFileTypeFromString(const std::string& imageTypeString);

  /**
   * Given a (band,x,y) position, calculates an index (in bytes) into a non-interleaved square raster buffer
   * @param bandIndex Number of the current band Range:(1,inf)
   * @param x
   * @param y
   * @param length Length of the square raster buffer
   * @param numBytesPerEntireBand Number of bytes in one entire band
   * @param numBytesPerBandValue Number of bytes in a single band value
   * @return Index into a non-interleaved raster buffer
   */
  int getDstPixelsIndex(const int& bandIndex, const int& y, const int& x,
                        const int& length, const int& numBytesPerEntireBand, const int& numBytesPerBandValue);

  /**
   * A base class for a texture subclass
   * This is currently serving as a method for not introducing SimObjLib TexLib
   * dependencies into new code.
   */
  class Texture
  {
    protected:
      Texture() {}
      virtual ~Texture() {}
    public:
      virtual std::string Name() const = 0;
      virtual void TurnOn() = 0;
      virtual void SetTexParam(GLenum paramName, GLfloat paramValue) = 0;
      virtual void SetTexParamV(GLenum paramName, const GLfloat* paramValues, int size) = 0;
  };

  /** A function for finding a texture by name */
  typedef Texture* (CreateTextureFunction)(const std::string& textureName);
  typedef void (DeleteTextureFunction)(Texture*);

#ifdef USE_SIMDIS_SDK
} // namespace simVis_db
#endif

#endif /* RASTER_COMMON_H */
