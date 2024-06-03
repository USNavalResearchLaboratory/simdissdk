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
#include <cassert>
#include "osg/BlendFunc"
#include "osg/Depth"
#include "osg/Geometry"
#include "osgEarth/LineDrawable"
#include "simCore/Calc/Math.h"
#include "simCore/String/Format.h"
#include "simVis/Constants.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/Shaders.h"
#include "simVis/AreaHighlight.h"

namespace simVis
{

namespace
{

/// Controls rotational speed -- higher values spin faster; positive counterclockwise, negative clockwise
static constexpr float ROTATE_FREQUENCY = -2.0f;
/// Controls speed of the flashing -- higher values pulse more frequently
static constexpr float GLOW_FREQUENCY = 3.5f;
/// Controls amplitude of the flashing -- higher values strobe more drastically
static constexpr float GLOW_AMPLITUDE = 0.2f;
/// Added to the pulsing amplitude's alpha so the highlight does not become completely transparent
static constexpr float GLOW_MINIMUM_ALPHA = 0.6f;

/// Minimum number of line segments in a highlight circle
static constexpr int MIN_NUM_LINE_SEGMENTS = 90;
}

// --------------------------------------------------------------------------
HighlightNode::HighlightNode()
  : Group()
{
}

HighlightNode::HighlightNode(const HighlightNode& rhs, const osg::CopyOp& copyOp)
  : Group(rhs, copyOp)
{
}

HighlightNode::~HighlightNode()
{
}

// --------------------------------------------------------------------------
AreaHighlightNode::AreaHighlightNode()
  : HighlightNode(),
    color_(new osg::Uniform("simvis_areahighlight_color", simVis::Color::White)),
    radius_(new osg::Uniform("simvis_areahighlight_scale", 1.f))
{
  setName("AreaHighlight");
  init_();
}

AreaHighlightNode::AreaHighlightNode(const AreaHighlightNode& rhs, const osg::CopyOp& copyOp)
  : HighlightNode(rhs, copyOp),
    color_(copyOp(color_.get())),
    radius_(copyOp(radius_.get()))
{
  osg::StateSet* stateSet = getOrCreateStateSet();
  // Add Uniforms
  stateSet->addUniform(color_);
  stateSet->addUniform(radius_);
}

AreaHighlightNode::~AreaHighlightNode()
{
}

void AreaHighlightNode::setColor(const osg::Vec4f& rgba)
{
  color_->set(rgba);
}

void AreaHighlightNode::setRadius(float radius)
{
  radius_->set(radius);
}

// Creates a circle with a triangle at each cardinal point
void AreaHighlightNode::init_()
{
  // Use local grid mask to avoid issues with mouse picking
  setNodeMask(DISPLAY_MASK_LOCAL_GRID);

  osg::StateSet* stateSet = getOrCreateStateSet();
  // Disable lighting
  simVis::setLighting(stateSet, osg::StateAttribute::OFF);
  // Places the highlight in a low-priority bin, and turn off depth writes to prevent it from covering other models
  stateSet->setRenderBinDetails(BIN_AREA_HIGHLIGHT, BIN_GLOBAL_SIMSDK);
  stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false));
  // Tells OpenGL to use the default blend function
  stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
  // Turn off backface culling
  stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
  // Add Uniforms
  stateSet->addUniform(color_);
  stateSet->addUniform(radius_);

  // Shaders needed to rotate and flash
  Shaders package;
  // Apply static consts as text replacements
  package.replace("$ROTATE_FREQUENCY", simCore::buildString("", ROTATE_FREQUENCY, 0, 2));
  package.replace("$GLOW_FREQUENCY", simCore::buildString("", GLOW_FREQUENCY, 0, 2));
  package.replace("$GLOW_AMPLITUDE", simCore::buildString("", GLOW_AMPLITUDE, 0, 2));
  package.replace("$GLOW_MINIMUM_ALPHA", simCore::buildString("", GLOW_MINIMUM_ALPHA, 0, 2));

  // Load the shaders into the virtual program
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateSet);
  vp->setName("simVis.AreaHighlightNode");
  package.load(vp, package.areaHighlightVertex());
  package.load(vp, package.areaHighlightFragment());

  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
  geom->setName("simVis::AreaHighlight");
  geom->setUseVertexBufferObjects(true);

  osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
  geom->setVertexArray(vertexArray.get());

  osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
  geom->setColorArray(colorArray.get());

  // Declare color for the center of the circle and the triangles
  const osg::Vec4f& color = simVis::Color::White;

  // Center of the circle
  vertexArray->push_back(osg::Vec3());
  colorArray->push_back(color);

  // Make the edge of the circle darker and more transparent
  const osg::Vec4f edgeColor = color * 0.8f;

  float inc = M_TWOPI / static_cast<float>(MIN_NUM_LINE_SEGMENTS);
  for (int j = MIN_NUM_LINE_SEGMENTS; j > 0; --j)
  {
    const float angle = inc * j;
    const float x = sin(angle);
    const float y = cos(angle);
    vertexArray->push_back(osg::Vec3(x, y, 0.f));
    colorArray->push_back(edgeColor);
  }

  // Push in the initial vertex again, to close the circle
  vertexArray->push_back((*vertexArray)[1]);
  colorArray->push_back((*colorArray)[1]);

  geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, vertexArray->size()));
  addChild(geom);

  // Begin triangle creation ------------------------------
  osg::ref_ptr<osg::Geometry> triGeom = new osg::Geometry();
  triGeom->setName("simVis::AreaHighlight");
  triGeom->setUseVertexBufferObjects(true);

  osg::ref_ptr<osg::Vec3Array> triVertexArray = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
  triGeom->setVertexArray(triVertexArray.get());

  osg::ref_ptr<osg::Vec4Array> triColorArray = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
  triGeom->setColorArray(triColorArray.get());

  // Separates the triangles from the circle an infinitesimal amount
  constexpr float fudgeFactor = 0.001f;

  // Draws 4 triangle (0,TIP_Y) to (SIDE_X,BASE_Y) to (-SIDE_X, BASE_Y)
  constexpr float TRI_TIP_Y = 0.8f;
  constexpr float TRI_BASE_Y = 0.9f;
  constexpr float TRI_SIDE_X = 0.1f;

  // Triangle one
  triVertexArray->push_back(osg::Vec3(0, TRI_TIP_Y, fudgeFactor));
  triColorArray->push_back(color);
  triVertexArray->push_back(osg::Vec3(TRI_SIDE_X, TRI_BASE_Y, fudgeFactor));
  triColorArray->push_back(color);
  triVertexArray->push_back(osg::Vec3(-TRI_SIDE_X, TRI_BASE_Y, fudgeFactor));
  triColorArray->push_back(color);

  // Triangle two
  triVertexArray->push_back(osg::Vec3(0, -TRI_TIP_Y, fudgeFactor));
  triColorArray->push_back(color);
  triVertexArray->push_back(osg::Vec3(-TRI_SIDE_X, -TRI_BASE_Y, fudgeFactor));
  triColorArray->push_back(color);
  triVertexArray->push_back(osg::Vec3(TRI_SIDE_X, -TRI_BASE_Y, fudgeFactor));
  triColorArray->push_back(color);

  // Triangle three
  triVertexArray->push_back(osg::Vec3(-TRI_TIP_Y, 0, fudgeFactor));
  triColorArray->push_back(color);
  triVertexArray->push_back(osg::Vec3(-TRI_BASE_Y, TRI_SIDE_X, fudgeFactor));
  triColorArray->push_back(color);
  triVertexArray->push_back(osg::Vec3(-TRI_BASE_Y, -TRI_SIDE_X, fudgeFactor));
  triColorArray->push_back(color);

  // Triangle four
  triVertexArray->push_back(osg::Vec3(TRI_TIP_Y, 0, fudgeFactor));
  triColorArray->push_back(color);
  triVertexArray->push_back(osg::Vec3(TRI_BASE_Y, -TRI_SIDE_X, fudgeFactor));
  triColorArray->push_back(color);
  triVertexArray->push_back(osg::Vec3(TRI_BASE_Y, TRI_SIDE_X, fudgeFactor));
  triColorArray->push_back(color);

  triGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, triVertexArray->size()));
  addChild(triGeom);
}

