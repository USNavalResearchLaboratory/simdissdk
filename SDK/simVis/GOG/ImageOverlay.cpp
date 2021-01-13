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
#include "osgDB/ReadFile"
#include "osgEarth/ImageOverlay"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/GOG/GogShape.h"
#include "simCore/String/Utils.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"
#include "simVis/Utils.h"
#include "simVis/GOG/ImageOverlay.h"

namespace simVis { namespace GOG {


GogNodeInterface* ImageOverlay::deserialize(
                            const ParsedShape&       parsedShape,
                            simVis::GOG::ParserData& p,
                            const GOGNodeType&       nodeType,
                            const GOGContext&        context,
                            const GogMetaData&       metaData,
                            osgEarth::MapNode*       mapNode)
{
  GogNodeInterface* rv = nullptr;

  if (!parsedShape.hasValue(GOG_ICON))
    return rv;

  std::string iconFile = parsedShape.stringValue(GOG_ICON);
  osg::ref_ptr<osg::Image> image = osgDB::readImageFile(simCore::StringUtils::trim(iconFile, "\""));
  // if icon can't load, return failure
  if (!image.valid())
  {
    SIM_WARN << "Failed to load image file " << iconFile << "\n";
    return rv;
  }
  double north = p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, p.parseAngle(parsedShape.stringValue(GOG_LLABOX_N), 0.0));
  double south = p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, p.parseAngle(parsedShape.stringValue(GOG_LLABOX_S), 0.0));
  double east = p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, p.parseAngle(parsedShape.stringValue(GOG_LLABOX_E), 0.0));
  double west = p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, p.parseAngle(parsedShape.stringValue(GOG_LLABOX_W), 0.0));
  double rot = p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, p.parseAngle(parsedShape.stringValue(GOG_LLABOX_ROT), 0.0));
  osgEarth::Angular rotation(rot, osgEarth::Units::DEGREES);

  osgEarth::ImageOverlay* imageNode = new osgEarth::ImageOverlay(mapNode, image.get());
  imageNode->setBoundsAndRotation(osgEarth::Bounds(west, south, east, north), rotation);
  imageNode->setDynamic(true);
  imageNode->setPriority(8000);

  rv = new ImageOverlayInterface(imageNode, metaData);

  return rv;
}

GogNodeInterface* ImageOverlay::createImageOverlay(const simCore::GOG::ImageOverlay& imageOverlay, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  std::string iconFile = imageOverlay.imageFile();

  osg::ref_ptr<osg::Image> image = osgDB::readImageFile(simCore::StringUtils::trim(iconFile, "\""));
  // if icon can't load, return failure
  if (!image.valid())
  {
    SIM_WARN << "Failed to load image file " << iconFile << "\n";
    return nullptr;
  }
  double north = imageOverlay.north() * simCore::RAD2DEG;
  double south = imageOverlay.south() * simCore::RAD2DEG;
  double east = imageOverlay.east() * simCore::RAD2DEG;
  double west = imageOverlay.west() * simCore::RAD2DEG;
  double rot = imageOverlay.getRotation() * simCore::RAD2DEG;
  osgEarth::Angular rotation(rot, osgEarth::Units::DEGREES);

  osgEarth::ImageOverlay* imageNode = new osgEarth::ImageOverlay(mapNode, image.get());
  imageNode->setBoundsAndRotation(osgEarth::Bounds(west, south, east, north), rotation);
  imageNode->setDynamic(true);
  imageNode->setPriority(8000);

  GogMetaData metaData;
  // need to specify KML load format for these shapes, which attach to the map node differently than normal GOG shapes
  metaData.loadFormat = FORMAT_KML;
  return new ImageOverlayInterface(imageNode, metaData);
}

} }
