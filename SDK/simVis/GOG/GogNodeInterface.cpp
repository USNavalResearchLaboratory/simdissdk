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
*               EW Modeling and Simulation, Code 5770
*               4555 Overlook Ave.
*               Washington, D.C. 20375-5339
*
* For more information please send email to simdis@enews.nrl.navy.mil
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*
*/
#include <cassert>
#include <iostream>
#include "osgEarth/Units"
#include "osgEarthSymbology/Style"
#include "osgEarthSymbology/Color"
#include "osgEarthSymbology/TextSymbol"
#include "osgEarthSymbology/PolygonSymbol"
#include "osgEarthSymbology/LineSymbol"
#include "osgEarthSymbology/AltitudeSymbol"
#include "osgEarthSymbology/RenderSymbol"
#include "osgEarthAnnotation/LabelNode"
#include "osgEarthAnnotation/PlaceNode"
#include "osgEarthAnnotation/GeoPositionNode"
#include "osgEarthAnnotation/LocalGeometryNode"
#include "osgEarthAnnotation/FeatureNode"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simNotify/Notify.h"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simVis/Constants.h"
#include "simVis/Registry.h"
#include "simVis/OverheadMode.h"
#include "simVis/GOG/GOG.h"
#include "simVis/GOG/GOGNode.h"
#include "simVis/GOG/Arc.h"
#include "simVis/GOG/Circle.h"
#include "simVis/GOG/Ellipse.h"
#include "simVis/GOG/Ellipsoid.h"
#include "simVis/GOG/Line.h"
#include "simVis/GOG/LineSegs.h"
#include "simVis/GOG/Parser.h"
#include "simVis/GOG/Points.h"
#include "simVis/GOG/Polygon.h"
#include "simVis/GOG/GogNodeInterface.h"

namespace {

  /**
  * Calculates the position of the node (GeoPositionNode) passed in, applying the offset if it has one.
  * Will use the referencePosition if it is provided, otherwise uses the node's internal position
  * Returns 0 on success, non-zero if no position could be found
  */
  template<class T>
  int findLocalGeometryPosition(T* node, osgEarth::GeoPoint* referencePosition, osg::Vec3d& position)
  {
    if (!node || (!referencePosition && !node->getPosition().isValid()))
      return 1;
    const osg::Vec3d localOffset = node->getLocalOffset();
    // use reference point if it's valid, otherwise use the node's position
    osgEarth::GeoPoint refPosition = referencePosition != NULL ? *referencePosition : node->getPosition();

    // if the offsets are 0, just pass back the position
    if (localOffset == osg::Vec3d(0.0, 0.0, 0.0))
    {
      position = refPosition.vec3d();
      return 0;
    }
    // if the offsets are non-zero, apply the offsets to our reference position
    simCore::CoordinateConverter converter;
    converter.setReferenceOrigin(refPosition.y() * simCore::DEG2RAD, refPosition.x() * simCore::DEG2RAD, refPosition.z());
    simCore::Coordinate coord(simCore::COORD_SYS_XEAST, simCore::Vec3(localOffset.x(), localOffset.y(), localOffset.z()));
    simCore::Coordinate outCoord;
    converter.convert(coord, outCoord, simCore::COORD_SYS_LLA);
    const simCore::Vec3& outCoordPos = outCoord.position();
    position.x() = outCoordPos.y() * simCore::RAD2DEG;
    position.y() = outCoordPos.x() * simCore::RAD2DEG;
    position.z() = outCoordPos.z();

    return 0;
  }
}

