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

#include <sstream>

#ifndef USE_SIMDIS_SDK
#include "iostreamc"
#include "File/FileBase.h"
#include "Time/NetworkTimeStamp.h"
#else // used in SIMDIS SDK
#include <iostream>
#include "simCore/Time/Utils.h"
#endif

#include "swapbytes.h"
#include "QSCommon.h"
#include "SQLiteDataBaseReadUtil.h"

using namespace std;
#ifndef USE_SIMDIS_SDK
using namespace UTILS;
#else
using namespace simCore;
using namespace simVis_db;
#endif

namespace
{
  static const int gMaxBufferSize = 20000000;

  std::string printExtendedErrorMessage(sqlite3* sqlite3Db)
  {
    std::stringstream errStr;
    int extendedErrorCode = sqlite3_extended_errcode(sqlite3Db);
    errStr << "  Ext Err Code("<< extendedErrorCode << ") ";
    // Extended Result Codes: http://www.sqlite.org/c3ref/c_abort_rollback.html
    switch (extendedErrorCode)
    {
    case SQLITE_IOERR_READ: errStr << "SQLITE_IOERR_READ"; break;
    case SQLITE_IOERR_SHORT_READ: errStr << "SQLITE_SHORT_READ"; break;
    case SQLITE_IOERR_WRITE: errStr << "SQLITE_IOERR_WRITE"; break;
    case SQLITE_IOERR_FSYNC: errStr << "SQLITE_IOERR_FSYNC"; break;
    case SQLITE_IOERR_DIR_FSYNC: errStr << "SQLITE_IOERR_DIR_FSYNC"; break;
    case SQLITE_IOERR_TRUNCATE: errStr << "SQLITE_IOERR_TRUNCATE"; break;
    case SQLITE_IOERR_FSTAT: errStr << "SQLITE_IOERR_FSTAT"; break;
    case SQLITE_IOERR_UNLOCK: errStr << "SQLITE_IOERR_UNLOCK"; break;
    case SQLITE_IOERR_RDLOCK: errStr << "SQLITE_IOERR_RDLOCK"; break;
    case SQLITE_IOERR_DELETE: errStr << "SQLITE_IOERR_DELETE"; break;
    case SQLITE_IOERR_BLOCKED: errStr << "SQLITE_IOERR_BLOCKED"; break;
    case SQLITE_IOERR_NOMEM: errStr << "SQLITE_IOERR_NOMEM"; break;
    case SQLITE_IOERR_ACCESS: errStr << "SQLITE_IOERR_ACCESS"; break;
    case SQLITE_IOERR_CHECKRESERVEDLOCK: errStr << "SQLITE_IOERR_CHECKRESERVEDLOCK"; break;
    case SQLITE_IOERR_LOCK: errStr << "SQLITE_IOERR_LOCK"; break;
    case SQLITE_IOERR_CLOSE: errStr << "SQLITE_IOERR_CLOSE"; break;
    case SQLITE_IOERR_DIR_CLOSE: errStr << "SQLITE_IOERR_DIR_CLOSE"; break;
    case SQLITE_IOERR_SHMOPEN: errStr << "SQLITE_IOERR_SHMOPEN"; break;
    case SQLITE_IOERR_SHMSIZE: errStr << "SQLITE_IOERR_SHMSIZE"; break;
    case SQLITE_IOERR_SHMLOCK: errStr << "SQLITE_IOERR_SHMLOCK"; break;
    case SQLITE_IOERR_SHMMAP: errStr << "SQLITE_IOERR_SHMMAP"; break;
    case SQLITE_IOERR_SEEK: errStr << "SQLITE_IOERR_SEEK"; break;
    case SQLITE_LOCKED_SHAREDCACHE: errStr << "SQLITE_LOCKED_SHAREDCACHE"; break;
    case SQLITE_BUSY_RECOVERY: errStr << "SQLITE_BUSY_RECOVERY"; break;
    case SQLITE_CANTOPEN_NOTEMPDIR: errStr << "SQLITE_CANTOPEN_NOTEMPDIR"; break;
    case SQLITE_CANTOPEN_ISDIR: errStr << "SQLITE_CANTOPEN_ISDIR"; break;
    case SQLITE_CORRUPT_VTAB: errStr << "SQLITE_CORRUPT_VTAB"; break;
    case SQLITE_READONLY_RECOVERY: errStr << "SQLITE_READONLY_RECOVERY"; break;
    case SQLITE_READONLY_CANTLOCK: errStr << "SQLITE_READONLY_CANTLOCK"; break;
    case SQLITE_ABORT_ROLLBACK: errStr << "SQLITE_ABORT_ROLLBACK"; break;
    default: errStr << "UNK"; break;
    }
    errStr << ", Desc: " << sqlite3_errmsg(sqlite3Db) << "\n";
    return errStr.str();
  }
}

