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

#ifndef QS_COMMON_H
#define QS_COMMON_H

#include <string>
#include <set>
#include <map>

#include "stdgl.h"

#ifndef USE_SIMDIS_SDK
#include <assert.h>
#include "Raster/RasterCommon.h"
#include "iostreamc"
#include "inttypesc.h"
#include "strvecc"
#else // used in SIMDIS SDK
#include <cassert>
#include <iostream>
#include <vector>
#include "simCore/Common/Common.h"
#include "Utils/Raster/RasterCommon.h"
typedef std::vector<std::string> StrVec;
#endif

#include "QSNodeID96.h"
#include "QSCommonIntTypes.h"

#ifdef USE_SIMDIS_SDK
namespace simVis_db
{
#endif
  static const char* SPLITTER_STRING_SRCIMAGE = "srcDataSet";
  static const char* SPLITTER_STRING_OUTPUTDIR = "outputDir";
  static const char* SPLITTER_STRING_OUTPUTDB_TS_NAME = "outputDbTsName";
  static const char* SPLITTER_STRING_OUTPUT_TIME_STAMPED_DIR = "outputTimeStampedDir";
  static const char* SPLITTER_STRING_OUTPUT_TIME_STAMPED_DBDIR = "outputTimeStampedDbDir";
  static const char* SPLITTER_STRING_OUTPUTFORMAT = "outputFormat";
  static const char* SPLITTER_STRING_SHALLOW = "shallowestLevel";
  static const char* SPLITTER_STRING_DEEP = "deepestLevel";
  static const char* SPLITTER_STRING_OPAQUE_IS_255 = "opaqueIs255";
  static const char* SPLITTER_STRING_GDALWARP_EXE = "gdalWarpExectuable";
  static const char* SPLITTER_STRING_SOURCE = "source";
  static const char* SPLITTER_STRING_CLASSIFICATION = "classification";
  static const char* SPLITTER_STRING_DESCRIPTION = "description";
  static const char* SPLITTER_STRING_TIME = "time";
  static const char* SPLITTER_STRING_NODATAVALUE = "noDataValue";
  static const char* SPLITTER_STRING_TREAT_NO_DATA_AS_ALTITUDE = "treatNoDataAsAltitude";
  static const char* SPLITTER_STRING_IGNOREABOVEVALUE = "ignoreAboveValue";
  static const char* SPLITTER_STRING_IGNOREBELOWVALUE = "ignoreBelowValue";
  static const char* SPLITTER_STRING_REPLACEMENTVALUE = "replacementValue";
  static const char* SPLITTER_STRING_DESIRED_ANGULAR_SPACING = "desiredAngularSpacing";
  static const char* SPLITTER_STRING_ANGULAR_SPACING_OF_BLOCKS = "angularSpacingOfBlocks";
  static const char* SPLITTER_STRING_OVERWRITE = "overwrite";
  static const char* SPLITTER_STRING_OVERWRITE_IF_SETTINGS_DIFFER = "overwriteIfSettingsDiffer";

  static const char* RASTER_URL  = "rasterURL";
  static const char* WORLDFILE_URL  = "worldFileURL";
  static const char* RASTER_EXTENTS  = "rasterExtents";
  static const char* NAV_KEYWORD  = "nav";

  static const char* SD_SPLIT_TIME_STAMPED_KEYWORD = "TimeStamped";
  static const char* SD_SPLIT_OUTPUT_TYPE_KEYWORD = "SplitRasterNodeFormat";
  static const char* SD_SPLIT_TEXTURE_SET_KEYWORD = "textureSet";
  static const char* SD_SPLIT_PIXEL_LENGTH = "pixelLength";
  static const char* SD_SPLIT_SHALLOWEST_KEYWORD = "shallowestLevel";
  static const char* SD_SPLIT_DEEPEST_KEYWORD = "deepestLevel";
  static const char* SD_SPLIT_EXTENTS_KEYWORD = "extents";
  static const char* SD_SPLIT_SOURCE_KEYWORD = "source";
  static const char* SD_SPLIT_CLASSIFICATION_KEYWORD = "classification";
  static const char* SD_SPLIT_DESCRIPTION_KEYWORD = "description";
  static const char* SD_SPLIT_ALPHA_PROCESSING_KEYWORD = "alphaProcessing";
  static const char* SD_SPLIT_USE_CLOUD_THRESHOLDS_KEYWORD = "cloudHiLow";
  static const char* SD_SPLIT_CLOUD_THRESHOLD_OPAQUE_KEYWORD = "cloudThresholdOpaque";
  static const char* SD_SPLIT_CLOUD_THRESHOLD_CLEAR_KEYWORD = "cloudThresholdClear";

