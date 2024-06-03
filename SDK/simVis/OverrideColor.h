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
#ifndef SIMVIS_OVERRIDE_COLOR_H
#define SIMVIS_OVERRIDE_COLOR_H

#include "osgEarth/VirtualProgram"
#include "simCore/Common/Common.h"
#include "simVis/Types.h"

namespace simVis
{

/**
 * Sets the override color via a shader.  To use this, first install the shader
 * program (via installShaderProgram()) on a node at or above the one you want
 * colored.  Then on node(s) to color, instantiate this class with the state set,
 * using setColor() to change the color.
 */
class SDKVIS_EXPORT OverrideColor : public osg::Referenced
{
public:
  /// Enumeration of how the color for override color gets used
  enum CombineMode
  {
    /// Do not use override color
    OFF,
    /// Multiply the override color against incoming color; good for shaded items and 2D images
    MULTIPLY_COLOR,
    /// Replace the incoming color with the override color; good for flat items
    REPLACE_COLOR,
    /// Apply color by copying the previous color intensity and replacing with this one, retaining shading better than REPLACE_COLOR
    INTENSITY_GRADIENT
  };

  /**
   * Declares uniform variables for using and setting the override color
   */
  explicit OverrideColor(osg::StateSet* stateset);

  /**
   * Sets the override color via uniform variables.  Application of the color depends on
   * the combine mode.
   */
  void setColor(const simVis::Color& color);

  /**
   * Sets the combine mode to use for override color.  Classic SIMDIS always used a
   * MULTIPLY_COLOR combination that merges the override color with the incoming color.
   * The REPLACE_COLOR mode respects alpha blending but replaces the source color completely.
   * The INTENSITY_GRADIENT respects alpha blending and shading, replacing source color.
   */
  void setCombineMode(CombineMode combineMode);

  /**
   * Before using this class a call to installShaderProgram is required.  This
   * method installs the shader program and default uniform variables for
   * controlling the shader.
   */
  static void installShaderProgram(osg::StateSet* intoStateSet);

  /**
   * Sets the override color via uniform variables.  Application of the color depends on
   * the combine mode.
   */
  static void setColor(osg::StateSet* stateset, const simVis::Color& color);

  /**
   * Sets the combine mode to use for override color.  Classic SIMDIS always used a
   * MULTIPLY_COLOR combination that merges the override color with the incoming color.
   * The REPLACE_COLOR mode respects alpha blending but replaces the source color completely.
   * The INTENSITY_GRADIENT respects alpha blending and shading, replacing source color.
   */
  static void setCombineMode(osg::StateSet* stateset, CombineMode combineMode);

protected:
  /// osg::Referenced-derived
  virtual ~OverrideColor();

private:
  /// Set the use to false and set the color to white
  static void setDefaultValues_(osg::StateSet* stateSet);

  osg::observer_ptr<osg::StateSet> stateset_;
};

} // namespace simVis

#endif // SIMVIS_OVERRIDE_COLOR_H

