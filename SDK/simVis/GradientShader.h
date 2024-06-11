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
#ifndef SIMVIS_GRADIENT_SHADER_H
#define SIMVIS_GRADIENT_SHADER_H

#include <map>
#include <string>
#include "osg/Vec4f"
#include "simCore/Common/Common.h"

namespace simVis
{

/**
 * This class is responsible for generating the source code to a GLSL shader that implements a gradient.
 * The gradient is specified with either setColorMap() (all at once) or setColor (piecemeal).  Colors are
 * clamped to the minimum and maximum values.  The color returned is based on the following conditions:
 *
 * Discrete On:
 *   value[x] <= inValue < value[x + 1]: color[x]
 *
 * Discrete Off:
 *   mix(color[x], color[x + 1]), mixed based on percentage through the values
 *
 * The function name in the generated code is configurable.  The default is simvis_gradient.  The generated
 * code will look something like:
 *
 * <code>
 * #version 330
 * vec4 simvis_gradient(in float value)
 * {
 *   // <special case code, if provided>
 *
 *   ... return vec4(...) ...
 * }
 * </code>
 *
 * You can add this to your Program, VirtualProgram, or shader code as you see fit and call it directly.
 * This class is only responsible for generating the shader code, and does not compile, link, or add it
 * to a state set.
 *
 * You can link the code to your fragment or vertex shader, such as:
 *
 * <code>
 * GradientShader gs;
 * // ... configure gs ...
 * osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateSet);
 * // Attach code to the vertex shader
 * vp->setShader(gs.functionName(), new osg::Shader(osg::Shader::VERTEX, gs.buildShader()));
 * </code>
 */
class SDKVIS_EXPORT GradientShader
{
public:
  /** map of threshold values to color */
  typedef std::map<float, osg::Vec4f> ColorMap;

  /** Creates a new GradientShader */
  GradientShader();
  /** Virtual destructor */
  virtual ~GradientShader();

  /** Sets the function name for the generated shader.  Default value is "simvis_gradient". */
  void setFunctionName(const std::string& functionName);
  /** Returns the function name for the generated shader. */
  std::string functionName() const;

  /** Sets special case code for special value detection.  Inserted after function starts, before gradient processing.  Incoming value named 'value'. */
  void setSpecialCaseCode(const std::string& specialCase);
  /** Retrieves any set special case code. */
  std::string specialCaseCode() const;

  /** Sets the color of the given value in the gradient */
  void setColor(float value, const osg::Vec4f& color);
  /** Sets all the colors for the gradient. Clears out any current colors in the map and replaces with the color map specified */
  void setColorMap(const ColorMap& colors);
  /** Retrieves currently set colors */
  const ColorMap& colorMap() const;
  /** Clears all colors in the gradient */
  void clear();

  /** Gets whether to return discrete values along the gradient.  Discrete values do not interpolate. */
  bool isDiscrete() const;
  /**
   * Sets whether to return discrete values along the gradient.
   * @param discrete If true, discrete values in the gradient will be returned.  If false, colors will be interpolated along the gradient
   */
  void setDiscrete(bool discrete);

  /** Set alpha value for all colors present in the gradient.  Only changes colors currently in color map. */
  void setAlpha(float value);

  /** Generates the shader code required to fulfill the color mapping configured in the shader. */
  std::string buildShader() const;

private:
  /** Converts a color into a GLSL vec4 string */
  std::string colorToVec4_(const osg::Vec4f& color) const;
  /** Prints a floating point value with sufficient precision to a string, for inclusion in GLSL */
  std::string printFloat_(float f) const;

  /** Name of the shader function */
  std::string functionName_;
  /** Special case code, if any, added to beginning of function */
  std::string specialCaseCode_;
  /** Maps all values to a color */
  ColorMap colors_;
  /** Flags whether to use discrete colors or interpolate */
  bool discrete_;
};

}

#endif /* SIMVIS_RFPROP_GRADIENT_COLOR_PROVIDER_H */
