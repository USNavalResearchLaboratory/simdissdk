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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgDB/ReadFile"
#include "osgEarth/LabelNode"
#include "osgEarth/PlaceNode"
#include "simNotify/Notify.h"
#include "simCore/String/Utils.h"
#include "simVis/GOG/Annotation.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"
#include "simVis/Utils.h"
#include "simVis/OverheadMode.h"

using namespace osgEarth;

namespace simVis { namespace GOG {

// default placemark icon
static const std::string PLACEMARK_ICON = "data/models/imageIcons/ylw-pushpin64.png";
// scale value for placemark icons, use a default until we add support for the KML icon scale tag
static const float PLACEMARK_ICON_SCALE = 0.45;

GogNodeInterface* TextAnnotation::deserialize(
                            const ParsedShape&       parsedShape,
                            simVis::GOG::ParserData& p,
                            const GOGNodeType&       nodeType,
                            const GOGContext&        context,
                            const GogMetaData&       metaData,
                            MapNode*                 mapNode)
{
  // parse:
  const std::string text = simCore::GOG::GogUtils::decodeAnnotation(parsedShape.stringValue(GOG_TEXT));

  p.parseGeometry<Geometry>(parsedShape);
  GogNodeInterface* rv = nullptr;
  osgEarth::GeoPositionNode* label = nullptr;
  if (parsedShape.hasValue(GOG_IMAGEFILE))
  {
    const std::string& iconFile = parsedShape.stringValue(GOG_IMAGEFILE);
    osg::ref_ptr<osg::Image> image = Utils::readRefImage(iconFile);

    // if icon can't load, use default icon
    if (!image.valid())
    {
      SIM_WARN << "Failed to load image file " << iconFile << "\n";
      image = osgDB::readImageFile(PLACEMARK_ICON);
    }

    // set the icon scale
    osgEarth::IconSymbol* icon = p.style_.getOrCreateSymbol<osgEarth::IconSymbol>();
    icon->scale() = PLACEMARK_ICON_SCALE;
    label = new osgEarth::PlaceNode(text, p.style_, image.get());
  }
  else
    label = new osgEarth::LabelNode(text, p.style_);
  label->setName("GOG Label");
  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    label->setPosition(p.getMapPosition());
    label->setMapNode(mapNode);
  }
  else
  {
    osg::PositionAttitudeTransform* trans = label->getPositionAttitudeTransform();
    if (trans != nullptr)
      trans->setPosition(p.getLTPOffset());
  }
  label->setDynamic(true);
  label->setPriority(8000);

  // in overhead mode, clamp the label's position to the ellipsoid.
  simVis::OverheadMode::enableGeoTransformClamping(true, label->getGeoTransform());

  // Circumvent osgEarth bug with annotation and style here by setting the priority forcefully
  const TextSymbol* textSymbol = p.style_.getSymbol<TextSymbol>();
  if (textSymbol && textSymbol->priority().isSet())
    label->setPriority(textSymbol->priority().value().eval());

  rv = new LabelNodeInterface(label, metaData);
  rv->applyToStyle(parsedShape, p.units_);

  return rv;
}

GogNodeInterface* TextAnnotation::createAnnotation(const simCore::GOG::Annotation& anno, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  const std::string text = anno.text();

  GogNodeInterface* rv = nullptr;
  osgEarth::GeoPositionNode* label = nullptr;
  osgEarth::Style style;

  std::string imageFile = "";
  if (anno.getImageFile(imageFile) == 0)
  {
    osg::ref_ptr<osg::Image> image = Utils::readRefImage(imageFile);
    // if icon can't load, use default icon
    if (!image.valid())
    {
      SIM_WARN << "Failed to load image file " << imageFile << "\n";
      image = osgDB::readImageFile(PLACEMARK_ICON);
    }

    // set the icon scale
    osgEarth::IconSymbol* icon = style.getOrCreateSymbol<osgEarth::IconSymbol>();
    icon->scale() = PLACEMARK_ICON_SCALE;
    label = new osgEarth::PlaceNode(text, style, image.get());
  }
  else
    label = new osgEarth::LabelNode(text, style);
  label->setName("GOG Label");

  simCore::Vec3 position;
  if (anno.getPosition(position) != 0 && !attached)
    position = refPoint;
  if (!attached)
  {
    label->setPosition(LoaderUtils::getShapeGeoPosition(anno, position, refPoint, false));
    label->setMapNode(mapNode);
  }
  else
  {
    osg::PositionAttitudeTransform* trans = label->getPositionAttitudeTransform();
    if (trans != nullptr)
      trans->setPosition(osg::Vec3d(position.x(), position.y(), position.z()));
  }
  label->setDynamic(true);
  label->setPriority(8000);

  // in overhead mode, clamp the label's position to the ellipsoid.
  simVis::OverheadMode::enableGeoTransformClamping(true, label->getGeoTransform());

  GogMetaData metaData;
  rv = new LabelNodeInterface(label, metaData);

  return rv;
}

} }
