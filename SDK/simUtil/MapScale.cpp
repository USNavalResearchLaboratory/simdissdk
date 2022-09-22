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
#include "osg/Callback"
#include "osg/ShadeModel"
#include "osgEarth/Capabilities"
#include "osgEarth/TerrainEngineNode"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Units.h"
#include "simVis/Registry.h"
#include "simVis/SceneManager.h"
#include "simVis/Text.h"
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "simUtil/Shaders.h"
#include "simUtil/MapScale.h"

namespace simUtil {

/** Pixels above and below the bar of empty space before showing text */
static const float BAR_BUFFER_PX = 1.f;

/** Helper callback for bindToFocusManager() that calls setView() whenever focus changes. */
class SetMapScaleViewCallback : public simVis::FocusManager::Callback
{
public:
  explicit SetMapScaleViewCallback(simUtil::MapScale* mapScale)
    : mapScale_(mapScale)
  {
  }

  // Override from Callback
  virtual void operator()(simVis::View* view, const EventType& evt)
  {
    if (evt == VIEW_FOCUSED)
    {
      osg::ref_ptr<simUtil::MapScale> mapScale;
      if (mapScale_.lock(mapScale))
        mapScale->setView(view);
    }
  }

private:
  osg::observer_ptr<simUtil::MapScale> mapScale_;
};

//////////////////////////////////////////////////////////////////

/** Update callback that recalculates the scale appropriately on each update cycle */
class MapScale::UpdateCallback : public osg::Callback
{
public:
  explicit UpdateCallback(MapScale* scale)
    : scale_(scale)
  {
  }

  virtual bool run(osg::Object* object, osg::Object* data)
  {
    osg::ref_ptr<MapScale> scale;
    if (scale_.lock(scale))
      scale->recalculatePixelDistance_();
    return traverse(object, data);
  }

private:
  osg::observer_ptr<MapScale> scale_;
};

//////////////////////////////////////////////////////////////////

MapScale::MapScale()
  : heightPx_(0.f),
    widthPx_(500.f),
    barHeightPx_(8.f),
    barColor1_(0.f, 0.f, 0.f, 1.f),
    barColor2_(1.f, 1.f, 1.f, 1.f),
    lrtbBgPadding_(10.f, 10.f, 5.f, 5.f),
    unitsProvider_(new MapScaleTwoUnitsProvider(simCore::Units::METERS, simCore::Units::KILOMETERS, 10000.0)),
    condenseText_(false)
{
  getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

  osgText::Font* font = simVis::Registry::instance()->getOrCreateFont("arial.ttf");

  unitsText_ = new simVis::Text;
  unitsText_->setFont(font);
  unitsText_->setCharacterSize(simVis::osgFontSize(12.f));
  unitsText_->setPosition(osg::Vec3f(0.f, 0.f, 0.f));
  unitsText_->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
  unitsText_->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
  unitsText_->setDataVariance(osg::Object::DYNAMIC);

  valueTextPrototype_ = new simVis::Text();
  valueTextPrototype_->setFont(font);
  valueTextPrototype_->setCharacterSize(simVis::osgFontSize(13.f));
  valueTextPrototype_->setPosition(osg::Vec3f(0.f, 0.f, 0.f));
  valueTextPrototype_->setAlignment(osgText::TextBase::CENTER_TOP);
  valueTextPrototype_->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
  valueTextPrototype_->setDataVariance(osg::Object::STATIC);

  // Create the background
  bgMatrix_ = new osg::MatrixTransform;
  bgMatrix_->setName("Background Scale Matrix");
  bgMatrix_->setDataVariance(osg::Object::DYNAMIC);
  osg::Geometry* backgroundGeom = new osg::Geometry;
  backgroundGeom->setName("Background");
  backgroundGeom->setUseVertexBufferObjects(true);
  bgMatrix_->addChild(backgroundGeom);

  // Create vertices for background
  osg::Vec3Array* bgVerts = new osg::Vec3Array(4);
  (*bgVerts)[0].set(osg::Vec3f(0.f, 0.f, 0.f));
  (*bgVerts)[1].set(osg::Vec3f(0.f, 1.f, 0.f));
  (*bgVerts)[2].set(osg::Vec3f(1.f, 0.f, 0.f));
  (*bgVerts)[3].set(osg::Vec3f(1.f, 1.f, 0.f));
  backgroundGeom->setVertexArray(bgVerts);

  // Create colors
  bgColorArray_ = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
  bgColorArray_->setDataVariance(osg::Object::DYNAMIC);
  backgroundGeom->setColorArray(bgColorArray_.get());
  // Note that setting the background color to 0.f alpha hides the background
  setBackgroundColor(osg::Vec4f(0.f, 0.f, 0.f, 0.f));

  // Create the primitive set
  backgroundGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 4));

  // Separate all the text into a separate group so the geode shader doesn't apply to it
  textGroup_ = new osg::Group;
  textGroup_->setName("Demarcations");
  textGroup_->addChild(unitsText_);
  geode_ = new osg::Geode;

  paddingGroup_ = new osg::MatrixTransform;
  paddingGroup_->setName("Padding Adjustment");
  paddingGroup_->setDataVariance(osg::Object::DYNAMIC);
  paddingGroup_->addChild(textGroup_.get());
  paddingGroup_->addChild(geode_.get());
  addChild(bgMatrix_.get());
  addChild(paddingGroup_.get());

  recalculateHeight_();

  // Include an update callback that will correctly configure the scale distance
  addUpdateCallback(new UpdateCallback(this));

  // If GLSL 3.3 is supported, use the MapScale shader to get flat coloring
  if (osgEarth::Registry::capabilities().supportsGLSL(3.3f))
  {
    osg::ref_ptr<osgEarth::VirtualProgram> vp = osgEarth::VirtualProgram::getOrCreate(geode_->getOrCreateStateSet());
    simUtil::Shaders shaderPackage;
    shaderPackage.load(vp.get(), shaderPackage.mapScale());
  }
  else // Fall back to FFP implementation
    getOrCreateStateSet()->setAttributeAndModes(new osg::ShadeModel(osg::ShadeModel::FLAT));
}