// --------------------------------------------------------------------------

LineDrawableHighlightNode::LineDrawableHighlightNode()
 : HighlightNode()
{
  init_();
}

LineDrawableHighlightNode::LineDrawableHighlightNode(const LineDrawableHighlightNode& rhs, const osg::CopyOp& copyOp)
 : HighlightNode(rhs, copyOp)
{
  if (getNumChildren())
    removeChildren(0, getNumChildren());
  init_();
}

LineDrawableHighlightNode::~LineDrawableHighlightNode()
{
}

void LineDrawableHighlightNode::init_()
{
  // Use local grid mask to avoid issues with mouse picking
  setNodeMask(DISPLAY_MASK_LOCAL_GRID);

  osg::StateSet* stateSet = getOrCreateStateSet();
  // Disable lighting
  simVis::setLighting(stateSet, osg::StateAttribute::OFF);
  // Places the highlight in a low-priority bin
  stateSet->setRenderBinDetails(BIN_AREA_HIGHLIGHT, BIN_GLOBAL_SIMSDK);
  // Protect depth changes, since Overhead Mode does OVERRIDE on an osg::Depth.  Turn off depth read/writes
  // since this appears on the overlay and shouldn't be obscured.
  stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
  stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);

  // Billboard the shape
  billboard_ = new simVis::BillboardAutoTransform();
  billboard_->setName("Line Drawable Billboard");
  billboard_->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
  billboard_->setRotateInScreenSpace(true);
  billboard_->setAutoScaleToScreen(false);
  billboard_->setRotation(osg::Quat());

  addChild(billboard_.get());

  // Need some shape to start
  makeDiamond();
}

