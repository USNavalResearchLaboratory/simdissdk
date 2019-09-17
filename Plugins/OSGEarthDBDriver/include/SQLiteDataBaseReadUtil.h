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

#ifndef SQLITE_DATABASE_READ_UTIL_H
#define SQLITE_DATABASE_READ_UTIL_H

#include <string>
#include "sqlite3.h"
#include "simCore/Time/TimeClass.h"

#ifndef USE_SIMDIS_SDK
#include "Raster/RasterCommon.h"
#else
#include "Utils/Raster/RasterCommon.h"
#endif

#include "QSCommonIntTypes.h"
#include "QSError.h"
#include "QSNodeID96.h"
#include "QSPosXYExtents.h"

// Temporary defines until we update sqlite
#ifndef SQLITE_OPEN_READONLY
#define SQLITE_OPEN_READONLY         0x00000001
#define SQLITE_OPEN_READWRITE        0x00000002
#define SQLITE_OPEN_CREATE           0x00000004
#define SQLITE_OPEN_DELETEONCLOSE    0x00000008
#define SQLITE_OPEN_EXCLUSIVE        0x00000010
#define SQLITE_OPEN_MAIN_DB          0x00000100
#define SQLITE_OPEN_TEMP_DB          0x00000200
#define SQLITE_OPEN_TRANSIENT_DB     0x00000400
#define SQLITE_OPEN_MAIN_JOURNAL     0x00000800
#define SQLITE_OPEN_TEMP_JOURNAL     0x00001000
#define SQLITE_OPEN_SUBJOURNAL       0x00002000
#define SQLITE_OPEN_MASTER_JOURNAL   0x00004000
#define SQLITE_OPEN_NOMUTEX          0x00008000
#define SQLITE_OPEN_FULLMUTEX        0x00010000
#endif /* SQLITE_OPEN_READONLY */

#ifdef USE_SIMDIS_SDK
namespace simVis_db
{
#endif
  static const char* QS_TO_ID = "id";

  //=====================================================================================
  static const char* QS_DEFAULT_SET_TABLE_NAME = "default";
  static const char* SPLITTER_STRING_OUTPUTDB = "dbFile";
  static const char* SIMQS_CONFIG_TABLENAME_KEYWORD = "tableName";
  static const char* QS_LIST_OF_TEXTURE_SETS_TABLE_NAME = "ListOfTextureSets";
  static const char* QS_TSO_NAME_OF_TEXTURE_SET_TABLE = "nt";
  static const char* QS_TSO_OUTPUT_TYPE = "ot";
  static const char* QS_TSO_PIXEL_LENGTH = "pl";
  static const char* QS_TSO_SHALLOWEST_LEVEL = "sl";
  static const char* QS_TSO_DEEPEST_LEVEL = "dl";
  static const char* QS_TSO_EXTENTS = "ex";
  static const char* QS_TSO_SOURCE = "s";
  static const char* QS_TSO_CLASSIFICATION = "c";
  static const char* QS_TSO_DESCRIPTION = "ds";
  static const char* QS_TSO_TIME_SPECIFIED = "ts";

  //=====================================================================================
  typedef std::vector<sqlite3*> vSqlite3;
  void CloseSqliteDBs(vSqlite3*);

  //=====================================================================================
  class SQLiteDataBaseReadUtil
  {
  public:
    SQLiteDataBaseReadUtil();
    virtual ~SQLiteDataBaseReadUtil();

    /** Opens a database file */
    QsErrorType OpenDataBaseFile(const std::string& dbFileName,
                                  sqlite3** sqlite3Db,
                                  const int& flags) const;

    /**
     * Gets TextureSet information about a data table
     * @param[in] sqlite3Db Pointer to a SQLite database object
     * @param[in] tableName Name of the table to access within the given database
     * The following are TextureSet creation options
     * @param[out] rasterFormat Flag that determines how the texture image is drawn
     * @param[out] pixelLength Tile size of the TextureSet
     * @param[out] shallowLevel Minimum depth of the TextureSet
     * @param[out] deepLevel Maximum depth of the TextureSet
     * @param[out] tmpExtents Stores the TextureSet's X/Y extent values
     * @param[out] source Name of the TextureSet's source file
     * @param[out] classification Classification information of the loaded TextureSet
     * @param[out] description Description of the loaded TextureSet
     * @param[out] timeSpecified Whether or not a valid timeStamp was specified for the source file
     * @param[out] timeStamp Loads a time value, if there is a valid timeStamp on the file
     * @return Returns 0 on success, otherwise returns an error value mapped to QsErrorType.
     */
    QsErrorType TsGetSetFromListOfSetsTable(sqlite3* sqlite3Db,
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
                                            simCore::TimeStamp& timeStamp) const;

    /**
     * Reads a node's data buffer from a sets table; caller is responsible for deleting buffer
     * @param[in] sqlite3Db Pointer to a SQLite database object
     * @param[in] dbFileName Name of a SQLite database file, used to fetch a database if sqlite3Db == NULL
     * @param[in] dataTableName Name of the table to access within the given database
     * @param[in] faceIndex Mapping to a face index/orientation, used to create a SQLite idBlob
     * @param[in] nodeID Used to fill the idBlob
     * @param[out] buffer Destination for data from the SQLite database
     * @param[in, out] bufferSize Current max size of the buffer, will be changed if data to be copied is greater than max
     * @param[out] currentRasterSize Size (bytes) of the data from the SQLite database
     * @param[in] allowLocalDB Determines whether to fall back to a local database pointed to by dbFileName
     * @param[in] displayErrorMessage Determines whether to display error messages to console when failing
     * @return An error value, mapped to QsErrorType
     */
    QsErrorType TsReadDataBuffer(sqlite3* sqlite3Db,
                                  const std::string& dbFileName,
                                  const std::string& dataTableName,
                                  const FaceIndexType& faceIndex,
                                  const QSNodeId& nodeID,
                                  TextureDataType** buffer,
                                  uint32_t* bufferSize,
                                  uint32_t* currentRasterSize,
                                  bool allowLocalDB,
                                  bool displayErrorMessage=false) const;
  protected:
    int sizeOfIdBlob_;

    std::string textureSetSelectCommand_;
    std::string textureSetSelectFileCommand1_;
    std::string textureSetSelectFileCommand2_;

    // ids for inserting a "texture set" into a "list of texture sets" table
    int tsInsertFileIdData_;
    int tsInsertSetTextureSetName_;
    int tsInsertSetIdRasterFormat_;
    int tsInsertSetIdPixelLength_;
    int tsInsertSetIdShallowestLevel_;
    int tsInsertSetIdDeepestLevel_;
    int tsInsertSetIdExtents_;
    int tsInsertSetIdSource_;
    int tsInsertSetIdClassification_;
    int tsInsertSetIdDescription_;
    int tsInsertSetIdTimeSpecified_;
    int tsInsertSetIdTimeValue_;
  };

#ifdef USE_SIMDIS_SDK
} // namespace simVis_db
#endif

#endif /* SQLITE_DATABASE_READ_UTIL_H */
