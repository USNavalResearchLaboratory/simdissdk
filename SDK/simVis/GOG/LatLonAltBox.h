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
#ifndef SIMVIS_GOG_LATLONALTBOX_H
#define SIMVIS_GOG_LATLONALTBOX_H

#include "simVis/GOG/GOGNode.h"

namespace osgEarth { class MapNode; }
namespace simCore { namespace GOG { class LatLonAltBox; } }

namespace simVis { namespace GOG {

class GogNodeInterface;
class ParsedShape;
class ParserData;

/** Display GOG LatLonAltBox */
class SDKVIS_EXPORT LatLonAltBox
{
public:
  /** Create the LatLonAltBox from the parser data and GOG meta data */
  GogNodeInterface* deserialize(
    const ParsedShape&       parsedShape,
    simVis::GOG::ParserData& p,
    const GOGNodeType&       nodeType,
    const GOGContext&        context,
    const GogMetaData&       metaData,
    osgEarth::MapNode*       mapNode);

  /** Create the LatLonAltBox from the shape object */
  static GogNodeInterface* createLatLonAltBox(const simCore::GOG::LatLonAltBox& llab, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode);
};

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_LATLONALTBOX_H
