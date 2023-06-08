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
#ifndef SIMVIS_FRAGMENTEFFECT_H
#define SIMVIS_FRAGMENTEFFECT_H

#include "osg/Referenced"
#include "simCore/Common/Common.h"
#include "simData/DataTypes.h"

namespace osg { class StateSet; }

namespace simVis
{

/**
 * Applies various shader effects, typically fragment-related. These are defined in FragmentEffect.glsl
 * and can potentially be customized by end users if desired. This is the implementation behind the
 * commonPrefs.fragmentEffect value.
 */
class SDKVIS_EXPORT FragmentEffect : public osg::Referenced
{
public:
  /** Changes the fragment effect value on the given state set. Must have called installShader() on a node at/above this level in scene. */
  static void set(osg::StateSet& stateSet, simData::FragmentEffect effect);

  /** Installs the shader program and sets the defaults on the given state set. This can be done at a high level in the scene. */
  static void installShaderProgram(osg::StateSet& stateSet);
};

}

#endif /* SIMVIS_FRAGMENTEFFECT_H */
