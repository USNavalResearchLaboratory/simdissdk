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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/Depth"
#include "osg/Geode"
#include "osg/Geometry"
#include "osgEarth/DepthOffset"
#include "osgEarth/LineDrawable"
#include "osgEarth/NodeUtils"
#include "osgEarth/Registry"
#include "osgEarth/ShaderGenerator"
#include "osgEarth/StateSetCache"
#include "osgEarth/LabelNode"

#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/DatumConvert.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Units.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/Propagation.h"
#include "simCore/String/Constants.h"
#include "simCore/Time/TimeClass.h"

#include "simVis/AlphaTest.h"
#include "simVis/Antenna.h"
#include "simVis/Beam.h"
#include "simVis/ElevationQueryProxy.h"
#include "simVis/LocatorNode.h"
#include "simVis/OverheadMode.h"
#include "simVis/Platform.h"
#include "simVis/LobGroup.h" // Must come after Platform.h
#include "simVis/PolygonStipple.h"
#include "simVis/Registry.h"
#include "simVis/RFProp/RFPropagationFacade.h"
#include "simVis/RFProp/RFPropagationManager.h"
#include "simVis/Scenario.h"
#include "simVis/Utils.h"

#include "simVis/RangeTool.h"

/// Minimum depth bias for offsetting in meters
const int DEPTH_BUFFER_MIN_BIAS = 5000;
/** Reject pixels with an alpha equal or less than this value.  Useful for blending text correctly against SilverLining. */
static const float ALPHA_THRESHOLD = 0.05f;

namespace
{
  // convenience func to return an osg ENU vector calculated from the input ypr/orientation
  osg::Vec3d calcYprVector(const simCore::Vec3& ypr)
  {
    simCore::Vec3 enuVector;
    simCore::calculateVelocity(1.0, ypr.yaw(), ypr.pitch(), enuVector);
    return osg::Vec3d(enuVector.x(), enuVector.y(), enuVector.z());
  }

  // Labels close to each other should be put on the same line
  struct CloseEnoughCompare {
    bool operator()(const osg::Vec3& lhs, const osg::Vec3& rhs) const
    {
      if (!simCore::areEqual(lhs.x(), rhs.x(), 1.0))
        return lhs.x() < rhs.x();

      if (!simCore::areEqual(lhs.y(), rhs.y(), 1.0))
        return lhs.y() < rhs.y();

      if (!simCore::areEqual(lhs.z(), rhs.z(), 1.0))
        return lhs.z() < rhs.z();

      return false;
    }
  };
}

//------------------------------------------------------------------------

namespace simVis {

void RangeTool::RefreshGroup::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == nv.UPDATE_VISITOR && tool_.valid())
  {
    // send a null scenario and invalid timestamp, these will be handled appropriately below
    tool_->update(nullptr, simCore::INFINITE_TIME_STAMP);
    ADJUST_UPDATE_TRAV_COUNT(this, -1);
  }
  osg::Group::traverse(nv);
}

void RangeTool::RefreshGroup::scheduleRefresh()
{
//  OE_NOTICE << "SCHEDULING REFRESH..." << std::endl;
  if (this->getNumChildrenRequiringUpdateTraversal() == 0)
  {
    ADJUST_UPDATE_TRAV_COUNT(this, 1);
//    OE_NOTICE << "...DONE." << std::endl;
  }
}

//------------------------------------------------------------------------

RangeTool::RangeTool()
{
}

void RangeTool::onInstall(const ScenarioManager& scenario)
{
  root_ = new RefreshGroup(this);
  root_->setName("Range Tool Root Node");

  lastScenario_ = &scenario;
  // set the render bin order so that the tools will draw after the terrain.
  root_->getOrCreateStateSet()->setRenderBinDetails(BIN_RANGE_TOOL, BIN_GLOBAL_SIMSDK);
}

void RangeTool::onUninstall(const ScenarioManager& scenario)
{
  // remove all range tool state related to scenario
  associations_.clear();
  // scenario has already removed us from the scenegraph
  root_ = nullptr;
  lastScenario_ = nullptr;
}

void RangeTool::onUpdate(const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp, const EntityVector& updates)
{
  lastScenario_ = &scenario;

  for (AssociationVector::iterator i = associations_.begin(); i != associations_.end(); ++i)
  {
    (*i)->update(scenario, timeStamp);
  }

  resetDirty();
}

void RangeTool::update(const ScenarioManager* scenario, const simCore::TimeStamp& timeStamp)
{
  if (!scenario)
    scenario = lastScenario_.get();
  if (scenario)
    onUpdate(*scenario, timeStamp, EntityVector());
}

RangeTool::Association* RangeTool::add(simData::ObjectId obj1, simData::ObjectId obj2)
{
  Association* a = new Association(obj1, obj2);
  associations_.push_back(a);
  a->addParent(this);
  root_->addChild(a->getNode());
  setDirty();
  return a;
}

void RangeTool::remove(Association* assoc)
{
  AssociationVector::iterator i = std::find(associations_.begin(), associations_.end(), assoc);
  if (i != associations_.end())
  {
    root_->removeChild(i->get()->getNode());
    associations_.erase(i);
    setDirty();
  }
}

void RangeTool::setDirty()
{
  osgEarth::DirtyNotifier::setDirty();
  root_->scheduleRefresh();
}

//------------------------------------------------------------------------

RangeTool::GraphicOptions::GraphicOptions()
  : lineColor1_(1, .5, 0, 1), // orange
    lineColor2_(0, 0, 1, 1),  // blue
    lineStipple1_(0x00FF),
    lineStipple2_(0xFF00),
    lineWidth_(1),
    pieColor_(1, .5, 0, 1),  // orange
    pieSegments_(24),
    usePercentOfSlantDistance_(true),
    pieRadiusPercent_(0.30f),
    pieRadiusValue_(100.0f),
    pieRadiusUnits_(simCore::Units::METERS),
    useDepthTest_(true),
    showGraphics_(true)
{
  //nop
}

//------------------------------------------------------------------------

RangeTool::TextOptions::TextOptions()
  : displayAssociationName_(false),
    useScaleFont_(false),
    dynamicScaleFont_(false),
    outlineType_(OUTLINE_THICK),
    outlineColor_(0, 0, 0, 1),       // black
    font_("arial.ttf"),
    fontSize_(24.0f),
    scaleFontSize_(0.0f),
    xOffset_(0.0f),
    yOffset_(0.0f),
    color_(.5, .5, .5, 1),     // gray
    showText_(FULL),
    textLocation_(PAIRING_LINE)
{
  //nop
}


//------------------------------------------------------------------------

RangeTool::Calculation::Calculation(const std::string& name)
  : name_(name),
    labelPrecision_(2),
    angleType_(AZIMUTH),
    visible_(true),
    valid_(true),
    lastValue_(0.0)
{
  //nop
}

