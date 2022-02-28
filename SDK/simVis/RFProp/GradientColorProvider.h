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
#ifndef SIMVIS_RFPROP_GRADIENT_COLOR_PROVIDER_H
#define SIMVIS_RFPROP_GRADIENT_COLOR_PROVIDER_H

#include <map>
#include "simVis/GradientShader.h"
#include "simVis/RFProp/ColorProvider.h"

namespace simRF
{

/** A ColorProvider that provides colors along a gradient.  Adapts simVis::GradientShader to the ColorProvider interface. */
class SDKVIS_EXPORT GradientColorProvider : public ColorProvider
{
public:
  /// map of threshold values to color
  typedef std::map<float, osg::Vec4f> ColorMap;

  /** Creates a new GradientColorProvider */
  GradientColorProvider();

  /** Gets the display color mode */
  virtual ColorMode getMode() const { return COLORMODE_GRADIENT; }

  /** Set alpha value for all colors in the gradient */
  void setAlpha(float value);

  /** Sets the color of the given value in the gradient */
  void setColor(float value, const osg::Vec4f& color);

  /** Sets all the colors for the gradient. Clears out any current colors in the map and replaces with the color map specified */
  void setColorMap(const ColorMap& colors);

  /** Gets whether to return discrete values along the gradient */
  bool getDiscrete() const;

  /**
   * Sets whether to return discrete values along the gradient.
   * @param discrete If true, discrete values in the gradient will be returned.  If false, colors will be interpolated along the gradient
   */
  void setDiscrete(bool discrete);

  /** Clears all colors in the gradient */
  void clear();

  /** Installs this color provider from the given state set */
  virtual void install(osg::StateSet* stateset);

  /** Uninstall this color provider from the given state set */
  virtual void uninstall(osg::StateSet* stateset);

protected:
  /// osg::Referenced-derived
  virtual ~GradientColorProvider() {}

  /** Reloads the shader */
  void reloadShader_();

protected:
  /** Vertex shader */
  osg::ref_ptr<osg::Shader> vertShader_;
  /** Fragment shader */
  osg::ref_ptr<osg::Shader> fragShader_;

  /** Manages the underlying gradient and generates GLSL code. */
  simVis::GradientShader gradient_;
};
}

#endif /* SIMVIS_RFPROP_GRADIENT_COLOR_PROVIDER_H */
