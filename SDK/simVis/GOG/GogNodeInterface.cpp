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
 * For more information please send email to simdis@enews.nrl.navy.mil
 *
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 */
#include <cassert>
#include <iostream>
#include <limits>
#include "osg/Depth"
#include "osg/PolygonOffset"
#include "osgDB/ReadFile"
#include "osgEarth/AltitudeSymbol"
#include "osgEarth/FeatureNode"
#include "osgEarth/GeoPositionNode"
#include "osgEarth/ImageOverlay"
#include "osgEarth/LabelNode"
#include "osgEarth/LineSymbol"
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/PlaceNode"
#include "osgEarth/PolygonSymbol"
#include "osgEarth/RenderSymbol"
#include "osgEarth/Style"
#include "osgEarth/TextSymbol"
#include "osgEarth/Units"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simVis/Constants.h"
#include "simVis/OverrideColor.h"
#include "simVis/OverheadMode.h"
#include "simVis/Registry.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/GOG/GOG.h"
#include "simVis/GOG/GOGNode.h"
#include "simVis/GOG/Arc.h"
#include "simVis/GOG/Circle.h"
#include "simVis/GOG/Ellipse.h"
#include "simVis/GOG/Ellipsoid.h"
#include "simVis/GOG/Line.h"
#include "simVis/GOG/LineSegs.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Parser.h"
#include "simVis/GOG/Points.h"
#include "simVis/GOG/Polygon.h"
#include "simVis/GOG/GogNodeInterface.h"

#ifndef GL_CLIP_DISTANCE0
#define GL_CLIP_DISTANCE0 0x3000
#endif

namespace {

  /**
  * Calculates the position of the node (GeoPositionNode) passed in, applying the offset if it has one.
  * Will use the referencePosition if it is provided, otherwise uses the node's internal position.
  * If useLocalOffset is specified, will use the node's local offset, otherwise uses the center of the bounding sphere
  * Returns 0 on success, non-zero if no position could be found
  */
  template<class T>
  int findLocalGeometryPosition(T* node, osgEarth::GeoPoint* referencePosition, osg::Vec3d& position, bool useLocalOffset)
  {
    if (!node || (!referencePosition && !node->getPosition().isValid()))
      return 1;
    // use reference point if it's valid, otherwise use the node's position
    osgEarth::GeoPoint refPosition = referencePosition != nullptr ? *referencePosition : node->getPosition();

    osg::Vec3d centerPoint;

    if (useLocalOffset)
    {
      centerPoint = node->getLocalOffset();
    // if the offsets are 0, just pass back the position
      if (centerPoint == osg::Vec3d(0.0, 0.0, 0.0))
      {
        position = refPosition.vec3d();
        return 0;
      }
    }
    else // use bounding sphere for center
      centerPoint = node->getBound().center();

    simCore::Coordinate llaCoord;
    // apply the offset to the ref position if using the local offset or if the map node is nullptr which indicates this is a hosted node (relative to ref position)
    if (useLocalOffset || node->getMapNode() == nullptr)
    {
      // if the offsets are non-zero, apply the offsets to our reference position
      simCore::CoordinateConverter converter;
      converter.setReferenceOrigin(refPosition.y() * simCore::DEG2RAD, refPosition.x() * simCore::DEG2RAD, refPosition.z());
      simCore::Coordinate xEastCoord(simCore::COORD_SYS_XEAST, simCore::Vec3(centerPoint.x(), centerPoint.y(), centerPoint.z()));
      converter.convert(xEastCoord, llaCoord, simCore::COORD_SYS_LLA);
    }
    else // convert from absolute center, ECEF to LLA
    {
      const simCore::Coordinate ecefCoord(simCore::COORD_SYS_ECEF, simCore::Vec3(centerPoint.x(), centerPoint.y(), centerPoint.z()));
      simCore::CoordinateConverter converter;
      converter.convert(ecefCoord, llaCoord, simCore::COORD_SYS_LLA);
    }

    position = osg::Vec3d(llaCoord.lon()*simCore::RAD2DEG, llaCoord.lat()*simCore::RAD2DEG, llaCoord.alt());
    return 0;
  }
}