void RangeTool::Calculation::addGraphic(Graphic* graphic, bool useAsLabelGraphic)
{
  graphics_.push_back(graphic);
  graphic->addParent(this);

  if (useAsLabelGraphic)
  {
    setLabelGraphic(graphic);
  }

  setDirty();
}

void RangeTool::Calculation::setLabelGraphic(Graphic* graphic)
{
  labelGraphic_ = graphic;
  setDirty();
}

void RangeTool::Calculation::setLabelMeasurement(Measurement* measurement)
{
  labelMeasurement_ = measurement;
  setDirty();
}

void RangeTool::Calculation::setLabelUnits(const simCore::Units& units)
{
  labelUnits_ = units;
  setDirty();
}

void RangeTool::Calculation::setLabelPrecision(unsigned int precision)
{
  labelPrecision_ = precision;
  setDirty();
}

void RangeTool::Calculation::setVisible(bool visible)
{
  visible_ = visible;
  setDirty();
}

void RangeTool::Calculation::setAngleType(AngleType type)
{
  angleType_ = type;
  setDirty();
}

void RangeTool::Calculation::setLastValue(double value)
{
  valid_ = true;
  lastValue_ = value;
}

double RangeTool::Calculation::lastValue(const simCore::Units& outputUnits) const
{
  return labelMeasurement()->units().convertTo(outputUnits, lastValue_);
}

void RangeTool::Calculation::setValid(bool value)
{
  valid_ = value;
}

//------------------------------------------------------------------------

RangeTool::Association::Association(simData::ObjectId id1, simData::ObjectId id2)
  : id1_(id1),
    id2_(id2),
    visible_(true),
    xform_(nullptr)
{
  geode_ = new osg::Geode();
  osg::StateSet* s = geode_->getOrCreateStateSet();
  simVis::setLighting(s, 0);
  s->setMode(GL_BLEND, 1);
  s->setMode(GL_CULL_FACE, 0);
  s->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, false));
  geode_->setName("Line");

  labels_ = new osg::Group();
  s = labels_->getOrCreateStateSet();
  simVis::setLighting(s, 0);
  s->setMode(GL_BLEND, 1);
  s->setMode(GL_CULL_FACE, 0);
  s->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, false));
  labels_->setName("Graphics");
  osgEarth::HorizonCullCallback* horizonCull = new osgEarth::HorizonCullCallback();
  horizonCull->setCullByCenterPointOnly(true);
  labels_->setCullCallback(horizonCull);

  xform_ = new LocatorNode();
  xform_->addChild(geode_);
  xform_->addChild(labels_);
  xform_->setName("Range Tool Association");
  // enable flattening on the graphics, but not on the label node
  OverheadMode::enableGeometryFlattening(true, geode_);

  // create a state, and a magnetic datum convert for any measurements we might want to make
  state_ = new SimdisRangeToolState(new SimdisEntityState, new SimdisEntityState);
  state_->earthModel_ = simCore::WGS_84;

  labelPos_ = new SlantLineGraphic;
}

RangeTool::Association::~Association()
{
  delete state_;
}

void RangeTool::Association::add(Calculation* calc)
{
  calculations_.push_back(calc);
  calc->addParent(this);
  setDirty();
}

void RangeTool::Association::remove(Calculation* calc)
{
  CalculationVector::iterator i = std::find(calculations_.begin(), calculations_.end(), calc);
  if (i != calculations_.end())
  {
    calculations_.erase(i);
    setDirty();
  }
}

bool RangeTool::Association::update(const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp)
{
  // verify that both objects still exist in the scenario:
  osg::ref_ptr<EntityNode> obj1 = obj1_obs_.get();
  if (!obj1.valid())
  {
    obj1 = scenario.find(id1_);
    if (!obj1.valid())
    {
      osg::ref_ptr<EntityNode> obj2 = obj2_obs_.get();
      refresh_(obj1.get(), obj2.get(), scenario, timeStamp);
      return false;
    }

    obj1_obs_ = obj1.get();
    // xform will automatically track position of this entity
    xform_->setLocator(obj1_obs_->getLocator(), Locator::COMP_POSITION);
  }

  osg::ref_ptr<EntityNode> obj2 = obj2_obs_.get();
  if (!obj2.valid())
  {
    obj2 = scenario.find(id2_);
    if (!obj2.valid())
    {
      refresh_(obj1.get(), obj2.get(), scenario, timeStamp);
      return false;
    }

    obj2_obs_ = obj2.get();
  }

  // update visibility (association is visible only if both entities are visible)
  if (obj1->isVisible() && obj2->isVisible() && visible_ && xform_->getNodeMask() == 0)
  {
    xform_->setNodeMask(~0);
  }
  else if ((!obj1->isVisible() || !obj2->isVisible() || !visible_) && xform_->getNodeMask() != 0)
  {
    // This refresh will cause the last calculated values to become invalid, which is good thing
    refresh_(obj1.get(), obj2.get(), scenario, timeStamp);
    xform_->setNodeMask(0);
  }

  // see if either entity changed location
  if (isDirty()                                          ||
      obj1->getLocator()->outOfSyncWith(obj1LocatorRev_) ||
      obj2->getLocator()->outOfSyncWith(obj2LocatorRev_))
  {
    refresh_(obj1.get(), obj2.get(), scenario, timeStamp);

    obj1->getLocator()->sync(obj1LocatorRev_);
    obj2->getLocator()->sync(obj2LocatorRev_);

    resetDirty();
  }

  return true;
}

void RangeTool::Association::setDirty()
{
  labels_->removeChildren(0, labels_->getNumChildren()); // Clear existing labels to force a refresh to update colors if needed
  osgEarth::DirtyNotifier::setDirty();
}

osg::Node* RangeTool::Association::getNode() const
{
  return xform_.get();
}