namespace simVis { namespace GOG {

GogNodeInterface::GogNodeInterface(osg::Node* osgNode, const simVis::GOG::GogMetaData& metaData)
  :osgNode_(osgNode),
  metaData_(metaData),
  filled_(false),
  extruded_(false),
  outlined_(false),
  depthBuffer_(false),
  depthBufferOverride_(DEPTHBUFFER_IGNORE_OVERRIDE),
  extrudedHeight_(0.0),
  deferringStyleUpdate_(false)
{
  if (osgNode_.valid())
  {
    osgNode_->setNodeMask(simVis::DISPLAY_MASK_GOG);

    // flatten in overhead mode by default - subclass might change this
    simVis::OverheadMode::enableGeometryFlattening(true, osgNode_.get());
  }
}

void GogNodeInterface::applyConfigToStyle(const osgEarth::Config& parent, const UnitsState& units)
{
  // for performance reasons, cache all style updates, apply once when done
  beginStyleUpdates_();

  metaData_.allowSetExplicitly(false);  ///< setFields will incorrectly respond to defaults here, so cache the correct value and restore it at the end

  const std::string& key = parent.key();
  const simVis::GOG::GogShape gogShape = metaData_.shape;
  bool is3dShape = (gogShape == GOG_SPHERE || gogShape == GOG_ELLIPSOID || gogShape == GOG_HEMISPHERE ||
                    gogShape == GOG_CYLINDER || gogShape == GOG_LATLONALTBOX);

  // do we need an ExtrusionSymbol? Note that 3D shapes cannot be extruded
  bool isExtruded = (simCore::caseCompare(parent.value("extrude"), "true") == 0) && !is3dShape;

  // do we need a PolygonSymbol?
  bool isFillable = isExtruded || key == "poly" || key == "polygon" || key == "ellipse" || key == "circle" || key == "arc" || is3dShape;
  bool isFilled   = isFillable && (simCore::caseCompare(parent.value("filled"), "true") == 0);

  // do we need a LineSymbol?
  bool isOutlined = (simCore::caseCompare(parent.value("outline"), "true") == 0);
  bool hasLineAttrs = parent.hasValue("linecolor") || parent.hasValue("linewidth") || parent.hasValue("linestyle") || isOutlined;
  // Tessellate behaves badly with cirles, arcs, ellipses and 3dShapes, do not apply
  bool isTessellated = (simCore::caseCompare(parent.value("tessellate"), "true") == 0) && !(is3dShape || key == "circle" || key == "ellipse" || key == "arc");
  // need to create a LineSymbol if the shape is filled or has some line attributes or is tessellated, since tessellation is handled in the LineSymbol
  bool isLined = isFilled || hasLineAttrs || isTessellated;
  bool isText = (key == "annotation");

  // POINT attributes
  if (gogShape == GOG_POINTS && parent.hasValue("pointsize"))
    setPointSize(parent.value<int>("pointsize", 1));

  // LINE attributes
  if (isLined)
  {

    if (parent.hasValue("outline"))
      setOutlineState(isOutlined);
    else
      setOutlineState(true);

    if (parent.hasValue("linecolor"))
      setLineColor(osgEarth::Symbology::Color(parent.value("linecolor")));

    if (parent.hasValue("linewidth"))
      setLineWidth(parent.value<int>("linewidth", 1));

    if (parent.hasValue("linestyle"))
    {
      const std::string& ls = parent.value("linestyle");
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
    if (parent.hasValue("fillcolor"))
      setFillColor(osgEarth::Symbology::Color(parent.value("fillcolor")));
    setFilledState(isFilled);
  }

  // altitude offset
  if (parent.hasValue("3d offsetalt"))
    setAltOffset(parent.value<double>("3d offsetalt", 0.));

  // ALTITUDE mode, handles extrude attribute, which requires a specific AltitudeSymbol
  AltitudeMode altMode = ALTITUDE_NONE;
  if (simCore::caseCompare(parent.value("altitudemode"), "relativetoground") == 0)
    altMode = ALTITUDE_GROUND_RELATIVE;
  else if (simCore::caseCompare(parent.value("altitudemode"), "clamptoground") == 0)
    altMode = ALTITUDE_GROUND_CLAMPED;
  else if (isExtruded)
    altMode = ALTITUDE_EXTRUDE;
  setAltitudeMode(altMode);

  // process extrude height if extrude is set and if an extrude height was specified
  if (altMode == ALTITUDE_EXTRUDE && parent.hasChild("extrudeheight"))
  {
    double extrudeHeight = static_cast<double>(parent.value<int>("extrudeheight", 0));
    // convert from gog file altitude units to meters; gog file default units are ft, but file can specify different units
    extrudeHeight = units.altitudeUnits_.convertTo(osgEarth::Units::METERS, extrudeHeight);
    setExtrudedHeight(extrudeHeight);
  }

  // TESSELLATION attribute
  simVis::GOG::TessellationStyle tessStyle = TESSELLATE_NONE;
  if (isTessellated)
  {
    // default to rhumbline
    tessStyle = TESSELLATE_RHUMBLINE;
    if (parent.hasValue("lineprojection") && (simCore::caseCompare(parent.value("lineprojection"), "greatcircle") == 0))
      tessStyle = TESSELLATE_GREAT_CIRCLE_PROJECTION;
  }
  setTessellation(tessStyle);

  // TEXT attributes
  if (isText)
  {
    // default to font arial 15, color red
    std::string fontName = "arial.ttf";
    int fontSize = 15;
    osg::Vec4f fontColor = osgEarth::Symbology::Color::Red;
    // fonts.
    if (parent.hasValue("fontname"))
      fontName = parent.value("fontname");

    if (parent.hasValue("fontsize"))
      fontSize = parent.value<int>("fontsize", 15);

    if (parent.hasValue("linecolor"))
      fontColor = osgEarth::Symbology::Color(parent.value("linecolor"));

    setFont(fontName, fontSize, fontColor);

    // default to black outline
    setTextOutline(true, osgEarth::Symbology::Color::Black);
  }

  // DEPTH BUFFER attribute
  // depth buffer defaults to disable to match SIMDIS 9
  bool depthTest = false;
  if (parent.hasValue("depthbuffer"))
    depthTest = (simCore::caseCompare(parent.value("depthbuffer"), "true") == 0);
  setDepthBuffer(depthTest);

  // 3D shapes and extruded shapes get backface culling; otherwise turn it off so we can see both sides.
  // Note that extruded lines are the only extruded symbol that need backface culling off (because it
  // extrudes to a filled polygon instead of a 3D shape).
  const bool isLine = (gogShape == GOG_LINE || gogShape == GOG_LINESEGS);
  if (is3dShape || (isExtruded && !isLine))
    style_.getOrCreateSymbol<osgEarth::Symbology::RenderSymbol>()->backfaceCulling() = true;
  else
    style_.getOrCreateSymbol<osgEarth::Symbology::RenderSymbol>()->backfaceCulling() = false;

  metaData_.allowSetExplicitly(true);

  // done deferring style updates; apply them all at once
  endStyleUpdates_();
  setStyle_(style_);
}

osg::Node* GogNodeInterface::osgNode() const
{
  return osgNode_.get();
}

simVis::GOG::GogShape GogNodeInterface::shape() const
{
  return metaData_.shape;
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

    // add the metadata
    gogOutputStream << metaData;

    if (serializeGeometry)
    {
      // alt units are meters
      gogOutputStream << "altitudeunits meters\n";

      // if relative, the xy range units are in meters
      if (relativeShape)
        gogOutputStream << "rangeunits meters\n";

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
      gogOutputStream << "3d offsetalt " << altOffset << "\n";

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
}

int GogNodeInterface::getAltitudeMode(AltitudeMode& altMode) const
{
  bool isValid = false;
  int rv = getExtruded(isValid);
  if (rv == 0 && isValid)
  {
    altMode = ALTITUDE_EXTRUDE;
    return rv;
  }

  // Pull out the altitude symbol to determine if it's clamped or relative
  const osgEarth::Symbology::AltitudeSymbol* alt = style_.getSymbol<osgEarth::Symbology::AltitudeSymbol>();
  altMode = ALTITUDE_NONE;
  if (alt != NULL)
  {
    if (alt->clamping() == osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN)
      altMode = ALTITUDE_GROUND_CLAMPED;
    else if (alt->clamping() == osgEarth::Symbology::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN)
      altMode = ALTITUDE_GROUND_RELATIVE;
  }
  return 0;
}

int GogNodeInterface::getAltOffset(double& altOffset) const
{
  return 1;
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
  if (1 == getExtruded(isExtruded) || !isExtruded)
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
  if (!(style_.has<osgEarth::Symbology::LineSymbol>()))
    return 0;

  const osgEarth::Symbology::LineSymbol* linePtr = style_.getSymbol<osgEarth::Symbology::LineSymbol>();
  lineWidth = static_cast<int>(*(linePtr->stroke()->width()));
  // now figure out line style based on the stipple value
  unsigned short stipple = *(linePtr->stroke()->stipple());
  lineStyle = Utils::getLineStyleFromStipple(stipple);
  return 0;
}

int GogNodeInterface::getPointSize(int& pointSize) const
{
  if (!style_.has<osgEarth::Symbology::PointSymbol>())
    return 1;
  pointSize = static_cast<int>(*(style_.getSymbol<osgEarth::Symbology::PointSymbol>()->size()));
  return 0;
}

int GogNodeInterface::getTessellation(TessellationStyle& style) const
{
  // tessellation only applies to feature nodes
  return 1;
}

int GogNodeInterface::getTextOutline(bool& draw, osg::Vec4f& outlineColor) const
{
  // only applies to label nodes
  return 1;
}

void GogNodeInterface::setAltitudeMode(AltitudeMode altMode)
{
  setExtrude(altMode == ALTITUDE_EXTRUDE);

  if (style_.has<osgEarth::Symbology::ExtrusionSymbol>())
  {
    // Assertion failure means failure in setExtrude()
    assert(altMode == ALTITUDE_EXTRUDE);
    return;
  }

  switch (altMode)
  {
  case ALTITUDE_NONE:
    style_.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_NONE;
    style_.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;
    break;
  case ALTITUDE_GROUND_CLAMPED:
    style_.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
    style_.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_SCENE;
    style_.getOrCreate<osgEarth::Symbology::RenderSymbol>()->depthOffset()->automatic() = true;
    break;
  case ALTITUDE_GROUND_RELATIVE:
    style_.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;
    style_.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_SCENE;
    style_.getOrCreate<osgEarth::Symbology::RenderSymbol>()->depthOffset()->automatic() = true;
    break;
  case ALTITUDE_EXTRUDE:
    // Shouldn't get here; failure in setExtrude()
    // SIM-5324 - extrude should not be a choosable option for shapes that do not support extrusion
    break;
  }
  setStyle_(style_);
}

void GogNodeInterface::setAltOffset(double altOffsetMeters)
{
  // NOP
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
  style_.getOrCreate<osgEarth::Symbology::RenderSymbol>()->depthTest() = depthBuffer;
  if (!depthBuffer) // unset the clip pane if depth buffer turned off
    style_.getOrCreate<osgEarth::Symbology::RenderSymbol>()->clipPlane() = simVis::CLIPPLANE_VISIBLE_HORIZON;
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

  style_.getOrCreate<osgEarth::Symbology::RenderSymbol>()->depthTest() = depthBuffer;
  if (!depthBuffer) // unset the clip pane if depth buffer turned off
    style_.getOrCreate<osgEarth::Symbology::RenderSymbol>()->clipPlane() = simVis::CLIPPLANE_VISIBLE_HORIZON;
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
  style_.getOrCreate<osgEarth::Symbology::RenderSymbol>()->backfaceCulling() = extrude && !isLine;

  if (extrude)
  {
    // need to add an invisible polygon symbol here, since for some reason extrusion fills itself with the default color otherwise
    if (!filled_)
    {
      osgEarth::Symbology::PolygonSymbol* polygonSymbol = style_.getOrCreate<osgEarth::Symbology::PolygonSymbol>();
      polygonSymbol->fill()->color() = osg::Vec4(0., 0., 0., 0.);
    }
    osgEarth::Symbology::ExtrusionSymbol* ext = style_.getOrCreate<osgEarth::Symbology::ExtrusionSymbol>();
    // set the height value if necessary, otherwise unset to allow extrude to ground
    if (height != 0.f)
      ext->height() = height;
    else
      ext->height().unset();
    osgEarth::Symbology::AltitudeSymbol* alt = style_.getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
    alt->clamping()  = osgEarth::Symbology::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;
    alt->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_SCENE;
  }
  else
  {
    osgEarth::Symbology::ExtrusionSymbol* ext = style_.getSymbol<osgEarth::Symbology::ExtrusionSymbol>();
    if (ext)
    {
      // return altitude clamping to none. Note this will turn off ground relative if it was set
      osgEarth::Symbology::AltitudeSymbol* alt = style_.getSymbol<osgEarth::Symbology::AltitudeSymbol>();
      if (alt)
        alt->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_NONE;
      style_.removeSymbol(ext);
    }
  }
  setStyle_(style_);

  // Need to have a polygon symbol if extruded
  if (extruded_)
    assert(style_.has<osgEarth::Symbology::PolygonSymbol>());
  // Polygon symbol must be invisible if not filled
  if (extruded_ && !filled_)
    assert(style_.getSymbol<osgEarth::Symbology::PolygonSymbol>()->fill()->color()[3] == 0.);

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
    osgEarth::Symbology::PolygonSymbol* polygonSymbol = style_.getOrCreate<osgEarth::Symbology::PolygonSymbol>();
    polygonSymbol->fill()->color() = fillColor_;
    setStyle_(style_);
  }
  else if (style_.has<osgEarth::Symbology::PolygonSymbol>())
  {
    // since extrusion requires an invisible polygon symbol (if not filled), make it invisible if extruded, otherwise remove the symbol altogether
    if (extruded_)
      style_.getSymbol<osgEarth::Symbology::PolygonSymbol>()->fill()->color() = osg::Vec4(0., 0., 0., 0.);
    else
      style_.remove<osgEarth::Symbology::PolygonSymbol>();
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
  osgEarth::Symbology::PolygonSymbol* poly = style_.getSymbol<osgEarth::Symbology::PolygonSymbol>();
  if (!poly) // don't update color if no polygon symbol to update
    return;

  // set the new color in the PolygonSymbol
  osgEarth::Symbology::Color colorVec(color);
  poly->fill()->color() = colorVec;
  setStyle_(style_);
}

void GogNodeInterface::setFont(const std::string& fontName, int fontSize, const osg::Vec4f& color)
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
  osgEarth::Symbology::Color colorVec(color);

  if (metaData_.shape == simVis::GOG::GOG_POINTS)
    style_.getOrCreate<osgEarth::Symbology::PointSymbol>()->fill() = colorVec;
  else
    style_.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = colorVec;

  setStyle_(style_);
}

void GogNodeInterface::setOutlineState(bool outlineState)
{
  if (!isLined_(metaData_.shape))
    return;

  outlined_ = outlineState;

  metaData_.setExplicitly(GOG_OUTLINE_SET);

  // turn on the outline by applying the current line color. Note that if the current line color has alpha value of 0, this will have no effect
  osgEarth::Symbology::Color newColor = (outlineState ? osgEarth::Symbology::Color(lineColor_) : osgEarth::Symbology::Color(0.f, 0.f, 0.f, 0.f));

  // Points use line color, but applies to the PointSymbol
  if (metaData_.shape == simVis::GOG::GOG_POINTS)
    style_.getOrCreate<osgEarth::Symbology::PointSymbol>()->fill() = newColor;
  else
    style_.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = newColor;

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
  osgEarth::Symbology::LineSymbol* lineSymbol = style_.getOrCreate<osgEarth::Symbology::LineSymbol>();
  lineSymbol->stroke()->stipple() = lineStyle;
  setStyle_(style_);
}

void GogNodeInterface::setLineWidth(int lineWidth)
{
  // only applies to certain shapes
  if (metaData_.shape == simVis::GOG::GOG_POINTS || !isLined_(metaData_.shape))
    return;

  metaData_.setExplicitly(GOG_LINE_WIDTH_SET);

  osgEarth::Symbology::LineSymbol* lineSymbol = style_.getOrCreate<osgEarth::Symbology::LineSymbol>();
  lineSymbol->stroke()->width() = static_cast<float>(lineWidth);
  setStyle_(style_);
}

void GogNodeInterface::setPointSize(int pointSize)
{
  if (metaData_.shape != simVis::GOG::GOG_POINTS)
    return;

  metaData_.setExplicitly(GOG_POINT_SIZE_SET);

  style_.getOrCreate<osgEarth::Symbology::PointSymbol>()->size() = static_cast<float>(pointSize);
  setStyle_(style_);
}

void GogNodeInterface::setTessellation(TessellationStyle style)
{
  // only feature node has tessellation
}

void GogNodeInterface::setTextOutline(bool draw, const osg::Vec4f& outlineColor)
{
  // NOP only applies to label nodes
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

bool GogNodeInterface::getMetaDataFlag_(const std::string& flag, std::string& metaData)
{
  size_t keywordIndex = metaData.find(flag);
  if (keywordIndex == std::string::npos)
    return false;
  // erase our flag keyword
  metaData.erase(keywordIndex, flag.size());
  return true;
}

void GogNodeInterface::initializeFillColor_()
{
  if (style_.has<osgEarth::Symbology::PolygonSymbol>())
    fillColor_ = style_.getSymbol<osgEarth::Symbology::PolygonSymbol>()->fill()->color();
  else
    fillColor_ = osgEarth::Symbology::Color::Red; // default to red
}

void GogNodeInterface::initializeLineColor_()
{
  if (style_.has<osgEarth::Symbology::LineSymbol>())
    lineColor_ = style_.getSymbol<osgEarth::Symbology::LineSymbol>()->stroke()->color();
  else
    lineColor_ = osgEarth::Symbology::Color::Red; // default to red
}

void GogNodeInterface::setLocalNodeAltOffset_(osgEarth::Annotation::LocalGeometryNode* node, double altOffsetMeters)
{
  if (!node)
    return;
  osg::Vec3d offset = node->getLocalOffset();
  if (offset[2] == altOffsetMeters)
    return;
  offset[2] = altOffsetMeters;
  node->setLocalOffset(offset);

  AltitudeMode currentMode = ALTITUDE_NONE;
  if (getAltitudeMode(currentMode) == 0 && currentMode == ALTITUDE_GROUND_RELATIVE)
  {
    // toggle altitude mode to get the altitude offset to apply. Due to the way osgEarth LocalGeometryNode works, it won't update the altitude offset otherwise.
    setAltitudeMode(ALTITUDE_NONE);
    setAltitudeMode(ALTITUDE_GROUND_RELATIVE);
  }
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
    return true;
  default:
    break;
  }
  return false;
};

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
    return true;
  default:
    break;
  }
  return false;
};

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

const osgEarth::Symbology::Style& GogNodeInterface::getStyle_() const
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

FeatureNodeInterface::FeatureNodeInterface(osgEarth::Annotation::FeatureNode* featureNode, const simVis::GOG::GogMetaData& metaData)
  : GogNodeInterface(featureNode, metaData),
    featureNode_(featureNode),
    altitudeOffset_(0.)
{
  if (featureNode_.valid() && featureNode_->getFeature())
    style_ = *(featureNode_->getFeature()->style());

  initializeFillColor_();
  initializeLineColor_();

  // initialize our original altitudes
  osgEarth::Symbology::Geometry* geometry = featureNode_->getFeature()->getGeometry();
  for (size_t i = 0; i < geometry->size(); ++i)
  {
    originalAltitude_.push_back((*geometry)[i].z());
  }

}

int FeatureNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  if (!featureNode_.valid() || !featureNode_->getFeature() || !featureNode_->getFeature()->getGeometry())
    return 1;