namespace simVis { namespace GOG {

GogNodeInterface::GogNodeInterface(osg::Node* osgNode, const simVis::GOG::GogMetaData& metaData)
  : osgNode_(osgNode),
    metaData_(metaData),
    filled_(false),
    extruded_(false),
    outlined_(false),
    depthBuffer_(false),
    depthBufferOverride_(DEPTHBUFFER_IGNORE_OVERRIDE),
    extrudedHeight_(0.0),
    hasDefaultStyle_(false),
    altitude_(0.),
    altOffset_(0.),
    altMode_(ALTITUDE_NONE),
    hasMapNode_(false),
    deferringStyleUpdate_(false),
    defaultFont_("arial.ttf"),
    defaultTextSize_(15),
    defaultTextColor_(simVis::Color::Red),
    rangeUnits_(simCore::Units::YARDS),
    opacity_(1.f)
{
  if (osgNode_.valid())
  {
    osgNode_->setNodeMask(simVis::DISPLAY_MASK_GOG);

    // Initialize the override color
    simVis::OverrideColor::setCombineMode(osgNode_->getOrCreateStateSet(), simVis::OverrideColor::MULTIPLY_COLOR);
    simVis::OverrideColor::setColor(osgNode_->getOrCreateStateSet(), osg::Vec4f(1.f, 1.f, 1.f, 1.f));

    // flatten in overhead mode by default - subclass might change this
    simVis::OverheadMode::enableGeometryFlattening(true, osgNode_.get());
  }
}

void GogNodeInterface::setDefaultFont(const std::string& fontName)
{
  defaultFont_ = fontName;
}

void GogNodeInterface::setDefaultTextSize(int textSize)
{
  defaultTextSize_ = textSize;
}

void GogNodeInterface::setDefaultTextColor(const osg::Vec4f& textColor)
{
  defaultTextColor_ = textColor;
}

void GogNodeInterface::storeDefaultStyle()
{
  defaultStyle_ = style_;
  hasDefaultStyle_ = true;
}

void GogNodeInterface::revertToDefaultStyle()
{
  if (hasDefaultStyle_)
  {
    style_ = defaultStyle_;
    setStyle_(style_);
  }
}

void GogNodeInterface::applyToStyle(const ParsedShape& parent, const UnitsState& units)
{
  // for performance reasons, cache all style updates, apply once when done
  beginStyleUpdates_();

  metaData_.allowSetExplicitly(false);  // setFields will incorrectly respond to defaults here, so cache the correct value and restore it at the end
  metaData_.altitudeUnits_ = units.altitudeUnits_; // need to cache altitude units here, since some altitude values can be changed
  const std::string& key = parent.shape();
  const simVis::GOG::GogShape gogShape = metaData_.shape;
  bool is3dShape = (gogShape == GOG_SPHERE || gogShape == GOG_ELLIPSOID || gogShape == GOG_HEMISPHERE ||
    gogShape == GOG_CYLINDER || gogShape == GOG_LATLONALTBOX || gogShape == GOG_CONE);

  // do we need an ExtrusionSymbol? Note that 3D shapes cannot be extruded
  bool isExtruded = simCore::stringIsTrueToken(parent.stringValue(GOG_EXTRUDE)) && !is3dShape;

  // do we need a PolygonSymbol?
  bool isFillable = isExtruded || key == "poly" || key == "polygon" || key == "ellipse" || key == "circle" || key == "arc" || key == "orbit" || is3dShape;
  bool isFilled   = isFillable && simCore::stringIsTrueToken(parent.stringValue(GOG_FILLED));

  // do we need a LineSymbol?
  bool isOutlined = simCore::stringIsTrueToken(parent.stringValue(GOG_OUTLINE));
  bool hasLineAttrs = parent.hasValue(GOG_LINECOLOR) || parent.hasValue(GOG_LINEWIDTH) || parent.hasValue(GOG_LINESTYLE) || isOutlined;
  // Tessellate behaves badly with cirles, arcs, ellipses and 3dShapes, do not apply
  bool isTessellated = simCore::stringIsTrueToken(parent.stringValue(GOG_TESSELLATE)) && !(is3dShape || key == "circle" || key == "ellipse" || key == "arc");
  // need to create a LineSymbol if the shape is filled or has some line attributes or is tessellated, since tessellation is handled in the LineSymbol
  bool isLined = isFilled || hasLineAttrs || isTessellated;
  bool isText = (key == "annotation");

  // POINT attributes
  if (gogShape == GOG_POINTS && parent.hasValue(GOG_POINTSIZE))
    setPointSize(parent.doubleValue(GOG_POINTSIZE, 1));

  // LINE attributes
  if (isLined)
  {

    if (parent.hasValue(GOG_OUTLINE))
      setOutlineState(isOutlined);
    else
      setOutlineState(true);

    if (parent.hasValue(GOG_LINECOLOR))
      setLineColor(simVis::Color(parent.stringValue(GOG_LINECOLOR)));

    if (parent.hasValue(GOG_LINEWIDTH))
      setLineWidth(parent.doubleValue(GOG_LINEWIDTH, 1));

    if (parent.hasValue(GOG_LINESTYLE))
    {
      const std::string& ls = parent.stringValue(GOG_LINESTYLE);
      if (simCore::caseCompare(ls, "dash") == 0 || simCore::caseCompare(ls, "dashed") == 0)
        setLineStyle(Utils::LINE_DASHED);
      else if (simCore::caseCompare(ls, "dot") == 0 || simCore::caseCompare(ls, "dotted") == 0)
        setLineStyle(Utils::LINE_DOTTED);
      else if (simCore::caseCompare(ls, "solid") != 0)
      {
        SIM_WARN << "Found invalid linestyle value \"" << ls << "\" while parsing GOG\n";
      }
    }
  }

  // FILL attributes
  if (isFillable)
  {
    if (parent.hasValue(GOG_FILLCOLOR))
      setFillColor(simVis::Color(parent.stringValue(GOG_FILLCOLOR)));
    else if (parent.hasValue(GOG_LINECOLOR))
      setFillColor(simVis::Color(parent.stringValue(GOG_LINECOLOR)));  // Default to the line color if the fill color is not set
    setFilledState(isFilled);
  }
  // only points and annotation do not support the fillcolor keyword
  else if ((gogShape == GOG_POINTS || gogShape == GOG_ANNOTATION) && parent.hasValue(GOG_FILLCOLOR))
  {
    SIM_WARN << "The GOG keyword " << key << " does not support fillcolor.\n";
  }

  // altitude offset
  if (parent.hasValue(GOG_3D_OFFSETALT))
  {
    double altOffset = parent.doubleValue(GOG_3D_OFFSETALT, 0.);
    // convert from gog file altitude units to meters; gog file default units are ft, but file can specify different units
    altOffset = units.altitudeUnits_.convertTo(simCore::Units::METERS, altOffset);
    setAltOffset(altOffset);
  }
  // ALTITUDE mode, handles extrude attribute, which requires a specific AltitudeSymbol
  AltitudeMode altMode = ALTITUDE_NONE;
  if (simCore::caseCompare(parent.stringValue(GOG_ALTITUDEMODE), "relativetoground") == 0)
    altMode = ALTITUDE_GROUND_RELATIVE;
  else if (simCore::caseCompare(parent.stringValue(GOG_ALTITUDEMODE), "clamptoground") == 0)
    altMode = ALTITUDE_GROUND_CLAMPED;
  else if (isExtruded)
    altMode = ALTITUDE_EXTRUDE;
  setAltitudeMode(altMode);

  // process extrude height if extrude is set and if an extrude height was specified
  if (altMode == ALTITUDE_EXTRUDE && parent.hasValue(GOG_EXTRUDE_HEIGHT))
  {
    double extrudeHeight = static_cast<double>(parent.doubleValue(GOG_EXTRUDE_HEIGHT, 0));
    // convert from gog file altitude units to meters; gog file default units are ft, but file can specify different units
    extrudeHeight = units.altitudeUnits_.convertTo(simCore::Units::METERS, extrudeHeight);
    setExtrudedHeight(extrudeHeight);
  }

  // TESSELLATION attribute
  simVis::GOG::TessellationStyle tessStyle = TESSELLATE_NONE;
  if (isTessellated)
  {
    // default to rhumbline
    tessStyle = TESSELLATE_RHUMBLINE;
    if (parent.hasValue(GOG_LINEPROJECTION) && (simCore::caseCompare(parent.stringValue(GOG_LINEPROJECTION), "greatcircle") == 0))
      tessStyle = TESSELLATE_GREAT_CIRCLE_PROJECTION;
  }
  setTessellation(tessStyle);

  // TEXT attributes
  if (isText)
  {
    // default to font arial 15, color red
    std::string fontName = defaultFont_;
    int fontSize = defaultTextSize_;
    osg::Vec4f fontColor = defaultTextColor_;
    // fonts.
    if (parent.hasValue(GOG_FONTNAME))
      fontName = parent.stringValue(GOG_FONTNAME);

    if (parent.hasValue(GOG_FONTSIZE))
      fontSize = parent.doubleValue(GOG_FONTSIZE, fontSize);

    if (parent.hasValue(GOG_LINECOLOR))
      fontColor = simVis::Color(parent.stringValue(GOG_LINECOLOR));

    setFont(fontName, fontSize, fontColor);

    osgEarth::Color outlineColor = osgEarth::Color::Black;
    if (parent.hasValue(GOG_TEXTOUTLINECOLOR))
      outlineColor = osgEarth::Color(parent.stringValue(GOG_TEXTOUTLINECOLOR));

    simData::TextOutline outlineThickness = simData::TO_THIN;
    if (parent.hasValue(GOG_TEXTOUTLINETHICKNESS))
    {
      std::string thicknessStr = parent.stringValue(GOG_TEXTOUTLINETHICKNESS);
      if (simCore::caseCompare(thicknessStr, "thick") == 0)
        outlineThickness = simData::TO_THICK;
      else if (simCore::caseCompare(thicknessStr, "none") == 0)
        outlineThickness = simData::TO_NONE;
      else if (simCore::caseCompare(thicknessStr, "thin") != 0)
        SIM_WARN << "Found invalid text outline thickness value \"" << thicknessStr << "\" while parsing GOG\n";
  }

    setTextOutline(outlineColor, outlineThickness);
  }

  // DEPTH BUFFER attribute
  // depth buffer defaults to disable to match SIMDIS 9
  bool depthTest = false;
  if (parent.hasValue(GOG_DEPTHBUFFER))
    depthTest = simCore::stringIsTrueToken(parent.stringValue(GOG_DEPTHBUFFER));
  setDepthBuffer(depthTest);

  // apply backface culling here
  applyBackfaceCulling();

  metaData_.allowSetExplicitly(true);

  // done deferring style updates; apply them all at once
  endStyleUpdates_();
  setStyle_(style_);
}

osg::Node* GogNodeInterface::osgNode() const
{
  return osgNode_.get();
}

simVis::GOG::LoadFormat GogNodeInterface::loadFormat() const
{
  return metaData_.loadFormat;
}

void GogNodeInterface::setRangeUnits(const simCore::Units& units)
{
  rangeUnits_ = units;
}

const simCore::Units& GogNodeInterface::rangeUnits() const
{
  return rangeUnits_;
}

simVis::GOG::GogShape GogNodeInterface::shape() const
{
  return metaData_.shape;
}

size_t GogNodeInterface::lineNumber() const
{
  return metaData_.lineNumber;
}

void GogNodeInterface::serializeToStream(std::ostream& gogOutputStream)
{
  std::string metaData = metaData_.metadata;
  simVis::GOG::GogShape shape = metaData_.shape;

  // first add the shape keyword
  serializeKeyword_(gogOutputStream);

  // check for keyword flags
  bool serializeGeometry = Utils::canSerializeGeometry_(shape);
  bool relativeShape = getMetaDataFlag_(simVis::GOG::RelativeShapeKeyword, metaData);
  bool referencePoint = getMetaDataFlag_(simVis::GOG::ReferencePointKeyword, metaData);

  // add the metadata
  gogOutputStream << metaData;

  // serialize geometry where it is possible to extract geometry information from the nodes. Otherwise, geometry will have been stored in meta data
  if (serializeGeometry)
  {
    // alt units are meters
    gogOutputStream << "altitudeunits meters\n";

    // if relative, the xy range units are in meters
    if (relativeShape)
    {
      // if relative shape has a reference position, serialize it, if possible
      if (referencePoint)
      {
        osg::Vec3d position;
        // note that in osg position syntax, lat is y, lon is x, alt is z
        if (getReferencePosition(position) == 0)
          gogOutputStream << "referencepoint " << position.y() << " " << position.x() << " " << position.z() << "\n";
      }
      gogOutputStream << "rangeunits meters\n";
    }

    // try to serialize the geometry
    serializeGeometry_(relativeShape, gogOutputStream);
  }
  // now add the style data

  // draw flag
  if (!getDraw())
    gogOutputStream << "off\n";

  // line style
  int lineWidth = 1;
  bool outlineState = false;
  osg::Vec4f lineColor;
  Utils::LineStyle lineStyle = Utils::LINE_SOLID;
  if (getLineState(outlineState, lineColor, lineStyle, lineWidth) == 0)
  {
    if (metaData_.isSetExplicitly(GOG_LINE_WIDTH_SET))
      gogOutputStream << "linewidth " << lineWidth << "\n";
    if (metaData_.isSetExplicitly(GOG_LINE_COLOR_SET))
      gogOutputStream << "linecolor hex " << Utils::serializeOsgColor(lineColor) << "\n";
    if (metaData_.isSetExplicitly(GOG_LINE_STYLE_SET))
      gogOutputStream << "linestyle " << Utils::serializeLineStyle(lineStyle) << "\n";
    if (outlineState && metaData_.isSetExplicitly(GOG_OUTLINE_SET))
      gogOutputStream << "outline true\n";
    else if (!outlineState && metaData_.isSetExplicitly(GOG_OUTLINE_SET))
      gogOutputStream << "outline false\n";
  }

  int pointSize;
  if ((getPointSize(pointSize) == 0) && metaData_.isSetExplicitly(GOG_POINT_SIZE_SET))
    gogOutputStream << "pointsize" << pointSize << "\n";

  // fill style
  bool fillState = false;
  osg::Vec4f fillColor;
  if ((getFilledState(fillState, fillColor) == 0))
  {
    if (metaData_.isSetExplicitly(GOG_FILL_COLOR_SET))
      gogOutputStream << "fillcolor hex " << Utils::serializeOsgColor(fillColor) << "\n";
    if (fillState)
      gogOutputStream << "filled\n";
  }

  // depth buffer
  bool depthBuffer = false;
  if (getDepthBuffer(depthBuffer) == 0 && metaData_.isSetExplicitly(GOG_DEPTH_BUFFER_SET))
    gogOutputStream << "depthBuffer " << (depthBuffer ? "true" : "false") << "\n";

  // altoffset
  double altOffset = 0.0;
  if (getAltOffset(altOffset) == 0 && metaData_.isSetExplicitly(GOG_THREE_D_OFFSET_ALT_SET))
  {
    // if not serializing geometry, which always uses meters, convert to the stored altitude units
    if (!serializeGeometry)
    {
      simCore::Units curUnits(simCore::Units::METERS);
      altOffset = curUnits.convertTo(metaData_.altitudeUnits_, altOffset);
    }
    gogOutputStream << "3d offsetalt " << altOffset << "\n";
  }
  // font
  int fontSize;
  std::string fontFile;
  osg::Vec4f fontColor;
  if (getFont(fontFile, fontSize, fontColor) == 0)
  {
    // font file is full path, serialize only file name
    std::string fontFileNoPath = simCore::backslashToFrontslash(fontFile);
    fontFileNoPath = fontFileNoPath.substr((fontFileNoPath.rfind("/") + 1)); //< increment index so the slash is not saved out with the font name
    if (metaData_.isSetExplicitly(GOG_FONT_NAME_SET))
      gogOutputStream << "fontname " << fontFileNoPath << "\n";
    if (metaData_.isSetExplicitly(GOG_FONT_SIZE_SET))
      gogOutputStream << "fontsize " << fontSize << "\n";
    if (metaData_.isSetExplicitly(GOG_LINE_COLOR_SET))
      gogOutputStream << "linecolor hex " << Utils::serializeOsgColor(fontColor) << "\n";
  }

  // text outline
  osg::Vec4f outlineColor;
  simData::TextOutline outlineThickness;
  if (getTextOutline(outlineColor, outlineThickness) == 0)
  {
    if (metaData_.isSetExplicitly(GOG_TEXT_OUTLINE_COLOR_SET))
      gogOutputStream << "textoutlinecolor hex " << Utils::serializeOsgColor(outlineColor) << "\n";
    if (metaData_.isSetExplicitly(GOG_TEXT_OUTLINE_THICKNESS_SET))
    {
      std::string outlineThicknessStr;
      switch (outlineThickness)
      {
      case simData::TO_THICK:
        outlineThicknessStr = "thick";
        break;
      case simData::TO_THIN:
        outlineThicknessStr = "thin";
        break;
      case simData::TO_NONE:
        outlineThicknessStr = "none";
        break;
      }

      // Assertion failure means there's an unhandled value in the above switch
      assert(!outlineThicknessStr.empty());
      gogOutputStream << "textoutlinethickness " << outlineThicknessStr << "\n";
    }
  }

  // extrude
  bool extruded = false;
  if ((getExtruded(extruded) == 0) && extruded)
  {
    double extrudeHeightM;
    if (0 == getExtrudedHeight(extrudeHeightM))
      gogOutputStream << "extrude true " << extrudeHeightM << "\n";
    else
      gogOutputStream << "extrude true\n";
  }
  else if (metaData_.isSetExplicitly(GOG_EXTRUDE_SET))
    gogOutputStream << "extrude false\n";

  // tessellate
  TessellationStyle tessellate = TESSELLATE_NONE;
  if ((getTessellation(tessellate) == 0) && tessellate != TESSELLATE_NONE)
  {
    gogOutputStream << "tessellate true\n";
    if (metaData_.isSetExplicitly(GOG_LINE_PROJECTION_SET))
      gogOutputStream << "lineprojection " << (tessellate == TESSELLATE_GREAT_CIRCLE_PROJECTION ? "greatcircle" : "rhumbline") << "\n";
  }
  else if (metaData_.isSetExplicitly(GOG_TESSELLATE_SET))
    gogOutputStream << "tessellate false\n";

  // altitude mode
  simVis::GOG::AltitudeMode altMode;
  if (getAltitudeMode(altMode) == 0)
  {
    if (altMode == simVis::GOG::ALTITUDE_NONE && metaData_.isSetExplicitly(GOG_ALTITUDE_MODE_SET))
      gogOutputStream << "altitudemode none\n";
    else if (altMode == simVis::GOG::ALTITUDE_GROUND_RELATIVE)
      gogOutputStream << "altitudemode relativetoground\n";
    else if (altMode == simVis::GOG::ALTITUDE_GROUND_CLAMPED)
      gogOutputStream << "altitudemode clamptoground\n";
    // simVis::GOG::ALTITUDE_EXTRUDE is covered by the extrude keyword
  }
  // Follow data is not currently serialized out
}

int GogNodeInterface::getAltitudeMode(AltitudeMode& altMode) const
{
  if (!hasMapNode_)
    return 1;

  bool isValid = false;

  int rv = getExtruded(isValid);
  if (rv == 0 && isValid)
  {
    altMode = ALTITUDE_EXTRUDE;
    return rv;
  }

  altMode = altMode_;
  return 0;
}

int GogNodeInterface::getAltOffset(double& altOffset) const
{
  altOffset = altOffset_;
  return 0;
}

int GogNodeInterface::getDepthBuffer(bool& depthBuffer) const
{
  depthBuffer = depthBuffer_;
  return 0;
}

int GogNodeInterface::getDepthBufferOverrideState(DepthBufferOverride& state)
{
  state = depthBufferOverride_;
  return 0;
}

bool GogNodeInterface::getDraw() const
{
  return osgNode_.valid() ? osgNode_->getNodeMask() != simVis::DISPLAY_MASK_NONE : false;
}

int GogNodeInterface::getExtruded(bool& extruded) const
{
  switch (metaData_.shape)
  {
  case simVis::GOG::GOG_POINTS:
  case simVis::GOG::GOG_POLYGON:
  case simVis::GOG::GOG_CIRCLE:
  case simVis::GOG::GOG_ELLIPSE:
  case simVis::GOG::GOG_LINE:
  case simVis::GOG::GOG_LINESEGS:
  case simVis::GOG::GOG_ARC:
    break;
  default:
    return 1;
  }
  extruded = extruded_;
  return 0;
}

int GogNodeInterface::getExtrudedHeight(double& extrudeHeightM) const
{
  bool isExtruded = false;
  if (1 == getExtruded(isExtruded) || !isExtruded || extrudedHeight_ == 0.)
    return 1;
  extrudeHeightM = extrudedHeight_;
  return 0;
}

int GogNodeInterface::getFilledState(bool& state, osg::Vec4f& color) const
{
  // only applies to certain shapes
  if (!isFillable_(metaData_.shape))
    return 1;

  color = fillColor_;
  state = filled_;

  return 0;
}

int GogNodeInterface::getFont(std::string& fontFile, int& fontSize, osg::Vec4f& fontColor) const
{
  // only applies to label nodes
  return 1;
}

int GogNodeInterface::getDeclutterPriority(int& priority) const
{
  // only applies to label nodes
  return 1;
}

int GogNodeInterface::getLineState(bool& outlineState, osg::Vec4f& color, Utils::LineStyle& lineStyle, int& lineWidth) const
{
  // only applies to certain shapes
  if (!isLined_(metaData_.shape))
    return 1;

  outlineState = outlined_;
  color = lineColor_;
  lineStyle = Utils::LINE_SOLID;
  lineWidth = 1;

  // no line symbol loaded, just return
  if (!(style_.has<osgEarth::LineSymbol>()))
    return 0;

  const osgEarth::LineSymbol* linePtr = style_.getSymbol<osgEarth::LineSymbol>();
  lineWidth = static_cast<int>(*(linePtr->stroke()->width()));
  // now figure out line style based on the stipple value
  unsigned short stipple = *(linePtr->stroke()->stipple());
  lineStyle = Utils::getLineStyleFromStipple(stipple);
  return 0;
}

int GogNodeInterface::getPointSize(int& pointSize) const
{
  if (!style_.has<osgEarth::PointSymbol>())
    return 1;
  pointSize = static_cast<int>(*(style_.getSymbol<osgEarth::PointSymbol>()->size()));
  return 0;
}

int GogNodeInterface::getReferencePosition(osg::Vec3d& referencePosition) const
{
  return 1;
}

int GogNodeInterface::getTessellation(TessellationStyle& style) const
{
  // tessellation only applies to feature nodes
  return 1;
}

int GogNodeInterface::getTextOutline(osg::Vec4f& outlineColor, simData::TextOutline& outlineThickness) const
{
  // only applies to label nodes
  return 1;
}

int GogNodeInterface::getOpacity(float& opacity) const
{
  opacity = opacity_;
  return 0;
}

void GogNodeInterface::setAltitudeMode(AltitudeMode altMode)
{
  metaData_.setExplicitly(GOG_ALTITUDE_MODE_SET);
  if (altMode_ == altMode)
    return;
  altMode_ = altMode;
  adjustAltitude_();
}

void GogNodeInterface::setAltOffset(double altOffsetMeters)
{
  if (altOffset_ == altOffsetMeters)
    return;
  metaData_.setExplicitly(GOG_THREE_D_OFFSET_ALT_SET);
  altOffset_ = altOffsetMeters;
  adjustAltitude_();
}

void GogNodeInterface::setDepthBuffer(bool depthBuffer)
{
  metaData_.setExplicitly(GOG_DEPTH_BUFFER_SET);

  // Note that we cannot cleanly break early here because we can get out of sync
  // with style due to an early call to configure-for-clipping.

  depthBuffer_ = depthBuffer;
  // if override is in effect, nothing more to do
  if (depthBufferOverride_ != DEPTHBUFFER_IGNORE_OVERRIDE)
    return;
  style_.getOrCreate<osgEarth::RenderSymbol>()->depthTest() = depthBuffer;
  if (!depthBuffer) // unset the clip pane if depth buffer turned off
    style_.getOrCreate<osgEarth::RenderSymbol>()->clipPlane() = simVis::CLIPPLANE_VISIBLE_HORIZON;
  else
  {
    style_.getOrCreate<osgEarth::RenderSymbol>()->clipPlane().unset();
    // Explicitly remove all clip planes settings from child nodes
    if (osgNode_.valid())
    {
      RemoveModeVisitor removeClipPlane(simVis::CLIPPLANE_VISIBLE_HORIZON_GL_MODE);
      osgNode_->accept(removeClipPlane);
    }
  }

  setStyle_(style_);
}

void GogNodeInterface::setDepthBufferOverrideState(DepthBufferOverride state)
{
  if (depthBufferOverride_ == state)
    return;
  depthBufferOverride_ = state;
  bool depthBuffer = false; // default to off
  switch(state)
  {
  case DEPTHBUFFER_FORCE_ON:
    depthBuffer = true;
    break;
  case DEPTHBUFFER_FORCE_OFF:
    break;
  case DEPTHBUFFER_IGNORE_OVERRIDE: // if turning off override, revert back to the manual depth buffer state
    depthBuffer = depthBuffer_;
    break;
  }

  style_.getOrCreate<osgEarth::RenderSymbol>()->depthTest() = depthBuffer;
  if (!depthBuffer) // unset the clip pane if depth buffer turned off
    style_.getOrCreate<osgEarth::RenderSymbol>()->clipPlane() = simVis::CLIPPLANE_VISIBLE_HORIZON;
  setStyle_(style_);
}

int GogNodeInterface::setDrawState(bool draw)
{
  if (getDraw() == draw)
    return 1;
  if (!osgNode_.valid())
    return 1;
  osg::Node::NodeMask mask = (draw ? simVis::DISPLAY_MASK_GOG : simVis::DISPLAY_MASK_NONE);
  osgNode_->setNodeMask(mask);
  fireDrawChanged_();
  return 0;
}

void GogNodeInterface::setExtrude(bool extrude)
{
  metaData_.setExplicitly(GOG_EXTRUDE_SET);
  float height = static_cast<float>(extrudedHeight_);
  switch (metaData_.shape)
  {
    // no extrusion on 3d shapes
  case simVis::GOG::GOG_CYLINDER:
  case simVis::GOG::GOG_HEMISPHERE:
  case simVis::GOG::GOG_SPHERE:
  case simVis::GOG::GOG_CONE:
    return;
    // need to specify height for circular shapes
  case simVis::GOG::GOG_ARC:
  case simVis::GOG::GOG_CIRCLE:
  case simVis::GOG::GOG_ELLIPSE:
    // get the current altitude if extruded and no extrudedHeight is set
    if (extrude && height == 0.f)
    {
      osg::Vec3d position;
      getPosition(position);
      height = static_cast<double>(-position.z());
    }
    break;
  default:
    // all other shapes automatically extrude to ground as long as height is unset
    // if height is set, extrusion will be to the set height
    break;
  }

  extruded_ = extrude;

  // some shapes can only be filled if extruded
  if (fillOnlyWhenExtruded_(metaData_.shape))
  {
    // don't change the filled_ state because of extrusion, just how filled_ state is displayed, so cache the filled_ value and reassign it afer the call to setFilledState
    bool cacheFilled = filled_;
    setFilledState(extrude && filled_);
    filled_ = cacheFilled;
  }

  // Force backface culling off for extruded shapes, and on for flat shapes so we can see their backsides.
  // There is an exception for Lines/LineSegs due to extrusion to 2D instead of 3D
  const bool isLine = (metaData_.shape == GOG_LINE || metaData_.shape == GOG_LINESEGS);
  style_.getOrCreate<osgEarth::RenderSymbol>()->backfaceCulling() = extrude && !isLine;
  // In some cases it appears that extrusion can cause lighting
  style_.getOrCreate<osgEarth::RenderSymbol>()->lighting() = false;

  if (extrude)
  {
    // need to add an invisible polygon symbol here, since for some reason extrusion fills itself with the default color otherwise
    if (!filled_)
    {
      osgEarth::PolygonSymbol* polygonSymbol = style_.getOrCreate<osgEarth::PolygonSymbol>();
      polygonSymbol->fill()->color() = osg::Vec4(); // transparent
    }
    osgEarth::ExtrusionSymbol* ext = style_.getOrCreate<osgEarth::ExtrusionSymbol>();
    // set the height value if necessary, otherwise unset to allow extrude to ground
    if (height != 0.f)
      ext->height() = height;
    else
      ext->height().unset();
  }
  else
  {
    osgEarth::ExtrusionSymbol* ext = style_.getSymbol<osgEarth::ExtrusionSymbol>();
    if (ext)
     style_.removeSymbol(ext);
  }
  setStyle_(style_);

  // Need to have a polygon symbol if extruded
  if (extruded_)
    assert(style_.has<osgEarth::PolygonSymbol>());
  // Polygon symbol must be invisible if not filled
  if (extruded_ && !filled_)
    assert(style_.getSymbol<osgEarth::PolygonSymbol>()->fill()->color()[3] == 0.);

}

void GogNodeInterface::setExtrudedHeight(double extrudeHeightM)
{
  extrudedHeight_ = extrudeHeightM;
  // update extrusion to apply the new height
  setExtrude(extruded_);
}

void GogNodeInterface::setFilledState(bool state)
{
  if (!isFillable_(metaData_.shape))
    return;

  // shape is fillable, and set to be filled
  filled_ = state;

  // some shapes can only be filled if they are extruded
  if (filled_ && fillOnlyWhenExtruded_(metaData_.shape) && !extruded_)
    state = false;

  if (state)
  {
    // fill requires polygon symbol (this will mean that osgEarth treats it as a polygon)
    osgEarth::PolygonSymbol* polygonSymbol = style_.getOrCreate<osgEarth::PolygonSymbol>();
    polygonSymbol->fill()->color() = fillColor_;
    setStyle_(style_);
  }
  else if (style_.has<osgEarth::PolygonSymbol>())
  {
    // since extrusion requires an invisible polygon symbol (if not filled), make it invisible if extruded, otherwise remove the symbol altogether
    if (extruded_)
      style_.getSymbol<osgEarth::PolygonSymbol>()->fill()->color() = osg::Vec4(); // transparent
    else
      style_.remove<osgEarth::PolygonSymbol>();
    setStyle_(style_);
  }
}

void GogNodeInterface::setFillColor(const osg::Vec4f& color)
{
  if (!isFillable_(metaData_.shape))
    return;
  // cache the color
  fillColor_ = color;

  metaData_.setExplicitly(GOG_FILL_COLOR_SET);

  // if not filled, just return
  if (!filled_)
    return;

  // Pull out the polygon style to update it
  osgEarth::PolygonSymbol* poly = style_.getSymbol<osgEarth::PolygonSymbol>();
  if (!poly) // don't update color if no polygon symbol to update
    return;

  // set the new color in the PolygonSymbol
  simVis::Color colorVec(color);
  poly->fill()->color() = colorVec;
  setStyle_(style_);
}

void GogNodeInterface::setFont(const std::string& fontName, int fontSize, const osg::Vec4f& color)
{
  // NOP only applies to label nodes
}

void GogNodeInterface::setDeclutterPriority(int priority)
{
  // NOP only applies to label nodes
}

void GogNodeInterface::setLineColor(const osg::Vec4f& color)
{
  // only applies to certain shapes
  if (!isLined_(metaData_.shape))
    return;

  metaData_.setExplicitly(GOG_LINE_COLOR_SET);

  lineColor_ = color;
  // if not currently outlined, just return
  if (!outlined_)
    return;
  // update the line symbol color
  simVis::Color colorVec(color);

  if (metaData_.shape == simVis::GOG::GOG_POINTS)
    style_.getOrCreate<osgEarth::PointSymbol>()->fill() = colorVec;
  else
    style_.getOrCreate<osgEarth::LineSymbol>()->stroke()->color() = colorVec;

  setStyle_(style_);
}

void GogNodeInterface::setOutlineState(bool outlineState)
{
  if (!isLined_(metaData_.shape))
    return;

  outlined_ = outlineState;

  metaData_.setExplicitly(GOG_OUTLINE_SET);

  // turn on the outline by applying the current line color. Note that if the current line color has alpha value of 0, this will have no effect
  simVis::Color newColor = (outlineState ? simVis::Color(lineColor_) : simVis::Color(0.f, 0.f, 0.f, 0.f));

  // Points use line color, but applies to the PointSymbol
  if (metaData_.shape == simVis::GOG::GOG_POINTS)
    style_.getOrCreate<osgEarth::PointSymbol>()->fill() = newColor;
  else
    style_.getOrCreate<osgEarth::LineSymbol>()->stroke()->color() = newColor;

  setStyle_(style_);
}

void GogNodeInterface::setLineStyle(Utils::LineStyle style)
{
  // only applies to certain shapes
  if (metaData_.shape == simVis::GOG::GOG_POINTS || !isLined_(metaData_.shape))
    return;

  metaData_.setExplicitly(GOG_LINE_STYLE_SET);

  // use some default values to represent various draw styles
  unsigned short lineStyle = Utils::getStippleFromLineStyle(style);
  osgEarth::LineSymbol* lineSymbol = style_.getOrCreate<osgEarth::LineSymbol>();
  lineSymbol->stroke()->stipple() = lineStyle;
  setStyle_(style_);
}

void GogNodeInterface::setLineWidth(int lineWidth)
{
  // only applies to certain shapes
  if (metaData_.shape == simVis::GOG::GOG_POINTS || !isLined_(metaData_.shape))
    return;

  metaData_.setExplicitly(GOG_LINE_WIDTH_SET);

  osgEarth::LineSymbol* lineSymbol = style_.getOrCreate<osgEarth::LineSymbol>();
  lineSymbol->stroke()->width() = static_cast<float>(lineWidth);
  setStyle_(style_);
}

void GogNodeInterface::setPointSize(int pointSize)
{
  if (metaData_.shape != simVis::GOG::GOG_POINTS)
    return;

  metaData_.setExplicitly(GOG_POINT_SIZE_SET);

  style_.getOrCreate<osgEarth::PointSymbol>()->size() = static_cast<float>(pointSize);
  setStyle_(style_);
}

void GogNodeInterface::setTessellation(TessellationStyle style)
{
  // only feature node has tessellation
}

void GogNodeInterface::setTextOutline(const osg::Vec4f& outlineColor, simData::TextOutline outlineThickness)
{
  // NOP only applies to label nodes
}

void GogNodeInterface::setOpacity(float opacity)
{
  if (opacity == opacity_)
    return;
  opacity_ = opacity;
  if (osgNode_.valid())
    simVis::OverrideColor::setColor(osgNode_->getOrCreateStateSet(), osg::Vec4f(1.f, 1.f, 1.f, opacity_));
}

void GogNodeInterface::addGogNodeListener(GogNodeListenerPtr listener)
{
  std::vector<GogNodeListenerPtr>::iterator i = std::find(listeners_.begin(), listeners_.end(), listener);
  if (i == listeners_.end())
    listeners_.push_back(listener);
  else
    assert(0); // adding a listener more than once
}

void GogNodeInterface::removeGogNodeListener(GogNodeListenerPtr listener)
{
  std::vector<GogNodeListenerPtr>::iterator i = std::find(listeners_.begin(), listeners_.end(), listener);
  if (i != listeners_.end())
    listeners_.erase(i);
}

void GogNodeInterface::fireDrawChanged_() const
{
  for (std::vector<GogNodeListenerPtr>::const_iterator iter = listeners_.begin(); iter != listeners_.end(); ++iter)
  {
    (*iter)->drawChanged(this);
  }
}

void GogNodeInterface::setGeoPositionAltitude_(osgEarth::GeoPositionNode& node, double altitudeAdjustment)
{
  bool extrude = false;
  osgEarth::AltitudeMode mode = osgEarth::ALTMODE_ABSOLUTE;
  double altitude = altitude_ + altOffset_ + altitudeAdjustment;

  switch (altMode_)
  {
  case ALTITUDE_NONE:
    break;
  case ALTITUDE_GROUND_RELATIVE:
    mode = osgEarth::ALTMODE_RELATIVE;
    break;
  case ALTITUDE_GROUND_CLAMPED:
    mode = osgEarth::ALTMODE_RELATIVE;
    altitude = altitudeAdjustment;
    break;
  case ALTITUDE_EXTRUDE:
    mode = osgEarth::ALTMODE_RELATIVE;
    extrude = true;
    break;
  }

  // hosted shape must apply altitude to the attitude transform
  if (!hasMapNode_)
  {
    osg::Vec3d hostPos = node.getPositionAttitudeTransform()->getPosition();
    hostPos.z() = altitude;
    node.getPositionAttitudeTransform()->setPosition(hostPos);
  }
  else // geo nodes apply altitude directly to the node's position
  {
    osgEarth::GeoPoint pos = node.getPosition();
    pos.altitudeMode() = mode;
    pos.alt() = altitude;
    node.setPosition(pos);
  }
  // make sure to update any extrusion
  setExtrude(extrude);
}

void GogNodeInterface::initializeFromGeoPositionNode_(const osgEarth::GeoPositionNode& node)
{
  hasMapNode_ = (node.getMapNode() != nullptr);
  // use node position if there is a map node
  if (hasMapNode_)
    altitude_ = node.getPosition().alt();
  else // otherwise, use the attitude transform position
    altitude_ = node.getPositionAttitudeTransform()->getPosition().z();
}

bool GogNodeInterface::hasValidAltitudeMode() const
{
  // The combinations here match those applied in the setAltitudeMode or initializeAltitudeSymbol_ methods, since those are the known valid combinations.

  // check for altitude mode ALTITUDE_EXTRUDE
  if (style_.has<osgEarth::ExtrusionSymbol>())
    return true;

  // check for an AltitudeSymbol
  const osgEarth::AltitudeSymbol* alt = style_.getSymbol<osgEarth::AltitudeSymbol>();
  if (alt == nullptr)
    return false;

  // check for altitude mode ALTITUDE_NONE
  if (alt->clamping() == osgEarth::AltitudeSymbol::CLAMP_NONE &&
    alt->technique() == osgEarth::AltitudeSymbol::TECHNIQUE_GPU)
    return true;
  // check for altitude mode ALTITUDE_GROUND_CLAMPED
  if (alt->clamping() == osgEarth::AltitudeSymbol::CLAMP_TO_TERRAIN &&
    alt->technique() == osgEarth::AltitudeSymbol::TECHNIQUE_SCENE)
    return true;
  // check for altitude mode ALTITUDE_GROUND_RELATIVE
  if (alt->clamping() == osgEarth::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN &&
    alt->technique() == osgEarth::AltitudeSymbol::TECHNIQUE_SCENE)
    return true;

  return false;
}

void GogNodeInterface::applyBackfaceCulling()
{
  // 3D shapes and extruded shapes get backface culling; otherwise turn it off so we can see both sides.
  // Note that extruded lines are the only extruded symbol that need backface culling off (because it
  // extrudes to a filled polygon instead of a 3D shape).

  bool isClosed3dShape = (shape() == GOG_SPHERE || shape() == GOG_ELLIPSOID || shape() == GOG_CYLINDER || shape() == GOG_LATLONALTBOX || shape() == GOG_CONE);

  // Semitransparent hemispheres without depth buffer need backface culling on else odd artifacts show through
  if (shape() == GOG_HEMISPHERE && fillColor_.a() < 1.0 && !depthBuffer_)
    isClosed3dShape = true;

  const bool isLine = (shape() == GOG_LINE || shape() == GOG_LINESEGS);
  if (isClosed3dShape || (extruded_ && !isLine))
    style_.getOrCreateSymbol<osgEarth::RenderSymbol>()->backfaceCulling() = true;
  else
    style_.getOrCreateSymbol<osgEarth::RenderSymbol>()->backfaceCulling() = false;

  setStyle_(style_);
}
bool GogNodeInterface::getMetaDataFlag_(const std::string& flag, std::string& metaData)
{
  size_t keywordIndex = metaData.find(flag);
  if (keywordIndex == std::string::npos)
    return false;
  // erase our flag keyword
  metaData.erase(keywordIndex, flag.size());
  return true;
}

void GogNodeInterface::initializeAltitudeSymbol_()
{
  osgEarth::AltitudeSymbol* alt = style_.getOrCreate<osgEarth::AltitudeSymbol>();
  alt->clamping() = osgEarth::AltitudeSymbol::CLAMP_TO_TERRAIN;
  alt->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_SCENE;
  alt->binding() = osgEarth::AltitudeSymbol::BINDING_VERTEX;
  setStyle_(style_);
}

void GogNodeInterface::initializeFillColor_()
{
  if (style_.has<osgEarth::PolygonSymbol>())
    fillColor_ = style_.getSymbol<osgEarth::PolygonSymbol>()->fill()->color();
  else
    fillColor_ = simVis::Color::Red; // default to red
}

void GogNodeInterface::initializeLineColor_()
{
  if (style_.has<osgEarth::LineSymbol>())
    lineColor_ = style_.getSymbol<osgEarth::LineSymbol>()->stroke()->color();
  else
    lineColor_ = simVis::Color::Red; // default to red
}

bool GogNodeInterface::isFillable_(simVis::GOG::GogShape shape) const
{
  switch (shape)
  {
  case simVis::GOG::GOG_ARC:
  case simVis::GOG::GOG_CIRCLE:
  case simVis::GOG::GOG_CYLINDER:
  case simVis::GOG::GOG_ELLIPSE:
  case simVis::GOG::GOG_LATLONALTBOX:
  case simVis::GOG::GOG_LINE:
  case simVis::GOG::GOG_LINESEGS:
  case simVis::GOG::GOG_POINTS:
  case simVis::GOG::GOG_POLYGON:
  case simVis::GOG::GOG_CONE:
  case simVis::GOG::GOG_ORBIT:
    return true;
  default:
    break;
  }
  return false;
}

bool GogNodeInterface::isLined_(simVis::GOG::GogShape shape) const
{
  switch (shape)
  {
  case simVis::GOG::GOG_ARC:
  case simVis::GOG::GOG_CIRCLE:
  case simVis::GOG::GOG_CYLINDER:
  case simVis::GOG::GOG_ELLIPSE:
  case simVis::GOG::GOG_LATLONALTBOX:
  case simVis::GOG::GOG_LINE:
  case simVis::GOG::GOG_LINESEGS:
  case simVis::GOG::GOG_POINTS:
  case simVis::GOG::GOG_POLYGON:
  case simVis::GOG::GOG_ORBIT:
    return true;
  default:
    break;
  }
  return false;
}

bool GogNodeInterface::fillOnlyWhenExtruded_(simVis::GOG::GogShape shape) const
{
  switch (shape)
  {
  case simVis::GOG::GOG_LINE:
  case simVis::GOG::GOG_POINTS:
    return true;
  default:
    break;
  }
  return false;
}

const osgEarth::Style& GogNodeInterface::getStyle_() const
{
  return style_;
}

void GogNodeInterface::beginStyleUpdates_()
{
  // Avoid nested begin/ends
  assert(!deferringStyleUpdate_);
  deferringStyleUpdate_ = true;
}

void GogNodeInterface::endStyleUpdates_()
{
  // Avoid nested begin/ends
  assert(deferringStyleUpdate_);
  deferringStyleUpdate_ = false;
}

bool GogNodeInterface::deferringStyleUpdates_() const
{
  return deferringStyleUpdate_;
}

void GogNodeInterface::serializeKeyword_(std::ostream& gogOutputStream) const
{
  gogOutputStream << simVis::GOG::Parser::getKeywordFromShape(metaData_.shape) << "\n";
}

///////////////////////////////////////////////////////////////////

AnnotationNodeInterface::AnnotationNodeInterface(osgEarth::AnnotationNode* annotationNode, const simVis::GOG::GogMetaData& metaData)
  : GogNodeInterface(annotationNode, metaData),
  annotationNode_(annotationNode)
{
  if (annotationNode_.valid())
    style_ = annotationNode_->getStyle();

  initializeFillColor_();
  initializeLineColor_();
}

AnnotationNodeInterface::~AnnotationNodeInterface()
{
}

int AnnotationNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  if (!annotationNode_.valid())
    return 1;