void LineDrawableHighlightNode::makeDiamond()
{
  resetLines_(1, GL_LINE_LOOP);
  // Make diamond the same size as square, so go out to sqrt(2)
  constexpr float SQRT_2 = 1.4142136f;
  auto line = lines_[0];
  line->pushVertex(osg::Vec3f(0.f, SQRT_2, 0.f));
  line->pushVertex(osg::Vec3f(-SQRT_2, 0.f, 0.f));
  line->pushVertex(osg::Vec3f(0.f, -SQRT_2, 0.f));
  line->pushVertex(osg::Vec3f(SQRT_2, 0.f, 0.f));
  line->finish();
}

void LineDrawableHighlightNode::makeSquare()
{
  resetLines_(1, GL_LINE_LOOP);
  auto line = lines_[0];
  line->pushVertex(osg::Vec3f(1.f, 1.f, 0.f));
  line->pushVertex(osg::Vec3f(-1.f, 1.f, 0.f));
  line->pushVertex(osg::Vec3f(-1.f, -1.f, 0.f));
  line->pushVertex(osg::Vec3f(1.f, -1.f, 0.f));
  line->finish();
}

void LineDrawableHighlightNode::makeCircle()
{
  resetLines_(1, GL_LINE_LOOP);
  auto line = lines_[0];
  constexpr float inc = M_TWOPI / static_cast<float>(MIN_NUM_LINE_SEGMENTS);
  for (int j = MIN_NUM_LINE_SEGMENTS; j > 0; --j)
  {
    const float angle = inc * j;
    const float x = sin(angle);
    const float y = cos(angle);
    line->pushVertex(osg::Vec3(x, y, 0.f));
  }
  line->finish();
}

