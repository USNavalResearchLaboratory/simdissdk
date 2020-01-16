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
#include <string>
#include "osgDB/FileNameUtils"
#include "osgDB/Registry"
#include "osgEarth/TileSource"
#include "DBTileSource.h"

class SimSdkOSGEarthDBDriverPlugin : public osgEarth::Contrib::TileSourceDriver
{
public:
  SimSdkOSGEarthDBDriverPlugin() { }

  const char* className()
  {
    return "OSGEarth DB Driver";
  }

  bool acceptsExtension(const std::string& extension) const
  {
    return osgDB::equalCaseInsensitive("osgearth_db", extension);
  }

  osgDB::ReaderWriter::ReadResult readObject(const std::string& uri, const osgDB::Options* options) const
  {
    std::string ext = osgDB::getFileExtension(uri);
    if (!acceptsExtension(ext))
    {
      return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
    }

    return osgDB::ReaderWriter::ReadResult(
      new simVis_db::DBTileSource(getTileSourceOptions(options)));
  }
};

REGISTER_OSGPLUGIN(osgearth_db, SimSdkOSGEarthDBDriverPlugin)
