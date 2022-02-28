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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_TYPES_H
#define SIMVIS_TYPES_H

#include <vector>
#include "osg/ref_ptr"
#include "osgEarth/Color"
#include "simCore/Common/Common.h"

namespace simVis
{
  class EntityNode;
  class ScenarioTool;

  /**
   * @file simVis/Types.h
   * Helpful aliases and typedefs
   */

  /** Bring osgEarth::Color into the simVis namespace */
  typedef osgEarth::Color Color;

  /** Vector of EntityNode ref_ptr */
  typedef std::vector< osg::ref_ptr<EntityNode> > EntityVector;

  /** Vector of ScenarioTool ref_ptr */
  typedef std::vector< osg::ref_ptr<ScenarioTool> > ScenarioToolVector;

} // namespace simVis


#endif // SIMVIS_TYPES_H