  static const char* SD_DOT_NAV_GOES_GETTER_EXE = "sdDotNavGoesGetter";
  static const char* SD_GEOGRAPHIC_RASTER_GETTER_EXE = "sdGeographicRasterGetter";
  static const char* SD_EQUI_IMAGERY_SPLITTER_EXE = "sdEquiRectangularImagerySplitter";
  static const char* SD_GEOREFERENCER_EXE = "sdGeoReferencer";
  static const char* SD_IR_MASKER_EXE = "sdIrMasker";
  static const char* NORMALIZER_NAME_EXE = "sdNormalizer";
  static const char* SD_GEO3BANDCOMBINER_EXE = "sdGeo3BandCombiner";
  static const char* NODATAREPLACER_EXE = "sdNoDataAlphaReplacer";
  static const char* SD_SETTRANSLATOR_EXE = "sdSetTranslator";
  static const char* SD_PIXELGETTERDOTNAV_EXE = "sdPixelGetterDotNav";
  static const char* SD_PIXELFILLER_EXE = "sdPixelFiller";
  static const char* SD_COLORREPLACER_EXE = "sdColorReplacer";
  static const char* SD_GDALINFO_EXE = "sdgdalInfo";

  static const char* SD_TEXTURE_SET_INFO = "/textureSetInfo.txt";
  static const char* SD_ALT_SET_INFO = "/elevSetInfo.txt";

  typedef int IRProcessingType;
  static const IRProcessingType IR_UNKNOWN = 0;
  static const IRProcessingType IR_NDVI_LAND_GEQUAL = 1;
  static const IRProcessingType IR_NDWI_LAND_LESS   = 2;
  static const char* IR_KEYWORD_UNKNOWN = "unknown";
  static const char* IR_KEYWORD_NDVI_LAND_GEQUAL = "ndviGEqual";
  static const char* IR_KEYWORD_NDWI_LAND_LESS   = "ndwiLess";
  static const char* IR_KEYWORD_NDVI_LAND_GEQUAL_VALUE = "ndviGEqualValue";
  static const char* IR_KEYWORD_NDWI_LAND_LESS_VALUE   = "ndwiLessValue";
  static const double IR_DEFAULT_NDVI_LAND_GEQUAL_VALUE = -0.4;
  static const double IR_DEFAULT_NDWI_LAND_LESS_VALUE   = 0.35;
  static const char* PROCESSING_KEYWORD = "irProcessing";
  static const char* LAND_KEYWORD = "land";
  const char* GetIrProcessingTypeString(const IRProcessingType& type);
  IRProcessingType GetIrProcessingType(const std::string& givenString);

  static const char* NODATA_SRC = "src";
  static const char* NODATA_DST = "dst";
  static const char* NODATA_COLORS = "colors";

  static const char* COLORREPLACER_SRC = "src";
  static const char* COLORREPLACER_DST = "dst";
  static const char* COLORREPLACER_OLD_COLORS = "old";
  static const char* COLORREPLACER_NEW_COLOR = "new";
  static const char* COLORREPLACER_AVG_ABOVE = "avgAbove";
  static const char* COLORREPLACER_AVG_BELOW = "avgBelow";

  static int NORM_DEFAULT_BUFFER = 5;
  static double NORM_DEFAULT_SCALAR = 0.1;
  static const char* NORM_KEYWORD_SRC = "src";
  static const char* NORM_KEYWORD_DST = "dst";
  static const char* NORM_KEYWORD_BUFFER = "buffer";
  static const char* NORM_KEYWORD_SCALAR = "avgScalar";