void LineDrawableHighlightNode::makeSquareReticle()
{
  resetLines_(4, GL_LINE_STRIP);
  // Measures from RET_SIDE to 1.0, how much the reticle is visible
  constexpr float RET_SIDE = 0.5;
  // top right
  lines_[0]->pushVertex(osg::Vec3f(RET_SIDE, 1.f, 0.f));
  lines_[0]->pushVertex(osg::Vec3f(1.f, 1.f, 0.f));
  lines_[0]->pushVertex(osg::Vec3f(1.f, RET_SIDE, 0.f));
  lines_[0]->finish();
  // top left
  lines_[1]->pushVertex(osg::Vec3f(-RET_SIDE, 1.f, 0.f));
  lines_[1]->pushVertex(osg::Vec3f(-1.f, 1.f, 0.f));
  lines_[1]->pushVertex(osg::Vec3f(-1.f, RET_SIDE, 0.f));
  lines_[1]->finish();
  // bottom left
  lines_[2]->pushVertex(osg::Vec3f(-RET_SIDE, -1.f, 0.f));
  lines_[2]->pushVertex(osg::Vec3f(-1.f, -1.f, 0.f));
  lines_[2]->pushVertex(osg::Vec3f(-1.f, -RET_SIDE, 0.f));
  lines_[2]->finish();
  // bottom right
  lines_[3]->pushVertex(osg::Vec3f(RET_SIDE, -1.f, 0.f));
  lines_[3]->pushVertex(osg::Vec3f(1.f, -1.f, 0.f));
  lines_[3]->pushVertex(osg::Vec3f(1.f, -RET_SIDE, 0.f));
  lines_[3]->finish();
}

void LineDrawableHighlightNode::makeCoffin()
{
  // Scale value to apply to image to make larger or smaller as needed
  constexpr double SCALE = 1.0;

  // in box coordinates, y value of top of the head of coffin
  constexpr double HEAD_Y = 1.0 * SCALE;
  // y value of the bump-out (about 75% from bottom)
  constexpr double SHOULDER_Y = 0.5 * SCALE;
  // y value of the bottom
  constexpr double FEET_Y = -1.0 * SCALE;

  // positive X coordinate of the bump-out on right side near shoulders
  constexpr double SHOULDER_X = 0.5 * SCALE;
  // positive X coordinate of the shorter bump-out for head and feet
  constexpr double HEAD_X = 0.3 * SCALE;

  resetLines_(1, GL_LINE_LOOP);
  auto& line = lines_[0];
  line->pushVertex(osg::Vec3f(HEAD_X, HEAD_Y, 0.f));
  line->pushVertex(osg::Vec3f(SHOULDER_X, SHOULDER_Y, 0.f));
  line->pushVertex(osg::Vec3f(HEAD_X, FEET_Y, 0.f));
  line->pushVertex(osg::Vec3f(-HEAD_X, FEET_Y, 0.f));
  line->pushVertex(osg::Vec3f(-SHOULDER_X, SHOULDER_Y, 0.f));
  line->pushVertex(osg::Vec3f(-HEAD_X, HEAD_Y, 0.f));
  line->finish();
}

void LineDrawableHighlightNode::resetLines_(size_t newLineCount, int glMode)
{
  // Remove excess lines
  if (lines_.size() > newLineCount)
  {
    auto firstToDrop = lines_.begin() + newLineCount;
    for (auto iter = firstToDrop; iter != lines_.end(); ++iter)
      billboard_->removeChild(*iter);
    lines_.erase(firstToDrop, lines_.end());
  }

  // Clear and reset all existing lines
  for (auto iter = lines_.begin(); iter != lines_.end(); ++iter)
  {
    (*iter)->clear();
    (*iter)->setMode(glMode);
  }

  // Add new lines as needed
  while (lines_.size() < newLineCount)
  {
    osgEarth::LineDrawable* line = new osgEarth::LineDrawable(glMode);
    line->setName("Line Drawable Highlight Outline");
    line->setLineWidth(3.f);
    line->setLineSmooth(true);
    if (!lines_.empty())
      line->setColor(lines_[0]->getColor());
    billboard_->addChild(line);
    lines_.push_back(line);
  }
}

void LineDrawableHighlightNode::setColor(const osg::Vec4f& rgba)
{
  for (auto iter = lines_.begin(); iter != lines_.end(); ++iter)
  {
    (*iter)->setColor(rgba);
    (*iter)->dirty();
  }
}

void LineDrawableHighlightNode::setRadius(float radius)
{
  billboard_->setScale(osg::Vec3f(radius, radius, radius));
}

void LineDrawableHighlightNode::setAutoRotate(bool autoRotate)
{
  if (autoRotate_ == autoRotate)
    return;
  autoRotate_ = autoRotate;
    billboard_->setAutoRotateMode(autoRotate ? osg::AutoTransform::ROTATE_TO_SCREEN : osg::AutoTransform::NO_ROTATION);
  // update screen space rotation if auto rotate changed
  billboard_->setScreenSpaceRotation(autoRotate_ ? 0. : rotateRad_);
}