  // Convert ecef position to lla
  simCore::CoordinateConverter cc;
  osg::Vec3f ecefPos = annotationNode_->getBound().center();
  const simCore::Coordinate ecefCoord(simCore::COORD_SYS_ECEF, simCore::Vec3(ecefPos.x(), ecefPos.y(), ecefPos.z()));
  simCore::Coordinate llaCoord;
  cc.convert(ecefCoord, llaCoord, simCore::COORD_SYS_LLA);

  // Convert lat and lon from radians to degrees, swap lat and lon to match osg system
  simCore::Vec3 llaPos = llaCoord.position();
  position = osg::Vec3d(llaPos.lon() * simCore::RAD2DEG, llaPos.lat() * simCore::RAD2DEG, llaPos.alt());
  return 0;
}

void AnnotationNodeInterface::adjustAltitude_()
{
  // No-op.  AnnotationNodeInterface is a "best attempt" when loading an unrecognized osg node.
}

void AnnotationNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  // No-op.  AnnotationNodeInterface is a "best attempt" when loading an unrecognized osg node.  Can't serialize generically
}

void AnnotationNodeInterface::setStyle_(const osgEarth::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (annotationNode_.valid())
    annotationNode_->setStyle(style);
}


///////////////////////////////////////////////////////////////////