//=====================================================================================
void CloseSqliteDBs(vSqlite3* given)
{
  if (given == NULL || given->empty())
    return;
  vSqlite3::iterator iter;
  for (iter = given->begin(); iter != given->end(); ++iter)
  {
    if (*iter == NULL)
      continue;
    int errorCode  = sqlite3_close(*iter);
    if (errorCode != SQLITE_OK)
      cerr << "sqlite3_close: " << printExtendedErrorMessage(*iter);
  }
  given->clear();
}

//=====================================================================================
SQLiteDataBaseReadUtil::SQLiteDataBaseReadUtil()
  : textureSetSelectCommand_(""),
    tsInsertFileIdData_(2),
    tsInsertSetTextureSetName_(1),
    tsInsertSetIdRasterFormat_(2),
    tsInsertSetIdPixelLength_(3),
    tsInsertSetIdShallowestLevel_(4),
    tsInsertSetIdDeepestLevel_(5),
    tsInsertSetIdExtents_(6),
    tsInsertSetIdSource_(7),
    tsInsertSetIdClassification_(8),
    tsInsertSetIdDescription_(9),
    tsInsertSetIdTimeSpecified_(10),
    tsInsertSetIdTimeValue_(11)
{
  QSNodeId nodeID;
  sizeOfIdBlob_ = sizeof(FaceIndexType) + nodeID.SizeOf();

  // Creates the command for reading an image from a "texture set" table
  textureSetSelectFileCommand1_ = "SELECT * From \"";
  textureSetSelectFileCommand2_ = "\" WHERE ";
  textureSetSelectFileCommand2_.append(QS_TO_ID);
  textureSetSelectFileCommand2_.append("=?");

  // Creates the command for select a texture set row from a "list of texture sets" table
  textureSetSelectCommand_ = "SELECT * From ";
  textureSetSelectCommand_.append(QS_LIST_OF_TEXTURE_SETS_TABLE_NAME);
  textureSetSelectCommand_.append(" WHERE ");
  textureSetSelectCommand_.append(QS_TSO_NAME_OF_TEXTURE_SET_TABLE);
  textureSetSelectCommand_.append("=?");
}

//-------------------------------------------------------------------------------------
SQLiteDataBaseReadUtil::~SQLiteDataBaseReadUtil()
{
}

//-------------------------------------------------------------------------------------
QsErrorType SQLiteDataBaseReadUtil::OpenDataBaseFile(const std::string& dbFileName,
                                                     sqlite3** sqlite3Db,
                                                     const int& flags) const
{
#ifdef DATABASE_UTIL_FUNCTION_ENTRY_DEBUG
  cerr << "DBUTIL FUNCTION OpenDataBaseFile  " << __LINE__ << "\n";
#endif
  if (dbFileName.empty() || (sqlite3Db == NULL))
    return QS_IS_UNABLE_TO_OPEN_DB;

#ifndef USE_SIMDIS_SDK
  // Checks if creation of the db file is not allowed
  if ((flags & SQLITE_OPEN_CREATE) == 0)
  {
    // db file creation is not allowed
    if (UTILS::fileIsFile(dbFileName) == false)
      return QS_IS_UNABLE_TO_OPEN_DB;
  }
#endif

  // Attempts to open the database file
  int errorCode = sqlite3_open_v2(dbFileName.c_str(), sqlite3Db, flags, NULL);
  if ((*sqlite3Db == 0) || (errorCode != SQLITE_OK))
  {
    if ((errorCode == SQLITE_BUSY) ||
       (errorCode == SQLITE_LOCKED))
      return QS_IS_BUSY;
    cerr << "OpenDataBaseFile sqlite3_open_v2 Error: " << dbFileName << "\n" << printExtendedErrorMessage(*sqlite3Db);
    return QS_IS_UNABLE_TO_OPEN_DB;
  }
  if (sqlite3_exec(*sqlite3Db, "PRAGMA CACHE_SIZE=100;", NULL, NULL, NULL) != SQLITE_OK)
  {
    cerr << "Unable to set SQLite cache size " << dbFileName << "\n";
    cerr << printExtendedErrorMessage(*sqlite3Db);
  }

  return QS_IS_OK;
}