MapScale::MapScale(const MapScale& rhs, const osg::CopyOp& copyop)
  : Group(rhs, copyop),
    heightPx_(rhs.heightPx_),
    widthPx_(rhs.widthPx_),
    valueTextPrototype_(rhs.valueTextPrototype_),
    unitsText_(rhs.unitsText_),
    barHeightPx_(rhs.barHeightPx_),
    barColor1_(rhs.barColor1_),
    barColor2_(rhs.barColor2_),
    lrtbBgPadding_(rhs.lrtbBgPadding_),
    view_(rhs.view_),
    unitsProvider_(rhs.unitsProvider_)
{
}

MapScale::~MapScale()
{
}

void MapScale::setView(simVis::View* view)
{
  if (view_.get() != view)
  {
    view_ = view;
    recalculatePixelDistance_();
  }
}

void MapScale::bindToFocusManager(simVis::FocusManager* focusManager)
{
  if (!focusManager)
    return;
  setView(focusManager->getFocusedView());
  focusManager->addCallback(new SetMapScaleViewCallback(this));
}

void MapScale::setUnitsProvider(UnitsProvider* unitsProvider)
{
  // Fall back to a reasonable implementation rather than setting to nullptr
  if (unitsProvider)
    unitsProvider_ = unitsProvider;
  else
    unitsProvider_ = new MapScaleTwoUnitsProvider(simCore::Units::METERS, simCore::Units::KILOMETERS, 10000.0);
}

MapScale::UnitsProvider* MapScale::unitsProvider() const
{
  return unitsProvider_.get();
}

void MapScale::setCondenseText(bool condense)
{
  condenseText_ = condense;
}

bool MapScale::condenseText() const
{
  return condenseText_;
}

float MapScale::height() const
{
  // Include padding in the returned value
  return heightPx_ + lrtbBgPadding_[2] + lrtbBgPadding_[3];
}

void MapScale::setWidth(float widthPx)
{
  // Adjust widthPx to omit padding
  widthPx -= (lrtbBgPadding_[0] + lrtbBgPadding_[1]);
  if (widthPx_ == widthPx)
    return;
  widthPx_ = widthPx;
  fixBackgroundPosition_();
}

float MapScale::width() const
{
  // Include padding in the returned value
  return widthPx_ + lrtbBgPadding_[0] + lrtbBgPadding_[1];
}

void MapScale::setUnitsColor(const osg::Vec4f& color)
{
  unitsText_->setColor(color);
}

void MapScale::setUnitsFont(osgText::Font* font)
{
  unitsText_->setFont(font);
}

void MapScale::setUnitsCharacterSize(float sizePx)
{
  if (!simCore::areEqual(sizePx, unitsText_->getCharacterHeight()))
  {
    unitsText_->setCharacterSize(sizePx);
    recalculateHeight_();
  }
}

void MapScale::setUnitsVisible(bool visible)
{
  unitsText_->setNodeMask(visible ? ~0 : 0);
  recalculateHeight_();
}

void MapScale::setValuesColor(const osg::Vec4f& color)
{
  valueTextPrototype_->setColor(color);
}

void MapScale::setValuesFont(osgText::Font* font)
{
  valueTextPrototype_->setFont(font);
}

