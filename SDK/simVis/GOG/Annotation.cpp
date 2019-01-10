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
#include "osgEarthAnnotation/LabelNode"
#include "simVis/GOG/Annotation.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"
#include "simVis/Utils.h"
#include "simVis/OverheadMode.h"

using namespace simVis;
using namespace simVis::GOG;
using namespace osgEarth::Symbology;

GogNodeInterface* TextAnnotation::deserialize(
                            const ParsedShape&       parsedShape,
                            simVis::GOG::ParserData& p,
                            const GOGNodeType&       nodeType,
                            const GOGContext&        context,
                            const GogMetaData&       metaData,
                            MapNode*                 mapNode)
{
  // parse:
  const std::string text = Utils::decodeAnnotation(parsedShape.stringValue(GOG_TEXT));

  p.parseGeometry<Geometry>(parsedShape);
  GogNodeInterface* rv = NULL;
  osgEarth::Annotation::LabelNode* label = NULL;
  label = new osgEarth::Annotation::LabelNode(text, p.style_);
  label->setName("GOG Label");
  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    label->setPosition(p.getMapPosition());
    label->setMapNode(mapNode);
  }
  else
  {
    osg::PositionAttitudeTransform* trans = label->getPositionAttitudeTransform();
    if (trans != NULL)
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