void RangeTool::Association::refresh_(EntityNode* obj0, EntityNode* obj1, const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp)
{
  int rv = state_->populateEntityState(scenario, obj0, state_->beginEntity_);
  rv += state_->populateEntityState(scenario, obj1, state_->endEntity_);

  // clear out the geode
  geode_->removeDrawables(0, geode_->getNumDrawables());

  // If one of the entities is not valid at this time or the association is not visible; remove labels and return (graphics were removed above)
  if ((rv != 0) || !visible_)
  {
    labels_->removeChildren(0, labels_->getNumChildren());
    for (CalculationVector::iterator c = calculations_.begin(); c != calculations_.end(); ++c)
      (*c)->setValid(false);
    return;
  }

  // reset the coord_ cache
  state_->resetCoordCache();

  // ignore the invalid timestamp sent by RangeTool::RefreshGroup::traverse, reuse whatever timestamp was last used
  if (timeStamp != simCore::INFINITE_TIME_STAMP)
    state_->timeStamp_ = timeStamp;

  // initialize coordinate system and converter to optimize repeated conversions and support other values (flat projections)
  state_->coordConv_.setReferenceOrigin(state_->beginEntity_->lla_);

  // ensure that xform is synced with its locator
  xform_->syncWithLocator();

  // localizes all geometry to the reference point of obj0, preventing precision jitter
  state_->local2world_ = xform_->getMatrix();

  // invert to support ECEF->ENU conversions
  state_->world2local_.invert(state_->local2world_);

  state_->mapNode_ = scenario.mapNode();

  typedef std::pair<CalculationVector, TextOptions> LabelSetup;
  typedef std::map<osg::Vec3, LabelSetup, CloseEnoughCompare> Labels;
  Labels labels;
  osg::Vec3 labelPos = labelPos_->labelPos(*state_);

  for (CalculationVector::const_iterator c = calculations_.begin(); c != calculations_.end(); ++c)
  {
    Calculation* calc = c->get();

    //TODO: better change tracking
    calc->resetDirty();

    if (!calc->visible())
      continue;

    Measurement* calcMeasurement = calc->labelMeasurement();

    const GraphicVector& graphics = calc->graphics();

    for (GraphicVector::const_iterator g = graphics.begin(); g != graphics.end(); ++g)
    {
      Graphic* graphic = g->get();

      graphic->resetDirty();

      if (!graphic->graphicOptions().showGraphics_)
        continue;

      // pie slice graphics include special support for measurement
      if (graphic->graphicType() == Graphic::PIE_SLICE && calcMeasurement)
      {
        PieSliceGraphic* psg = dynamic_cast<PieSliceGraphic*>(graphic);
        if (psg)
          psg->setMeasuredValue(calcMeasurement->value(*state_));
        else
          assert(0);
      }

      graphic->render(geode_, *state_);

      if (graphic->graphicOptions().useDepthTest_ == false)
      {
        geode_->getOrCreateStateSet()->setAttributeAndModes(
          new osg::Depth(osg::Depth::ALWAYS, 0.0, 1.0, false),
          osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
        labels_->getOrCreateStateSet()->setAttributeAndModes(
          new osg::Depth(osg::Depth::ALWAYS, 0.0, 1.0, false),
          osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
      }
      else if (geode_->getStateSet())
      {
        geode_->getStateSet()->removeAttribute(osg::StateAttribute::DEPTH);
        if (labels_->getStateSet())
          labels_->getStateSet()->removeAttribute(osg::StateAttribute::DEPTH);
      }
    }

    if (calcMeasurement)
    {
      Graphic* posGraphic = calc->labelGraphic();
      if (posGraphic)
        posGraphic->resetDirty();

      if (!posGraphic && graphics.size() > 0)
      {
        posGraphic = graphics[0].get();
      }

      if (posGraphic)
      {
        if (calc->textOptions().textLocation_ == TextOptions::ALL)
          labelPos = posGraphic->labelPos(*state_);
        CalculationVector& calcs = labels[labelPos].first;
        calcs.push_back(calc);
        if (calcs.size() == 1)
        {
          labels[labelPos].second = calc->textOptions();
        }
      }
    }
  }

  // finally, assemble the labels.
  unsigned int labelCount = 0;
  unsigned int originalLabelCount = labels_->getNumChildren();
  for (Labels::const_iterator i = labels.begin(); i != labels.end(); ++i)
  {
    osg::Vec3                pos         = i->first;
    const LabelSetup&        setup       = i->second;
    const CalculationVector& calcs       = setup.first;
    const TextOptions&       textOptions = setup.second;
    std::stringstream        bufUtf8;

    if (textOptions.displayAssociationName_)
    {
      const std::string& name0 = obj0->getEntityName(EntityNode::DISPLAY_NAME);
      const std::string& name1 = obj1->getEntityName(EntityNode::DISPLAY_NAME);
      if (!name0.empty() && !name1.empty())
      {
        bufUtf8 << name0 << " to " << name1 << std::endl;
      }
    }

    bufUtf8 << std::fixed;

    for (CalculationVector::const_iterator c = calcs.begin(); c != calcs.end(); ++c)
    {
      Calculation* calc = c->get();

      if (c != calcs.begin())
      {
        if (textOptions.textLocation_ == TextOptions::ALL)
          bufUtf8 << ", ";
        else
          bufUtf8 << "\n";
      }

      Measurement* m = calc->labelMeasurement();
      const simCore::Units& units =
        calc->labelUnits().isSet() ?
        *calc->labelUnits() :
        m->units();

      double value = m->value(*state_);
      calc->setLastValue(value);
      value = m->units().convertTo(units, value);

      if (textOptions.showText_ == TextOptions::FULL)
        bufUtf8 << m->typeAbbr() << ": ";
      bufUtf8 << m->formatter()->stringValue(value, static_cast<int>(calc->labelPrecision()));
      if (units == simCore::Units::DEGREES)
        bufUtf8 << simCore::STR_DEGREE_SYMBOL_UTF8;
      else
        bufUtf8 << " " << units.abbreviation();
      if ((units == simCore::Units::DEGREES) && (textOptions.showText_ == TextOptions::VALUES_ONLY))
      {
        // If an angle was True or Magnetic add it to the back of the value if Values Only
        if (m->typeAbbr().find("(T)") != std::string::npos)
          bufUtf8 << "T";
        else if (m->typeAbbr().find("(M)") != std::string::npos)
          bufUtf8 << "M";
      }
    }

    if (textOptions.showText_ == TextOptions::NONE)
      continue;

    osgEarth::LabelNode* text = nullptr;
    if (labelCount >= labels_->getNumChildren())
    {
      osgEarth::Style style;
      osgEarth::TextSymbol* ts = style.getOrCreate<osgEarth::TextSymbol>();
      ts->alignment() = osgEarth::TextSymbol::ALIGN_CENTER_CENTER;
      ts->pixelOffset() = osg::Vec2s(textOptions.xOffset_, textOptions.yOffset_);
      // Font color
      ts->fill() = osgEarth::Fill(textOptions.color_.r(), textOptions.color_.g(), textOptions.color_.b(), textOptions.color_.a());
      // Outline
      if (textOptions.outlineType_ != TextOptions::OUTLINE_NONE &&  textOptions.outlineColor_.a() != 0)
      {
        ts->halo()->color() = textOptions.outlineColor_;
        ts->haloOffset() = simVis::outlineThickness(static_cast<simData::TextOutline>(textOptions.outlineType_));
        ts->halo()->width() = simVis::outlineThickness(static_cast<simData::TextOutline>(textOptions.outlineType_));
        ts->haloBackdropType() = osgText::Text::OUTLINE;
      }
      else
      {
        ts->halo()->color() = osg::Vec4();
        ts->haloOffset() = 0.f;
        ts->haloBackdropType() = osgText::Text::NONE;
      }
      // Font
      if (!textOptions.font_.empty())
      {
        std::string fileFullPath = simVis::Registry::instance()->findFontFile(textOptions.font_);
        if (!fileFullPath.empty()) // only set if font file found, uses default OS font otherwise
          ts->font() = fileFullPath;
      }
      // Explicitly enable UTF-8 encoding
      ts->encoding() = osgEarth::TextSymbol::ENCODING_UTF8;

#if OSG_VERSION_GREATER_OR_EQUAL(3,6,0)
      // Font sizes changed at 3.6, so rescale to keep a constant size
      ts->size() = simVis::osgFontSize(textOptions.fontSize_);
#else
      ts->size() = textOptions.fontSize_;
#endif

      text = new osgEarth::LabelNode("", style);
      text->setDynamic(true);
      text->setNodeMask(simVis::DISPLAY_MASK_LABEL);
      text->setHorizonCulling(false);
      text->setOcclusionCulling(false);

      // Set various states in order to make rendering text look better against SilverLining
      osg::StateSet* stateSet = text->getOrCreateStateSet();

      // Always write to the depth buffer, overriding the osgEarth internal settings
      stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, true), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
      AlphaTest::setValues(stateSet, ALPHA_THRESHOLD, osg::StateAttribute::ON);
      labels_->addChild(text);
    }
    else
      text = static_cast<osgEarth::LabelNode*>(labels_->getChild(labelCount));

    labelCount++;

    text->getPositionAttitudeTransform()->setPosition(pos);
    text->setText(bufUtf8.str());
  }

  // shader needed to draw text properly
  if (labelCount != originalLabelCount)
  {
    if (labelCount < originalLabelCount)
      labels_->removeChildren(labelCount, originalLabelCount - labelCount);
    osgEarth::Registry::shaderGenerator().run(labels_);
  }
}

//----------------------------------------------------------------------------

void RangeTool::LineGraphic::createGeometry(osg::Vec3Array* verts, GLenum mode, osg::Geode* geode, RangeToolState& state)
{
  if (verts && verts->size() >= 2)
  {
    // To support the double-stippling pattern we have to make two geometries. If the first
    // stipple is 0xFFFF, just make one.
    for (unsigned int i = 0; i < 2; ++i)
    {
      osgEarth::LineDrawable* geom = new osgEarth::LineDrawable(mode);
      geom->importVertexArray(verts);
      geom->setColor(i==0 ? options_.lineColor1_ : options_.lineColor2_);
      geom->setStipplePattern(i==0 ? options_.lineStipple1_ : options_.lineStipple2_);
      geom->setLineWidth(options_.lineWidth_);
      // geode installs the LineDrawable shader by default, so no need to do so here

      geode->addChild(geom);

      // don't bother drawing the second line if the first has a full stipple OR if the
      // second stipple is set to zero
      if (options_.lineStipple1_ == 0xFFFF || options_.lineStipple2_ == 0)
        break;
    }
  }
}

//----------------------------------------------------------------------------

void RangeTool::PieSliceGraphic::createGeometry(const osg::Vec3& originVec, osg::Vec3d startVec, osg::Vec3d endVec, double angle, osg::Geode* geode, RangeToolState& state)
{
  osg::Geometry*  arcEndVecGeom = nullptr;
  osg::Geometry*  startVecGeom  = nullptr;
  osg::Vec3Array* verts         = nullptr;

  if (geode)
  {
    arcEndVecGeom = new osg::Geometry();
    arcEndVecGeom->setUseVertexBufferObjects(true);

    verts = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    arcEndVecGeom->setVertexArray(verts);

    osg::Vec4Array* colors = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    (*colors)[0] = options_.pieColor_;
    arcEndVecGeom->setColorArray(colors);

    osg::StateSet* ss = arcEndVecGeom->getOrCreateStateSet();
    simVis::PolygonStipple::setValues(ss, true, 0);

    geode->addDrawable(arcEndVecGeom);

    // the geometry that holds the start vector; it shares the contents of the
    // first geometry, but applies a different state set.
    startVecGeom = new osg::Geometry();
    startVecGeom->setUseVertexBufferObjects(true);
    startVecGeom->setVertexArray(verts);
    startVecGeom->setColorArray(colors);
    geode->addDrawable(startVecGeom);
  }

  osg::BoundingBox bbox;
  startVec.normalize();
  endVec.normalize();

  double pieRadius = options_.pieRadiusUnits_.convertTo(simCore::Units::METERS, options_.pieRadiusValue_);
  if (options_.usePercentOfSlantDistance_)
  {
    // using the RAE entity's range if both RAE entities share the same host
    if (state.beginEntity_->hostId_ == state.endEntity_->hostId_)
    {
      if (state.beginEntity_->type_ != simData::PLATFORM)
        pieRadius = static_cast<SimdisEntityState*>(state.beginEntity_)->node_->range();
      else
        pieRadius = static_cast<SimdisEntityState*>(state.endEntity_)->node_->range();
    }
    else
    {
      SlantDistanceMeasurement slant;
      pieRadius = slant.value(state);
    }

    // If radius is still zero use the default value otherwise scale radius by the percentage
    if (pieRadius <= 0.0)
      pieRadius = options_.pieRadiusUnits_.convertTo(simCore::Units::METERS, options_.pieRadiusValue_);
    else
      pieRadius *= options_.pieRadiusPercent_;
  }

  // center of the arc
  if (geode)
    verts->push_back(originVec);

  // a quat to rotate between the two vectors:
  osg::Quat q1;
  q1.makeRotate(startVec, endVec);

  // interpolator. if the angle is > 180 degrees, go the long way.
  Math::QuatSlerp slerp(osg::Quat(), q1, angle > M_PI);

  unsigned int seg;
  // sweep between the vecs
  for (seg = 0; seg <= options_.pieSegments_; ++seg)
  {
    const osg::Quat& rot = slerp(static_cast<double>(seg) / options_.pieSegments_);
    const osg::Vec3 vert = rot * startVec * pieRadius + originVec;
    bbox.expandBy(vert);
    if (geode)
      verts->push_back(vert);
  }

  if (geode)
  {
    arcEndVecGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, seg+1));

    verts->push_back(startVec * pieRadius * 1.5 + originVec);
    verts->push_back(endVec   * pieRadius * 1.5 + originVec);

    osgEarth::LineDrawable* vecs = new osgEarth::LineDrawable(GL_LINES);
    vecs->allocate(4);
    vecs->setVertex(0, verts->front());
    vecs->setVertex(1, (*verts)[verts->size()-2]);
    vecs->setVertex(2, verts->front());
    vecs->setVertex(3, verts->back());
    vecs->setColor(options_.pieColor_);
    // geode installs the LineDrawable shader by default, not needed here

    geode->addChild(vecs);
  }