  static const char* GEO3_KEYWORD_RUN_NODATA = "runNoDataAlphaReplacer";
  static const bool GEO3_DEFAULT_RUN_NODATA = true;

  static const char* IR_MASKER_IR_KEYWORD = "ir";
  static const char* IR_MASKER_SRC_KEYWORD = "src";
  static const char* IR_MASKER_DST_KEYWORD = "dst";
  static const char* IR_MASKER_RI_KEYWORD = "r";;
  static const char* IR_MASKER_GI_KEYWORD = "g";;
  static const char* IR_MASKER_BI_KEYWORD = "b";
  static const char* IR_MASKER_RO_KEYWORD = "ro";;
  static const char* IR_MASKER_GO_KEYWORD = "go";;
  static const char* IR_MASKER_BO_KEYWORD = "bo";

  static const char* QS_TS_CO_NONE = "none";

  static const char* SIMQS_CONFIG_SPHERE_KEYWORD = "sphere";

  static const int SD_TILE_SIZE = 256;
  static const double SD_TILE_SIZED = 256.0;

  static GLubyte SD_NO_DATA_VALUE = 0xff;

  static const char* QsFaceIndexStringWW = "ww";
  static const char* QsFaceIndexStringW  = "w";
  static const char* QsFaceIndexStringE  = "e";
  static const char* QsFaceIndexStringEE = "ee";
  static const char* QsFaceIndexStringN  = "n";
  static const char* QsFaceIndexStringS  = "s";

  typedef uint8_t QsViewportIDType;

  //===========================================================================

  /** Function for mapping a face index to a face string */
  const char* GetFaceString(const FaceIndexType&);

  /** Function for mapping a face string to a face index */
  bool GetFaceIndex(const std::string& faceString, FaceIndexType* faceIndex);

  //=====================================================================================
  void GetDataFileName(std::string* newFileName,
		       const std::string& baseDirectory, const QSNodeId& afterBase,
		       const std::string& extension);
  void GetDataFileName(std::string* newFileName,
		       const std::string& baseDirectory, const QSNodeId& afterBase,
		       const RasterFormat& imageType);

  //=====================================================================================
  std::string getColorReplaceCommand(const std::string& srcFileName,
				     const std::string& dstFileName,
				     const std::string& oldColors,
				     const std::string& newColor,
				     const bool& avgAboveSpecified=false, const double& avgAbove=0,
				     const bool& avgBelowSpecified=false, const double& avgBelow=0);

  //===========================================================================
  void appendChildString(QSNodeId* givenString, const LevelInt& childLevel, const ChildIndexInt& childIndex);
  void removeChildString(QSNodeId* givenString, const LevelInt& childLevel);
  bool popChildString(QSNodeId* givenString, ChildIndexInt* poppedChildIndex);
  LevelInt getLevelFromID(const QSNodeId& id);
  std::string childIndexString(const ChildIndexInt&);
  QSNodeId getSubPath(const QSNodeId& givenString, const LevelInt& level);

  //===========================================================================
  double LimitValue(const double& minVal, const double& maxVal, const double& otherVal);

  //===========================================================================
  template <class SomeType> void checkNULL(SomeType* pointer)
  {
    if (pointer == NULL)
    {
      std::cerr << "ERROR:  Unexpected NULL.\n";
      assert(0);
      exit(0);
    }
  }

  //===========================================================================
  bool checkSize(const StrVec& tokens, const uint32_t& numberOfTokens, const uint32_t& lineNumber, const std::string& fileName);

  //===========================================================================
  std::string getTempName(const std::string& extension);

  //===========================================================================
  void removeTempFiles(const std::string& tmpFileName);
  void removeTempFiles(const StrVec& listOfFilesToRemove);
  void removeBinAndHdr(const std::string& binFileName, const bool& createdByGetTemp=false);
  void removeBinAndHdr(const StrVec& listOfFilesToRemove, const bool& createdByGetTemp=false);
  void removeFiles(const StrVec& listOfFilesToRemove);


