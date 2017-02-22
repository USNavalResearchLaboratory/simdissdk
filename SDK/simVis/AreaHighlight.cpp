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
#include "osg/Geode"
#include "osg/Geometry"
#include "osg/Depth"
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
AreaHighlightNode::AreaHighlightNode()
  : Geode(),
    color_(new osg::Uniform("simvis_areahighlight_color", simVis::Color::White)),
    radius_(new osg::Uniform("simvis_areahighlight_scale", 1.f))
{
  setName("AreaHighlight");
  init_();
}

AreaHighlightNode::AreaHighlightNode(const AreaHighlightNode &rhs, const osg::CopyOp& copyOp)
  : Geode(rhs, copyOp),
    color_(copyOp(color_)),
    radius_(copyOp(radius_))
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

  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
  geom->setUseVertexBufferObjects(true);

  osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array();
  geom->setVertexArray(vertexArray);

  osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
  geom->setColorArray(colorArray);
  geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  // Declare color for the center of the circle and the triangles
  osg::Vec4f color = osg::Vec4f(1, 1, 1, 1);

  // Center of the circle
  vertexArray->push_back(osg::Vec3(0, 0, 0));
  colorArray->push_back(color);

  // Make the edge of the circle darker and more transparent
  osg::Vec4f edgeColor = color * 0.8;

  float inc = M_TWOPI / static_cast<float>(MIN_NUM_LINE_SEGMENTS);
  for (int j = MIN_NUM_LINE_SEGMENTS; j > 0; --j)
  {
    const float angle = inc * j;
    const float x = sin(angle);
    const float y = cos(angle);
    vertexArray->push_back(osg::Vec3(x, y, 0));
    colorArray->push_back(edgeColor);
  }

  // Push in the initial vertex again, to close the circle
  vertexArray->push_back((*vertexArray)[1]);
  colorArray->push_back((*colorArray)[1]);

  geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, vertexArray->size()));
  addDrawable(geom);

  // Begin triangle creation ------------------------------
  osg::ref_ptr<osg::Geometry> triGeom = new osg::Geometry();
  triGeom->setUseVertexBufferObjects(true);

  osg::ref_ptr<osg::Vec3Array> triVertexArray = new osg::Vec3Array();
  triGeom->setVertexArray(triVertexArray);

  osg::ref_ptr<osg::Vec4Array> triColorArray = new osg::Vec4Array();
  triGeom->setColorArray(triColorArray);
  triGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

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
  addDrawable(triGeom);
}

} //namespace simVis
