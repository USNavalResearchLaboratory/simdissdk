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
#ifndef SIMVIS_GOG_CIRCLE_H
#define SIMVIS_GOG_CIRCLE_H

#include "simVis/GOG/GOGNode.h"

namespace osgEarth { class MapNode; }

namespace simVis { namespace GOG {

class GogNodeInterface;
class ParsedShape;
class ParserData;

/** Display GOG Circle */
class SDKVIS_EXPORT Circle
{
public:
  /** Create the circle from the parser data and GOG meta data */
  GogNodeInterface* deserialize(
    const ParsedShape&       parsedShape,
    simVis::GOG::ParserData& p,
    const GOGNodeType&       nodeType,
    const GOGContext&        context,
    const GogMetaData&       metaData,
    osgEarth::MapNode*       mapNode);

  /** Create the circle from the shape object */
  static GogNodeInterface* createCircle(const simCore::GOG::Circle& circle, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode);
};

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_CIRCLE_H
