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

#ifndef QS_ERROR_H
#define QS_ERROR_H

#include <string>

#ifndef USE_SIMDIS_SDK
#include "inttypesc.h"
#else
#include "simCore/Common/Common.h"

namespace simVis_db
{
#endif
  //===========================================================================
  typedef int32_t QsErrorType;
  static const QsErrorType QS_IS_OK                                  = 0;
  static const QsErrorType QS_IS_COMMAND_LINE                        = 1;
  static const QsErrorType QS_IS_UNEXPECTED_NULL                     = 2;
  static const QsErrorType QS_IS_NO_TMPDIR                           = 3;
  static const QsErrorType QS_IS_UNABLE_TO_OPEN_DB                   = 4;
  static const QsErrorType QS_IS_UNABLE_TO_OPEN_SRC_DS               = 5;
  static const QsErrorType QS_IS_UNABLE_TO_GET_SRC_DRIVER            = 6;
  static const QsErrorType QS_IS_UNABLE_TO_GET_SRC_DRIVER_NAME       = 7;
  static const QsErrorType QS_IS_UNABLE_TO_CONVERT_TO_GEOGRAPHIC     = 8;
  static const QsErrorType QS_IS_UNABLE_TO_CREATE_INFO_FILE          = 9;
  static const QsErrorType QS_IS_UNABLE_TO_OPEN_INFO_FILE            = 10;
  static const QsErrorType QS_IS_TIME_STRING_ERROR                   = 11;
  static const QsErrorType QS_IS_UNABLE_TO_GET_FACE_STRING           = 12;
  static const QsErrorType QS_IS_DB_NOT_INITIALIZED                  = 13;
  static const QsErrorType QS_IS_UNABLE_TO_CREATE_TN_INDEX           = 14;
  static const QsErrorType QS_IS_UNABLE_TO_INSERT_TS_INTO_LIST       = 15;
  static const QsErrorType QS_IS_EMPTY_TABLE_NAME                    = 16;
  static const QsErrorType QS_IS_UNABLE_TO_CREATE_TABLE              = 17;
  static const QsErrorType QS_IS_UNABLE_TO_CREATE_ID_INDEX           = 18;
  static const QsErrorType QS_IS_UNABLE_TO_CREATE_P_IMAGE            = 19;
  static const QsErrorType QS_IS_UNABLE_TO_READ_FROM_RASTER          = 20;
  static const QsErrorType QS_IS_FORMAT_NOT_IMPLEMENTED              = 21;
  static const QsErrorType QS_IS_UNABLE_TO_WRITE_TO_BLOB             = 22;
  static const QsErrorType QS_IS_UNABLE_SCALE_IMAGE                  = 23;
  static const QsErrorType QS_IS_PREPARE_ERROR                       = 24;
  static const QsErrorType QS_IS_TS_NOT_FOUND                        = 25;
  static const QsErrorType QS_IS_UNABLE_TO_UPDATE_EXTENTS            = 26;
  static const QsErrorType QS_IS_UNABLE_TO_WRITE_DATA_BUFFER         = 27;
  static const QsErrorType QS_IS_UNABLE_TO_READ_DATA_BUFFER          = 28;
  static const QsErrorType QS_IS_BUSY                                = 29;
  static const QsErrorType QS_IS_UNABLE_TO_UPDATE_TIME               = 30;
  static const QsErrorType QS_IS_UNABLE_TO_SET_NUM_BYTES             = 31;
  static const QsErrorType QS_IS_UNABLE_TO_GET_AVERAGE_REQUEST_TIME  = 32;
  static const QsErrorType QS_IS_UNABLE_TO_REMOVE_OLDER_THAN_AVG     = 33;
  static const QsErrorType QS_IS_UNABLE_TO_SUM_NUMBER_OF_BYTES       = 34;
  static const QsErrorType QS_IS_UNABLE_TO_WRITE_TO_OUTPUT_DIR       = 35;
  static const QsErrorType QS_IS_COULD_NOT_CREATE_FUNCTION           = 36;
  static const QsErrorType QS_IS_EMPTY_FUNCTION_NAME                 = 37;
  static const QsErrorType QS_IS_UNABLE_TO_EXECUTE_FUNCTION          = 38;
  static const QsErrorType QS_IS_UNABLE_TO_WRITE_TO_RASTER           = 39;
  static const QsErrorType QS_IS_EMPTY_FILENAME                      = 40;
  static const QsErrorType QS_IS_SET_CREATION_FAILED                 = 41;
  static const QsErrorType QS_IS_NO_TIME_STAMP                       = 42;

  //===========================================================================
  const char* GetErrorString(const QsErrorType&);

#ifdef USE_SIMDIS_SDK
} // namespace simVis_db
#endif

#endif /* QS_ERROR_H */
