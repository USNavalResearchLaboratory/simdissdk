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
#ifndef SIMCORE_STRING_FILE_PATTERNS_H
#define SIMCORE_STRING_FILE_PATTERNS_H

#include <string>

// Standard File Extensions for SIMDIS
namespace simCore
{

/** All SIMDIS file patterns, separated by commas */
static const std::string ALL_SIMDIS_FILE_PATTERNS = "*.fct,*.asi,*.spy,*.discn,*.csv,*.tle,*.kp,*.gpx,*.otg,*.wam,*.pet,*.zip";
/** Dialog-oriented user-friendly list of SIMDIS file patterns */
static const std::string SIMDIS_FILE_PATTERNS =
  "All Supported SIMDIS Formats (*.fct,*.asi,*.spy,*.discn,*.csv,*.tle,*.kp,*.gpx,*.otg,*.wam,*.pet,*.zip)\n"
  "SIMDIS Binary (*.fct)\n"
  "SIMDIS ASCII (*.asi)\n"
  "SIMDIS Script (*.spy)\n"
  "SIMDIS Data Initialization Scenario (*.discn)\n"
  "SCORE & PMRF Comma Separated Value (*.csv)\n"
  "NORAD Two-Line Element (*.tle)\n"
  "Kepler Propagation (*.kp)\n"
  "GPS Exchange Format (*.gpx)\n"
  "OTG Format (*.otg)\n"
  "WAM Format (*.wam)\n"
  "PET Format (*.pet)\n"
  "ZIP Archive of supported formats (*.zip)\n"
  "All Files (*)";

/** SIMDIS ASCII file patterns, separated by commas */
static const std::string ALL_SIMDIS_ASCII_FILE_PATTERNS = "*.asi,*.discn,*.csv,*.tle,*.gpx,*.otg,*.wam,*.pet,*.zip";
/** Dialog-oriented user-friendly list of SIMDIS ASCII file patterns */
static const std::string SIMDIS_ASCII_FILE_PATTERNS =
  "All Supported ASCII Formats (*.asi,*.discn,*.csv,*.tle,*.gpx,*.otg,*.wam,*.pet,*.zip)\n"
  "SIMDIS ASCII (*.asi)\n"
  "SIMDIS Data Initialization Scenario (*.discn)\n"
  "SCORE & PMRF Comma Separated Value (*.csv)\n"
  "NORAD Two-Line Element (*.tle)\n"
  "GPS Exchange Format (*.gpx)\n"
  "OTG Format (*.otg)\n"
  "WAM Format (*.wam)\n"
  "PET Format (*.pet)\n"
  "ZIP Archive of supported formats (*.zip)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS ASI file patterns */
static const std::string SIMDIS_ASI_FILE_PATTERNS =
  "All Supported ASI Formats (*.asi,*.discn)\n"
  "SIMDIS ASCII File (*.asi)\n"
  "SIMDIS Data Initialization Scenario (*.discn)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS FCT file patterns */
static const std::string SIMDIS_FCT_FILE_PATTERNS =
  "SIMDIS Binary File (*.fct)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS ZIP archive file patterns */
static const std::string SIMDIS_ZIP_FILE_PATTERNS =
  "ZIP Archive of supported formats (*.zip)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS 9 Preferences File patterns */
static const std::string SIMDIS_PREFS_FILE_PATTERNS =
  "SIMDIS Prefs File (*.prefs)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS Preference Rule File patterns */
static const std::string SIMDIS_RULE_FILE_PATTERNS =
  "SIMDIS Pref Rule Files (*.rul)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS 9 View File patterns */
static const std::string SIMDIS_VIEW_FILE_PATTERNS =
  "SIMDIS View Files (*.view,*.svml)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS 10 only View File patterns */
static const std::string SIMDIS10_VIEW_FILE_PATTERNS =
  "SIMDIS View Files (*.svml)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of supported SIMDIS Bookmark format patterns to open*/
static const std::string SIMDIS_BOOKMARK_FILE_PATTERNS =
  "All Supported Bookmark Formats (*.bml, *.toc, *.log, *.htm, *.html, *.csv)\n"
  "Bookmark XML (*.bml)\n"
  "Media Player Bookmarks (*.toc)\n"
  "SCORE Event Log (*.log)\n"
  "NetMeeting Chat Log (*.htm,*.html)\n"
  "Chat Surfer Log (*.csv)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of supported SIMDIS Bookmark format patterns to save/export */
static const std::string SIMDIS_BOOKMARK_SAVE_FILE_PATTERNS =
  "Bookmark XML (*.bml)\n"
  "Media Player Bookmarks (*.toc)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS 9 Terrain Configuration File patterns */
static const std::string SIMDIS_IMAGERY_CONFIG_FILE_PATTERNS =
  "SIMDIS Imagery and Terrain Config File (*.txt)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS 9 Terrain File patterns */
static const std::string SIMDIS_RASTER_DB_FILE_PATTERNS =
  "SIMDIS Raster Set File (*.db)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of Terrain File Patterns which can be loaded by SIMDIS 10 */
static const std::string SIMDIS10_IMAGERY_CONFIG_LOAD_PATTERNS =
    "Terrain Configuration Files (*.earth,*.txt)\n"
    "osgEarth Earth File (*.earth)\n"
    "SIMDIS 9 Terrain Configuration File (*.txt)\n"
    "All Files (*)";

/** Dialog-oriented user-friendly list of Terrain File Patterns which can be written by SIMDIS 10 */
static const std::string SIMDIS10_IMAGERY_CONFIG_SAVE_PATTERNS =
    "osgEarth Earth File (*.earth)\n"
    "All Files (*)";

/** SIMDIS supported model format file patterns, separated by commas. */
static const std::string ALL_SIMDIS_MODEL_FILE_PATTERNS = "*.opt,*.3db,*.flt,*.osg,*.osga,*.osgb,*.ive,*.obj,*.3ds,*.lwo,*.dxf,*.ac,*.png,*.gif,*.jpg,*.jpeg,*.rgb,*.tif,*.tiff,*.bmp,*.tmd,*.lst";
/** SIMDIS supported texture file format file patterns, separated by commas */
static const std::string ALL_SIMDIS_TEXTURE_FILE_PATTERNS = "*.png,*.gif,*.jpg,*.jpeg,*.rgba,*.rgb,*.tiff,*.tif,*.bmp,*.bw,*.inta,*.int,*.raw,*.pgm";
/** Dialog-oriented user-friendly list of SIMDIS supported model format patterns. */
static const std::string SIMDIS_MODEL_FILE_PATTERNS =
  "All Supported Formats (*.opt,*.3db,*.flt,*.osg,*.osga,*.osgb,*.ive,*.obj,*.3ds,*.lwo,*.dxf,*.ac,*.png,*.gif,*.jpg,*.jpeg,*.rgb,*.tif,*.tiff,*.bmp,*.tmd,*.lst)\n"
  "SIMDIS Model Files (*.opt,*.3db,*.flt,*.osg,*.osga,*.osgb,*.ive,*.obj,*.3ds,*.lwo,*.dxf,*.ac)\n"
  "Image Files (*.png,*.gif,*.jpg,*.jpeg,*.rgb,*.tif,*.tiff,*.bmp)\n"
  "Media Player Video Icons (*.tmd,*.lst)\n"
  "All Files (*)";
/** Dialog-oriented user-friendly list of SIMDIS supported image format patterns. */
static const std::string SIMDIS_IMAGE_FILE_PATTERNS =
  "Image Files (*.png,*.gif,*.jpg,*.jpeg,*.rgb,*.tif,*.tiff,*.bmp)\n"
  "All Files (*)";

/** Media Player file format patterns, separated by commas */
static const std::string ALL_MEDIA_FILE_PATTERNS = "*.avi,*.wmv,*.mpeg,*.mpg,*.m1v,*.mp2,*.mpa,*.mpe,*.mov,*.asf,*.aiff,*.aifc,*.audio,*.au,*.m4a,*.m4v,*.mp4,*.mp3,*.wav,*.wma,*.tmd,*.lst,*.pst,*.toc";
/** Dialog-oriented user-friendly list of SIMDIS Media Player file format patterns */
static const std::string MEDIA_FILE_PATTERNS =
  "All Supported Media Formats (" + ALL_MEDIA_FILE_PATTERNS + ")\n"
  "Media Player Files (*.tmd,*.lst,*.pst,*.toc)\n"
  "Media Playlist (*.lst,*.pst)\n"
  "Time Sync Media Player (*.tmd)\n"
  "Playlist (*.lst)\n"
  "List of Playlists (*.pst)\n"
  "Bookmarks (*.toc)\n"
  "Audio File (*.aiff,*.aifc,*.audio,*.au,*.m4a,*.mp3,*.wav,*.wma)\n"
  "Video File (*.avi,*.wmv)\nMovie File [MPEG] (*.mpeg,*.mpg,*.m1v,*.mp2,*.mpa,*.mpe,*.mp4,*.m4v)\n"
  "Movie File [Quicktime] (*.mov)\n"
  "Windows Media File (*.asf)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS Media File formats */
static const std::string SIMDIS_MEDIA_FILE_PATTERNS =
  "SIMDIS Media Player (*.tmd,*.lst,*.pst,*.toc)\n"
  "All Files (*)\n";

/** SIMDIS 9 Antenna Pattern format patterns, separated by commas */
static const std::string ALL_ANTENNA_FILE_PATTERNS = "*.aptf,*.aprf,*.apmf,*.apbf,*.nsm,*.txt,*.uan";
/** Dialog-oriented user-friendly list of SIMDIS 9 Antenna Pattern files */
static const std::string ANTENNA_FILE_PATTERNS =
  "All Supported Antenna Pattern Formats (*.aptf,*.aprf,*.apmf,*.apbf,*.nsm,*.txt,*.uan)\n"
  "Antenna Pattern Table (*.aptf)\n"
  "Antenna Pattern Relative (*.aprf)\n"
  "Antenna Pattern Monopulse (*.apmf)\n"
  "Antenna Pattern Bilinear (*.apbf)\n"
  "Antenna Pattern NSMA (*.nsm)\n"
  "Antenna Pattern EZNEC (*.txt)\n"
  "Antenna Pattern XFDTD (*.uan)\n"
  "All Files (*)\n";

/** SIMDIS GOG Overlay Format patterns, separated by commas */
static const std::string ALL_GOG_FILE_PATTERNS = "*.gog,*.ll,*.lla,*.xy,*.xyz,*.rxy,*.rxyz,*.ovl,*.kml,*.kmz,*.lgf,*.asc";
/** Dialog-oriented user-friendly list of SIMDIS GOG Overlay Format files */
static const std::string GOG_FILE_PATTERNS =
  "All Supported GOG Formats (*.gog,*.ll,*.lla,*.xy,*.xyz,*.rxy,*.rxyz,*.ovl,*.kml,*.kmz,*.lgf,*.asc)\n"
  "GOG Files (*.gog,*.ll,*.lla,*.xy,*.xyz,*.rxy,*.rxyz)\n"
  "Misc GOG (*.gog)\nxygeo (*.ll,*.lla)\nxygeo and crossplot (*.xy,*.xyz)\n"
  "Relative xygeo and crossplot (*.rxy,*.rxyz)\n"
  "KML Files (*.kml,*.kmz)\n"
  "LATR GEO Files (*.lgf)\n"
  "GCCS OVLY Files (*.ovl)\n"
  "JDS ASC Files (*.asc)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS absolute GOG Overlay Format files */
static const std::string ABSOLUTE_GOG_FILE_PATTERNS =
  "Absolute GOG (*.gog,*.ll,*.lla)\n"
  "GOG (*.gog)\n"
  "All Files (*)\n";

/** Dialog-oriented user-friendly list of SIMDIS relative GOG Overlay Format files */
static const std::string RELATIVE_GOG_FILE_PATTERNS =
  "Relative GOG (*.xy,*.xyz,*.rxy,*.rxyz)\n"
  "GOG (*.gog)\n"
  "All Files (*)\n";

/** Dialog-oriented user-friendly list of exportable GOG format files */
static const std::string GOG_FILE_SAVE_PATTERNS =
  "All Supported GOG Formats (*.gog,*.ll,*.lla,*.xy,*.xyz,*.rxy,*.rxyz)\n"
  "Absolute GOG (*.gog,*.ll,*.lla)\n"
  "Relative GOG (*.xy,*.xyz,*.rxy,*.rxyz)\n"
  "All Files (*)\n";

/** Point map file format extensions, separated by commas */
static const std::string ALL_POINT_MAP_FILE_PATTERNS = "*.dat,*.pmd";
/** Dialog-oriented user-friendly list of Point Map file format extensions */
static const std::string POINT_MAP_FILE_PATTERNS =
  "Point Map Files (*.dat,*.pmd)\n"
  "Point Map Files v2 (*.dat)\n"
  "Point Map Files v3 (*.pmd))\n"
  "All Files (*)";

/** WVS File Formats, separated by commas */
static const std::string ALL_WVS_FILE_PATTERNS = "*.wvs,*.lwvs";
/** Dialog-oriented user-friendly list of WVS files */
static const std::string WVS_FILE_PATTERNS =
  "World Vector Shoreline File (*.wvs,*.lwvs)\n"
  "All Files (*)";

/** RFProp Configuration Input File extensions, separated by commas */
static const std::string ALL_RFPROP_CONFIG_FILE_PATTERNS = "*.in,*.txt";
/** Dialog-oriented user-friendly list of RFProp Configuration Input File extensions */
static const std::string RFPROP_CONFIG_FILE_PATTERNS =
  "All Supported RFProp Formats (*.in,*.txt)\n"
  "APM Input (*.in)\n"
  "AREPS Pattern (*.txt)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of AREPS configuration files */
static const std::string AREPS_CONFIG_FILE_PATTERNS =
  "AREPS Pattern (*.txt)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of APM Configuration files */
static const std::string APM_CONFIG_FILE_PATTERNS =
  "APM Input (*.in)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of Radar Cross Section files */
static const std::string RCS_CONFIG_FILE_PATTERNS =
  "Radar Cross Section (*.rcs)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of HDF5 files */
static const std::string HDF5_FILE_PATTERNS =
  "HDF5 Files (*.hdf5)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of Plot-XY Track Status Plot file extensions */
static const std::string TRACKSTATUS_REPORT_FILE_PATTERNS =
  "Track Status Report Text File (*.txt)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of HotKey Configuration File extensions */
static const std::string HOTKEY_FILE_PATTERNS =
  "Hot Key Settings File (*.hotkey.ini)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of HotKey Configuration File extensions for Qt */
static const std::string QHOTKEY_FILE_PATTERNS =
  "Qt Hot Key Settings File (*.qhotkey)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of UDP Player and Recorder formatted files */
static const std::string UDP_CAPTURE_FILE_PATTERNS =
  "All Supported UDP Files (*.udpx,*.udp,*.dstor)\n"
  "UDP Capture Lite (*.udpx)\n"
  "UDP Capture (*.udp)\n"
  "Data Storage (*.dstor)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of SIMDIS 10 Settings files */
static const std::string SIMDIS10_SETTINGS_FILE_PATTERNS =
  "SIMDIS 10 Configuration Files (*.ini)\n"
  "All Files (*)";

/** Dialog-oriented user-friendly list of Plot-XY Settings files */
static const std::string PLOT_SETTINGS_FILE_PATTERNS =
  "Plot-XY Configuration Files (*.ini)\n"
  "All Files (*)";

/** Combination of all data file patterns */
static const std::string ALL_DATA_FILE_PATTERNS = ALL_SIMDIS_FILE_PATTERNS +
  std::string(",.prefs,.rul,.view,.rcs,.hdf5,") +
  ALL_GOG_FILE_PATTERNS + std::string(",") +
  ALL_SIMDIS_TEXTURE_FILE_PATTERNS + std::string(",") +
  ALL_MEDIA_FILE_PATTERNS + std::string(",") +
  ALL_RFPROP_CONFIG_FILE_PATTERNS + std::string(",") +
  ALL_ANTENNA_FILE_PATTERNS + std::string(",") +
  ALL_POINT_MAP_FILE_PATTERNS + std::string(",") +
  ALL_WVS_FILE_PATTERNS + std::string(",") +
  simCore::ALL_SIMDIS_MODEL_FILE_PATTERNS;

/** GDAL files for osgEarth GDAL driver image and elevation layers */
static const std::string GDAL_IMAGE_LAYER_FILE_PATTERNS =
  "GDAL Files (*.tif,*.jpeg,*.png,*.img,*.sid,*.jp2,*.bil,*.hgt)\n"
  "All Files (*)";

/** MBTiles files for osgEarth MBTiles driver image layers */
static const std::string MBTILES_IMAGE_LAYER_FILE_PATTERNS =
  "MBTiles Files (*.mbtiles)\n"
  "All Files (*)";

/** Pattern for GDAL, DB, MBTiles files -- image file layer pattern */
static const std::string FILEBASED_IMAGE_LAYER_FILE_PATTERNS =
  "All Layer Files (*.mbtiles,*.db,*.tif,*.jpeg,*.png,*.img,*.sid,*.jp2,*.bil)\n"
  "MBTiles Files (*.mbtiles)\n"
  "DB Files (*.db)\n"
  "GDAL Files (*.tif,*.jpeg,*.png,*.img,*.sid,*.jp2,*.bil)\n"
  "All Files (*)";

/** Pattern for GDAL, DB, MBTiles files -- elevation file layer pattern */
static const std::string FILEBASED_ELEVATION_LAYER_FILE_PATTERNS =
  "All Layer Files (*.mbtiles,*.db,*.tif,*.sid,*.jp2,*.bil,*.hgt,*.dt0,*.dt1,*.dt2)\n"
  "MBTiles Files (*.mbtiles)\n"
  "DB Files (*.db)\n"
  "GDAL Files (*.tif,*.sid,*.jp2,*.bil,*.hgt)\n"
  "DTED Elevation Files (*.dt0,*.dt1,*.dt2)\n"
  "All Files (*)";

/** Generic pattern for all files */
static const std::string XML_FILE_PATTERNS =
  "XML Files (*.xml)\n"
  "All Files (*)";

} // namespace simCore

#endif /* SIMCORE_STRING_FILE_PATTERNS_H */