void LineDrawableHighlightNode::setScreenRotation(float rotateRad)
{
  rotateRad_ = rotateRad;
  if (!autoRotate_)
    billboard_->setScreenSpaceRotation(rotateRad_);
}

// --------------------------------------------------------------------------

CompositeHighlightNode::CompositeHighlightNode(simData::CircleHilightShape shape)
 : HighlightNode(),
   shape_(shape),
   rgba_(simVis::Color::White),
   radius_(1.f)
{
  setShape(shape_);
}

CompositeHighlightNode::CompositeHighlightNode(const CompositeHighlightNode& rhs, const osg::CopyOp& copyOp)
 : HighlightNode(rhs, copyOp),
   shape_(rhs.shape_),
   rgba_(rhs.rgba_),
   radius_(rhs.radius_),
   autoRotate_(rhs.autoRotate_)
{
  setShape(shape_);
}

CompositeHighlightNode::~CompositeHighlightNode()
{
}

void CompositeHighlightNode::setShape(simData::CircleHilightShape shape)
{
  if (child_.valid() && shape_ == shape)
    return;
  shape_ = shape;
  if (child_.valid())
    removeChild(child_.get());

  // Clear out child_, but hold onto it for the scope of this function
  osg::ref_ptr<HighlightNode> oldNode = child_;
  child_ = nullptr;

  // Most types are line geometry; try to cast up to avoid deleting
  LineDrawableHighlightNode* asLineDrawable = dynamic_cast<LineDrawableHighlightNode*>(oldNode.get());
  switch (shape_)
  {
  case simData::CH_PULSING_CIRCLE:
    child_ = new AreaHighlightNode();
    break;

  case simData::CH_CIRCLE:
    if (!asLineDrawable)
      asLineDrawable = new LineDrawableHighlightNode();
    child_ = asLineDrawable;
    asLineDrawable->makeCircle();
    break;

  case simData::CH_DIAMOND:
    if (!asLineDrawable)
      asLineDrawable = new LineDrawableHighlightNode();
    child_ = asLineDrawable;
    asLineDrawable->makeDiamond();
    break;

  case simData::CH_SQUARE:
    if (!asLineDrawable)
      asLineDrawable = new LineDrawableHighlightNode();
    child_ = asLineDrawable;
    asLineDrawable->makeSquare();
    break;

  case simData::CH_SQUARE_RETICLE:
    if (!asLineDrawable)
      asLineDrawable = new LineDrawableHighlightNode();
    child_ = asLineDrawable;
    asLineDrawable->makeSquareReticle();
    break;

  case simData::CH_COFFIN:
    if (!asLineDrawable)
      asLineDrawable = new LineDrawableHighlightNode();
    child_ = asLineDrawable;
    asLineDrawable->makeCoffin();
    break;
  }

  // Assert failure means an enum was added that isn't covered, or user somehow got
  // a bad value into the protobuf structure.
  assert(child_.valid());
  if (!child_.valid())
    return;
  addChild(child_.get());
  child_->setAutoRotate(autoRotate_);
  child_->setRadius(radius_);
  child_->setColor(rgba_);
  child_->setScreenRotation(rotateRad_);
}

void CompositeHighlightNode::setColor(const osg::Vec4f& rgba)
{
  if (rgba_ == rgba)
    return;
  rgba_ = rgba;
  if (child_.valid())
    child_->setColor(rgba_);
}

void CompositeHighlightNode::setRadius(float radius)
{
  if (radius_ == radius)
    return;
  radius_ = radius;
  if (child_.valid())
    child_->setRadius(radius_);
}

void CompositeHighlightNode::setAutoRotate(bool autoRotate)
{
  if (autoRotate_ == autoRotate)
    return;
  autoRotate_ = autoRotate;
  if (child_.valid())
    child_->setAutoRotate(autoRotate);
}

void CompositeHighlightNode::setScreenRotation(float yawRad)
{
  rotateRad_ = yawRad;
  if (child_.valid())
    child_->setScreenRotation(rotateRad_);
}

} //namespace simVis