  // initialize bounding values
  osg::Vec3d lowerBound(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 0.0);
  osg::Vec3d upperBound(-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), 0.0);

  osgEarth::Symbology::Geometry* geometry = featureNode_->getFeature()->getGeometry();
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
  if (!style_.has<osgEarth::Symbology::LineSymbol>())
    return 1;
  const osgEarth::Symbology::LineSymbol* ls = style_.getSymbol<osgEarth::Symbology::LineSymbol>();

  if (!ls->tessellationSize().isSet() || *(ls->tessellationSize()) == 0)
  {
    tessellation = TESSELLATE_NONE;
    return 0;
  }
  switch (*(featureNode_->getFeature()->geoInterp()))
  {
  case osgEarth::Symbology::GEOINTERP_RHUMB_LINE:
    tessellation = TESSELLATE_RHUMBLINE;
    break;
  case osgEarth::Symbology::GEOINTERP_GREAT_CIRCLE:
    tessellation = TESSELLATE_GREAT_CIRCLE_PROJECTION;
    break;
  default:
    tessellation = TESSELLATE_NONE;
    break;
  }
  return 0;
}

int FeatureNodeInterface::getAltOffset(double& altOffset) const
{
  altOffset = altitudeOffset_;
  return 0;
}

void FeatureNodeInterface::setAltOffset(double altOffsetMeters)
{
  if (altOffsetMeters == altitudeOffset_)
    return;
  osgEarth::Symbology::Geometry* geometry = featureNode_->getFeature()->getGeometry();
  if (!geometry)
    return;
  if (geometry->size() != originalAltitude_.size())
  {
    assert(0); // somehow our vector of original altitude values is out of synch with the geometry
    return;
  }
  metaData_.setExplicitly(GOG_THREE_D_OFFSET_ALT_SET);
  altitudeOffset_ = altOffsetMeters;
  // now apply the altitude offset to all of our position points
  for (size_t i = 0; i < geometry->size(); ++i)
  {
    (*geometry)[i].z() = originalAltitude_.at(i) + altOffsetMeters;
  }
  featureNode_->dirty();
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
    featureNode_->getFeature()->geoInterp() = osgEarth::Symbology::GEOINTERP_RHUMB_LINE;
  else if (style == TESSELLATE_GREAT_CIRCLE_PROJECTION)
    featureNode_->getFeature()->geoInterp() = osgEarth::Symbology::GEOINTERP_GREAT_CIRCLE;
  else if (style == TESSELLATE_NONE)
    tessellate = false;

  if (!style_.has<osgEarth::Symbology::LineSymbol>())
    return;
  osgEarth::Symbology::LineSymbol* ls = style_.getSymbol<osgEarth::Symbology::LineSymbol>();

  if (tessellate)
  {
    // need to unset any tessellation value, so that the tessellationSize attribute takes precedence
    ls->tessellation().unset();
    // set default tessellation spacing in meters, functions as a minimum for large features
    double tessellationSpacingM = 10000.0;

    // adjust tessellation based on feature dimension
    const osgEarth::Features::Feature* feature = featureNode_->getFeature();
    if (feature != NULL)
    {
      const osgEarth::SpatialReference* srs = feature->getSRS();
      if (srs != NULL)
      {
        osg::BoundingSphered boundS;
        if (feature->getWorldBound(srs->getECEF(), boundS))
        {
          // ensure a minimum of 50m spacing, otherwise approximately 80 posts along major dimension of feature
          tessellationSpacingM = simCore::sdkMax(50.0, simCore::sdkMin(tessellationSpacingM, 0.025 * boundS.radius()));
        }
      }
    }
    ls->tessellationSize() = tessellationSpacingM; // in meters
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

void FeatureNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  osgEarth::Symbology::Geometry* geometry = featureNode_->getFeature()->getGeometry();
  if (!geometry)
    return;

  if (geometry->size() != originalAltitude_.size())
  {
    assert(0); // somehow our vector of original altitude values is out of synch with the geometry
    return;
  }

  // since we may have applied an altitude offset, get the original altitude values before serializing
  osgEarth::Symbology::Geometry originalGeometry = *geometry;
  for (size_t i = 0; i < originalGeometry.size(); ++i)
  {
    originalGeometry[i].z() = originalAltitude_.at(i);
  }
  Utils::serializeShapeGeometry(&originalGeometry, relativeShape, gogOutputStream);
}

void FeatureNodeInterface::setStyle_(const osgEarth::Symbology::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (!deferringStyleUpdates_() && featureNode_.valid())
    featureNode_->setStyle(style_);
}


///////////////////////////////////////////////////////////////////

LocalGeometryNodeInterface::LocalGeometryNodeInterface(osgEarth::Annotation::LocalGeometryNode* localNode, const simVis::GOG::GogMetaData& metaData)
  :GogNodeInterface(localNode, metaData),
   localNode_(localNode),
   altitude_(0.0)
{
  if (localNode_.valid())
  {
    altitude_ = localNode->getPosition().alt();
    style_ = localNode_->getStyle();
  }

  initializeFillColor_();
  initializeLineColor_();
}

int LocalGeometryNodeInterface::getAltOffset(double& altOffset) const
{
  if (!localNode_.valid())
    return 1;
  // third item in vector is the altitude offset
  altOffset = localNode_->getLocalOffset()[2];
  return 0;
}

int LocalGeometryNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  return findLocalGeometryPosition(localNode_.get(), referencePosition, position);
}