//-------------------------------------------------------------------------------------
QsErrorType SQLiteDataBaseReadUtil::TsReadDataBuffer(sqlite3* sqlite3Db,
                                                     const string& dbFileName,
                                                     const string& dataTableName,
                                                     const FaceIndexType& faceIndex,
                                                     const QSNodeId& nodeID,
                                                     TextureDataType** buffer,
                                                     uint32_t* bufferSize,
                                                     uint32_t* currentRasterSize,
                                                     bool allowLocalDB,
                                                     bool displayErrorMessage) const
{
#ifdef DATABASE_UTIL_FUNCTION_ENTRY_DEBUG
  cerr << "DBUTIL FUNCTION TsReadDataBuffer  " << __LINE__ << "\n";
#endif
  int returnValue;

  if ((buffer == NULL) || (bufferSize == NULL) || (currentRasterSize == NULL))
    return QS_IS_UNEXPECTED_NULL;

  *currentRasterSize = 0;

  if (dataTableName.empty() || dbFileName.empty())
    return QS_IS_EMPTY_TABLE_NAME;

  // opens the database
  bool localDb = false;
  QsErrorType tmpReturnValue;
  if (sqlite3Db == NULL)
  {
    if (allowLocalDB == false)
      return QS_IS_DB_NOT_INITIALIZED;
    localDb = true;
    tmpReturnValue = OpenDataBaseFile(dbFileName, &sqlite3Db, SQLITE_OPEN_READONLY|SQLITE_OPEN_FULLMUTEX);
    if (tmpReturnValue != QS_IS_OK)
      return tmpReturnValue;
  }

  string sqlCommand;
  sqlCommand = textureSetSelectFileCommand1_;
  sqlCommand.append(dataTableName);
  sqlCommand.append(textureSetSelectFileCommand2_);

  // prepares the statement
  sqlite3_stmt* stmt = 0;
  returnValue = sqlite3_prepare_v2(sqlite3Db, sqlCommand.c_str(), static_cast<int>(sqlCommand.length()), &stmt, NULL);
  if (returnValue != SQLITE_OK)
  {
    if (displayErrorMessage && (returnValue != SQLITE_BUSY && returnValue != SQLITE_LOCKED))
    {
      cerr << "TsReadDataBuffer sqlite3_prepare_v2 Error(" << returnValue << "): " << dbFileName << "\n" << printExtendedErrorMessage(sqlite3Db);
    }
    if (stmt != NULL) sqlite3_finalize(stmt);
    if (localDb) sqlite3_close(sqlite3Db);
    if ((returnValue == SQLITE_BUSY) ||
       (returnValue == SQLITE_LOCKED))
      return QS_IS_BUSY;
    return QS_IS_PREPARE_ERROR;
  }

  // binds id
  uint8_t* idBlob = new uint8_t[sizeOfIdBlob_];
  bewrite(idBlob, &faceIndex);
  nodeID.Pack(idBlob+sizeof(FaceIndexType));
  returnValue = sqlite3_bind_blob(stmt, 1, idBlob, sizeOfIdBlob_, SQLITE_TRANSIENT);
  if (returnValue != SQLITE_OK && displayErrorMessage)
  {
    cerr << "TsReadDataBuffer sqlite3_bind_blob Error(" << returnValue << "): " << dbFileName << "\n" << printExtendedErrorMessage(sqlite3Db);
  }
  delete [] idBlob;

  // executes the statement
  returnValue = sqlite3_step(stmt);
  QsErrorType otherReturnValue = QS_IS_UNABLE_TO_READ_DATA_BUFFER;
  if ((returnValue == SQLITE_ROW) || (returnValue == SQLITE_DONE))
  {
    // gets the data
    *currentRasterSize = sqlite3_column_bytes(stmt, tsInsertFileIdData_ - 1);
    if ((*currentRasterSize > 0) && (*currentRasterSize <= static_cast<uint32_t>(gMaxBufferSize)))
    {
      if (*currentRasterSize > (*bufferSize))
      {
        if ((*buffer) != NULL)
          delete [] *buffer;
        *buffer = new uint8_t[*currentRasterSize];
        *bufferSize = (*currentRasterSize);
      }
      memcpy(*buffer, (const uint8_t*)sqlite3_column_blob(stmt, tsInsertFileIdData_-1), *currentRasterSize);
    }
    otherReturnValue = QS_IS_OK;
  }
  else if ((returnValue == SQLITE_BUSY) || (returnValue == SQLITE_LOCKED))
  {
    otherReturnValue = QS_IS_BUSY;
  }
  else
  {
    if (displayErrorMessage)
    {
      cerr << "TsReadDataBuffer sqlite3_step Error(" << returnValue << "): " << dbFileName << "\n";
      cerr << "not done (" << nodeID.FormatAsHex().c_str() << ") " << printExtendedErrorMessage(sqlite3Db);
    }
    otherReturnValue = QS_IS_UNABLE_TO_READ_DATA_BUFFER;
  }

  if (stmt != NULL) sqlite3_finalize(stmt);
  if (localDb)
  {
    returnValue = sqlite3_close(sqlite3Db);
    if (returnValue != SQLITE_OK && displayErrorMessage)
    {
      cerr << "TsReadDataBuffer localDb sqlite3_close Error(" << returnValue << "): " << dbFileName << "\n" << printExtendedErrorMessage(sqlite3Db);
    }
  }
  return otherReturnValue;
}