FeatureNodeInterface::FeatureNodeInterface(osgEarth::FeatureNode* featureNode, const simVis::GOG::GogMetaData& metaData)
  : GogNodeInterface(featureNode, metaData),
    featureNode_(featureNode)
{
  init_();
}

FeatureNodeInterface::FeatureNodeInterface(osg::Group* node, osgEarth::FeatureNode* featureNode, const simVis::GOG::GogMetaData& metaData)
  : GogNodeInterface(node, metaData),
    featureNode_(featureNode)
{
  init_();
}

int FeatureNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  if (!featureNode_.valid() || !featureNode_->getFeature() || !featureNode_->getFeature()->getGeometry())
    return 1;

  // initialize bounding values
  osg::Vec3d lowerBound(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 0.0);
  osg::Vec3d upperBound(-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), 0.0);

  osgEarth::Geometry* geometry = featureNode_->getFeature()->getGeometry();
  std::vector<osg::Vec3d> points;
  Utils::getGeometryPoints(geometry, points);

  // no points found return failure
  if (points.empty())
    return 1;

  // calculate the corners of the bounding box
  for (std::vector<osg::Vec3d>::const_iterator iter = points.begin(); iter != points.end(); ++iter)
  {
    lowerBound.x() = simCore::sdkMin(lowerBound.x(), iter->x());
    upperBound.x() = simCore::sdkMax(upperBound.x(), iter->x());
    lowerBound.y() = simCore::sdkMin(lowerBound.y(), iter->y());
    upperBound.y() = simCore::sdkMax(upperBound.y(), iter->y());
  }

  const double minX = lowerBound.x();
  const double maxX = upperBound.x();
  const double minY = lowerBound.y();
  const double maxY = upperBound.y();

  // assign the center of the bounding box
  position.x() = minX + (maxX - minX) / 2;
  position.y() = minY + (maxY - minY) / 2;
  position.z() = 0.0;

  /* Account for crossing the date line.  Assume a gog does not span more than half the earth */
  if ((maxX - minX) > 180.0)
  {
    double offset = (- minX - maxX)/2;
    if (offset > 0.0)
    {
      position.x() = 180.0 - offset; // Mid point is on positive side of date line
    }
    else
    {
      position.x() = -180.0 - offset; // Mid point is on negative side of date line
    }
  }

  return 0;
}