void MapScale::setValuesCharacterSize(float sizePx)
{
  if (!simCore::areEqual(sizePx, valueTextPrototype_->getCharacterHeight()))
  {
    valueTextPrototype_->setCharacterSize(sizePx);
    recalculateHeight_();
  }
}

void MapScale::setBarHeight(float sizePx)
{
  if (!simCore::areEqual(sizePx, barHeightPx_))
  {
    barHeightPx_ = sizePx;
    recalculateHeight_();
  }
}

void MapScale::setBarColor1(const osg::Vec4f& color)
{
  barColor1_ = color;
}

void MapScale::setBarColor2(const osg::Vec4f& color)
{
  barColor2_ = color;
}

void MapScale::recalculateHeight_()
{
  heightPx_ = 2 * BAR_BUFFER_PX + barHeightPx_ + valueTextPrototype_->getCharacterHeight();
  if (unitsText_->getNodeMask() != 0)
    heightPx_ += unitsText_->getCharacterHeight();

  // Fix the height on the value text so it is positioned correctly
  valueTextPrototype_->setPosition(osg::Vec3f(0.f, heightPx_, 0.f));
  // Fix background box
  fixBackgroundPosition_();
}

void MapScale::recalculatePixelDistance_()
{
  osg::ref_ptr<simVis::View> view;
  if (!view_.lock(view))
  {
    setVisible_(false);
    return;
  }
  simVis::SceneManager* sm = view->getSceneManager();
  if (!sm || !sm->getMapNode() || !sm->getMapNode()->getTerrainEngine())
  {
    setVisible_(false);
    return;
  }
  osg::NodePath mapNodePath;
#if OSGEARTH_SOVERSION >= 104
  mapNodePath.push_back(sm->getMapNode()->getTerrainEngine()->getNode());
#else
  mapNodePath.push_back(sm->getMapNode()->getTerrainEngine());
#endif

  const osg::Viewport* viewport = view->getCamera()->getViewport();
  if (!viewport)
  {
    setVisible_(false);
    return;
  }
  const int x = viewport->x() + viewport->width() / 2;
  const int y = viewport->y() + viewport->height() / 2;

  osgUtil::LineSegmentIntersector::Intersections results;
  // Note the need to subtract a pixel in order to stay inside the viewport on both left and right
  const float calcWidth = simCore::sdkMin(widthPx_, static_cast<float>(viewport->width() - 1));
  const float halfWidth = calcWidth * 0.5f;
  if (view->computeIntersections(x - halfWidth, y, mapNodePath, results))
  {
    const osg::Vec3d point1 = results.begin()->getWorldIntersectPoint();
    results.clear();
    if (view->computeIntersections(x + halfWidth, y, mapNodePath, results))
    {
      const osg::Vec3d point2 = results.begin()->getWorldIntersectPoint();
      // Calculate total distance, then scale it down to the pixel range
      const double dist1 = (point2 - point1).length();
      const double distPerPixel = dist1 / calcWidth;

      // Scale it back up to the range of widthPx_
      recalculateScale_(distPerPixel * widthPx_);
      return;
    }
  }

  // Turn off the geode, hiding all graphics
  setVisible_(false);
}

void MapScale::recalculateScale_(double maxDataRangeM)
{
  // Convert the data range into the range units we expect
  const simCore::Units& targetUnits = unitsProvider_->units(maxDataRangeM);
  const double inUnitsRange = simCore::Units::METERS.convertTo(targetUnits, maxDataRangeM);
  if (condenseText_)
    unitsText_->setText(targetUnits.abbreviation());
  else
    unitsText_->setText(targetUnits.name());

  // Determine exponent and significand
  int exponent = 0;
  const double significand = simCore::toScientific(inUnitsRange, &exponent);

  // Use 2.5-5-10 scaling, breaking into an appropriate number of subdivisions
  double multiply = 0.0;
  const unsigned int divisions = 5;
  if (significand <= 2.5)
    multiply = 1.0; // 0, 2, 4, 6, 8, 10
  else if (significand <= 5.0)
    multiply = 2.5; // 0, 5, 10, 15, 20, 25
  else
    multiply = 5.0; // 0, 10, 20, 30, 40, 50

  const double displayedMax = multiply * pow(10.0, exponent);
  // Assertion failure means the mapping to values doesn't work out correctly
  assert(displayedMax <= inUnitsRange);

  const unsigned int precision = simCore::sdkMax(0, static_cast<int>(1 - exponent));
  // Note that the on-screen width is trimmed to match positioning of displayedMax value
  drawBars_(displayedMax, divisions, widthPx_ * displayedMax / inUnitsRange, precision);
}

std::string MapScale::valueToString_(double value, unsigned int precision) const
{
  std::stringstream ss;
  ss << std::setprecision(precision) << std::fixed << value;
  return ss.str();
}