//-------------------------------------------------------------------------------------
QsErrorType SQLiteDataBaseReadUtil::TsGetSetFromListOfSetsTable(sqlite3* sqlite3Db,
                                                                const std::string& tableName,
                                                                int& rasterFormat,
                                                                int& pixelLength,
                                                                int& shallowLevel,
                                                                int& deepLevel,
                                                                PosXPosYExtents tmpExtents[6],
                                                                std::string& source,
                                                                std::string& classification,
                                                                std::string& description,
                                                                bool& timeSpecified,
                                                                simCore::TimeStamp& timeStamp) const
{
#ifdef DATABASE_UTIL_FUNCTION_ENTRY_DEBUG
  cerr << "DBUTIL FUNCTION TsGetSetFromListOfSetsTable  " << __LINE__ << "\n";
#endif
  if (sqlite3Db == NULL)
    return QS_IS_DB_NOT_INITIALIZED;
  if (tableName.empty())
    return QS_IS_EMPTY_TABLE_NAME;

  int returnValue;
  QsErrorType otherReturnValue;

  // prepares the statement
  sqlite3_stmt* stmt = 0;
  returnValue = sqlite3_prepare_v2(sqlite3Db, textureSetSelectCommand_.c_str(), static_cast<int>(textureSetSelectCommand_.length()), &stmt, NULL);
  if (returnValue != SQLITE_OK)
  {
    cerr << "TsGetSetFromListOfSetsTable sqlite3_prepare_v2 Error(" << returnValue << ")\n" << printExtendedErrorMessage(sqlite3Db);
    if (stmt != NULL) sqlite3_finalize(stmt);
    return QS_IS_PREPARE_ERROR;
  }

  // binds the texture set name
  returnValue = sqlite3_bind_text(stmt, 1, tableName.c_str(), int(tableName.length()), SQLITE_TRANSIENT);
  if (returnValue != SQLITE_OK)
  {
    cerr << "TsGetSetFromListOfSetsTable sqlite3_bind_text Error(" << returnValue << ")\n" << printExtendedErrorMessage(sqlite3Db);
  }

  // executes the statement
  returnValue = sqlite3_step(stmt);
  if (returnValue > SQLITE_OK && returnValue < SQLITE_ROW)
  {
    cerr << "TsGetSetFromListOfSetsTable sqlite3_step Error(" << returnValue << ")\n" << printExtendedErrorMessage(sqlite3Db);
  }

  if (returnValue == SQLITE_ROW)
  {
    // sets some texture set creation options
    rasterFormat = sqlite3_column_int(stmt, tsInsertSetIdRasterFormat_ - 1);
    pixelLength = sqlite3_column_int(stmt, tsInsertSetIdPixelLength_ - 1);
    shallowLevel = sqlite3_column_int(stmt, tsInsertSetIdShallowestLevel_ - 1);
    deepLevel = sqlite3_column_int(stmt, tsInsertSetIdDeepestLevel_ - 1);
    UnPackArray(tmpExtents, (const uint8_t*)sqlite3_column_blob(stmt, tsInsertSetIdExtents_ - 1), 6);
    source = (const char*)sqlite3_column_text(stmt, tsInsertSetIdSource_ - 1);
    classification = (const char*)sqlite3_column_text(stmt, tsInsertSetIdClassification_ - 1);
    description = (const char*)sqlite3_column_text(stmt, tsInsertSetIdDescription_ - 1);
    timeSpecified = (sqlite3_column_int(stmt, tsInsertSetIdTimeSpecified_ - 1) != 0);
#ifndef USE_SIMDIS_SDK
    if (timeSpecified == true)
    {
      UTILS::unPackTimeStamp(&timeStamp, (const uint8_t*)sqlite3_column_blob(stmt, tsInsertSetIdTimeValue_ - 1));
    }
    otherReturnValue = QS_IS_OK;
#else
    if (timeSpecified)
    {
      // TODO support timestamps from db files
      otherReturnValue = QS_IS_OK;
    }
    else
    {
      otherReturnValue = QS_IS_OK;
    }
#endif
  }
  else if (returnValue == SQLITE_BUSY)
  {
    otherReturnValue = QS_IS_BUSY;
  }
  else
  {
    otherReturnValue = QS_IS_TS_NOT_FOUND;
  }

  if (stmt != NULL) sqlite3_finalize(stmt);
  return otherReturnValue;
}

