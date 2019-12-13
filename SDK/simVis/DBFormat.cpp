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
#include "osgDB/FileUtils"
#include "simVis/DBFormat.h"

using namespace simVis;

//...........................................................
#if 0
#include "simCore/Common/Common.h"
#include "simCore/Calc/MathConstants.h"

namespace simVis { namespace DB
{
  typedef int16_t LevelInt;
  static const LevelInt QT_MIN_LEVEL = 0;
  static const LevelInt QT_MAX_LEVEL = 32;

  typedef uint8_t ChildIndexInt;
  static const ChildIndexInt QT_CHILD_NE = 0;
  static const ChildIndexInt QT_CHILD_NW = 1;
  static const ChildIndexInt QT_CHILD_SW = 2;
  static const ChildIndexInt QT_CHILD_SE = 3;

  typedef uint8_t FaceIndexType;
  static const FaceIndexType QsFaceIndexWW = 0;
  static const FaceIndexType QsFaceIndexW = 1;
  static const FaceIndexType QsFaceIndexE = 2;
  static const FaceIndexType QsFaceIndexEE = 3;
  static const FaceIndexType QsFaceIndexN = 4;
  static const FaceIndexType QsFaceIndexS = 5;

  typedef float AltitudeDataType;

  static const int16_t MAX_NUM_READ_THREADS = 128;

  typedef uint64_t QsPosType;

#if defined Linux || defined Solaris
  static const QsPosType gQsMaxLength = 4294967296LL;
  static const QsPosType gQsHalfMaxLength = 2147483648LL;
#else
  static const QsPosType gQsMaxLength = 4294967296;
  static const QsPosType gQsHalfMaxLength = 2147483648;
#endif
  static const double gQsDMaxLength = 4294967296.0;
  static const double gQsDHalfMaxLength = 2147483648.0;
  static const double gQsLatLonDelta = M_PI_2 / gQsDMaxLength;

  /** A bounding rectangle of x/y extents */
  struct PosXPosYExtents
  {
    QsPosType minX;
    QsPosType maxX;
    QsPosType minY;
    QsPosType maxY;

    PosXPosYExtents(QsPosType minX = gQsMaxLength, QsPosType maxX = 0, QsPosType minY = gQsMaxLength, QsPosType maxY = 0);

    /** Sets up invalid extents */
    void Initialize();

    /** Confirms validity of extents */
    bool Valid() const;

    /** Sets the extents */
    void SetAll(const PosXPosYExtents& given);
    void SetAll(const QsPosType& minX, const QsPosType& maxX, const QsPosType& minY, const QsPosType& maxY);

    /** Packs/unpacks the extents into or from a buffer */
    void Pack(uint8_t*) const;
    void UnPack(const uint8_t*);
    void UnPackHexChars(const char*);

    /** Prints the extents to the console */
    void Print();
  };

  // --- SQLiteDataBaseReadUtil

  static const char* QS_TO_ID = "id";
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
      bool displayErrorMessage = false) const;
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
} }
#endif

#if 0
//...........................................................

void DB::Options::readFrom(const osgEarth::Config& conf)
{
}

void DB::Options::writeTo(osgEarth::Config& conf) const
{
}
//...........................................................

