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
#ifndef SIMVIS_GOG_ARC_H
#define SIMVIS_GOG_ARC_H

#include "simVis/GOG/GOGNode.h"

namespace osgEarth {
  class MapNode;
  class Config;
}

namespace simVis { namespace GOG
{

class GogNodeInterface;
class ParserData;

/** Display GOG Arc */
class SDKVIS_EXPORT Arc
{
public:
  /** Create the arc from the parser data and GOG meta data */
  GogNodeInterface* deserialize(
    const osgEarth::Config&  conf,
    simVis::GOG::ParserData& p,
    const GOGNodeType&       nodeType,
    const GOGContext&        context,
    const GogMetaData&       metaData,
    osgEarth::MapNode*       mapNode);
};

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_ARC_H