#ifdef DRAW_PIE_NORMAL

  osg::Vec3 normal(0, 0, 1);
  if (fabs(startVec * endVec) < 0.99)
    normal = (startVec ^ endVec);

  verts->push_back(normal * radius);
  startVecPrim->push_back(0);
  startVecPrim->push_back(verts->size()-1);

#endif

  labelPos_ = bbox.center();
}

osg::Vec3 RangeTool::PieSliceGraphic::labelPos(RangeToolState& state)
{
  if (!labelPos_.isSet())
    render(nullptr, state);
  return *labelPos_;
}


//----------------------------------------------------------------------------

bool RangeTool::Graphic::hasPosition_(simData::ObjectType type) const
{
  return ((type == simData::PLATFORM) || (type == simData::CUSTOM_RENDERING));
}

//----------------------------------------------------------------------------

RangeTool::GroundLineGraphic::GroundLineGraphic()
  : LineGraphic("GroundLine", LINE)
{ }

void RangeTool::GroundLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  simCore::Vec3 lla0(state.beginEntity_->lla_.x(), state.beginEntity_->lla_.y(), 0.0);
  simCore::Vec3 lla1(state.endEntity_->lla_.x(), state.endEntity_->lla_.y(), 0.0);
  state.line(lla0, lla1, 1.0, verts.get());
  createGeometry(verts.get(), GL_LINE_STRIP, geode, state);
}