osgEarth::Status DB::Driver::open(
      const osgEarth::URI& uri,
      osg::ref_ptr<const osgEarth::Profile>& profile,
      osgEarth::DataExtentList& dataExtents,
      const osgDB::Options* readOptions)
{
  pathname_ = osgDB::findDataFile(uri, readOptions);

  if (DB::OpenDataBaseFile(pathname_, &db_, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX) != QS_IS_OK)
  {
    db_ = NULL;
    return Status::Error(Stringify() << "Failed to open DB file at " << options_.url()->full());
  }
  else
  {
    QsErrorType err = DBUtil::TsGetSetFromListOfSetsTable(
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
      return Status::Error(Stringify() << "Failed to read metadata for " << pathname_);
    }

    // Set up as a unified cube:
    profile = new osgEarth::UnifiedCubeProfile();

    // Lat/long extents (for debugging)
    GeoExtent llex[6];

    // Tell the engine how deep the data actually goes:
    for (unsigned int f = 0; f < 6; ++f)
    {
      if (extents_[f].minX < extents_[f].maxX && extents_[f].minY < extents_[f].maxY)
      {
        const double x0 = extents_[f].minX / gQsDMaxLength;
        const double x1 = extents_[f].maxX / gQsDMaxLength;
        const double y0 = extents_[f].minY / gQsDMaxLength;
        const double y1 = extents_[f].maxY / gQsDMaxLength;

        GeoExtent cubeEx(profile->getSRS(), f + x0, y0, f + x1, y1);

        // Transform to lat/long for the debugging msgs
        cubeEx.transform(profile->getSRS()->getGeodeticSRS(), llex[f]);

        dataExtents.push_back(DataExtent(cubeEx, shallowLevel_, deepLevel_));
      }
    }

    // Set time value of image if a time was found in the db
    if (timeStamp_ != simCore::INFINITE_TIME_STAMP)
    {
      DateTime osgTime(timeStamp_.secondsSinceRefYear(1970));
      // Set time as a user value since config is not editable from here
      setUserValue("time", osgTime.asISO8601());
    }

    OE_INFO << LC
      << "Table: " << uri.full() << std::endl
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
  return STATUS_OK;
}

osgEarth::ReadResult DB::Driver::read(
      const osgEarth::TileKey& key,
      osgEarth::ProgressCallback* progress,
      const osgDB::Options* readOptions) const
{
  //todo
}
#endif

//...........................................................

osgEarth::Config DBImageLayer::Options::getConfig() const
{
  osgEarth::Config conf = osgEarth::ImageLayer::Options::getConfig();
  conf.merge(driver()->getConfig());
  return conf;
}

void DBImageLayer::Options::fromConfig(const osgEarth::Config& conf)
{
  driver() = simVis::DBOptions(conf);
}

void DBImageLayer::setURL(const osgEarth::URI& value)
{
  options().driver()->url() = value;
}

const osgEarth::URI& DBImageLayer::getURL() const
{
  return options().driver()->url().get();
}

void DBImageLayer::setDeepestLevel(const unsigned int& value)
{
  options().driver()->deepestLevel() = value;
}

const unsigned int& DBImageLayer::getDeepestLevel() const
{
  return options().driver()->deepestLevel().get();
}

osgEarth::TileSource* DBImageLayer::createTileSource()
{
  return osgEarth::TileSourceFactory::create(options().driver().get());
}

void DBImageLayer::init()
{
  osgEarth::ImageLayer::init();
  setTileSourceExpected(true);
}

osgEarth::Status DBImageLayer::openImplementation()
{
  return osgEarth::ImageLayer::openImplementation();
}

osgEarth::GeoImage DBImageLayer::createImageImplementation(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress) const
{
  //todo
  return osgEarth::ImageLayer::createImageImplementation(key, progress);
}

//...........................................................

osgEarth::Config DBElevationLayer::Options::getConfig() const
{
  osgEarth::Config conf = osgEarth::ElevationLayer::Options::getConfig();
  conf.merge(driver()->getConfig());
  return conf;
}

void DBElevationLayer::Options::fromConfig(const osgEarth::Config& conf)
{
  driver() = simVis::DBOptions(conf);
}

void DBElevationLayer::setURL(const osgEarth::URI& value)
{
  options().driver()->url() = value;
}

const osgEarth::URI& DBElevationLayer::getURL() const
{
  return options().driver()->url().get();
}

void DBElevationLayer::setDeepestLevel(const unsigned int& value)
{
  options().driver()->deepestLevel() = value;
}

const unsigned int& DBElevationLayer::getDeepestLevel() const
{
  return options().driver()->deepestLevel().get();
}

osgEarth::TileSource* DBElevationLayer::createTileSource()
{
  return osgEarth::TileSourceFactory::create(options().driver().get());
}

void DBElevationLayer::init()
{
  osgEarth::ElevationLayer::init();
  setTileSourceExpected(true);
}

osgEarth::Status DBElevationLayer::openImplementation()
{
  return osgEarth::ElevationLayer::openImplementation();
}

osgEarth::GeoHeightField DBElevationLayer::createHeightFieldImplementation(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress) const
{
  //todo
  return osgEarth::ElevationLayer::createHeightFieldImplementation(key, progress);
}