void LocalGeometryNodeInterface::setAltOffset(double altOffsetMeters)
{
  metaData_.setExplicitly(GOG_THREE_D_OFFSET_ALT_SET);
  setLocalNodeAltOffset_(localNode_.get(), altOffsetMeters);
  // need to update extrusion height if extruded
  if (extruded_)
  {
    metaData_.allowSetExplicitly(false);
    setExtrude(extruded_);
    metaData_.allowSetExplicitly(true); // set fields could be incorrectly changed by this call to setExtrude
  }
}

void LocalGeometryNodeInterface::setAltitudeMode(AltitudeMode altMode)
{
  // call to setAltitudeMode will not initiate a redraw, so call before setPosition, which will
  GogNodeInterface::setAltitudeMode(altMode);
  // In osgEarth LocalGeometryNode::clamp(), it is always adding the node's altituvalue as the offset.
  // So this means that both osgEarth::Symbology::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN (relativeToGround)
  // and osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN (clampToGround) behave the same for our shapes.
  // By setting the altitude to 0, clamp() then sets the offset to 0, and clampToGround works as intended.
  // This applys to CylinderNodeInterface and ArcNodeInterface as well, which also wrap a LocalGeometryNode
  if (localNode_.valid())
  {
    double newAltitude = (altMode == ALTITUDE_GROUND_CLAMPED) ? 0.0 : altitude_;
    osgEarth::GeoPoint newPos = localNode_->getPosition();
    newPos.alt() = newAltitude;
    localNode_->setPosition(newPos);
  }
}

void LocalGeometryNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  const osgEarth::Symbology::Geometry* geometry = localNode_->getGeometry();
  if (geometry)
    Utils::serializeShapeGeometry(geometry, relativeShape, gogOutputStream);
}

void LocalGeometryNodeInterface::setStyle_(const osgEarth::Symbology::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (deferringStyleUpdates_())
    return;
  if (localNode_.valid())
    localNode_->setStyle(style_);
}

///////////////////////////////////////////////////////////////////

LabelNodeInterface::LabelNodeInterface(osgEarth::Annotation::LabelNode* labelNode, const simVis::GOG::GogMetaData& metaData)
 : GogNodeInterface(labelNode, metaData),
   labelNode_(labelNode)
{
  if (labelNode_.valid())
    style_ = labelNode_->getStyle();
  initializeFillColor_();
  initializeLineColor_();

  // override - labels should not be flattened in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(false, labelNode);
}

LabelNodeInterface::LabelNodeInterface(osgEarth::Annotation::PlaceNode* placeNode, const simVis::GOG::GogMetaData& metaData)
 : GogNodeInterface(placeNode, metaData),
   labelNode_(placeNode)
{
  if (labelNode_.valid())
    style_ = labelNode_->getStyle();

  initializeFillColor_();
  initializeLineColor_();

  // override - places should not be flattened in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(false, placeNode);
}

