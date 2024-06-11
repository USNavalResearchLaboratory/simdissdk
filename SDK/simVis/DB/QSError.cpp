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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#include <cassert>
#include <iostream>
#include "QSError.h"

namespace simVis_db {

static const char* QS_IS_UNRECOGNIZED_ERROR_VALUE_STR              = "Unrecognized Error Value";
static const char* QS_IS_OK_STR                                    = "\0";
static const char* QS_IS_COMMAND_LINE_STR                          = "Command line error";
static const char* QS_IS_UNEXPECTED_NULL_STR                       = "Unexpected NULL";
static const char* QS_IS_NO_TMPDIR_STR                             = "TMPDIR environment variable not set";
static const char* QS_IS_UNABLE_TO_OPEN_DB_STR                     = "Unable to open database";
static const char* QS_IS_UNABLE_TO_OPEN_SRC_DS_STR                 = "Unable to open src dataset";
static const char* QS_IS_UNABLE_TO_GET_SRC_DRIVER_STR              = "Unable to get src driver";
static const char* QS_IS_UNABLE_TO_GET_SRC_DRIVER_NAME_STR         = "Unable to get src driver name";
static const char* QS_IS_UNABLE_TO_CONVERT_TO_GEOGRAPHIC_STR       = "Unable to convert to geographic dataset";
static const char* QS_IS_UNABLE_TO_CREATE_INFO_FILE_STR            = "Unable to create info file";
static const char* QS_IS_UNABLE_TO_OPEN_INFO_FILE_STR              = "Unable to open info file";
static const char* QS_IS_TIME_STRING_ERROR_STR                     = "Time string error";
static const char* QS_IS_UNABLE_TO_GET_FACE_STRING_STR             = "Unable to get face string";
static const char* QS_IS_DB_NOT_INITIALIZED_STR                    = "Database not initialized";
static const char* QS_IS_UNABLE_TO_CREATE_TN_INDEX_STR             = "Unable to create texture name index";
static const char* QS_IS_UNABLE_TO_INSERT_TS_INTO_LIST_STR         = "Unable to insert texture set into list";
static const char* QS_IS_EMPTY_TABLE_NAME_STR                      = "Empty table name";
static const char* QS_IS_UNABLE_TO_CREATE_TABLE_STR                = "Unable to create table";
static const char* QS_IS_UNABLE_TO_CREATE_ID_INDEX_STR             = "Unable to create id index";
static const char* QS_IS_UNABLE_TO_CREATE_P_IMAGE_STR              = "Unable to create polar image";
static const char* QS_IS_UNABLE_TO_READ_FROM_RASTER_STR            = "Unable to read from raster";
static const char* QS_IS_FORMAT_NOT_IMPLEMENTED_STR                = "Format not implemented";
static const char* QS_IS_UNABLE_TO_WRITE_TO_BLOB_STR               = "Unable to write to blob";
static const char* QS_IS_UNABLE_SCALE_IMAGE_STR                    = "Unable to scale image";
static const char* QS_IS_PREPARE_ERROR_STR                         = "Unable to prepare statement";
static const char* QS_IS_TS_NOT_FOUND_STR                          = "Texture set not found";
static const char* QS_IS_UNABLE_TO_UPDATE_EXTENTS_STR              = "Unable to update extents";
static const char* QS_IS_UNABLE_TO_WRITE_DATA_BUFFER_STR           = "Unable to write data buffer";
static const char* QS_IS_UNABLE_TO_READ_DATA_BUFFER_STR            = "Unable to read data buffer";
static const char* QS_IS_BUSY_STR                                  = "Database is busy";
static const char* QS_IS_UNABLE_TO_UPDATE_TIME_STR                 = "Unable to update time";
static const char* QS_IS_UNABLE_TO_SET_NUM_BYTES_STR               = "Unable to set num bytes";
static const char* QS_IS_UNABLE_TO_GET_AVERAGE_REQUEST_TIME_STR    = "Unable to get average request time";
static const char* QS_IS_UNABLE_TO_REMOVE_OLDER_THAN_AVG_STR       = "Unable to remove older than average";
static const char* QS_IS_UNABLE_TO_SUM_NUMBER_OF_BYTES_STR         = "Unable to sum number of bytes";
static const char* QS_IS_UNABLE_TO_WRITE_TO_OUTPUT_DIR_STR         = "Unable to write to output directory";
static const char* QS_IS_COULD_NOT_CREATE_FUNCTION_STR             = "Unable to create function";
static const char* QS_IS_EMPTY_FUNCTION_NAME_STR                   = "Empty function name";
static const char* QS_IS_UNABLE_TO_EXECUTE_FUNCTION_STR            = "Unable to execture function";
static const char* QS_IS_UNABLE_TO_WRITE_TO_RASTER_STR             = "Unable to write to raster";
static const char* QS_IS_EMPTY_FILENAME_STR                        = "Empty filename";
static const char* QS_IS_SET_CREATION_FAILED_STR                   = "Attempted raster set creation failed";
static const char* QS_IS_NO_TIME_STAMP_STR                         = "Unable to obtain timestamp";

const char* getErrorString(const QsErrorType& errorValue)
{
  switch (errorValue)
  {
  case QS_IS_OK:
    return QS_IS_OK_STR;
  case QS_IS_COMMAND_LINE:
    return QS_IS_COMMAND_LINE_STR;
  case QS_IS_UNEXPECTED_NULL:
    return QS_IS_UNEXPECTED_NULL_STR;
  case QS_IS_NO_TMPDIR:
    return QS_IS_NO_TMPDIR_STR;
  case QS_IS_UNABLE_TO_OPEN_DB:
    return QS_IS_UNABLE_TO_OPEN_DB_STR;
  case QS_IS_UNABLE_TO_OPEN_SRC_DS:
    return QS_IS_UNABLE_TO_OPEN_SRC_DS_STR;
  case QS_IS_UNABLE_TO_GET_SRC_DRIVER:
    return QS_IS_UNABLE_TO_GET_SRC_DRIVER_STR;
  case QS_IS_UNABLE_TO_GET_SRC_DRIVER_NAME:
    return QS_IS_UNABLE_TO_GET_SRC_DRIVER_NAME_STR;
  case QS_IS_UNABLE_TO_CONVERT_TO_GEOGRAPHIC:
    return QS_IS_UNABLE_TO_CONVERT_TO_GEOGRAPHIC_STR;
  case QS_IS_UNABLE_TO_CREATE_INFO_FILE:
    return QS_IS_UNABLE_TO_CREATE_INFO_FILE_STR;
  case QS_IS_UNABLE_TO_OPEN_INFO_FILE:
    return QS_IS_UNABLE_TO_OPEN_INFO_FILE_STR;
  case QS_IS_TIME_STRING_ERROR:
    return QS_IS_TIME_STRING_ERROR_STR;
  case QS_IS_UNABLE_TO_GET_FACE_STRING:
    return QS_IS_UNABLE_TO_GET_FACE_STRING_STR;
  case QS_IS_DB_NOT_INITIALIZED:
    return QS_IS_DB_NOT_INITIALIZED_STR;
  case QS_IS_UNABLE_TO_CREATE_TN_INDEX:
    return QS_IS_UNABLE_TO_CREATE_TN_INDEX_STR;
  case QS_IS_UNABLE_TO_INSERT_TS_INTO_LIST:
    return QS_IS_UNABLE_TO_INSERT_TS_INTO_LIST_STR;
  case QS_IS_EMPTY_TABLE_NAME:
    return QS_IS_EMPTY_TABLE_NAME_STR;
  case QS_IS_UNABLE_TO_CREATE_TABLE:
    return QS_IS_UNABLE_TO_CREATE_TABLE_STR;
  case QS_IS_UNABLE_TO_CREATE_ID_INDEX:
    return QS_IS_UNABLE_TO_CREATE_ID_INDEX_STR;
  case QS_IS_UNABLE_TO_CREATE_P_IMAGE:
    return QS_IS_UNABLE_TO_CREATE_P_IMAGE_STR;
  case QS_IS_UNABLE_TO_READ_FROM_RASTER:
    return QS_IS_UNABLE_TO_READ_FROM_RASTER_STR;
  case QS_IS_FORMAT_NOT_IMPLEMENTED:
    return QS_IS_FORMAT_NOT_IMPLEMENTED_STR;
  case QS_IS_UNABLE_TO_WRITE_TO_BLOB:
    return QS_IS_UNABLE_TO_WRITE_TO_BLOB_STR;
  case QS_IS_UNABLE_SCALE_IMAGE:
    return QS_IS_UNABLE_SCALE_IMAGE_STR;
  case QS_IS_PREPARE_ERROR:
    return QS_IS_PREPARE_ERROR_STR;
  case QS_IS_TS_NOT_FOUND:
    return QS_IS_TS_NOT_FOUND_STR;
  case QS_IS_UNABLE_TO_UPDATE_EXTENTS:
    return QS_IS_UNABLE_TO_UPDATE_EXTENTS_STR;
  case QS_IS_UNABLE_TO_WRITE_DATA_BUFFER:
    return QS_IS_UNABLE_TO_WRITE_DATA_BUFFER_STR;
  case QS_IS_UNABLE_TO_READ_DATA_BUFFER:
    return QS_IS_UNABLE_TO_READ_DATA_BUFFER_STR;
  case QS_IS_BUSY:
    return QS_IS_BUSY_STR;
  case QS_IS_UNABLE_TO_UPDATE_TIME:
    return QS_IS_UNABLE_TO_UPDATE_TIME_STR;
  case QS_IS_UNABLE_TO_SET_NUM_BYTES:
    return QS_IS_UNABLE_TO_SET_NUM_BYTES_STR;
  case QS_IS_UNABLE_TO_GET_AVERAGE_REQUEST_TIME:
    return QS_IS_UNABLE_TO_GET_AVERAGE_REQUEST_TIME_STR;
  case QS_IS_UNABLE_TO_REMOVE_OLDER_THAN_AVG:
    return QS_IS_UNABLE_TO_REMOVE_OLDER_THAN_AVG_STR;
  case QS_IS_UNABLE_TO_SUM_NUMBER_OF_BYTES:
    return QS_IS_UNABLE_TO_SUM_NUMBER_OF_BYTES_STR;
  case QS_IS_UNABLE_TO_WRITE_TO_OUTPUT_DIR:
    return QS_IS_UNABLE_TO_WRITE_TO_OUTPUT_DIR_STR;
  case QS_IS_COULD_NOT_CREATE_FUNCTION:
    return QS_IS_COULD_NOT_CREATE_FUNCTION_STR;
  case QS_IS_EMPTY_FUNCTION_NAME:
    return QS_IS_EMPTY_FUNCTION_NAME_STR;
  case QS_IS_UNABLE_TO_EXECUTE_FUNCTION:
    return QS_IS_UNABLE_TO_EXECUTE_FUNCTION_STR;
  case QS_IS_UNABLE_TO_WRITE_TO_RASTER:
    return QS_IS_UNABLE_TO_WRITE_TO_RASTER_STR;
  case QS_IS_EMPTY_FILENAME:
    return QS_IS_EMPTY_FILENAME_STR;
  case QS_IS_SET_CREATION_FAILED:
    return QS_IS_SET_CREATION_FAILED_STR;
  case QS_IS_NO_TIME_STAMP:
    return QS_IS_NO_TIME_STAMP_STR;
  default:
    std::cerr << "ERROR:  Unrecognized error value (" << errorValue << ").\n";
    assert(0);
    return QS_IS_UNRECOGNIZED_ERROR_VALUE_STR;
  }
}

}