osg::Vec3 RangeTool::GroundLineGraphic::labelPos(RangeToolState& state)
{
  simCore::Vec3 lla0(state.beginEntity_->lla_.x(), state.beginEntity_->lla_.y(), 0.0);
  simCore::Vec3 lla1(state.endEntity_->lla_.x(), state.endEntity_->lla_.y(), 0.0);
  auto mid = state.midPoint(lla0, lla1, 0.0);
  return state.lla2local(mid.x(), mid.y(), 0.0);
}

//----------------------------------------------------------------------------

RangeTool::SlantLineGraphic::SlantLineGraphic() :
LineGraphic("SlantLine", LINE) { }

void RangeTool::SlantLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_OBJ_0);
  (*verts)[1] = state.coord(RangeToolState::COORD_OBJ_1);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::SlantLineGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_OBJ_0) + state.coord(RangeToolState::COORD_OBJ_1)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeginAltitudeLineGraphic::BeginAltitudeLineGraphic()
  : LineGraphic("BeginAltitudeLine", LINE)
{ }

void RangeTool::BeginAltitudeLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_OBJ_0);
  (*verts)[1] = state.coord(RangeToolState::COORD_OBJ_0_0HAE);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::BeginAltitudeLineGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_OBJ_0) + state.coord(RangeToolState::COORD_OBJ_0_0HAE)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::EndAltitudeLineGraphic::EndAltitudeLineGraphic()
  : LineGraphic("EndAltitudeLine", LINE)
{ }

void RangeTool::EndAltitudeLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_OBJ_1);
  (*verts)[1] = state.coord(RangeToolState::COORD_OBJ_1_0HAE);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::EndAltitudeLineGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_OBJ_1) + state.coord(RangeToolState::COORD_OBJ_1_0HAE)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeginAltitudeLineToEndAltitudeGraphic::BeginAltitudeLineToEndAltitudeGraphic()
  : LineGraphic("BeginAltitudeLineToEndAltitude", LINE)
{ }

void RangeTool::BeginAltitudeLineToEndAltitudeGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_OBJ_0);
  (*verts)[1] = state.coord(RangeToolState::COORD_OBJ_0_AT_OBJ_1_ALT);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::BeginAltitudeLineToEndAltitudeGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_OBJ_0) + state.coord(RangeToolState::COORD_OBJ_0_AT_OBJ_1_ALT)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::EndAltitudeLineToBeginAltitudeGraphic::EndAltitudeLineToBeginAltitudeGraphic() :
LineGraphic("EndAltitudeLineToBeginAltitude", LINE) { }

void RangeTool::EndAltitudeLineToBeginAltitudeGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_OBJ_1);
  (*verts)[1] = state.coord(RangeToolState::COORD_OBJ_1_AT_OBJ_0_ALT);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::EndAltitudeLineToBeginAltitudeGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_OBJ_1) + state.coord(RangeToolState::COORD_OBJ_1_AT_OBJ_0_ALT)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeginToEndLineAtBeginAltitudeGraphic::BeginToEndLineAtBeginAltitudeGraphic()
  : LineGraphic("BeginToEndLineAtBeginAltitude", LINE)
{ }

void RangeTool::BeginToEndLineAtBeginAltitudeGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  simCore::Vec3 lla1(state.endEntity_->lla_.x(), state.endEntity_->lla_.y(), state.beginEntity_->lla_.z());
  state.line(state.beginEntity_->lla_, lla1, 0.0, verts.get());
  createGeometry(verts.get(), GL_LINE_STRIP, geode, state);
}

osg::Vec3 RangeTool::BeginToEndLineAtBeginAltitudeGraphic::labelPos(RangeToolState& state)
{
  return state.lla2local(state.endEntity_->lla_.x(), state.endEntity_->lla_.y(), state.beginEntity_->lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::BeginToEndLineAtEndAltitudeGraphic::BeginToEndLineAtEndAltitudeGraphic()
  : LineGraphic("BeginToEndLineAtEndAltitude", LINE)
{ }

void RangeTool::BeginToEndLineAtEndAltitudeGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  simCore::Vec3 lla0(state.beginEntity_->lla_.x(), state.beginEntity_->lla_.y(), state.endEntity_->lla_.z());
  state.line(lla0, state.endEntity_->lla_, 0.0, verts.get());
  createGeometry(verts.get(), GL_LINE_STRIP, geode, state);
}

osg::Vec3 RangeTool::BeginToEndLineAtEndAltitudeGraphic::labelPos(RangeToolState& state)
{
  return state.lla2local(state.beginEntity_->lla_.x(), state.beginEntity_->lla_.y(), state.endEntity_->lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::BeamGroundLineGraphic::BeamGroundLineGraphic()
  : LineGraphic("BeamGroundLine", LINE)
{ }

void RangeTool::BeamGroundLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();

  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  simCore::Vec3 lla0(from.x(), from.y(), 0.0);
  simCore::Vec3 lla1(to.x(), to.y(), 0.0);
  state.line(lla0, lla1, 1.0, verts.get());
  createGeometry(verts.get(), GL_LINE_STRIP, geode, state);
}

osg::Vec3 RangeTool::BeamGroundLineGraphic::labelPos(RangeToolState& state)
{
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  auto mid = state.midPoint(from, to, 0.0);
  return state.lla2local(mid.x(), mid.y(), 0.0);

}

//----------------------------------------------------------------------------

RangeTool::BeamSlantLineGraphic::BeamSlantLineGraphic()
  : LineGraphic("BeamSlantLine", LINE)
{ }

void RangeTool::BeamSlantLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_BEAM_0);
  (*verts)[1] = state.coord(RangeToolState::COORD_BEAM_1);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::BeamSlantLineGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_BEAM_0) + state.coord(RangeToolState::COORD_BEAM_1)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeamBeginAltitudeLineGraphic::BeamBeginAltitudeLineGraphic()
  : LineGraphic("BeamBeginAltitudeLine", LINE)
{ }