  //===========================================================================
  static const LevelInt SD_QUAD_DEEPESTLEVEL = 29;

  //===========================================================================
  // a class for storing an x/y position
  template <class PosType> class xyType
  {
    public:
      PosType _PosX;
      PosType _PosY;

      xyType(const PosType& x=0, const PosType& y=0)
        : _PosX(x),
	  _PosY(y)
      {}
  };
  template <class SomeXYType> bool equalFunction(const SomeXYType& a, const SomeXYType& b)
  {
    return ((a._PosX == b._PosX) && (a._PosY == b._PosY));
  }
  template <class SomeXYType> bool lessFunction(const SomeXYType& a, const SomeXYType& b)
  {
    return (a._PosX < b._PosX)||((a._PosX == b._PosX)&&(a._PosY < b._PosY)) ? true : false;
  }

  typedef xyType<uint32_t> xyType32;
  bool operator==(const xyType32& a, const xyType32& b);
  bool operator<(const xyType32& a, const xyType32& b);


  //===========================================================================
  bool runNoDataAlphaReplacer(const std::string& srcFileName,
			      const std::string& dstFileName,
			      const StrVec* colors);

  //=====================================================================================
  template <class SomeClass>
  void PackArray(const SomeClass* givenArray, uint8_t* givenBuffer, const uint32_t& numElements)
  {
    if ((givenArray == NULL) || (givenBuffer == NULL))
      return;

    size_t i;
    uint8_t tmpBuffer[sizeof(SomeClass)];
    for (i = 0; i < numElements; ++i)
    {
      givenArray[i].Pack(tmpBuffer);
      memcpy(givenBuffer+(sizeof(SomeClass)*i), tmpBuffer, sizeof(SomeClass));
    }
  }

  //=====================================================================================
  template <class SomeClass>
  void UnPackArray(SomeClass* givenArray, const uint8_t* givenBuffer, const uint32_t& numElements)
  {
    if ((givenArray == NULL) || (givenBuffer == NULL))
      return;

    size_t i;
    uint8_t tmpBuffer[sizeof(SomeClass)];
    for (i = 0; i < numElements; ++i)
    {
      memcpy(tmpBuffer, givenBuffer+(sizeof(SomeClass)*i), sizeof(SomeClass));
      givenArray[i].UnPack(tmpBuffer);
    }
  }

  //=====================================================================================
  typedef std::set<std::string> sString;

  //=====================================================================================
  typedef uint16_t QsProcessStatusType;
  static const QsProcessStatusType TS_STATUS_INACTIVE = 1;
  static const QsProcessStatusType TS_STATUS_WAITING = 2;
  static const QsProcessStatusType TS_STATUS_RUNNING = 3;
  static const QsProcessStatusType TS_STATUS_ERROR = 4;
  static const QsProcessStatusType TS_STATUS_COMPLETED = 5;
  const std::string StatusString(const QsProcessStatusType&);

  //=====================================================================================
  bool GetFacingMode(const std::string& token, GLenum& facingMode);

  //=====================================================================================
  std::string GetFacingModeString(const GLenum& token);

  //=====================================================================================
  GLenum GetAlphaFunc(const std::string& token);

  //=====================================================================================
  typedef uint8_t QsOpenGLCloudRenderingType;
  static QsOpenGLCloudRenderingType QS_GL_CLOUD_NONE             = 1;
  static QsOpenGLCloudRenderingType QS_GL_CLOUD_PALETTED         = 2;
  static QsOpenGLCloudRenderingType QS_GL_CLOUD_PALETTED_SHARED  = 3;
  static QsOpenGLCloudRenderingType QS_GL_CLOUD_FRAGMENT_PROGRAM = 4;

  //=====================================================================================
  typedef int32_t LayerId_t;
  typedef LayerId_t LayerIndex_t;
  typedef std::vector<LayerId_t> vLayerId;
  typedef std::map<LayerIndex_t, LayerIndex_t> mLayerIndex;

#ifdef USE_SIMDIS_SDK
} // namespace simVis_db
#endif

#endif /* QS_COMMON_H */
