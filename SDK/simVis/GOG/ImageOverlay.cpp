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
#include "osgDB/ReadFile"
#include "osgEarth/ImageOverlay"
#include "osgEarth/Version"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/GOG/GogShape.h"
#include "simCore/String/Utils.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Utils.h"
#include "simVis/Utils.h"
#include "simVis/GOG/ImageOverlay.h"

namespace simVis { namespace GOG {

GogNodeInterface* ImageOverlay::createImageOverlay(const simCore::GOG::ImageOverlay& imageOverlay, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  const std::string& iconFile = imageOverlay.imageFile();
  osg::ref_ptr<osg::Image> image = Utils::readRefImage(iconFile);

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
  // KML and GOG rotation is CCW; osgEarth rotation is CW
  double rot = -imageOverlay.getRotation() * simCore::RAD2DEG;
  osgEarth::Angular rotation(rot, osgEarth::Units::DEGREES);

  osgEarth::ImageOverlay* imageNode = new osgEarth::ImageOverlay(mapNode, image.get());
#if OSGEARTH_SOVERSION >= 138
  imageNode->setBoundsAndRotation(osgEarth::Bounds(west, south, 0.0, east, north, 0.0), rotation);
#else
  imageNode->setBoundsAndRotation(osgEarth::Bounds(west, south, east, north), rotation);
#endif
  imageNode->setDynamic(true);
  imageNode->setPriority(8000);

  GogMetaData metaData;
  auto* rv = new ImageOverlayInterface(imageNode, metaData);
  double shapeOpacity = 1.;
  imageOverlay.getOpacity(shapeOpacity);
  rv->setOpacity(shapeOpacity);
  return rv;
}

} }