void RangeTool::BeamBeginAltitudeLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_BEAM_0);
  (*verts)[1] = state.coord(RangeToolState::COORD_BEAM_0_0HAE);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::BeamBeginAltitudeLineGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_BEAM_0) + state.coord(RangeToolState::COORD_BEAM_0_0HAE)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeamEndAltitudeLineGraphic::BeamEndAltitudeLineGraphic()
  : LineGraphic("BeamEndAltitudeLine", LINE)
{ }

void RangeTool::BeamEndAltitudeLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_BEAM_1);
  (*verts)[1] = state.coord(RangeToolState::COORD_BEAM_1_0HAE);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::BeamEndAltitudeLineGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_BEAM_1) + state.coord(RangeToolState::COORD_BEAM_1_0HAE)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeamBeginAltitudeLineToEndAltitudeGraphic::BeamBeginAltitudeLineToEndAltitudeGraphic()
  : LineGraphic("BeamBeginAltitudeLineToEndAltitude", LINE)
{ }

void RangeTool::BeamBeginAltitudeLineToEndAltitudeGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_BEAM_0);
  (*verts)[1] = state.coord(RangeToolState::COORD_BEAM_0_AT_BEAM_1_ALT);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::BeamBeginAltitudeLineToEndAltitudeGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_BEAM_0) + state.coord(RangeToolState::COORD_BEAM_0_AT_BEAM_1_ALT)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeamEndAltitudeLineToBeginAltitudeGraphic::BeamEndAltitudeLineToBeginAltitudeGraphic()
  : LineGraphic("BeamEndAltitudeLineToBeginAltitude", LINE)
{ }

void RangeTool::BeamEndAltitudeLineToBeginAltitudeGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_BEAM_1);
  (*verts)[1] = state.coord(RangeToolState::COORD_BEAM_1_AT_BEAM_0_ALT);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::BeamEndAltitudeLineToBeginAltitudeGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_BEAM_1) + state.coord(RangeToolState::COORD_BEAM_1_AT_BEAM_0_ALT)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeamBeginToEndLineAtBeginAltitudeGraphic::BeamBeginToEndLineAtBeginAltitudeGraphic()
  : LineGraphic("BeamBeginToEndLineAtBeginAltitude", LINE)
{ }

void RangeTool::BeamBeginToEndLineAtBeginAltitudeGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  simCore::Vec3 lla1(to.x(), to.y(), from.z());
  state.line(from, lla1, 0.0, verts.get());
  createGeometry(verts.get(), GL_LINE_STRIP, geode, state);
}

osg::Vec3 RangeTool::BeamBeginToEndLineAtBeginAltitudeGraphic::labelPos(RangeToolState& state)
{
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  return state.lla2local(to.x(), to.y(), from.z());
}

//----------------------------------------------------------------------------

RangeTool::BeamBeginToEndLineAtEndAltitudeGraphic::BeamBeginToEndLineAtEndAltitudeGraphic()
  : LineGraphic("BeamBeginToEndLineAtEndAltitude", LINE)
{ }

void RangeTool::BeamBeginToEndLineAtEndAltitudeGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  simCore::Vec3 lla0(from.x(), from.y(), to.z());
  state.line(lla0, to, 0.0, verts.get());
  createGeometry(verts.get(), GL_LINE_STRIP, geode, state);
}

osg::Vec3 RangeTool::BeamBeginToEndLineAtEndAltitudeGraphic::labelPos(RangeToolState& state)
{
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  return state.lla2local(from.x(), from.y(), to.z());
}

//----------------------------------------------------------------------------

RangeTool::DownRangeLineGraphic::DownRangeLineGraphic()
  : LineGraphic("DownRangeLine", LINE)
{ }

void RangeTool::DownRangeLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  simCore::Vec3 crdr = state.local2lla(state.coord(RangeToolState::COORD_DR));
  state.line(state.beginEntity_->lla_, crdr, 0.0, verts.get());
  createGeometry(verts.get(), GL_LINE_STRIP, geode, state);
}