int LabelNodeInterface::getFont(std::string& fontFile, int& fontSize, osg::Vec4f& fontColor) const
{
  if (!style_.has<osgEarth::Symbology::TextSymbol>())
    return 1;
  const osgEarth::Symbology::TextSymbol* ts = style_.getSymbol<osgEarth::Symbology::TextSymbol>();
  if (ts->font()->size() > 0)
    fontFile = *(ts->font());
  fontSize = static_cast<int>(simVis::simdisFontSize(static_cast<float>(ts->size()->eval())));
  fontColor = ts->fill()->color();
  return 0;
}

int LabelNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  return findLocalGeometryPosition(labelNode_.get(), referencePosition, position);
}

int LabelNodeInterface::getTextOutline(bool& draw, osg::Vec4f& outlineColor) const
{
  if (!style_.has<osgEarth::Symbology::TextSymbol>())
    return 1;
  const osgEarth::Symbology::TextSymbol* ts = style_.getSymbol<osgEarth::Symbology::TextSymbol>();

  // draw state is defined by the halo color alpha value (4th item in the color array)
  draw = (ts->halo().isSet() && ts->halo()->color()[3] != 0.f);
  outlineColor = outlineColor_;
  return 0;
}

void LabelNodeInterface::setFont(const std::string& fontName, int fontSize, const osg::Vec4f& color)
{
  osgEarth::Symbology::TextSymbol* ts = style_.getOrCreate<osgEarth::Symbology::TextSymbol>();
  if (!ts)
    return;

  std::string fileFullPath = simVis::Registry::instance()->findFontFile(fontName);
  float osgFontSize = simVis::osgFontSize(static_cast<float>(fontSize));
  osgEarth::Symbology::Color colorVec(color);

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
  ts->haloOffset() = simVis::outlineThickness(simData::TO_THIN);
  setStyle_(style_);
}

