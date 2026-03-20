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
#ifndef SIMVIS_GOG_ELLIPSOID_H
#define SIMVIS_GOG_ELLIPSOID_H

#include "simVis/GOG/GOGNode.h"

namespace osgEarth { class MapNode; }
namespace simCore { namespace GOG { class Ellipsoid; } }

namespace simVis { namespace GOG {

class GogNodeInterface;

/** Display GOG Ellipsoid */
class SDKVIS_EXPORT Ellipsoid
{
public:
  /** Create the ellipsoid from the shape object */
  static GogNodeInterface* createEllipsoid(const simCore::GOG::Ellipsoid& ellipsoid, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode);
};

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_ELLIPSOID_H