int FeatureNodeInterface::getTessellation(TessellationStyle& tessellation) const
{
  switch (metaData_.shape)
  {
  case simVis::GOG::GOG_CYLINDER:
  case simVis::GOG::GOG_CIRCLE:
  case simVis::GOG::GOG_ELLIPSE:
  case simVis::GOG::GOG_ARC:
    return 1;
  default:
    break;
  }
  if (!style_.has<osgEarth::LineSymbol>())
    return 1;
  const osgEarth::LineSymbol* ls = style_.getSymbol<osgEarth::LineSymbol>();

  if (!ls->tessellationSize().isSet() || *(ls->tessellationSize()) == 0)
  {
    tessellation = TESSELLATE_NONE;
    return 0;
  }
  switch (*(featureNode_->getFeature()->geoInterp()))
  {
  case osgEarth::GEOINTERP_RHUMB_LINE:
    tessellation = TESSELLATE_RHUMBLINE;
    break;
  case osgEarth::GEOINTERP_GREAT_CIRCLE:
    tessellation = TESSELLATE_GREAT_CIRCLE_PROJECTION;
    break;
  default:
    tessellation = TESSELLATE_NONE;
    break;
  }
  return 0;
}

void FeatureNodeInterface::setAltOffset(double altOffsetMeters)
{
  if (altOffsetMeters == altOffset_ || !featureNode_.valid())
    return;

  osgEarth::Geometry* geometry = featureNode_->getFeature()->getGeometry();
  if (!geometry)
    return;
  if (geometry->size() != originalAltitude_.size())
  {
    assert(0); // somehow our vector of original altitude values is out of synch with the geometry
    return;
  }
  metaData_.setExplicitly(GOG_THREE_D_OFFSET_ALT_SET);
  altOffset_ = altOffsetMeters;
  // now apply the altitude offset to all of our position points
  for (size_t i = 0; i < geometry->size(); ++i)
  {
    (*geometry)[i].z() = originalAltitude_.at(i) + altOffsetMeters;
  }
  featureNode_->dirty();
}