void MapScale::drawBars_(double maxValue, unsigned int numDivisions, float width, unsigned int precision)
{
  if (maxValue == 0.0)
  {
    setVisible_(false);
    return;
  }
  setVisible_(true);
  // Remove all geode drawables, we'll replace them all here
  geode_->removeDrawables(0, geode_->getNumDrawables());

  const double dataIncrement = maxValue / numDivisions;
  const double pixelIncrement = width / numDivisions;

  // Create the vertex array
  const unsigned int numVertices = 2 * (numDivisions + 1);
  osg::Vec3Array* verts = new osg::Vec3Array(numVertices);
  const float vertsTop = heightPx_ - valueTextPrototype_->getCharacterHeight() - BAR_BUFFER_PX;
  const float vertsBottom = vertsTop - barHeightPx_;

  // Create geometry
  osg::Geometry* geom = new osg::Geometry;
  geom->setDataVariance(osg::Object::DYNAMIC);
  geode_->setDataVariance(osg::Object::DYNAMIC);
  geom->setVertexArray(verts);
  osg::VertexBufferObject* vbo = verts->getVertexBufferObject();
  if (vbo)
    vbo->setUsage(GL_DYNAMIC_DRAW_ARB);

  // Create colors
  osg::Vec4Array* colors = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX, numVertices);
  geom->setColorArray(colors);
  vbo = colors->getVertexBufferObject();
  if (vbo)
    vbo->setUsage(GL_DYNAMIC_DRAW_ARB);

  // Create the primitive set
  osg::DrawArrays* primSet = new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
  geom->addPrimitiveSet(primSet);
  geode_->addDrawable(geom);

  // Now update the text.  Remove all text except the unit text (#0)
  textGroup_->removeChildren(1, textGroup_->getNumChildren() - 1);

  // For 0 divisions, you still have 2 positions (left and right)
  size_t vertIndex = 0;
  for (unsigned int k = 0; k <= numDivisions; ++k)
  {
    if (condenseText_ && (k == 0))
    {
      simVis::Text* valueText = new simVis::Text(*valueTextPrototype_);
      valueText->setPosition(osg::Vec3f(pixelIncrement * k, heightPx_, 0.f));
      valueText->setAlignment(osgText::Text::LEFT_TOP);
      valueText->setText("Div: " + valueToString_(dataIncrement, precision) + " " + unitsText_->getText().createUTF8EncodedString());
      textGroup_->addChild(valueText);
    }
    else if (!condenseText_ || (k == numDivisions))
    {
      simVis::Text* valueText = new simVis::Text(*valueTextPrototype_);
      valueText->setPosition(osg::Vec3f(pixelIncrement * k, heightPx_, 0.f));
      if (condenseText_ && (k == numDivisions))
        valueText->setAlignment(osgText::Text::RIGHT_TOP);
      valueText->setText(valueToString_(dataIncrement * k, precision));
      textGroup_->addChild(valueText);
    }

    // Push back top and bottom vertices and colors
    (*verts)[vertIndex].set(pixelIncrement * k, vertsTop, 0.f);
    const osg::Vec4f& color = (k % 2 != 0 ? barColor1_ : barColor2_);
    (*colors)[vertIndex] = color;
    ++vertIndex;
    (*verts)[vertIndex].set(pixelIncrement * k, vertsBottom, 0.f);
    (*colors)[vertIndex] = color;
    ++vertIndex;
  }

  // Run the shader generator
  osgEarth::Registry::shaderGenerator().run(geode_.get());
}

void MapScale::setVisible_(bool isVisible)
{
  // Note that we can't just set the node mask on "this" because that will stop the update
  // callback from firing off, preventing detection of cases where the map scale needs
  // to turn back on.
  const unsigned int nodeMask = (isVisible ? ~0 : 0);
  geode_->setNodeMask(nodeMask);
  textGroup_->setNodeMask(nodeMask);
}

void MapScale::fixBackgroundPosition_()
{
  bgMatrix_->setMatrix(osg::Matrix::scale(width(), height(), 1.f));
  paddingGroup_->setMatrix(osg::Matrix::translate(lrtbBgPadding_[0], lrtbBgPadding_[3], 0.f));
}

void MapScale::setBackgroundColor(const osg::Vec4f& color)
{
  (*bgColorArray_)[0] = color;
  bgColorArray_->dirty();
  bgMatrix_->setNodeMask(color[3] == 0.f ? 0 : ~0);
}

void MapScale::setPadding(float left, float right, float top, float bottom)
{
  lrtbBgPadding_[0] = left;
  lrtbBgPadding_[1] = right;
  lrtbBgPadding_[2] = top;
  lrtbBgPadding_[3] = bottom;
  fixBackgroundPosition_();
}

}
