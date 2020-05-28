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
#ifndef SIMVIS_DB_QSCOMMON_H
#define SIMVIS_DB_QSCOMMON_H

#include "simCore/Common/Common.h"

namespace simVis_db
{
  typedef uint8_t FaceIndexType;
  typedef uint8_t TextureDataType;
  typedef uint8_t RasterFormat;

  /// Raster formats
  static const RasterFormat SPLIT_UNKNOWN = 0;
  static const RasterFormat SPLIT_SGI_RGB = 1;
  static const RasterFormat SPLIT_SGI_RGBA = 2;
  static const RasterFormat SPLIT_5551_GZ = 3;
  static const RasterFormat SPLIT_5551_ZLIB_COMPRESS = 4;
  static const RasterFormat SPLIT_RGBA_ZLIB_COMPRESS = 5;
  static const RasterFormat SPLIT_INTA_ZLIB_COMPRESS = 6;
  static const RasterFormat SPLIT_JPEG_2000 = 8;
  static const RasterFormat SPLIT_8BIT_GZ = 9;   // GL_LUMINANCE
  static const RasterFormat SPLIT_8BIT_ZLIB_COMPRESS = 10;
  static const RasterFormat SPLIT_FLOAT32_ZLIB_COMPRESS = 11;
  static const RasterFormat SPLIT_JPEG = 12;
  static const RasterFormat SPLIT_PNG = 13;
  static const RasterFormat SPLIT_TIFF = 14;
} // namespace simVis_db

#endif /* SIMVIS_DB_QSCOMMON_H */