void FeatureNodeInterface::setExtrude(bool extrude)
{
  if (extrude)
  {
    osgEarth::AltitudeSymbol* alt = style_.getOrCreate<osgEarth::AltitudeSymbol>();
    alt->clamping() = osgEarth::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;
    alt->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_SCENE;
  }
  else
  {
    osgEarth::AltitudeSymbol* alt = style_.getOrCreate<osgEarth::AltitudeSymbol>();
    if (alt)
      alt->clamping() = osgEarth::AltitudeSymbol::CLAMP_NONE;
  }
  GogNodeInterface::setExtrude(extrude);
}

void FeatureNodeInterface::setTessellation(TessellationStyle style)
{
  metaData_.setExplicitly(GOG_TESSELLATE_SET);
  metaData_.setExplicitly(GOG_LINE_PROJECTION_SET);

  // tessellation causes these shapes to freeze the system, just avoid them for now
  switch (metaData_.shape)
  {
  case simVis::GOG::GOG_CYLINDER:
  case simVis::GOG::GOG_CIRCLE:
  case simVis::GOG::GOG_ELLIPSE:
  case simVis::GOG::GOG_ARC:
    return;
  default:
    break;
  }
  // tessellation only applies to Features
  bool tessellate = true;
  if (style == TESSELLATE_RHUMBLINE)
    featureNode_->getFeature()->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;
  else if (style == TESSELLATE_GREAT_CIRCLE_PROJECTION)
    featureNode_->getFeature()->geoInterp() = osgEarth::GEOINTERP_GREAT_CIRCLE;
  else if (style == TESSELLATE_NONE)
    tessellate = false;

  if (!style_.has<osgEarth::LineSymbol>())
    return;
  osgEarth::LineSymbol* ls = style_.getSymbol<osgEarth::LineSymbol>();

  if (tessellate)
  {
    // need to unset any tessellation value, so that the tessellationSize attribute takes precedence
    ls->tessellation().unset();
    // set default tessellation spacing in meters, functions as a minimum for large features
    double tessellationSpacingM = 10000.0;

    // adjust tessellation based on feature dimension
    const osgEarth::Feature* feature = featureNode_->getFeature();
    if (feature != nullptr)
    {
      const osgEarth::SpatialReference* srs = feature->getSRS();
      if (srs != nullptr)
      {
        osg::BoundingSphered boundS;
        if (feature->getWorldBound(srs->getGeocentricSRS(), boundS))
        {
          // ensure a minimum of 50m spacing, otherwise approximately 80 posts along major dimension of feature
          tessellationSpacingM = simCore::sdkMax(50.0, simCore::sdkMin(tessellationSpacingM, 0.025 * boundS.radius()));
        }
      }
    }
    ls->tessellationSize()->set(tessellationSpacingM, osgEarth::Units::METERS); // in meters
  }
  else
  {
    // need to set tessellation to 0, otherwise a default tessellation value in LineSymbol will be assumed
    ls->tessellation() = 0;
    // make sure the tessellation size is unset
    ls->tessellationSize().unset();
  }

  setStyle_(style_);
}

void FeatureNodeInterface::setAltitudeMode(AltitudeMode altMode)
{
  metaData_.setExplicitly(GOG_ALTITUDE_MODE_SET);
  if (altMode_ == altMode)
    return;
  altMode_ = altMode;
  // The altitude mode combinations applied here should match those in the hasValidAltitudeMode() method. Update both methods with changes.
  setExtrude(altMode == ALTITUDE_EXTRUDE);

  if (style_.has<osgEarth::ExtrusionSymbol>())
  {
    // Assertion failure means failure in setExtrude()
    assert(altMode == ALTITUDE_EXTRUDE);
    return;
  }

  switch (altMode)
  {
  case ALTITUDE_NONE:
    style_.getOrCreate<osgEarth::AltitudeSymbol>()->clamping() = osgEarth::AltitudeSymbol::CLAMP_NONE;
    style_.getOrCreate<osgEarth::AltitudeSymbol>()->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_GPU;
    break;
  case ALTITUDE_GROUND_CLAMPED:
    style_.getOrCreate<osgEarth::AltitudeSymbol>()->clamping() = osgEarth::AltitudeSymbol::CLAMP_TO_TERRAIN;
    style_.getOrCreate<osgEarth::AltitudeSymbol>()->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_SCENE;
    style_.getOrCreate<osgEarth::RenderSymbol>()->depthOffset()->automatic() = true;
    break;
  case ALTITUDE_GROUND_RELATIVE:
    style_.getOrCreate<osgEarth::AltitudeSymbol>()->clamping() = osgEarth::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;
    style_.getOrCreate<osgEarth::AltitudeSymbol>()->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_SCENE;
    style_.getOrCreate<osgEarth::RenderSymbol>()->depthOffset()->automatic() = true;
    break;
  case ALTITUDE_EXTRUDE:
    // Shouldn't get here; failure in setExtrude()
    assert(0);
    // SIM-5324 - extrude should not be a choosable option for shapes that do not support extrusion
    break;
  }
  setStyle_(style_);
}

void FeatureNodeInterface::adjustAltitude_()
{
  // No-op in feature node
}

void FeatureNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  osgEarth::Geometry* geometry = featureNode_->getFeature()->getGeometry();
  if (!geometry)
    return;

  if (geometry->size() != originalAltitude_.size())
  {
    assert(0); // somehow our vector of original altitude values is out of synch with the geometry
    return;
  }

  // since we may have applied an altitude offset, get the original altitude values before serializing
  osgEarth::Geometry originalGeometry = *geometry;
  for (size_t i = 0; i < originalGeometry.size(); ++i)
  {
    originalGeometry[i].z() = originalAltitude_.at(i);
  }
  Utils::serializeShapeGeometry(&originalGeometry, relativeShape, gogOutputStream);
}

void FeatureNodeInterface::setStyle_(const osgEarth::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (!deferringStyleUpdates_() && featureNode_.valid())
  {
    featureNode_->setStyle(style_);
    featureNode_->getFeature()->style() = style_;
    featureNode_->dirty();
  }
}

void FeatureNodeInterface::init_()
{
  if (featureNode_.valid() && featureNode_->getFeature())
  {
    style_ = *(featureNode_->getFeature()->style());
    hasMapNode_ = true; // feature nodes always have a map node
    // initialize our original altitudes
    const osgEarth::Geometry* geometry = featureNode_->getFeature()->getGeometry();
    for (size_t i = 0; i < geometry->size(); ++i)
    {
      originalAltitude_.push_back((*geometry)[i].z());
    }
  }
  initializeFillColor_();
  initializeLineColor_();
}

///////////////////////////////////////////////////////////////////

LocalGeometryNodeInterface::LocalGeometryNodeInterface(osgEarth::LocalGeometryNode* localNode, const simVis::GOG::GogMetaData& metaData)
  : GogNodeInterface(localNode, metaData),
    localNode_(localNode)
{
  if (localNode_.valid())
  {
    initializeFromGeoPositionNode_(*localNode);
    style_ = localNode_->getStyle();
    initializeAltitudeSymbol_();
  }

  initializeFillColor_();
  initializeLineColor_();
}

int LocalGeometryNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  bool useLocalOffset = true;
  // line/poly shapes do not use the local offset
  switch (metaData_.shape)
  {
  case simVis::GOG::GOG_POINTS:
  case simVis::GOG::GOG_POLYGON:
  case simVis::GOG::GOG_LINE:
  case simVis::GOG::GOG_LINESEGS:
    useLocalOffset = false;
    break;
  default:
    break;
  }

  return findLocalGeometryPosition(localNode_.get(), referencePosition, position, useLocalOffset);
}

int LocalGeometryNodeInterface::getReferencePosition(osg::Vec3d& referencePosition) const
{
  if (!localNode_.valid())
    return 1;
  const osgEarth::GeoPoint& refPoint = localNode_->getPosition();
  referencePosition.x() = refPoint.x(); // note this is lon
  referencePosition.y() = refPoint.y(); // note this is lat
  referencePosition.z() = altitude_; // always use original altitude, since an altitude offset may have been applied
  return 0;
}

void LocalGeometryNodeInterface::adjustAltitude_()
{
  if (localNode_.valid())
    setGeoPositionAltitude_(*localNode_.get(), 0.);
}

void LocalGeometryNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  const osgEarth::Geometry* geometry = localNode_->getGeometry();
  if (geometry)
    Utils::serializeShapeGeometry(geometry, relativeShape, gogOutputStream);
}

void LocalGeometryNodeInterface::setStyle_(const osgEarth::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (deferringStyleUpdates_())
    return;
  if (localNode_.valid())
    localNode_->setStyle(style_);
}

///////////////////////////////////////////////////////////////////

LabelNodeInterface::LabelNodeInterface(osgEarth::GeoPositionNode* labelNode, const simVis::GOG::GogMetaData& metaData)
  : GogNodeInterface(labelNode, metaData),
    labelNode_(labelNode),
    outlineThickness_(simData::TO_THIN)
{
  if (labelNode_.valid())
  {
    style_ = labelNode_->getStyle();
    initializeFromGeoPositionNode_(*labelNode);
    initializeAltitudeSymbol_();
  }
  initializeFillColor_();
  initializeLineColor_();

  // override - labels should not be flattened in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(false, labelNode);
}

int LabelNodeInterface::getFont(std::string& fontFile, int& fontSize, osg::Vec4f& fontColor) const
{
  if (!style_.has<osgEarth::TextSymbol>())
    return 1;
  const osgEarth::TextSymbol* ts = style_.getSymbol<osgEarth::TextSymbol>();
  if (ts->font()->size() > 0)
    fontFile = *(ts->font());
  fontSize = static_cast<int>(simCore::round(simVis::simdisFontSize(static_cast<float>(ts->size()->eval()))));
  fontColor = ts->fill()->color();
  return 0;
}