osg::Vec3 RangeTool::DownRangeLineGraphic::labelPos(RangeToolState& state)
{
  simCore::Vec3 crdr = state.local2lla(state.coord(RangeToolState::COORD_DR));
  auto mid = state.midPoint(state.beginEntity_->lla_, crdr, 0.0);
  return state.lla2local(mid.x(), mid.y(), state.beginEntity_->lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::VelAzimDownRangeLineGraphic::VelAzimDownRangeLineGraphic()
  : LineGraphic("VelAzimDownRangeLine", LINE)
{ }

void RangeTool::VelAzimDownRangeLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  simCore::Vec3 end = state.local2lla(state.coord(RangeToolState::COORD_VEL_AZIM_DR));
  state.line(state.beginEntity_->lla_, end, 0.0, verts.get());
  createGeometry(verts.get(), GL_LINE_STRIP, geode, state);
}

osg::Vec3 RangeTool::VelAzimDownRangeLineGraphic::labelPos(RangeToolState& state)
{
  simCore::Vec3 end = state.local2lla(state.coord(RangeToolState::COORD_VEL_AZIM_DR));
  auto mid = state.midPoint(state.beginEntity_->lla_, end, 0.0);
  return state.lla2local(mid.x(), mid.y(), state.beginEntity_->lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::VelAzimCrossRangeLineGraphic::VelAzimCrossRangeLineGraphic()
  : LineGraphic("VelAzimCrossRangeLine", LINE)
{ }

void RangeTool::VelAzimCrossRangeLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  simCore::Vec3 start = state.local2lla(state.coord(RangeToolState::COORD_VEL_AZIM_DR));
  simCore::Vec3 end = state.local2lla(state.coord(RangeToolState::COORD_OBJ_1_AT_OBJ_0_ALT));
  state.line(start, end, 0.0, verts.get());
  createGeometry(verts.get(), GL_LINE_STRIP, geode, state);
}

osg::Vec3 RangeTool::VelAzimCrossRangeLineGraphic::labelPos(RangeToolState& state)
{
  simCore::Vec3 start = state.local2lla(state.coord(RangeToolState::COORD_VEL_AZIM_DR));
  auto mid = state.midPoint(state.endEntity_->lla_, start, 0.0);
  return state.lla2local(mid.x(), mid.y(), state.endEntity_->lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::CrossRangeLineGraphic::CrossRangeLineGraphic()
  : LineGraphic("CrossRangeLine", LINE)
{ }

void RangeTool::CrossRangeLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  simCore::Vec3 crdr = state.local2lla(state.coord(RangeToolState::COORD_DR));
  simCore::Vec3 lla1(state.endEntity_->lla_.x(), state.endEntity_->lla_.y(), state.beginEntity_->lla_.z());
  state.line(crdr, lla1, 0.0, verts.get());
  createGeometry(verts.get(), GL_LINE_STRIP, geode, state);
}

osg::Vec3 RangeTool::CrossRangeLineGraphic::labelPos(RangeToolState& state)
{
  simCore::Vec3 crdr = state.local2lla(state.coord(RangeToolState::COORD_DR));
  auto mid = state.midPoint(state.endEntity_->lla_, crdr, 0.0);
  return state.lla2local(mid.x(), mid.y(), state.beginEntity_->lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::DownRangeCrossRangeDownLineGraphic::DownRangeCrossRangeDownLineGraphic()
  : LineGraphic("CrossRangeLine", LINE)
{ }

void RangeTool::DownRangeCrossRangeDownLineGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(RangeToolState::COORD_OBJ_1_AT_OBJ_0_ALT);
  (*verts)[1] = state.coord(RangeToolState::COORD_OBJ_1);
  createGeometry(verts.get(), GL_LINES, geode, state);
}

osg::Vec3 RangeTool::DownRangeCrossRangeDownLineGraphic::labelPos(RangeToolState& state)
{
  return (state.coord(RangeToolState::COORD_OBJ_1_AT_OBJ_0_ALT) + state.coord(RangeToolState::COORD_OBJ_1)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::TrueAzimuthPieSliceGraphic::TrueAzimuthPieSliceGraphic()
  : PieSliceGraphic("True Azimuth")
{ }

void RangeTool::TrueAzimuthPieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::Vec3d endVec;

  if (state.beginEntity_->hostId_ != state.endEntity_->hostId_)
  {
    endVec = state.coord(RangeToolState::COORD_OBJ_1_AT_OBJ_0_ALT);
    endVec[2] = 0.0;  // COORD_OBJ_1_AT_OBJ_0_ALT accounts for the earth's curvature which we don't want, so jam into local plane
  }
  else
  {
    // Get the RAE object to get its angles
    simCore::Vec3& ori = state.beginEntity_->ypr_;
    if (!hasPosition_(state.endEntity_->type_))
      ori = state.endEntity_->ypr_;

    endVec = osg::Vec3d(sin(ori.x())*cos(ori.y()), cos(ori.x())*cos(ori.y()), 0.0);
  }
  createGeometry(state.coord(RangeToolState::COORD_OBJ_0), osg::Y_AXIS, endVec, measuredValue_, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::TrueElevationPieSliceGraphic::TrueElevationPieSliceGraphic()
  : PieSliceGraphic("True Elevation")
{
  options_.pieColor_.set(.5, .5, 1, 1); // blue
}

void RangeTool::TrueElevationPieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::Vec3d startVec;
  osg::Vec3d endVec;

  if (state.beginEntity_->hostId_ != state.endEntity_->hostId_)
  {
    startVec = state.coord(RangeToolState::COORD_OBJ_1_AT_OBJ_0_ALT);
    startVec[2] = 0.0;  // COORD_OBJ_1_AT_OBJ_0_ALT accounts for the earth's curvature which we don't want, so jam into local plane
    endVec = state.coord(RangeToolState::COORD_OBJ_1);
  }
  else
  {
    // Get the RAE object to get its angles
    simCore::Vec3& ori = state.beginEntity_->ypr_;
    if (!hasPosition_(state.endEntity_->type_))
      ori = state.endEntity_->ypr_;

    startVec = calcYprVector(ori);
    endVec = osg::Vec3d(startVec.x(), startVec.y(), 0.0);
  }

  createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVec, endVec, measuredValue_, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::TrueCompositeAnglePieSliceGraphic::TrueCompositeAnglePieSliceGraphic()
  : PieSliceGraphic("True Composite Angle")
{
  options_.pieColor_.set(.5, .5, .5, 1); // gray
}

void RangeTool::TrueCompositeAnglePieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::Vec3d endVec;

  if (state.beginEntity_->hostId_ != state.endEntity_->hostId_)
  {
    endVec = state.coord(RangeToolState::COORD_OBJ_1);
  }
  else
  {
    // Get the RAE object to get its angles
    simCore::Vec3& ori = state.beginEntity_->ypr_;
    if (!hasPosition_(state.endEntity_->type_))
      ori = state.endEntity_->ypr_;

    endVec = calcYprVector(ori);
  }

  createGeometry(state.coord(RangeToolState::COORD_OBJ_0), osg::Y_AXIS, endVec, measuredValue_, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::MagneticAzimuthPieSliceGraphic::MagneticAzimuthPieSliceGraphic()
  : PieSliceGraphic("Magnetic Azimuth")
{ }

void RangeTool::MagneticAzimuthPieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::Vec3d startVecENU;
  osg::Vec3d endVecENU;
  const double magAz = measuredValue_;

  if (state.beginEntity_->hostId_ != state.endEntity_->hostId_)
  {
    endVecENU = state.coord(RangeToolState::COORD_OBJ_1_AT_OBJ_0_ALT);
    endVecENU[2] = 0.0;  // COORD_OBJ_1_AT_OBJ_0_ALT accounts for the earth's curvature which we don't want, so jam into local plane
    // start vec is end vec (true azim to object 1) rotated by magAz
    startVecENU = state.rotateEndVec(magAz);
    startVecENU[2] = 0.0; // flatten this (also) onto the local tangent place
  }
  else
  {
    // Determine which is the RAE object, and get its angles
    simCore::Vec3 ori = (!hasPosition_(state.endEntity_->type_)) ? state.endEntity_->ypr_ : state.beginEntity_->ypr_;

    endVecENU = osg::Vec3d(sin(ori.x())*cos(ori.y()), cos(ori.x())*cos(ori.y()), 0.0);
    // start vec is end vec (true azim to rae object) rotated by magAz
    ori.setYaw(ori.yaw() - magAz);
    startVecENU = osg::Vec3d(sin(ori.x())*cos(ori.y()), cos(ori.x())*cos(ori.y()), 0.0);
  }

  createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, endVecENU, magAz, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::RelOriAzimuthPieSliceGraphic::RelOriAzimuthPieSliceGraphic()
  : PieSliceGraphic("Rel Ori Azimuth")
{ }

void RangeTool::RelOriAzimuthPieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  const simCore::Vec3& startOri = state.beginEntity_->ypr_;
  const osg::Vec3d& startVecENU = calcYprVector(startOri);
  RelOriAzimuthMeasurement m;
  const double relOriAzim = m.value(state);
  const simCore::Vec3& rotatedOri = simCore::rotateEulerAngle(startOri, simCore::Vec3(relOriAzim, 0., 0.));
  const osg::Vec3d& endVecENU = calcYprVector(rotatedOri);
  createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, endVecENU, relOriAzim, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::RelOriElevationPieSliceGraphic::RelOriElevationPieSliceGraphic()
  : PieSliceGraphic("Rel Ori Elevation")
{
  options_.pieColor_.set(.5, .5, 1, 1); // blue
}

void RangeTool::RelOriElevationPieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  osg::Vec3d startVecENU;
  // The RelOriAzimuthPieSliceGraphic endVec is used as the startVec for this graphic
  {
    RelOriAzimuthMeasurement m;
    const double relOriAzim = m.value(state);
    const simCore::Vec3& rotatedOri = simCore::rotateEulerAngle(state.beginEntity_->ypr_, simCore::Vec3(relOriAzim, 0., 0.));
    startVecENU = calcYprVector(rotatedOri);
  }

  const double relOriElev = measuredValue_;
  if (hasPosition_(state.beginEntity_->type_) && hasPosition_(state.endEntity_->type_))
  {
    createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, state.coord(RangeToolState::COORD_OBJ_1), relOriElev, geode, state);
  }
  else
  {
    // calc the endVec from the RAE endpoint's orientation
    const osg::Vec3d& endVecENU = calcYprVector(state.endEntity_->ypr_);
    createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, endVecENU, relOriElev, geode, state);
  }
}

//----------------------------------------------------------------------------

RangeTool::RelOriCompositeAnglePieSliceGraphic::RelOriCompositeAnglePieSliceGraphic()
  : PieSliceGraphic("Rel Ori Composite Angle")
{
  options_.pieColor_.set(.5, .5, .5, 1); // gray
}

void RangeTool::RelOriCompositeAnglePieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  const osg::Vec3d& startVecENU = calcYprVector(state.beginEntity_->ypr_);

  if (hasPosition_(state.beginEntity_->type_) && hasPosition_(state.endEntity_->type_))
  {
    createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, state.coord(RangeToolState::COORD_OBJ_1), measuredValue_, geode, state);
  }
  else
  {
    const osg::Vec3d& endVecENU = calcYprVector(state.endEntity_->ypr_);
    createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, endVecENU, measuredValue_, geode, state);
  }
}

//----------------------------------------------------------------------------

RangeTool::RelAspectAnglePieSliceGraphic::RelAspectAnglePieSliceGraphic()
  : PieSliceGraphic("Rel Aspect Angle")
{
  options_.pieColor_.set(.5, .5, .5, 1); // gray
}

void RangeTool::RelAspectAnglePieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  const double angle = measuredValue_;
  const osg::Vec3d& endVecENU = calcYprVector(state.endEntity_->ypr_);
  const osg::Vec3d startVec = state.coord(RangeToolState::COORD_OBJ_0) - state.coord(RangeToolState::COORD_OBJ_1);
  createGeometry(state.coord(RangeToolState::COORD_OBJ_1), startVec, endVecENU, angle, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::RelVelAzimuthPieSliceGraphic::RelVelAzimuthPieSliceGraphic()
  : PieSliceGraphic("Rel Vel Azimuth")
{ }

void RangeTool::RelVelAzimuthPieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  const simCore::Vec3& vel = state.beginEntity_->vel_;
  // relvel measurement is not meaningful when vel is zero
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
    return;

  const double relVelAzim = measuredValue_;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(vel, fpa);
  const simCore::Vec3& rotatedOri = simCore::rotateEulerAngle(fpa, simCore::Vec3(relVelAzim, 0., 0.));
  const osg::Vec3d& endVecENU = calcYprVector(rotatedOri);
  const osg::Vec3d startVecENU(vel.x(), vel.y(), vel.z());
  createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, endVecENU, relVelAzim, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::RelVelElevationPieSliceGraphic::RelVelElevationPieSliceGraphic()
  : PieSliceGraphic("Rel Vel Elevation")
{
  options_.pieColor_.set(.5, .5, 1, 1); // blue
}

void RangeTool::RelVelElevationPieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  // relvel measurement is not meaningful when vel is zero
  if (state.beginEntity_->vel_ == simCore::Vec3())
    return;

  // The RelVelAzimuthPieSliceGraphic endVec is used as the startVec for the graphic.
  osg::Vec3d startVecENU;
  {
    RelVelAzimuthMeasurement m;
    const double relVelAzim = m.value(state);
    simCore::Vec3 fpa;
    simCore::calculateFlightPathAngles(state.beginEntity_->vel_, fpa);
    const simCore::Vec3& rotatedOri = simCore::rotateEulerAngle(fpa, simCore::Vec3(relVelAzim, 0., 0.));
    startVecENU = calcYprVector(rotatedOri);
  }

  const double relVelElev = measuredValue_;
  if (hasPosition_(state.endEntity_->type_))
  {
    createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, state.coord(RangeToolState::COORD_OBJ_1), relVelElev, geode, state);
  }
  else
  {
    // calc the endVec from the RAE endpoint's orientation
    const osg::Vec3d& endVecENU = calcYprVector(state.endEntity_->ypr_);
    createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, endVecENU, relVelElev, geode, state);
  }
}

//----------------------------------------------------------------------------

RangeTool::RelVelCompositeAnglePieSliceGraphic::RelVelCompositeAnglePieSliceGraphic()
  : PieSliceGraphic("Rel Vel Composite Angle")
{
  options_.pieColor_.set(.5, .5, .5, 1); // gray
}

void RangeTool::RelVelCompositeAnglePieSliceGraphic::render(osg::Geode* geode, RangeToolState& state)
{
  // relvel measurement is not meaningful when vel is zero
  if (state.beginEntity_->vel_ == simCore::Vec3())
    return;

  const simCore::Vec3& vel = state.beginEntity_->vel_;
  const osg::Vec3d startVecENU(vel.x(), vel.y(), vel.z());
  const double relVelComposite = measuredValue_;
  if (hasPosition_(state.endEntity_->type_))
  {
    createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, state.coord(RangeToolState::COORD_OBJ_1), relVelComposite, geode, state);
  }
  else
  {
    // calc the endVec from the RAE endpoint's orientation
    const osg::Vec3d& endVecENU = calcYprVector(state.endEntity_->ypr_);
    createGeometry(state.coord(RangeToolState::COORD_OBJ_0), startVecENU, endVecENU, relVelComposite, geode, state);
  }
}

}
