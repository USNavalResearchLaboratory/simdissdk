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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cassert>
#include "osg/BlendFunc"
#include "osg/Depth"
#include "osg/Geometry"
#include "osg/MatrixTransform"
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
static const float ROTATE_FREQUENCY = -2.0f;
/// Controls speed of the flashing -- higher values pulse more frequently
static const float GLOW_FREQUENCY = 3.5f;
/// Controls amplitude of the flashing -- higher values strobe more drastically
static const float GLOW_AMPLITUDE = 0.2f;
/// Added to the pulsing amplitude's alpha so the highlight does not become completely transparent
static const float GLOW_MINIMUM_ALPHA = 0.6f;

/// Minimum number of line segments in a highlight circle
const int MIN_NUM_LINE_SEGMENTS = 90;
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
  const float fudgeFactor = 0.001f;

  // Draws 4 triangle (0,TIP_Y) to (SIDE_X,BASE_Y) to (-SIDE_X, BASE_Y)
  const float TRI_TIP_Y = 0.8f;
  const float TRI_BASE_Y = 0.9f;
  const float TRI_SIDE_X = 0.1f;

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
  // Places the highlight in a low-priority bin, and turn off depth writes to prevent it from covering other models
  stateSet->setRenderBinDetails(BIN_AREA_HIGHLIGHT, BIN_GLOBAL_SIMSDK);
  stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false));
  // Tells OpenGL to use the default blend function
  stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
  // Turn off backface culling
  stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

  // Create the matrix that handles scaling
  matrix_ = new osg::MatrixTransform();
  matrix_->setName("Line Drawable Size Matrix");
  addChild(matrix_.get());

  // Create the geometry
  line_ = new osgEarth::LineDrawable(GL_LINE_LOOP);
  line_->setName("Line Drawable Highlight Outline");
  line_->setLineWidth(3.f);
  line_->setLineSmooth(true);

  // Need some shape to start
  makeDiamond();
  matrix_->addChild(line_.get());
}

void LineDrawableHighlightNode::makeDiamond()
{
  line_->clear();
  line_->setMode(GL_LINE_LOOP);
  // Make diamond the same size as square, so go out to sqrt(2)
  const float SQRT_2 = 1.4142136f;
  line_->pushVertex(osg::Vec3f(0.f, SQRT_2, 0.f));
  line_->pushVertex(osg::Vec3f(-SQRT_2, 0.f, 0.f));
  line_->pushVertex(osg::Vec3f(0.f, -SQRT_2, 0.f));
  line_->pushVertex(osg::Vec3f(SQRT_2, 0.f, 0.f));
  line_->finish();
}

void LineDrawableHighlightNode::makeSquare()
{
  line_->clear();
  line_->setMode(GL_LINE_LOOP);
  line_->pushVertex(osg::Vec3f(1.f, 1.f, 0.f));
  line_->pushVertex(osg::Vec3f(-1.f, 1.f, 0.f));
  line_->pushVertex(osg::Vec3f(-1.f, -1.f, 0.f));
  line_->pushVertex(osg::Vec3f(1.f, -1.f, 0.f));
  line_->finish();
}

void LineDrawableHighlightNode::makeCircle()
{
  line_->clear();
  line_->setMode(GL_LINE_LOOP);
  float inc = M_TWOPI / static_cast<float>(MIN_NUM_LINE_SEGMENTS);
  for (int j = MIN_NUM_LINE_SEGMENTS; j > 0; --j)
  {
    const float angle = inc * j;
    const float x = sin(angle);
    const float y = cos(angle);
    line_->pushVertex(osg::Vec3(x, y, 0.f));
  }
  line_->finish();
}

void LineDrawableHighlightNode::makeSquareReticle()
{
  line_->clear();
  line_->setMode(GL_LINES);
  // Measures from RET_SIDE to 1.0, how much the reticle is visible
  const float RET_SIDE = 0.5;
  // top right
  line_->pushVertex(osg::Vec3f(RET_SIDE, 1.f, 0.f));
  line_->pushVertex(osg::Vec3f(1.f, 1.f, 0.f));
  line_->pushVertex(osg::Vec3f(1.f, 1.f, 0.f));
  line_->pushVertex(osg::Vec3f(1.f, RET_SIDE, 0.f));
  // top left
  line_->pushVertex(osg::Vec3f(-RET_SIDE, 1.f, 0.f));
  line_->pushVertex(osg::Vec3f(-1.f, 1.f, 0.f));
  line_->pushVertex(osg::Vec3f(-1.f, 1.f, 0.f));
  line_->pushVertex(osg::Vec3f(-1.f, RET_SIDE, 0.f));
  // bottom left
  line_->pushVertex(osg::Vec3f(-RET_SIDE, -1.f, 0.f));
  line_->pushVertex(osg::Vec3f(-1.f, -1.f, 0.f));
  line_->pushVertex(osg::Vec3f(-1.f, -1.f, 0.f));
  line_->pushVertex(osg::Vec3f(-1.f, -RET_SIDE, 0.f));
  // bottom right
  line_->pushVertex(osg::Vec3f(RET_SIDE, -1.f, 0.f));
  line_->pushVertex(osg::Vec3f(1.f, -1.f, 0.f));
  line_->pushVertex(osg::Vec3f(1.f, -1.f, 0.f));
  line_->pushVertex(osg::Vec3f(1.f, -RET_SIDE, 0.f));
  line_->finish();
}

void LineDrawableHighlightNode::setColor(const osg::Vec4f& rgba)
{
  line_->setColor(rgba);
}

void LineDrawableHighlightNode::setRadius(float radius)
{
  matrix_->setMatrix(osg::Matrix::scale(radius, radius, radius));
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
   radius_(rhs.radius_)
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
  child_ = NULL;

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
  }

  // Assert failure means an enum was added that isn't covered, or user somehow got
  // a bad value into the protobuf structure.
  assert(child_.valid());
  if (!child_.valid())
    return;
  addChild(child_.get());
  child_->setRadius(radius_);
  child_->setColor(rgba_);
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

} //namespace simVis