void LabelNodeInterface::setTextOutline(bool draw, const osg::Vec4f& outlineColor)
{
  outlineColor_ = outlineColor;
  osgEarth::Symbology::TextSymbol* ts = style_.getOrCreate<osgEarth::Symbology::TextSymbol>();
  if (!ts)
    return;

  if (draw)
    ts->halo()->color() = outlineColor_;
  else
    ts->halo()->color() = osgEarth::Symbology::Color(0.f, 0.f, 0.f, 0.f);

  setStyle_(style_);
}

void LabelNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  // nothing to do, labels don't serialize geometry
}

void LabelNodeInterface::serializeKeyword_(std::ostream& gogOutputStream) const
{
  // nothing to do, labels include the keyword in their text value element
}

void LabelNodeInterface::setStyle_(const osgEarth::Symbology::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (deferringStyleUpdates_())
    return;
  if (labelNode_.valid())
    labelNode_->setStyle(style_);
}

//////////////////////////////////////////////

CylinderNodeInterface::CylinderNodeInterface(osg::Group* groupNode, osgEarth::Annotation::LocalGeometryNode* sideNode, osgEarth::Annotation::LocalGeometryNode* topCapNode, osgEarth::Annotation::LocalGeometryNode* bottomCapNode, const simVis::GOG::GogMetaData& metaData)
  :GogNodeInterface(groupNode, metaData),
   sideNode_(sideNode),
   topCapNode_(topCapNode),
   bottomCapNode_(bottomCapNode),
   height_(0.0),
   altitude_(0.0)
{
  // height is from the side node's extrusion height, altitude is from side node's altitude
  if (sideNode_.valid() && sideNode_->getStyle().has<osgEarth::Annotation::ExtrusionSymbol>())
  {
    height_ = sideNode_->getStyle().getSymbol<osgEarth::Annotation::ExtrusionSymbol>()->height().value();
    altitude_ = sideNode_->getPosition().alt();
  }

  if (topCapNode_.valid())
  {
    // use style of the cap, since that will contain the line and other style options that matters for updating the characteristics of the cylinder
    style_ = topCapNode_->getStyle();
    // fill state is determined by the cap node's fill state
    filled_ = topCapNode_->getStyle().has<osgEarth::Annotation::PolygonSymbol>();
  }

  initializeFillColor_();
  initializeLineColor_();
}

int CylinderNodeInterface::getAltOffset(double& altOffset) const
{
  if (!sideNode_.valid())
    return 1;
  // third item in vector is the altitude offset
  altOffset = sideNode_->getLocalOffset()[2];
  // first answer is good enough
  return 0;
}

int CylinderNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  return findLocalGeometryPosition(sideNode_.get(), referencePosition, position);
}

void CylinderNodeInterface::setAltOffset(double altOffsetMeters)
{
  metaData_.setExplicitly(GOG_THREE_D_OFFSET_ALT_SET);

  setLocalNodeAltOffset_(sideNode_.get(), altOffsetMeters);
  setLocalNodeAltOffset_(topCapNode_.get(), altOffsetMeters + height_);
  setLocalNodeAltOffset_(bottomCapNode_.get(), altOffsetMeters);
}

void CylinderNodeInterface::setAltitudeMode(AltitudeMode altMode)
{
  // call to setAltitudeMode will not initiate a redraw, so call before setPosition, which will
  GogNodeInterface::setAltitudeMode(altMode);
  if (sideNode_.valid())
  {
    double newAltitude = (altMode == ALTITUDE_GROUND_CLAMPED) ? 0.0 : altitude_;
    osgEarth::GeoPoint newPos = sideNode_->getPosition();
    newPos.alt() = newAltitude;
    sideNode_->setPosition(newPos);
    if (bottomCapNode_.valid())
      bottomCapNode_->setPosition(newPos);
    if (topCapNode_.valid())
    {
      if (altMode == ALTITUDE_GROUND_CLAMPED)
        newPos.alt() = height_;
      topCapNode_->setPosition(newPos);
    }
  }
}

void CylinderNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  // Cylinder can't serialize its geometry, serialization of center, radius, height is stored in the meta data
}

void CylinderNodeInterface::setStyle_(const osgEarth::Symbology::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (deferringStyleUpdates_())
    return;
  // no updates if our two nodes are not valid
  if (!sideNode_.valid() || !topCapNode_.valid() || !bottomCapNode_.valid())
    return;

  // format style for the side node
  osgEarth::Symbology::Style sideStyle = style_;
  // need to add the extrusion symbol to the side style
  sideStyle.getOrCreate<osgEarth::Annotation::ExtrusionSymbol>()->height() = height_;
  // need to remove the line symbol from the side style
  sideStyle.remove<osgEarth::Symbology::LineSymbol>();
  // if not filled, need to make sure the side node has a fill color that matches the line color
  if (!filled_ && style_.has<osgEarth::Annotation::LineSymbol>())
    sideStyle.getOrCreate<osgEarth::Annotation::PolygonSymbol>()->fill()->color() = style_.getSymbol<osgEarth::Annotation::LineSymbol>()->stroke()->color();

  // If we are filled, then side's backface culling should be unset; if unfilled, then it should be set false
  if (filled_)
    sideStyle.getOrCreate<osgEarth::Symbology::RenderSymbol>()->backfaceCulling().unset();
  else
    sideStyle.getOrCreate<osgEarth::Symbology::RenderSymbol>()->backfaceCulling() = false;

  // sideNode_ cannot be clamped to ground, always set it as relative
  osgEarth::Symbology::AltitudeSymbol* alt = sideStyle.getSymbol<osgEarth::Symbology::AltitudeSymbol>();
  if (alt && alt->clamping() == osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN)
    alt->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;

  sideNode_->setStyle(sideStyle);

  // can't have an extrusion symbol for the cap node
  style_.remove<osgEarth::Symbology::ExtrusionSymbol>();
  topCapNode_->setStyle(style_);
  bottomCapNode_->setStyle(style_);
}