int LabelNodeInterface::getDeclutterPriority(int& priority) const
{
  priority = -1;
  if (!style_.has<osgEarth::TextSymbol>())
    return 1;
  const osgEarth::TextSymbol* ts = style_.getSymbol<osgEarth::TextSymbol>();
  if (ts->declutter().get())
    priority = ts->priority().get().eval();
  return 0;
}

int LabelNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  return findLocalGeometryPosition(labelNode_.get(), referencePosition, position, true);
}

int LabelNodeInterface::getTextOutline(osg::Vec4f& outlineColor, simData::TextOutline& outlineThickness) const
{
  outlineColor = outlineColor_;
  outlineThickness = outlineThickness_;
  return 0;
}

void LabelNodeInterface::setFont(const std::string& fontName, int fontSize, const osg::Vec4f& color)
{
  osgEarth::TextSymbol* ts = style_.getOrCreate<osgEarth::TextSymbol>();
  if (!ts)
    return;

  std::string fileFullPath = simVis::Registry::instance()->findFontFile(fontName);
  float osgFontSize = simVis::osgFontSize(static_cast<float>(fontSize));
  simVis::Color colorVec(color);

  if (ts->font() != fileFullPath)
    metaData_.setExplicitly(GOG_FONT_NAME_SET);
  if (ts->size().value().eval() != osgFontSize)
    metaData_.setExplicitly(GOG_FONT_SIZE_SET);
  if (ts->fill()->color() != colorVec)
    metaData_.setExplicitly(GOG_LINE_COLOR_SET);

  if (!fileFullPath.empty())
    ts->font() = fileFullPath;
  ts->size() = simVis::osgFontSize(static_cast<float>(fontSize));
  ts->fill()->color() = colorVec;
  setStyle_(style_);
}

void LabelNodeInterface::setDeclutterPriority(int priority)
{
  osgEarth::TextSymbol* ts = style_.getOrCreate<osgEarth::TextSymbol>();
  if (!ts)
    return;
  if (priority < 0)
  {
    ts->declutter() = false;
    ts->priority().clear();
  }
  else
  {
    ts->declutter() = true;
    ts->priority() = priority;
  }
  setStyle_(style_);
}

void LabelNodeInterface::setTextOutline(const osg::Vec4f& outlineColor, simData::TextOutline outlineThickness)
{
  osgEarth::TextSymbol* ts = style_.getOrCreate<osgEarth::TextSymbol>();
  if (!ts)
    return;

  // Check whether color or thickness have changed for serialization
  if (outlineColor_ != outlineColor)
    metaData_.setExplicitly(GOG_TEXT_OUTLINE_COLOR_SET);

  if (outlineThickness_ != outlineThickness)
    metaData_.setExplicitly(GOG_TEXT_OUTLINE_THICKNESS_SET);

  outlineColor_ = outlineColor;
  outlineThickness_ = outlineThickness;
  ts->halo()->color() = outlineColor_;

  ts->haloOffset() = simVis::outlineThickness(outlineThickness);
  // Backdrop type must be set to none when outline thickness is none to avoid artifacts
  ts->haloBackdropType() = (outlineThickness == simData::TO_NONE ? osgText::Text::NONE : osgText::Text::OUTLINE);

  setStyle_(style_);
}

void LabelNodeInterface::adjustAltitude_()
{
  if (labelNode_.valid())
    setGeoPositionAltitude_(*labelNode_.get(), 0.);
}

void LabelNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  // nothing to do, labels don't serialize geometry
}

void LabelNodeInterface::serializeKeyword_(std::ostream& gogOutputStream) const
{
  // nothing to do, labels include the keyword in their text value element
}

void LabelNodeInterface::setStyle_(const osgEarth::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (deferringStyleUpdates_())
    return;
  if (labelNode_.valid())
    labelNode_->setStyle(style_);
}

//////////////////////////////////////////////

CylinderNodeInterface::CylinderNodeInterface(osg::Group* groupNode, osgEarth::LocalGeometryNode* sideNode, osgEarth::LocalGeometryNode* topCapNode, osgEarth::LocalGeometryNode* bottomCapNode, const simVis::GOG::GogMetaData& metaData)
  : GogNodeInterface(groupNode, metaData),
    sideNode_(sideNode),
    topCapNode_(topCapNode),
    bottomCapNode_(bottomCapNode),
    height_(0.0)
{
  // height is from the side node's extrusion height, altitude is from side node's altitude
  if (sideNode_.valid() && sideNode_->getStyle().has<osgEarth::ExtrusionSymbol>())
  {
    height_ = sideNode_->getStyle().getSymbol<osgEarth::ExtrusionSymbol>()->height().value();
    initializeFromGeoPositionNode_(*sideNode);
  }

  if (topCapNode_.valid())
  {
    // use style of the cap, since that will contain the line and other style options that matters for updating the characteristics of the cylinder
    style_ = topCapNode_->getStyle();
    // fill state is determined by the cap node's fill state
    filled_ = topCapNode_->getStyle().has<osgEarth::PolygonSymbol>();
    initializeAltitudeSymbol_();
  }

  initializeFillColor_();
  initializeLineColor_();
}

CylinderNodeInterface::~CylinderNodeInterface()
{
}

int CylinderNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  return findLocalGeometryPosition(sideNode_.get(), referencePosition, position, true);
}

void CylinderNodeInterface::setAltitudeMode(AltitudeMode altMode)
{
  // cylinder doesn't support extrusion
  if (altMode == ALTITUDE_EXTRUDE)
    return;

  GogNodeInterface::setAltitudeMode(altMode);
}

void CylinderNodeInterface::adjustAltitude_()
{
  if (topCapNode_.valid())
    setGeoPositionAltitude_(*topCapNode_.get(), height_);
  if (sideNode_.valid())
    setGeoPositionAltitude_(*sideNode_.get(), 0.);
  if (bottomCapNode_.valid())
    setGeoPositionAltitude_(*bottomCapNode_.get(), 0.);
}

void CylinderNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  // Cylinder can't serialize its geometry, serialization of center, radius, height is stored in the meta data
}

void CylinderNodeInterface::setStyle_(const osgEarth::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (deferringStyleUpdates_())
    return;
  // no updates if our two nodes are not valid
  if (!sideNode_.valid() || !topCapNode_.valid() || !bottomCapNode_.valid())
    return;

  // format style for the side node
  osgEarth::Style sideStyle = style_;
  // need to add the extrusion symbol to the side style
  sideStyle.getOrCreate<osgEarth::ExtrusionSymbol>()->height() = height_;
  // In some cases it appears that extrusion can cause lighting
  sideStyle.getOrCreate<osgEarth::RenderSymbol>()->lighting() = false;
  // need to remove the line symbol from the side style
  sideStyle.remove<osgEarth::LineSymbol>();
  // if not filled, need to make sure the side node has a fill color that matches the line color
  if (!filled_ && style_.has<osgEarth::LineSymbol>())
    sideStyle.getOrCreate<osgEarth::PolygonSymbol>()->fill()->color() = style_.getSymbol<osgEarth::LineSymbol>()->stroke()->color();

  // If we are filled, then side's backface culling should be true; if unfilled, then it should be set false
  if (filled_)
    sideStyle.getOrCreate<osgEarth::RenderSymbol>()->backfaceCulling() = true;
  else
    sideStyle.getOrCreate<osgEarth::RenderSymbol>()->backfaceCulling() = false;
  sideNode_->setStyle(sideStyle);

  // can't have an extrusion symbol for the cap nodes
  style_.remove<osgEarth::ExtrusionSymbol>();
  topCapNode_->setStyle(style_);
  bottomCapNode_->setStyle(style_);
}

//////////////////////////////////////////////

ArcNodeInterface::ArcNodeInterface(osg::Group* groupNode, osgEarth::LocalGeometryNode* shapeNode, osgEarth::LocalGeometryNode* fillNode, const simVis::GOG::GogMetaData& metaData)
  : GogNodeInterface(groupNode, metaData),
    shapeNode_(shapeNode),
    fillNode_(fillNode)
{
  if (shapeNode_.valid())
    initializeFromGeoPositionNode_(*shapeNode);

  style_ = shapeNode_->getStyle();

  initializeAltitudeSymbol_();
  initializeLineColor_();

  osg::Node::NodeMask mask = simVis::DISPLAY_MASK_NONE;
  if (fillNode_.valid())
  {
    if (fillNode_->getStyle().has<osgEarth::PolygonSymbol>())
    {
      fillColor_ = fillNode_->getStyle().getSymbol<osgEarth::PolygonSymbol>()->fill()->color();
      if (fillColor_[3] > 0.f)
        mask = simVis::DISPLAY_MASK_GOG;
    }
    fillNode_->setNodeMask(mask);
  }
}

int ArcNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  return findLocalGeometryPosition(shapeNode_.get(), referencePosition, position, true);
}

void ArcNodeInterface::adjustAltitude_()
{
  if (shapeNode_.valid())
    setGeoPositionAltitude_(*shapeNode_.get(), 0.);
  if (fillNode_.valid())
    setGeoPositionAltitude_(*fillNode_.get(), 0.);
}

void ArcNodeInterface::setFilledState(bool state)
{
  GogNodeInterface::setFilledState(state);
  if (!fillNode_.valid())
    return;
  fillNode_->setNodeMask(state ? simVis::DISPLAY_MASK_GOG : simVis::DISPLAY_MASK_NONE);
  // the arc's fill node has some problems keeping up with elevation data, so just reset the position when changing fill state to jog its memory
  fillNode_->setPosition(fillNode_->getPosition());
}

void ArcNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  // Arc can't serialize its geometry, serialization for center and radius is stored in the meta data
}

void ArcNodeInterface::setStyle_(const osgEarth::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (deferringStyleUpdates_())
    return;
  // no updates if our shape node is not valid
  if (!shapeNode_.valid())
    return;

  // format style for the shape node
  osgEarth::Style shapeStyle = style_;

  // shape node must remove the PolygonSymbol if not extruded
  if (shapeStyle.has<osgEarth::PolygonSymbol>() && !extruded_)
    shapeStyle.remove<osgEarth::PolygonSymbol>();

  // shape node should not have a polygon symbol if not extruded
  if (!extruded_)
    assert(!shapeStyle.has<osgEarth::PolygonSymbol>());

  shapeNode_->setStyle(shapeStyle);

  if (!fillNode_.valid())
    return;

  // can't have a line symbol for the fill node
  osgEarth::Style fillStyle = style_;
  fillStyle.remove<osgEarth::LineSymbol>();
  fillNode_->setStyle(fillStyle);
}

SphericalNodeInterface::SphericalNodeInterface(osgEarth::LocalGeometryNode* localNode, const simVis::GOG::GogMetaData& metaData)
  : LocalGeometryNodeInterface(localNode, metaData)
{
}

