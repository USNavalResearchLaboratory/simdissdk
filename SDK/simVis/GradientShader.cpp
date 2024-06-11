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
#include <iomanip>
#include <sstream>
#include "osgEarth/VirtualProgram"
#include "simVis/GradientShader.h"

namespace {

/** Fixed precision for floating point values written to shader GLSL code */
static const int FLOAT_PRECISION = 8;
/** Color to return if there are no colors configured */
static const std::string DEFAULT_COLOR_STRING = "vec4(1.0, 1.0, 1.0, 1.0)";

}

namespace simVis {

GradientShader::GradientShader()
 : functionName_("simvis_gradient"),
  discrete_(true)
{
}

GradientShader::~GradientShader()
{
}

void GradientShader::setFunctionName(const std::string& functionName)
{
  functionName_ = functionName;
}

std::string GradientShader::functionName() const
{
  return functionName_;
}

void GradientShader::setSpecialCaseCode(const std::string& specialCase)
{
  specialCaseCode_ = specialCase;
}

std::string GradientShader::specialCaseCode() const
{
  return specialCaseCode_;
}

void GradientShader::clear()
{
  colors_.clear();
}

bool GradientShader::isDiscrete() const
{
  return discrete_;
}

void GradientShader::setAlpha(float value)
{
  for (auto valColor : colors_)
    valColor.second[3] = value;
}

void GradientShader::setDiscrete(bool discrete)
{
  discrete_ = discrete;
}

void GradientShader::setColor(float value, const osg::Vec4f& color)
{
  colors_[value] = color;
}

void GradientShader::setColorMap(const ColorMap& colors)
{
  colors_ = colors;
}

const GradientShader::ColorMap& GradientShader::colorMap() const
{
  return colors_;
}

std::string GradientShader::buildShader() const
{
  std::stringstream buf;

  buf << "vec4 " << functionName_ << "(in float value)\n";
  buf << "{\n";

  // Some users might have special cases.  For example, looking for sentinel values like -32767 in RF Prop loss data
  if (!specialCaseCode_.empty())
    buf << specialCaseCode_ << "\n";

  // No colors? Always return default color (white)
  if (colors_.empty())
  {
    buf << "  return " << DEFAULT_COLOR_STRING << ";\n";
    buf << "}\n";
    return buf.str();
  }

  buf << " float valueA = 0.0;\n";
  buf << " float valueB = 0.0;\n";
  buf << " vec4 colorA = vec4(1.0, 1.0, 1.0, 1.0);\n";
  buf << " vec4 colorB = vec4(1.0, 1.0, 1.0, 1.0);\n";

  ColorMap::const_iterator current = colors_.begin();

  // Special case, if the incoming value is less than the first value, return the first value
  buf << "if (value < " << printFloat_(current->first) << ") return " << colorToVec4_(current->second) << ";\n";

  for (unsigned int i = 0; i < colors_.size() - 1; i++)
  {
    ColorMap::const_iterator next = current;
    ++next;

    buf << "valueA = " << printFloat_(current->first) << ";\n";
    buf << "valueB = " << printFloat_(next->first) << ";\n";
    buf << "colorA = " << colorToVec4_(current->second) << ";\n";
    buf << "colorB = " << colorToVec4_(next->second) << ";\n";
    buf << "if (value >= valueA && value < valueB)\n";
    buf << "{\n";
    if (discrete_)
    {
      buf << "    return colorA;\n";
    }
    else
    {
      buf << "    return mix(colorA, colorB, (value - valueA) / (valueB - valueA) );\n";
    }
    buf << "}\n";

    ++current;
  }

  // Special case, if the value is greater than the last value, return the last value.
  buf << "return " << colorToVec4_(current->second) << ";\n";
  buf << "}\n";
  return buf.str();
}

std::string GradientShader::colorToVec4_(const osg::Vec4f& color) const
{
  std::stringstream buf;
  buf << "vec4(" << std::fixed << std::setprecision(FLOAT_PRECISION) << color.r() << ", "
    << std::fixed << std::setprecision(FLOAT_PRECISION) << color.g() << ", "
    << std::fixed << std::setprecision(FLOAT_PRECISION) << color.b() << ", "
    << std::fixed << std::setprecision(FLOAT_PRECISION) << color.a() << ")";
  return buf.str();
}

std::string GradientShader::printFloat_(float f) const
{
  std::stringstream buf;
  buf << std::fixed << std::setprecision(FLOAT_PRECISION) << f;
  return buf.str();
}

}