//////////////////////////////////////////////

ArcNodeInterface::ArcNodeInterface(osg::Group* groupNode, osgEarth::Annotation::LocalGeometryNode* shapeNode, osgEarth::Annotation::LocalGeometryNode* fillNode, const simVis::GOG::GogMetaData& metaData)
  :GogNodeInterface(groupNode, metaData),
   shapeNode_(shapeNode),
   fillNode_(fillNode),
   altitude_(0.0)
{
  if (shapeNode_.valid())
    altitude_ = shapeNode_->getPosition().alt();

  style_ = shapeNode_->getStyle();

  initializeLineColor_();

  osg::Node::NodeMask mask = simVis::DISPLAY_MASK_NONE;
  if (fillNode_.valid())
  {
    if (fillNode_->getStyle().has<osgEarth::Symbology::PolygonSymbol>())
    {
      fillColor_ = fillNode_->getStyle().getSymbol<osgEarth::Symbology::PolygonSymbol>()->fill()->color();
      if (fillColor_[3] > 0.f)
        mask = simVis::DISPLAY_MASK_GOG;
    }
    fillNode_->setNodeMask(mask);
  }
}

int ArcNodeInterface::getAltOffset(double& altOffset) const
{
  if (!shapeNode_.valid())
    return 1;
  // third item in vector is the altitude offset
  altOffset = shapeNode_->getLocalOffset()[2];
  // first answer is good enough
  return 0;
}

int ArcNodeInterface::getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition) const
{
  return findLocalGeometryPosition(shapeNode_.get(), referencePosition, position);
}

void ArcNodeInterface::setAltOffset(double altOffsetMeters)
{
  metaData_.setExplicitly(GOG_THREE_D_OFFSET_ALT_SET);

  setLocalNodeAltOffset_(shapeNode_.get(), altOffsetMeters);
  setLocalNodeAltOffset_(fillNode_.get(), altOffsetMeters);

  // need to update extrusion height if extruded
  if (extruded_)
  {
    metaData_.allowSetExplicitly(false);
    setExtrude(extruded_);
    metaData_.allowSetExplicitly(true); // setFields could be incorrectly changed by setExtrude
  }
}

void ArcNodeInterface::setAltitudeMode(AltitudeMode altMode)
{
  // call to setAltitudeMode will not initiate a redraw, so call before setPosition, which will
  GogNodeInterface::setAltitudeMode(altMode);
  if (shapeNode_.valid())
  {
    double newAltitude = (altMode == ALTITUDE_GROUND_CLAMPED) ? 0.0 : altitude_;
    osgEarth::GeoPoint newPos = shapeNode_->getPosition();
    newPos.alt() = newAltitude;
    shapeNode_->setPosition(newPos);
    if (fillNode_.valid())
      fillNode_->setPosition(newPos);
  }
}

void ArcNodeInterface::setFilledState(bool state)
{
  GogNodeInterface::setFilledState(state);
  if (!fillNode_.valid())
    return;
  fillNode_->setNodeMask(state ? simVis::DISPLAY_MASK_GOG : simVis::DISPLAY_MASK_NONE);
}

void ArcNodeInterface::serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const
{
  // Arc can't serialize its geometry, serialization for center and radius is stored in the meta data
}

void ArcNodeInterface::setStyle_(const osgEarth::Symbology::Style& style)
{
  if (&style != &style_)
    style_ = style;
  if (deferringStyleUpdates_())
    return;
  // no updates if our shape node is not valid
  if (!shapeNode_.valid())
    return;

  // format style for the shape node
  osgEarth::Symbology::Style shapeStyle = style_;

  // shape node must remove the PolygonSymbol if not extruded
  if (shapeStyle.has<osgEarth::Symbology::PolygonSymbol>() && !extruded_)
    shapeStyle.remove<osgEarth::Symbology::PolygonSymbol>();

  // shape node should not have a polygon symbol if not extruded
  if (!extruded_)
    assert(!shapeStyle.has<osgEarth::Symbology::PolygonSymbol>());

  shapeNode_->setStyle(shapeStyle);

  if (!fillNode_.valid())
    return;

  // can't have a line symbol for the fill node
  osgEarth::Symbology::Style fillStyle = style_;
  fillStyle.remove<osgEarth::Symbology::LineSymbol>();
  fillNode_->setStyle(fillStyle);
}

SphericalNodeInterface::SphericalNodeInterface(osgEarth::Annotation::LocalGeometryNode* localNode, const simVis::GOG::GogMetaData& metaData)
  :LocalGeometryNodeInterface(localNode, metaData)
{
}

int  SphericalNodeInterface::getFilledState(bool& state, osg::Vec4f& color) const
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
  osg::Node* node = localNode_->getNode();
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
  osg::Vec4Array* colorArray = new osg::Vec4Array(1);
  (*colorArray)[0] = color;
  geometry->setColorArray(colorArray);
  geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
}

} }