int SphericalNodeInterface::getFilledState(bool& state, osg::Vec4f& color) const
{
  state = filled_;
  color = fillColor_;
  return 0;
}

int SphericalNodeInterface::getLineState(bool& outlineState, osg::Vec4f& color, Utils::LineStyle& lineStyle, int& lineWidth) const
{
  outlineState = false; // has no concept of outline state
  color = lineColor_;
  return 0;
}

void SphericalNodeInterface::setFillColor(const osg::Vec4f& color)
{
  metaData_.setExplicitly(GOG_FILL_COLOR_SET);
  fillColor_ = color;
  if (filled_) // update color if filled
    setColor_(color);
}

void SphericalNodeInterface::setFilledState(bool state)
{
  if (filled_ == state)
    return;
  filled_ = state;
  if (filled_) // update color if now filled
    setColor_(fillColor_);
  else // update color with line color, since no longer filled
    setColor_(lineColor_);
}

void SphericalNodeInterface::setLineColor(const osg::Vec4f& color)
{
  metaData_.setExplicitly(GOG_LINE_COLOR_SET);
  lineColor_ = color;
  if (!filled_) // update color if not filled
    setColor_(color);
}

void SphericalNodeInterface::setColor_(const osg::Vec4f& color)
{
  // need to dig down into the LocalGeometryNode to get the underlying Geometry object to set its color array
  // NOTE: this assumes a specific implementation for spherical nodes. May fail if that implementation changes
  osg::Group* group = localNode_->getPositionAttitudeTransform();
  osg::Node* node = group->getNumChildren() > 0 ? group->getChild(0) : nullptr;
  if (!node)
    return;
  osg::Geode* geode = node->asGeode();
  if (!geode)
    return;
  osg::Drawable* drawable = geode->getDrawable(0);
  if (!drawable)
    return;
  osg::Geometry* geometry = drawable->asGeometry();
  if (!geometry)
    return;

  // set the color array, logic taken from osgEarthAnnotation's AnnotationUtils::createSphere(/hemisphere/ellipsoid)
  osg::Vec4Array* colorArray = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
  (*colorArray)[0] = color;
  geometry->setColorArray(colorArray);
}

void SphericalNodeInterface::setStyle_(const osgEarth::Style& style)
{
  LocalGeometryNodeInterface::setStyle_(style);
  if (deferringStyleUpdates_() || !localNode_)
    return;

  // Find the internal node
  osg::Group* group = localNode_->getPositionAttitudeTransform();
  osg::Node* node = group->getNumChildren() > 0 ? group->getChild(0) : nullptr;
  if (!node)
    return;

  // Pull out the render symbol
  const osgEarth::RenderSymbol* render = style_.get<osgEarth::RenderSymbol>();
  if (!render)
    return;

  // Subset of osgEarth applyRenderSymbology() supported out of the box

  if (render->depthTest().isSet())
  {
    node->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,
      (render->depthTest().get() ? osg::StateAttribute::ON : osg::StateAttribute::OFF) | osg::StateAttribute::OVERRIDE);
  }

  if (render->lighting().isSet())
  {
    simVis::setLighting(node->getOrCreateStateSet(),
      (render->lighting().get() ? osg::StateAttribute::ON : osg::StateAttribute::OFF) | osg::StateAttribute::OVERRIDE);
  }

  if (render->backfaceCulling().isSet())
  {
    node->getOrCreateStateSet()->setMode(GL_CULL_FACE,
      (render->backfaceCulling().get() ? osg::StateAttribute::ON : osg::StateAttribute::OFF) | osg::StateAttribute::OVERRIDE);
  }

#if !( defined(OSG_GLES2_AVAILABLE) || defined(OSG_GLES3_AVAILABLE) )
  if (render->clipPlane().isSet())
    node->getOrCreateStateSet()->setMode(GL_CLIP_DISTANCE0 + render->clipPlane().value(), 1);
#endif

  if (render->order().isSet() || render->renderBin().isSet())
  {
    osg::StateSet* ss = node->getOrCreateStateSet();
    int binNumber = render->order().isSet() ? (int)render->order()->eval() : ss->getBinNumber();
    std::string binName =
      render->renderBin().isSet() ? render->renderBin().get() :
        ss->useRenderBinDetails() ? ss->getBinName() : "DepthSortedBin";
    ss->setRenderBinDetails(binNumber, binName);
  }

  // Respect Transparent although we prefer renderBin and order
  if (render->transparent().get())
    node->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

  if (render->decal() == true)
  {
    osg::StateSet* ss = node->getOrCreateStateSet();
    ss->setAttributeAndModes(new osg::PolygonOffset(-1, -1), 1);
    ss->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, false));
  }
}

ConeNodeInterface::ConeNodeInterface(osgEarth::LocalGeometryNode* localNode, const simVis::GOG::GogMetaData& metaData)
  : LocalGeometryNodeInterface(localNode, metaData)
{
}

void ConeNodeInterface::setFillColor(const osg::Vec4f& color)
{
  metaData_.setExplicitly(GOG_FILL_COLOR_SET);
  fillColor_ = color;
  // Need to dig down into the LocalGeometryNode to get the underlying Geometry object to set its color array
  // NOTE: this assumes a specific implementation for cone nodes. May fail if that implementation changes

  // Set the color on the cone body
  osg::Group* group = localNode_->getPositionAttitudeTransform();
  osg::Node* node = group->getNumChildren() > 0 ? group->getChild(0) : nullptr;
  if (!node)
    return;
  osg::Geometry* geometry = node->asGeometry();
  if (!geometry)
    return;

  // Update the color array
  osg::Vec4Array* colorArray = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
  (*colorArray)[0] = color;
  geometry->setColorArray(colorArray);

  // Set the color on the cone cap
  osg::Node* capNode = group->getNumChildren() > 1 ? group->getChild(1) : nullptr;
  if (!capNode)
    return;
  osg::Geometry* capGeometry = capNode->asGeometry();
  if (!capGeometry)
    return;

  // Update the color array
  capGeometry->setColorArray(colorArray);
}

ImageOverlayInterface::ImageOverlayInterface(osgEarth::ImageOverlay* imageNode, const simVis::GOG::GogMetaData& metaData)
  : GogNodeInterface(imageNode, metaData),
    imageNode_(imageNode)
{
  // Turn off the color shader, since it doesn't work for image overlay
  simVis::OverrideColor::setCombineMode(imageNode_->getOrCreateStateSet(), simVis::OverrideColor::OFF);
}

int ImageOverlayInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  osg::Vec3d centerPoint = imageNode_->getBound().center();

  const simCore::Coordinate ecefCoord(simCore::COORD_SYS_ECEF, simCore::Vec3(centerPoint.x(), centerPoint.y(), centerPoint.z()));
  simCore::CoordinateConverter converter;
  simCore::Coordinate llaCoord;
  converter.convert(ecefCoord, llaCoord, simCore::COORD_SYS_LLA);
  position = osg::Vec3d(llaCoord.lon()*simCore::RAD2DEG, llaCoord.lat()*simCore::RAD2DEG, llaCoord.alt());

  return 0;
}

void ImageOverlayInterface::setOpacity(float opacity)
{
  GogNodeInterface::setOpacity(opacity);
  if (imageNode_.valid())
    imageNode_->setAlpha(opacity);
}

void ImageOverlayInterface::adjustAltitude_()
{
  // no-op
}

void ImageOverlayInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  // no-op since this is not officially supported in GOG format, the geometry will be part of the meta-data
}

void ImageOverlayInterface::setStyle_(const osgEarth::Style& style)
{
  // no-op, can't update style
}

LatLonAltBoxInterface::LatLonAltBoxInterface(osg::Group* node, osgEarth::FeatureNode* topNode, osgEarth::FeatureNode* bottomNode, const simVis::GOG::GogMetaData& metaData)
  : FeatureNodeInterface(node, topNode, metaData),
    bottomNode_(bottomNode)
{
  if (featureNode_.valid())
    initAltitudes_(*featureNode_.get(), originalAltitude_);
  if (bottomNode_.valid())
    initAltitudes_(*bottomNode_.get(), bottomAltitude_);
}

void LatLonAltBoxInterface::setAltOffset(double altOffsetMeters)
{
  if (altOffsetMeters == altOffset_)
    return;

  metaData_.setExplicitly(GOG_THREE_D_OFFSET_ALT_SET);
  altOffset_ = altOffsetMeters;

  if (featureNode_.valid())
    applyAltOffsets_(*featureNode_.get(), originalAltitude_);
  if (bottomNode_.valid())
    applyAltOffsets_(*bottomNode_.get(), bottomAltitude_);
}

void LatLonAltBoxInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  // no-op, LatLonAltBox corners are stored in the meta data
}

void LatLonAltBoxInterface::serializeKeyword_(std::ostream& gogOutputStream) const
{
  // nothing to do, LLA box includes the keyword in their metadata as part of the corner LLAs
}

void LatLonAltBoxInterface::setStyle_(const osgEarth::Style& style)
{
  // make sure backface culling is always on
  style_.getOrCreateSymbol<osgEarth::RenderSymbol>()->backfaceCulling() = true;

  FeatureNodeInterface::setStyle_(style);
  if (&style != &style_)
    style_ = style;
  if (!deferringStyleUpdates_() && bottomNode_.valid())
  {
    bottomNode_->setStyle(style_);
    bottomNode_->getFeature()->style() = style_;
    bottomNode_->dirty();
  }
}

void LatLonAltBoxInterface::initAltitudes_(osgEarth::FeatureNode& node, std::vector<double>& altitudes) const
{
  altitudes.clear();
  // use GeometryIterator to get all the points, since it works on MultiGeometries
  osgEarth::GeometryIterator iter(node.getFeature()->getGeometry(), false);
  while (iter.hasMore())
  {
    osgEarth::Geometry* part = iter.next();
    for (size_t i = 0; i < part->size(); ++i)
      altitudes.push_back((*part)[i].z());
  }
}

void LatLonAltBoxInterface::applyAltOffsets_(osgEarth::FeatureNode& node, const std::vector<double>& altitudes) const
{
  osgEarth::Geometry* geometry = node.getFeature()->getGeometry();
  if (!geometry)
    return;

  // now apply the altitude offset to all of our position points, use the GeometryIterator which works on MultiGeometries
  osgEarth::GeometryIterator iter(geometry, false);
  size_t altIndex = 0;
  while (iter.hasMore())
  {
    osgEarth::Geometry* part = iter.next();
    for (size_t i = 0; i < part->size() && altIndex < altitudes.size(); ++i, ++altIndex)
      (*part)[i].z() = altitudes.at(altIndex) + altOffset_;
  }
  node.dirty();
}

} }
