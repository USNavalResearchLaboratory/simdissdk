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

#ifndef SIMVIS_DB_SQLITEDATABASEREADUTIL_H
#define SIMVIS_DB_SQLITEDATABASEREADUTIL_H

#include <string>
#include "sqlite3.h"
#include "QSCommon.h"
#include "QSError.h"
#include "QSNodeID96.h"
#include "QSPosXYExtents.h"

namespace simCore { class TimeStamp; }

namespace simVis_db
{
  static const char* QS_TO_ID = "id";

  //=====================================================================================
  static const char* QS_DEFAULT_SET_TABLE_NAME = "default";
  static const char* QS_LIST_OF_TEXTURE_SETS_TABLE_NAME = "ListOfTextureSets";
  static const char* QS_TSO_NAME_OF_TEXTURE_SET_TABLE = "nt";

  //=====================================================================================
  class SQLiteDataBaseReadUtil
  {
  public:
    SQLiteDataBaseReadUtil();
    virtual ~SQLiteDataBaseReadUtil();

    /** Opens a database file */
    QsErrorType openDatabaseFile(const std::string& dbFileName,
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
    QsErrorType getSetFromListOfSetsTable(sqlite3* sqlite3Db,
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
    QsErrorType readDataBuffer(sqlite3* sqlite3Db,
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

} // namespace simVis_db

#endif /* SIMVIS_DB_SQLITEDATABASEREADUTIL_H */
