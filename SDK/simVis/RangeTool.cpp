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
#include "osg/Depth"
#include "osg/Geode"
#include "osg/Geometry"
#include "osg/LineStipple"
#include "osg/LineWidth"
#include "osg/PolygonStipple"
#include "osgText/Text"
#include "osgEarth/DepthOffset"
#include "osgEarth/NodeUtils"
#include "osgEarth/Registry"
#include "osgEarth/ShaderGenerator"
#include "osgEarth/StateSetCache"
#include "osgEarthUtil/Controls"

#include "simCore/Calc/Angle.h"
#include "simCore/Calc/DatumConvert.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/EM/Decibel.h"
#include "simCore/Time/TimeClass.h"

#include "simVis/Constants.h"
#include "simVis/Utils.h"
#include "simVis/Registry.h"
#include "simVis/Platform.h"
#include "simVis/Beam.h"
#include "simVis/Antenna.h"
#include "simVis/LobGroup.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/RFProp/RFPropagationFacade.h"
#include "simVis/RFProp/RFPropagationManager.h"
#include "simVis/Text.h"

#include "simVis/RangeTool.h"

using namespace simVis;
using namespace osgEarth::Util::Controls;
using namespace osgEarth::Symbology;

/// Minimum depth bias for offsetting in meters
const int DEPTH_BUFFER_MIN_BIAS = 5000;

namespace
{
  // convenience func to return an osg ENU vector calculated from the input ypr/orientation
  osg::Vec3d calcYprVector(const simCore::Vec3& ypr)
  {
    simCore::Vec3 enuVector;
    simCore::calculateVelocity(1.0, ypr.yaw(), ypr.pitch(), enuVector);
    return osg::Vec3d(enuVector.x(), enuVector.y(), enuVector.z());
  }
}


//------------------------------------------------------------------------

void RangeTool::RefreshGroup::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == nv.UPDATE_VISITOR && tool_.valid())
  {
    // send a null scenario and invalid timestamp, these will be handled appropriately below
    tool_->update(NULL, simCore::INFINITE_TIME_STAMP);
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
  root_ = NULL;
  lastScenario_ = NULL;
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
    pieRadiusUnits_(osgEarth::Units::METERS),
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
    textLocation_(ALL)
{
  //nop
}

//------------------------------------------------------------------------

std::string RangeTool::ValueFormatter::stringValue(double value, Calculation* calc) const
{
  std::stringstream buf;
  buf << std::fixed << std::setprecision(static_cast<int>(calc->labelPrecision())) << value;
  return buf.str();
}

//------------------------------------------------------------------------

std::string RangeTool::HorizonFormatter::stringValue(double value, Calculation* calc) const
{
  if (value == 0.0)
    return "Below";

  return "Above";
}

//------------------------------------------------------------------------

RangeTool::Measurement::Measurement(const std::string& typeName, const std::string& typeAbbr, const osgEarth::Units& units)
  : formatter_(new RangeTool::ValueFormatter),
    typeName_(typeName),
    typeAbbr_(typeAbbr),
    units_(units)
{
  //nop
}

double RangeTool::Measurement::value(const osgEarth::Units& outputUnits, State& state) const
{
  return units_.convertTo(outputUnits, value(state));
}

bool RangeTool::Measurement::isEntityToEntity_(simData::ObjectType fromType, simData::ObjectType toType) const
{
  if ((fromType == simData::NONE) || (fromType == simData::PROJECTOR))
    return false;

  if ((toType == simData::NONE) || (toType == simData::PROJECTOR))
    return false;

  return true;
}

bool RangeTool::Measurement::isPlatformToPlatform_(simData::ObjectType fromType, simData::ObjectType toType) const
{
  if ((fromType != simData::PLATFORM) || (toType != simData::PLATFORM))
    return false;

  return true;
}

bool RangeTool::Measurement::isBeamToNonBeamAssociation_(simData::ObjectType fromType, simData::ObjectType toType) const
{
  if (((fromType == simData::PLATFORM) ||
       (fromType == simData::GATE) ||
       (fromType == simData::LOB_GROUP) ||
       (fromType == simData::LASER)) &&
       (toType == simData::BEAM))
     return true;

  return (((toType == simData::PLATFORM) ||
           (toType == simData::GATE) ||
           (toType == simData::LOB_GROUP) ||
           (toType == simData::LASER)) &&
           (fromType == simData::BEAM));
}

bool RangeTool::Measurement::isBeamToEntity_(simData::ObjectType fromType, simData::ObjectType toType) const
{
  if (fromType != simData::BEAM)
    return false;

  return ((toType == simData::PLATFORM) ||
    (toType == simData::GATE) ||
    (toType == simData::LOB_GROUP) ||
    (toType == simData::LASER) ||
    (fromType == simData::BEAM));
}

bool RangeTool::Measurement::isRaeObject_(simData::ObjectType type) const
{
  return ((type == simData::GATE) ||
          (type == simData::LOB_GROUP) ||
          (type == simData::LASER) ||
          (type == simData::BEAM));
}

bool RangeTool::Measurement::isAngle_(simData::ObjectType fromType, simData::ObjectId fromHostId,
                                      simData::ObjectType toType, simData::ObjectId toHostId) const
{
  if (isRaeObject_(toType) && isRaeObject_(fromType) && (fromHostId != toHostId))
  {
    // not valid when RAE based objects are not on the same host platform
    return false;
  }
  else if ((fromType == simData::PLATFORM) && isRaeObject_(toType) && (fromHostId != toHostId))
  {
    // not valid when RAE based end entity is compared to a platform other than its host
    return false;
  }

  return true;
}

bool RangeTool::Measurement::isVelocityAngle_(simData::ObjectType fromType, simData::ObjectId fromHostId,
                                              simData::ObjectType toType, simData::ObjectId toHostId) const
{
  if (fromType != simData::PLATFORM)
    return false;

  if (isRaeObject_(toType) && (fromHostId != toHostId))
    return false;

  return true;
}

double RangeTool::Measurement::getCompositeAngle_(double bgnAz, double bgnEl, double endAz, double endEl) const
{
  // assumes both bgn and end are wrt the same point/host platform
  simCore::Vec3 bgnVec, endVec;
  simCore::v3SphtoRec(simCore::Vec3(1, bgnAz, bgnEl), bgnVec);
  simCore::v3SphtoRec(simCore::Vec3(1, endAz, endEl), endVec);
  return simCore::v3Angle(bgnVec, endVec);
}

void RangeTool::Measurement::calculateTrueAngles_(const RangeTool::State& state, double* az, double* el, double* cmp) const
{
  bool raeBeginEntity = isRaeObject_(state.beginEntity_.node_->type());
  bool raeEndEntity = isRaeObject_(state.endEntity_.node_->type());

  if ((raeBeginEntity && raeEndEntity && (state.beginEntity_.platformHostId_ == state.endEntity_.platformHostId_)) ||
      (raeEndEntity && (state.beginEntity_.platformHostId_ == state.endEntity_.platformHostId_)))
  {
    // handle cases where calculations are between RAE based objects on the same host platform or
    // between a host platform (begin) and one of its own RAE based objects (end)
    if (az)
      *az = state.endEntity_.ypr_.yaw();
    if (el)
      *el = state.endEntity_.ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(0 ,0, state.endEntity_.ypr_.yaw(), state.endEntity_.ypr_.pitch());
  }
  else if (raeBeginEntity && (state.beginEntity_.platformHostId_ == state.endEntity_.platformHostId_))
  {
    // between a host platform (end) and one of its own RAE based objects (begin)
    if (az)
      *az = state.beginEntity_.ypr_.yaw();
    if (el)
      *el = state.beginEntity_.ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(0 ,0, state.beginEntity_.ypr_.yaw(), state.beginEntity_.ypr_.pitch());
  }
  else
  {
    simCore::calculateAbsAzEl(state.beginEntity_.lla_, state.endEntity_.lla_, az, el, cmp, state.earthModel_, &state.coordConv_);
  }
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

void RangeTool::Calculation::setLabelUnits(const osgEarth::Units& units)
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

double RangeTool::Calculation::lastValue(const osgEarth::Units& outputUnits) const
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
    xform_(NULL)
{
  geode_ = new osg::Geode();
  osg::StateSet* s = geode_->getOrCreateStateSet();
  simVis::setLighting(s, 0);
  s->setMode(GL_BLEND, 1);
  s->setMode(GL_CULL_FACE, 0);
  s->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, false));
  geode_->setName("Line");

  labels_ = new osg::Geode();
  s = labels_->getOrCreateStateSet();
  simVis::setLighting(s, 0);
  s->setMode(GL_BLEND, 1);
  s->setMode(GL_CULL_FACE, 0);
  s->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, false));
  labels_->setName("Graphics");

  // group exists solely to house the horizon culler, since cull callbacks do not
  // work on a Geode. -gw
  osg::Group* labelsContainer = new osg::Group();
  labelsContainer->addChild(labels_);
  osgEarth::HorizonCullCallback* horizonCull = new osgEarth::HorizonCullCallback();
  horizonCull->setCullByCenterPointOnly(true);
  labelsContainer->setCullCallback(horizonCull);

  xform_ = new osg::MatrixTransform();
  xform_->addChild(geode_);
  xform_->addChild(labelsContainer);
  xform_->setName("Range Tool Association");
  // enable flattening on the graphics, but not on the label node
  OverheadMode::enableGeometryFlattening(true, geode_);

  // create a state, and a magnetic datum convert for any measurements we might want to make
  state_.earthModel_ = simCore::WGS_84;

  labelPos_ = new SlantLineGraphic;
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
  this->labels_->removeDrawables(0, labels_->getNumDrawables()); // Clear existing labels to force a refresh to update colors if needed
  osgEarth::DirtyNotifier::setDirty();
}

namespace
{
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

void RangeTool::Association::refresh_(EntityNode* obj0, EntityNode* obj1, const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp)
{
  int rv = state_.populateEntityState(scenario, obj0, state_.beginEntity_);
  rv += state_.populateEntityState(scenario, obj1, state_.endEntity_);

  // clear out the geode
  geode_->removeDrawables(0, geode_->getNumDrawables());

  // If one of the entities is not valid at this time or the association is not visible; remove labels and return (graphics were removed above)
  if ((rv != 0) || !visible_)
  {
    labels_->removeDrawables(0, labels_->getNumDrawables());
    for (CalculationVector::iterator c = calculations_.begin(); c != calculations_.end(); ++c)
      (*c)->setValid(false);
    return;
  }

  // reset the coord_ cache
  state_.resetCoordCache();

  // ignore the invalid timestamp sent by RangeTool::RefreshGroup::traverse, reuse whatever timestamp was last used
  if (timeStamp != simCore::INFINITE_TIME_STAMP)
    state_.timeStamp_ = timeStamp;

  // initialize coordinate system and converter to optimize repeated conversions and support other values (flat projections)
  state_.coordConv_.setReferenceOrigin(state_.beginEntity_.lla_);

  const Locator* loc0 = obj0->getLocator();
  loc0->getLocalTangentPlaneToWorldMatrix(state_.local2world_);
  state_.world2local_.invert(state_.local2world_);

  // localizes all geometry to the reference point of obj0, preventing precision jitter
  xform_->setMatrix(state_.local2world_);

  typedef std::pair<CalculationVector, TextOptions> LabelSetup;
  typedef std::map<osg::Vec3, LabelSetup, CloseEnoughCompare> Labels;
  Labels labels;
  osg::Vec3 labelPos = labelPos_->labelPos(state_);

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
          psg->setMeasuredValue(calcMeasurement->value(state_));
        else
          assert(0);
      }

      graphic->render(geode_, state_);

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
          labelPos = posGraphic->labelPos(state_);
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
  unsigned int originalLabelCount = labels_->getNumDrawables();
  for (Labels::const_iterator i = labels.begin(); i != labels.end(); ++i)
  {
    osg::Vec3                pos         = i->first;
    const LabelSetup&        setup       = i->second;
    const CalculationVector& calcs       = setup.first;
    const TextOptions&       textOptions = setup.second;
    std::stringstream        buf;

    if (textOptions.displayAssociationName_)
    {
      const std::string& name0 = obj0->getEntityName(EntityNode::DISPLAY_NAME);
      const std::string& name1 = obj1->getEntityName(EntityNode::DISPLAY_NAME);
      if (!name0.empty() && !name1.empty())
      {
        buf << name0 << " to " << name1 << std::endl;
      }
    }

    buf << std::fixed;

    for (CalculationVector::const_iterator c = calcs.begin(); c != calcs.end(); ++c)
    {
      Calculation* calc = c->get();

      if (c != calcs.begin())
      {
        if (textOptions.textLocation_ == TextOptions::ALL)
          buf << ", ";
        else
          buf << "\n";
      }

      Measurement* m = calc->labelMeasurement();
      const osgEarth::Units& units =
        calc->labelUnits().isSet() ?
        *calc->labelUnits() :
        m->units();

      double value = m->value(state_);
      calc->setLastValue(value);
      value = m->units().convertTo(units, value);

      if (textOptions.showText_ == TextOptions::FULL)
        buf << m->typeAbbr() << ": ";
      buf << m->formatter()->stringValue(value, calc);
      if (units != osgEarth::Units::DEGREES)
        buf << " ";
      buf << units.getAbbr();
      if ((units == osgEarth::Units::DEGREES) && (textOptions.showText_ == TextOptions::VALUES_ONLY))
      {
        // If an angle was True of Magnetic add it to the back of the value if Values Only
        if (m->typeAbbr().find("(T)") != std::string::npos)
          buf << "T";
        else if (m->typeAbbr().find("(M)") != std::string::npos)
          buf << "M";
      }
    }

    if (textOptions.showText_ == TextOptions::NONE)
      continue;

    simVis::Text* text = NULL;
    if (labelCount >= labels_->getNumDrawables())
    {
      text = new simVis::Text();
      text->setAutoRotateToScreen(true);
      text->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
      text->setAlignment(osgText::Text::CENTER_CENTER);
      text->setFont(simVis::Registry::instance()->getOrCreateFont(textOptions.font_));
      text->setCharacterSize(textOptions.fontSize_);
      text->setColor(textOptions.color_);
      text->setBackdropType(text->OUTLINE);
      text->setBackdropColor(textOptions.outlineColor_);
      text->setScreenOffset(textOptions.xOffset_, textOptions.yOffset_);
      switch (textOptions.outlineType_)
      {
      case TextOptions::OUTLINE_NONE: text->setBackdropOffset(simVis::outlineThickness(simData::TO_NONE));
        break;
      case TextOptions::OUTLINE_THIN: text->setBackdropOffset(simVis::outlineThickness(simData::TO_THIN));
        break;
      case TextOptions::OUTLINE_THICK: text->setBackdropOffset(simVis::outlineThickness(simData::TO_THICK));
        break;
      }
      text->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS), 1);
      text->getOrCreateStateSet()->setRenderBinDetails(BIN_LABEL, BIN_GLOBAL_SIMSDK);
      labels_->addDrawable(text);
    }
    else
      text = static_cast<simVis::Text*>(labels_->getDrawable(labelCount));

    labelCount++;

    text->setPosition(pos);
    text->setText(buf.str());
  }

  // shader needed to draw text properly
  if (labelCount != originalLabelCount)
  {
    if (labelCount < originalLabelCount)
      labels_->removeDrawables(labelCount, originalLabelCount - labelCount);
    osgEarth::Registry::shaderGenerator().run(labels_, osgEarth::Registry::stateSetCache());
  }
}

//----------------------------------------------------------------------------

void RangeTool::LineGraphic::createGeometry(osg::Vec3Array* verts, osg::PrimitiveSet* primSet, osg::Geode* geode, State& state, bool subdivide)
{
  if (primSet && primSet->getNumIndices() > 0)
  {
    // To support the double-stippling pattern we have to make two geometries. If the first
    // stipple is 0xFFFF, just make one.
    for (unsigned int i = 0; i < 2; ++i)
    {
      osg::Geometry* geom = new osg::Geometry();
      geom->setUseVertexBufferObjects(true);

      geom->setVertexArray(verts);
      geom->addPrimitiveSet(primSet);

      osg::Vec4Array* colors = new osg::Vec4Array(1);
      (*colors)[0] = (i==0) ? options_.lineColor1_ : options_.lineColor2_;
      geom->setColorArray(colors);
      geom->setColorBinding(osg::Geometry::BIND_OVERALL);

      osg::StateSet* ss = geom->getOrCreateStateSet();
      ss->setAttributeAndModes(new osg::LineStipple(1, (i==0) ? options_.lineStipple1_ : options_.lineStipple2_), 1);
      if (options_.lineWidth_ != 1.0f)
        ss->setAttributeAndModes(new osg::LineWidth(options_.lineWidth_), 1);

      geode->addDrawable(geom);

      // don't bother drawing the second line if the first has a full stipple OR if the
      // second stipple is set to zero
      if (options_.lineStipple1_ == 0xFFFF || options_.lineStipple2_ == 0)
        break;
    }
  }
}

//----------------------------------------------------------------------------

void RangeTool::PieSliceGraphic::createGeometry(const osg::Vec3& originVec, osg::Vec3d startVec, osg::Vec3d endVec, double angle, osg::Geode* geode, RangeTool::State& state)
{
  osg::Geometry*  arcEndVecGeom = NULL;
  osg::Geometry*  startVecGeom  = NULL;
  osg::Vec3Array* verts         = NULL;

  if (geode)
  {
    arcEndVecGeom = new osg::Geometry();
    arcEndVecGeom->setUseVertexBufferObjects(true);

    verts = new osg::Vec3Array();
    arcEndVecGeom->setVertexArray(verts);

    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0] = options_.pieColor_;
    arcEndVecGeom->setColorArray(colors);
    arcEndVecGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::StateSet* ss = arcEndVecGeom->getOrCreateStateSet();
    ss->setAttributeAndModes(new osg::PolygonStipple(gPatternMask1), 1);
    ss->setAttributeAndModes(new osg::LineStipple(1, options_.lineStipple1_), 1);

    geode->addDrawable(arcEndVecGeom);

    // the geometry that holds the start vector; it shares the contents of the
    // first geometry, but applies a different state set.
    startVecGeom = new osg::Geometry();
    startVecGeom->setUseVertexBufferObjects(true);
    startVecGeom->setVertexArray(verts);
    startVecGeom->setColorArray(colors);
    startVecGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
    geode->addDrawable(startVecGeom);
  }

  osg::BoundingBox bbox;
  startVec.normalize();
  endVec.normalize();

  double pieRadius = options_.pieRadiusUnits_.convertTo(osgEarth::Units::METERS, options_.pieRadiusValue_);
  if (options_.usePercentOfSlantDistance_)
  {
    // using the RAE entity's range if both RAE entities share the same host
    if (state.beginEntity_.platformHostId_ == state.endEntity_.platformHostId_)
    {
      if (state.beginEntity_.node_->type() != simData::PLATFORM)
        pieRadius = state.beginEntity_.node_->range();
      else
        pieRadius = state.endEntity_.node_->range();
    }
    else
    {
      SlantDistanceMeasurement slant;
      pieRadius = slant.value(state);
    }

    // If radius is still zero use the default value otherwise scale radius by the percentage
    if (pieRadius <= 0.0)
      pieRadius = options_.pieRadiusUnits_.convertTo(osgEarth::Units::METERS, options_.pieRadiusValue_);
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
    osg::Quat& rot = slerp(static_cast<double>(seg) / options_.pieSegments_);
    osg::Vec3 vert = rot * startVec * pieRadius + originVec;
    bbox.expandBy(vert);
    if (geode)
      verts->push_back(vert);
  }

  if (geode)
  {
    arcEndVecGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, seg+1));

    verts->push_back(startVec * pieRadius * 1.5 + originVec);
    verts->push_back(endVec   * pieRadius * 1.5 + originVec);

    osg::DrawElementsUByte* startVecPrim = new osg::DrawElementsUByte(GL_LINES);
    startVecPrim->push_back(0);
    startVecPrim->push_back(verts->size()-2);
    startVecGeom->addPrimitiveSet(startVecPrim);

    osg::DrawElementsUByte* endVecPrim = new osg::DrawElementsUByte(GL_LINES);
    endVecPrim->push_back(0);
    endVecPrim->push_back(verts->size()-1);
    arcEndVecGeom->addPrimitiveSet(endVecPrim);
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

osg::Vec3 RangeTool::PieSliceGraphic::labelPos(State& state)
{
  if (!labelPos_.isSet())
    render(NULL, state);
  return *labelPos_;
}

//----------------------------------------------------------------------------

void RangeTool::State::line(const simCore::Vec3& lla0, const simCore::Vec3& lla1, double altOffset, osg::Vec3Array* verts)
{
  // Use Sodano method to calculate azimuth and distance
  double azimuth = 0.0;
  const double distance = simCore::sodanoInverse(lla0.lat(), lla0.lon(), lla0.alt(), lla1.lat(), lla1.lon(), &azimuth);

  // purely vertical line will be drawn as a single segment
  if (simCore::areEqual(distance, 0.0))
  {
    verts->push_back(lla2local(lla0.x(), lla0.y(), lla0.z() + altOffset));
    verts->push_back(lla2local(lla1.x(), lla1.y(), lla1.z() + altOffset));
    return;
  }

  // if total distance of the line is less than the max segment length, use that
  double segmentLength = simCore::sdkMin(distance, MAX_SEGMENT_LENGTH);
  // When lines are at/close to surface, we might need to tessellate more closely
  if (fabs(lla0.alt()) < SUBDIVIDE_BY_GROUND_THRESHOLD && fabs(lla1.alt()) < SUBDIVIDE_BY_GROUND_THRESHOLD)
  {
    // if the total distance of the line is less than the max segment length, use that
    segmentLength = simCore::sdkMin(distance, MAX_SEGMENT_LENGTH_GROUNDED);
  }

  // make sure there's enough room. Don't bother shrinking.
  const unsigned int numSegs = simCore::sdkMax(MIN_NUM_SEGMENTS, simCore::sdkMin(MAX_NUM_SEGMENTS, static_cast<unsigned int>(distance / segmentLength)));
  verts->reserve(numSegs + 1);
  verts->clear();

  // Add points to the vertex list, from back to front, for consistent stippling.  Order
  // matters because it affects the line direction during stippling.
  for (unsigned int k = 0; k <= numSegs; ++k)
  {
    const float percentOfFull = static_cast<float>(k) / static_cast<float>(numSegs); // From 0 to 1

    // Calculate the LLA value of the point, and replace the altitude
    double lat = 0.0;
    double lon = 0.0;
    simCore::sodanoDirect(lla0.lat(), lla0.lon(), lla0.alt(), distance * percentOfFull, azimuth, &lat, &lon);
    verts->push_back(lla2local(lat, lon, lla0.z() + altOffset));
  }
}

simCore::Vec3 RangeTool::State::midPoint(const simCore::Vec3& lla0, const simCore::Vec3& lla1, double altOffset)
{
  // Use Sodano method to calculate azimuth and distance
  double azimuth = 0.0;
  const double distance = simCore::sodanoInverse(lla0.lat(), lla0.lon(), lla0.alt(), lla1.lat(), lla1.lon(), &azimuth);

  // purely vertical line will be drawn as a single segment
  if (simCore::areEqual(distance, 0.0))
    return lla0;

  // Calculate the LLA value of the point, and replace the altitude
  double lat = 0.0;
  double lon = 0.0;
  simCore::sodanoDirect(lla0.lat(), lla0.lon(), lla0.alt(), distance * 0.5, azimuth, &lat, &lon);
  return simCore::Vec3(lat, lon, (lla0.alt() + lla1.alt()) / 2.0 + altOffset);
}

osg::Vec3d RangeTool::State::rotateEndVec(double az)
{
  // Use Sodano method to calculate azimuth and distance from beginEntity_ to endEntity_
  double azimuth = 0.0;
  const double distance = simCore::sodanoInverse(beginEntity_.lla_.lat(), beginEntity_.lla_.lon(), beginEntity_.lla_.alt(), endEntity_.lla_.lat(), endEntity_.lla_.lon(), &azimuth);

  // purely vertical line returns the original end entity pos, in local coords
  if (simCore::areEqual(distance, 0.0))
    return coord(COORD_OBJ_1);

  // Calculate the LLA value of the point, and replace the altitude
  double lat = 0.0;
  double lon = 0.0;
  simCore::sodanoDirect(beginEntity_.lla_.lat(), beginEntity_.lla_.lon(), beginEntity_.lla_.alt(), distance, (azimuth - az), &lat, &lon);
  return lla2local(lat, lon, endEntity_.lla_.alt());
}

osg::Vec3 RangeTool::State::lla2local(double lat, double lon, double alt)
{
  simCore::Vec3 ecefPos;
  simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(lat, lon, alt), ecefPos);
  return simCore2osg(ecefPos) * world2local_;
}

simCore::Vec3 RangeTool::State::local2lla(const osg::Vec3d& local)
{
  const osg::Vec3d world = local * local2world_;
  simCore::Vec3 llaPos;
  simCore::CoordinateConverter::convertEcefToGeodeticPos(osg2simCore(world), llaPos);
  return llaPos;
}

int RangeTool::State::populateEntityState(const ScenarioManager& scenario, const simVis::EntityNode* node, EntityState& state)
{
  if (node == NULL)
    return 1;

  state.node_ = node;
  state.platformHostNode_ = dynamic_cast<const simVis::PlatformNode*>(scenario.getHostPlatform(node));
  // if no platform host return with error
  if (state.platformHostNode_ == NULL)
    return 1;
  state.platformHostId_ = state.platformHostNode_->getId();

  // Kick out only after setting non-location information
  if (!node->isActive())
    return 1;

  if (0 != node->getPositionOrientation(&state.lla_, &state.ypr_, simCore::COORD_SYS_LLA))
    return 1;

  if (state.node_->type() == simData::PLATFORM)
  {
    // Platforms need velocity which is not available from getPositionOrientation, so add it in
    const simVis::PlatformNode* platform = dynamic_cast<const simVis::PlatformNode*>(node);
    if (platform == NULL)
      return 1;

    const simData::PlatformUpdate* update = platform->update();
    if (update == NULL)
      return 1;

    simCore::Coordinate ecef(simCore::COORD_SYS_ECEF,
                            simCore::Vec3(update->x(), update->y(), update->z()),
                            simCore::Vec3(update->psi(), update->theta(), update->phi()),
                            simCore::Vec3(update->vx(), update->vy(), update->vz()));
    simCore::Coordinate needVelocity;
    simCore::CoordinateConverter::convertEcefToGeodetic(ecef, needVelocity);
    // Take only the velocity since the other values have not gone been modified by any preferences
    state.vel_ = needVelocity.velocity();
  }

  if (state.node_->type() == simData::BEAM)
  {
    simRF::RFPropagationManagerPtr manager = scenario.rfPropagationManager();
    state.rfPropagation_ = manager->getRFPropagation(node->getId());
  }

  return 0;
}

osg::Vec3d RangeTool::State::coord(RangeTool::State::Coord which)
{
  if (coord_[which].isSet())
    return *coord_[which];

  switch (which)
  {
  case COORD_OBJ_0:
    {
      simCore::Vec3 ecefPos;
      simCore::CoordinateConverter::convertGeodeticPosToEcef(beginEntity_.lla_, ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_OBJ_1:
    {
      simCore::Vec3 ecefPos;
      simCore::CoordinateConverter::convertGeodeticPosToEcef(endEntity_.lla_, ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_OBJ_0_0HAE:
    {
      simCore::Vec3 ecefPos;
      simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(beginEntity_.lla_.x(), beginEntity_.lla_.y(), 0.0), ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_OBJ_1_0HAE:
    {
      simCore::Vec3 ecefPos;
      simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(endEntity_.lla_.x(), endEntity_.lla_.y(), 0.0), ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_OBJ_1_AT_OBJ_0_ALT:
    {
      simCore::Vec3 ecefPos;
      simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(endEntity_.lla_.x(), endEntity_.lla_.y(), beginEntity_.lla_.z()), ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_OBJ_0_AT_OBJ_1_ALT:
    {
      simCore::Vec3 ecefPos;
      simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(beginEntity_.lla_.x(), beginEntity_.lla_.y(), endEntity_.lla_.z()), ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_DR:
    {
      double dr, cr, dv;
      simCore::calculateDRCRDownValue(beginEntity_.lla_, beginEntity_.ypr_.x(), endEntity_.lla_, earthModel_, &coordConv_, &dr, &cr, &dv);

      // down/cross range point in TP coords:
      // TODO: not sure this is correct, should it be calculated in TP space?
      coord_[which] = osg::Vec3d(dr*sin(beginEntity_.ypr_.x()), dr*cos(beginEntity_.ypr_.x()), 0.0);
    }
    break;
  case COORD_VEL_AZIM_DR:
    {
      double downRng=0;
      simCore::Vec3 fpa;
      simCore::calculateFlightPathAngles(beginEntity_.vel_, fpa);
      simCore::calculateDRCRDownValue(beginEntity_.lla_, fpa[0],
			              endEntity_.lla_,
			              earthModel_,
			              &coordConv_,
			              &downRng,
			              NULL,
			              NULL);
      coord_[which] = osg::Vec3d(downRng*sin(fpa[0]), downRng*cos(fpa[0]), 0.0);
    }
    break;
  case COORD_BEAM_LLA_0:
  case COORD_BEAM_LLA_1:
    if (beginEntity_.node_->type() == simData::BEAM)
    {
      const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(beginEntity_.node_.get());
      // Node not defined correctly; type() and pointer should match)
      assert(beam != NULL);
      if (beam != NULL)
      {
        simCore::Vec3 from;
        beam->getClosestPoint(endEntity_.lla_, from);
        coord_[COORD_BEAM_LLA_0] = simCore2osg(from);
        coord_[COORD_BEAM_LLA_1] = simCore2osg(endEntity_.lla_);
      }
    }
    else
    {
      // at least one side must be a beam.  Check willAccept for errors
      assert(endEntity_.node_->type() == simData::BEAM);
      const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(endEntity_.node_.get());
      // Node not defined correctly; type() and pointer should match)
      assert(beam != NULL);
      if (beam != NULL)
      {
        simCore::Vec3 to;
        beam->getClosestPoint(beginEntity_.lla_, to);
        coord_[COORD_BEAM_LLA_0] = simCore2osg(beginEntity_.lla_);
        coord_[COORD_BEAM_LLA_1] = simCore2osg(to);
      }
    }
    break;

  case COORD_BEAM_0:
    {
      simCore::Vec3 ecefPos;
      const osg::Vec3d& point = coord(COORD_BEAM_LLA_0);
      simCore::CoordinateConverter::convertGeodeticPosToEcef(osg2simCore(point), ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_BEAM_1:
    {
      simCore::Vec3 ecefPos;
      const osg::Vec3d& point = coord(COORD_BEAM_LLA_1);
      simCore::CoordinateConverter::convertGeodeticPosToEcef(osg2simCore(point), ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_BEAM_0_0HAE:
    {
      simCore::Vec3 ecefPos;
      const osg::Vec3d& point = coord(COORD_BEAM_LLA_0);
      simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(point.x(), point.y(), 0.0), ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_BEAM_1_0HAE:
    {
      simCore::Vec3 ecefPos;
      const osg::Vec3d& point = coord(COORD_BEAM_LLA_1);
      simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(point.x(), point.y(), 0.0), ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_BEAM_1_AT_BEAM_0_ALT:
    {
      simCore::Vec3 ecefPos;
      const simCore::Vec3& from = osg2simCore(coord(State::COORD_BEAM_LLA_0));
      const simCore::Vec3& to = osg2simCore(coord(State::COORD_BEAM_LLA_1));
      simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(to.x(), to.y(), from.z()), ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;

  case COORD_BEAM_0_AT_BEAM_1_ALT:
    {
      simCore::Vec3 ecefPos;
      const simCore::Vec3& from = osg2simCore(coord(State::COORD_BEAM_LLA_0));
      const simCore::Vec3& to = osg2simCore(coord(State::COORD_BEAM_LLA_1));
      simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(from.x(), from.y(), to.z()), ecefPos);
      coord_[which] = simCore2osg(ecefPos) * world2local_;
    }
    break;
  }
  return *coord_[which];
}

void RangeTool::State::resetCoordCache()
{
  for (size_t i = 0; i < COORD_CACHE_SIZE; i++)
    coord_[i].clear();
}

simCore::Vec3 RangeTool::State::osg2simCore(const osg::Vec3d& point) const
{
  return simCore::Vec3(point.x(), point.y(), point.z());
}

osg::Vec3d RangeTool::State::simCore2osg(const simCore::Vec3& point) const
{
  return osg::Vec3d(point.x(), point.y(), point.z());
}

//----------------------------------------------------------------------------

RangeTool::GroundLineGraphic::GroundLineGraphic()
  : LineGraphic("GroundLine", LINE)
{ }

void RangeTool::GroundLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array();
  simCore::Vec3 lla0(state.beginEntity_.lla_.x(), state.beginEntity_.lla_.y(), 0.0);
  simCore::Vec3 lla1(state.endEntity_.lla_.x(), state.endEntity_.lla_.y(), 0.0);
  state.line(lla0, lla1, 1.0, verts);
  createGeometry(verts, new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()), geode, state);
}

osg::Vec3 RangeTool::GroundLineGraphic::labelPos(RangeTool::State& state)
{
  simCore::Vec3 lla0(state.beginEntity_.lla_.x(), state.beginEntity_.lla_.y(), 0.0);
  simCore::Vec3 lla1(state.endEntity_.lla_.x(), state.endEntity_.lla_.y(), 0.0);
  auto mid = state.midPoint(lla0, lla1, 0.0);
  return state.lla2local(mid.x(), mid.y(), 0.0);
}

//----------------------------------------------------------------------------

RangeTool::SlantLineGraphic::SlantLineGraphic() :
LineGraphic("SlantLine", LINE) { }

void RangeTool::SlantLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_OBJ_0);
  (*verts)[1] = state.coord(State::COORD_OBJ_1);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::SlantLineGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_OBJ_0) + state.coord(State::COORD_OBJ_1)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeginAltitudeLineGraphic::BeginAltitudeLineGraphic()
  : LineGraphic("BeginAltitudeLine", LINE)
{ }

void RangeTool::BeginAltitudeLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_OBJ_0);
  (*verts)[1] = state.coord(State::COORD_OBJ_0_0HAE);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::BeginAltitudeLineGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_OBJ_0) + state.coord(State::COORD_OBJ_0_0HAE)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::EndAltitudeLineGraphic::EndAltitudeLineGraphic()
  : LineGraphic("EndAltitudeLine", LINE)
{ }

void RangeTool::EndAltitudeLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_OBJ_1);
  (*verts)[1] = state.coord(State::COORD_OBJ_1_0HAE);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::EndAltitudeLineGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_OBJ_1) + state.coord(State::COORD_OBJ_1_0HAE)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeginAltitudeLineToEndAltitudeGraphic::BeginAltitudeLineToEndAltitudeGraphic()
  : LineGraphic("BeginAltitudeLineToEndAltitude", LINE)
{ }

void RangeTool::BeginAltitudeLineToEndAltitudeGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_OBJ_0);
  (*verts)[1] = state.coord(State::COORD_OBJ_0_AT_OBJ_1_ALT);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::BeginAltitudeLineToEndAltitudeGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_OBJ_0) + state.coord(State::COORD_OBJ_0_AT_OBJ_1_ALT)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::EndAltitudeLineToBeginAltitudeGraphic::EndAltitudeLineToBeginAltitudeGraphic() :
LineGraphic("EndAltitudeLineToBeginAltitude", LINE) { }

void RangeTool::EndAltitudeLineToBeginAltitudeGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_OBJ_1);
  (*verts)[1] = state.coord(State::COORD_OBJ_1_AT_OBJ_0_ALT);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::EndAltitudeLineToBeginAltitudeGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_OBJ_1) + state.coord(State::COORD_OBJ_1_AT_OBJ_0_ALT)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeginToEndLineAtBeginAltitudeGraphic::BeginToEndLineAtBeginAltitudeGraphic()
  : LineGraphic("BeginToEndLineAtBeginAltitude", LINE)
{ }

void RangeTool::BeginToEndLineAtBeginAltitudeGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array();
  simCore::Vec3 lla1(state.endEntity_.lla_.x(), state.endEntity_.lla_.y(), state.beginEntity_.lla_.z());
  state.line(state.beginEntity_.lla_, lla1, 0.0, verts);
  createGeometry(verts, new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()), geode, state);
}

osg::Vec3 RangeTool::BeginToEndLineAtBeginAltitudeGraphic::labelPos(RangeTool::State& state)
{
  return state.lla2local(state.endEntity_.lla_.x(), state.endEntity_.lla_.y(), state.beginEntity_.lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::BeginToEndLineAtEndAltitudeGraphic::BeginToEndLineAtEndAltitudeGraphic()
  : LineGraphic("BeginToEndLineAtEndAltitude", LINE)
{ }

void RangeTool::BeginToEndLineAtEndAltitudeGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array();
  simCore::Vec3 lla0(state.beginEntity_.lla_.x(), state.beginEntity_.lla_.y(), state.endEntity_.lla_.z());
  state.line(lla0, state.endEntity_.lla_, 0.0, verts);
  createGeometry(verts, new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()), geode, state);
}

osg::Vec3 RangeTool::BeginToEndLineAtEndAltitudeGraphic::labelPos(RangeTool::State& state)
{
  return state.lla2local(state.beginEntity_.lla_.x(), state.beginEntity_.lla_.y(), state.endEntity_.lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::BeamGroundLineGraphic::BeamGroundLineGraphic()
  : LineGraphic("BeamGroundLine", LINE)
{ }

void RangeTool::BeamGroundLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array();

  simCore::Vec3 from = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_1));
  simCore::Vec3 lla0(from.x(), from.y(), 0.0);
  simCore::Vec3 lla1(to.x(), to.y(), 0.0);
  state.line(lla0, lla1, 1.0, verts);
  createGeometry(verts, new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()), geode, state);
}

osg::Vec3 RangeTool::BeamGroundLineGraphic::labelPos(RangeTool::State& state)
{
  simCore::Vec3 from = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_1));
  auto mid = state.midPoint(from, to, 0.0);
  return state.lla2local(mid.x(), mid.y(), 0.0);

}

//----------------------------------------------------------------------------

RangeTool::BeamSlantLineGraphic::BeamSlantLineGraphic()
  : LineGraphic("BeamSlantLine", LINE)
{ }

void RangeTool::BeamSlantLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_BEAM_0);
  (*verts)[1] = state.coord(State::COORD_BEAM_1);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::BeamSlantLineGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_BEAM_0) + state.coord(State::COORD_BEAM_1)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeamBeginAltitudeLineGraphic::BeamBeginAltitudeLineGraphic()
  : LineGraphic("BeamBeginAltitudeLine", LINE)
{ }

void RangeTool::BeamBeginAltitudeLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_BEAM_0);
  (*verts)[1] = state.coord(State::COORD_BEAM_0_0HAE);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::BeamBeginAltitudeLineGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_BEAM_0) + state.coord(State::COORD_BEAM_0_0HAE)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeamEndAltitudeLineGraphic::BeamEndAltitudeLineGraphic()
  : LineGraphic("BeamEndAltitudeLine", LINE)
{ }

void RangeTool::BeamEndAltitudeLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_BEAM_1);
  (*verts)[1] = state.coord(State::COORD_BEAM_1_0HAE);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::BeamEndAltitudeLineGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_BEAM_1) + state.coord(State::COORD_BEAM_1_0HAE)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeamBeginAltitudeLineToEndAltitudeGraphic::BeamBeginAltitudeLineToEndAltitudeGraphic()
  : LineGraphic("BeamBeginAltitudeLineToEndAltitude", LINE)
{ }

void RangeTool::BeamBeginAltitudeLineToEndAltitudeGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_BEAM_0);
  (*verts)[1] = state.coord(State::COORD_BEAM_0_AT_BEAM_1_ALT);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::BeamBeginAltitudeLineToEndAltitudeGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_BEAM_0) + state.coord(State::COORD_BEAM_0_AT_BEAM_1_ALT)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeamEndAltitudeLineToBeginAltitudeGraphic::BeamEndAltitudeLineToBeginAltitudeGraphic()
  : LineGraphic("BeamEndAltitudeLineToBeginAltitude", LINE)
{ }

void RangeTool::BeamEndAltitudeLineToBeginAltitudeGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_BEAM_1);
  (*verts)[1] = state.coord(State::COORD_BEAM_1_AT_BEAM_0_ALT);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::BeamEndAltitudeLineToBeginAltitudeGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_BEAM_1) + state.coord(State::COORD_BEAM_1_AT_BEAM_0_ALT)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::BeamBeginToEndLineAtBeginAltitudeGraphic::BeamBeginToEndLineAtBeginAltitudeGraphic()
  : LineGraphic("BeamBeginToEndLineAtBeginAltitude", LINE)
{ }

void RangeTool::BeamBeginToEndLineAtBeginAltitudeGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array();
  simCore::Vec3 from = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_1));
  simCore::Vec3 lla1(to.x(), to.y(), from.z());
  state.line(from, lla1, 0.0, verts);
  createGeometry(verts, new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()), geode, state);
}

osg::Vec3 RangeTool::BeamBeginToEndLineAtBeginAltitudeGraphic::labelPos(RangeTool::State& state)
{
  simCore::Vec3 from = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_1));
  return state.lla2local(to.x(), to.y(), from.z());
}

//----------------------------------------------------------------------------

RangeTool::BeamBeginToEndLineAtEndAltitudeGraphic::BeamBeginToEndLineAtEndAltitudeGraphic()
  : LineGraphic("BeamBeginToEndLineAtEndAltitude", LINE)
{ }

void RangeTool::BeamBeginToEndLineAtEndAltitudeGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array();
  simCore::Vec3 from = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_1));
  simCore::Vec3 lla0(from.x(), from.y(), to.z());
  state.line(lla0, to, 0.0, verts);
  createGeometry(verts, new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()), geode, state);
}

osg::Vec3 RangeTool::BeamBeginToEndLineAtEndAltitudeGraphic::labelPos(RangeTool::State& state)
{
  simCore::Vec3 from = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_1));
  return state.lla2local(from.x(), from.y(), to.z());
}

//----------------------------------------------------------------------------

RangeTool::DownRangeLineGraphic::DownRangeLineGraphic()
  : LineGraphic("DownRangeLine", LINE)
{ }

void RangeTool::DownRangeLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array();
  simCore::Vec3 crdr = state.local2lla(state.coord(State::COORD_DR));
  state.line(state.beginEntity_.lla_, crdr, 0.0, verts);
  createGeometry(verts, new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()), geode, state);
}

osg::Vec3 RangeTool::DownRangeLineGraphic::labelPos(RangeTool::State& state)
{
  simCore::Vec3 crdr = state.local2lla(state.coord(State::COORD_DR));
  auto mid = state.midPoint(state.beginEntity_.lla_, crdr, 0.0);
  return state.lla2local(mid.x(), mid.y(), state.beginEntity_.lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::VelAzimDownRangeLineGraphic::VelAzimDownRangeLineGraphic()
  : LineGraphic("VelAzimDownRangeLine", LINE)
{ }

void RangeTool::VelAzimDownRangeLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array();
  simCore::Vec3 end = state.local2lla(state.coord(State::COORD_VEL_AZIM_DR));
  state.line(state.beginEntity_.lla_, end, 0.0, verts);
  createGeometry(verts, new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()), geode, state);
}

osg::Vec3 RangeTool::VelAzimDownRangeLineGraphic::labelPos(RangeTool::State& state)
{
  simCore::Vec3 end = state.local2lla(state.coord(State::COORD_VEL_AZIM_DR));
  auto mid = state.midPoint(state.beginEntity_.lla_, end, 0.0);
  return state.lla2local(mid.x(), mid.y(), state.beginEntity_.lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::VelAzimCrossRangeLineGraphic::VelAzimCrossRangeLineGraphic()
  : LineGraphic("VelAzimCrossRangeLine", LINE)
{ }

void RangeTool::VelAzimCrossRangeLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array();
  simCore::Vec3 start = state.local2lla(state.coord(State::COORD_VEL_AZIM_DR));
  simCore::Vec3 end = state.local2lla(state.coord(State::COORD_OBJ_1_AT_OBJ_0_ALT));
  state.line(start, end, 0.0, verts);
  createGeometry(verts, new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()), geode, state);
}

osg::Vec3 RangeTool::VelAzimCrossRangeLineGraphic::labelPos(RangeTool::State& state)
{
  simCore::Vec3 start = state.local2lla(state.coord(State::COORD_VEL_AZIM_DR));
  auto mid = state.midPoint(state.endEntity_.lla_, start, 0.0);
  return state.lla2local(mid.x(), mid.y(), state.endEntity_.lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::CrossRangeLineGraphic::CrossRangeLineGraphic()
  : LineGraphic("CrossRangeLine", LINE)
{ }

void RangeTool::CrossRangeLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array();
  simCore::Vec3 crdr = state.local2lla(state.coord(State::COORD_DR));
  simCore::Vec3 lla1(state.endEntity_.lla_.x(), state.endEntity_.lla_.y(), state.beginEntity_.lla_.z());
  state.line(crdr, lla1, 0.0, verts);
  createGeometry(verts, new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()), geode, state);
}

osg::Vec3 RangeTool::CrossRangeLineGraphic::labelPos(RangeTool::State& state)
{
  simCore::Vec3 crdr = state.local2lla(state.coord(State::COORD_DR));
  auto mid = state.midPoint(state.endEntity_.lla_, crdr, 0.0);
  return state.lla2local(mid.x(), mid.y(), state.beginEntity_.lla_.z());
}

//----------------------------------------------------------------------------

RangeTool::DownRangeCrossRangeDownLineGraphic::DownRangeCrossRangeDownLineGraphic()
  : LineGraphic("CrossRangeLine", LINE)
{ }

void RangeTool::DownRangeCrossRangeDownLineGraphic::render(osg::Geode* geode, RangeTool::State& state)
{
  osg::Vec3Array* verts = new osg::Vec3Array(2);
  (*verts)[0] = state.coord(State::COORD_OBJ_1_AT_OBJ_0_ALT);
  (*verts)[1] = state.coord(State::COORD_OBJ_1);
  createGeometry(verts, new osg::DrawArrays(GL_LINES, 0, 2), geode, state);
}

osg::Vec3 RangeTool::DownRangeCrossRangeDownLineGraphic::labelPos(RangeTool::State& state)
{
  return (state.coord(State::COORD_OBJ_1_AT_OBJ_0_ALT) + state.coord(State::COORD_OBJ_1)) * 0.5;
}

//----------------------------------------------------------------------------

RangeTool::TrueAzimuthPieSliceGraphic::TrueAzimuthPieSliceGraphic()
  : PieSliceGraphic("True Azimuth")
{ }

void RangeTool::TrueAzimuthPieSliceGraphic::render(osg::Geode* geode, State& state)
{
  osg::Vec3d endVec;

  if (state.beginEntity_.platformHostId_ != state.endEntity_.platformHostId_)
  {
    endVec = state.coord(State::COORD_OBJ_1_AT_OBJ_0_ALT);
    endVec[2] = 0.0;  // COORD_OBJ_1_AT_OBJ_0_ALT accounts for the earth's curvature which we don't want, so jam into local plane
  }
  else
  {
    // Get the RAE object to get its angles
    simCore::Vec3& ori = state.beginEntity_.ypr_;
    if (state.endEntity_.node_->type() != simData::PLATFORM)
      ori = state.endEntity_.ypr_;

    endVec = osg::Vec3d(sin(ori.x())*cos(ori.y()), cos(ori.x())*cos(ori.y()), 0.0);
  }
  createGeometry(state.coord(State::COORD_OBJ_0), osg::Y_AXIS, endVec, measuredValue_, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::TrueElevationPieSliceGraphic::TrueElevationPieSliceGraphic()
  : PieSliceGraphic("True Elevation")
{
  options_.pieColor_.set(.5, .5, 1, 1); // blue
}

void RangeTool::TrueElevationPieSliceGraphic::render(osg::Geode* geode, State& state)
{
  osg::Vec3d startVec;
  osg::Vec3d endVec;

  if (state.beginEntity_.platformHostId_ != state.endEntity_.platformHostId_)
  {
    startVec = state.coord(State::COORD_OBJ_1_AT_OBJ_0_ALT);
    startVec[2] = 0.0;  // COORD_OBJ_1_AT_OBJ_0_ALT accounts for the earth's curvature which we don't want, so jam into local plane
    endVec = state.coord(State::COORD_OBJ_1);
  }
  else
  {
    // Get the RAE object to get its angles
    simCore::Vec3& ori = state.beginEntity_.ypr_;
    if (state.endEntity_.node_->type() != simData::PLATFORM)
      ori = state.endEntity_.ypr_;

    startVec = calcYprVector(ori);
    endVec = osg::Vec3d(startVec.x(), startVec.y(), 0.0);
  }

  createGeometry(state.coord(State::COORD_OBJ_0), startVec, endVec, measuredValue_, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::TrueCompositeAnglePieSliceGraphic::TrueCompositeAnglePieSliceGraphic()
  : PieSliceGraphic("True Composite Angle")
{
  options_.pieColor_.set(.5, .5, .5, 1); // gray
}

void RangeTool::TrueCompositeAnglePieSliceGraphic::render(osg::Geode* geode, State& state)
{
  osg::Vec3d endVec;

  if (state.beginEntity_.platformHostId_ != state.endEntity_.platformHostId_)
  {
    endVec = state.coord(State::COORD_OBJ_1);
  }
  else
  {
    // Get the RAE object to get its angles
    simCore::Vec3& ori = state.beginEntity_.ypr_;
    if (state.endEntity_.node_->type() != simData::PLATFORM)
      ori = state.endEntity_.ypr_;

    endVec = calcYprVector(ori);
  }

  createGeometry(state.coord(State::COORD_OBJ_0), osg::Y_AXIS, endVec, measuredValue_, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::MagneticAzimuthPieSliceGraphic::MagneticAzimuthPieSliceGraphic()
  : PieSliceGraphic("Magnetic Azimuth")
{ }

void RangeTool::MagneticAzimuthPieSliceGraphic::render(osg::Geode* geode, State& state)
{
  osg::Vec3d startVecENU;
  osg::Vec3d endVecENU;
  const double magAz = measuredValue_;

  if (state.beginEntity_.platformHostId_ != state.endEntity_.platformHostId_)
  {
    endVecENU = state.coord(State::COORD_OBJ_1_AT_OBJ_0_ALT);
    endVecENU[2] = 0.0;  // COORD_OBJ_1_AT_OBJ_0_ALT accounts for the earth's curvature which we don't want, so jam into local plane
    // start vec is end vec (true azim to object 1) rotated by magAz
    startVecENU = state.rotateEndVec(magAz);
    startVecENU[2] = 0.0; // flatten this (also) onto the local tangent place
  }
  else
  {
    // Determine which is the RAE object, and get its angles
    simCore::Vec3 ori = (state.endEntity_.node_->type() != simData::PLATFORM) ? state.endEntity_.ypr_ : state.beginEntity_.ypr_;

    endVecENU = osg::Vec3d(sin(ori.x())*cos(ori.y()), cos(ori.x())*cos(ori.y()), 0.0);
    // start vec is end vec (true azim to rae object) rotated by magAz
    ori.setYaw(ori.yaw() - magAz);
    startVecENU = osg::Vec3d(sin(ori.x())*cos(ori.y()), cos(ori.x())*cos(ori.y()), 0.0);
  }

  createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, endVecENU, magAz, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::RelOriAzimuthPieSliceGraphic::RelOriAzimuthPieSliceGraphic()
  : PieSliceGraphic("Rel Ori Azimuth")
{ }

void RangeTool::RelOriAzimuthPieSliceGraphic::render(osg::Geode* geode, State& state)
{
  const simCore::Vec3& startOri = state.beginEntity_.ypr_;
  const osg::Vec3d& startVecENU = calcYprVector(startOri);
  RelOriAzimuthMeasurement m;
  const double relOriAzim = m.value(state);
  const simCore::Vec3& rotatedOri = simCore::rotateEulerAngle(startOri, simCore::Vec3(relOriAzim, 0., 0.));
  const osg::Vec3d& endVecENU = calcYprVector(rotatedOri);
  createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, endVecENU, relOriAzim, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::RelOriElevationPieSliceGraphic::RelOriElevationPieSliceGraphic()
  : PieSliceGraphic("Rel Ori Elevation")
{
  options_.pieColor_.set(.5, .5, 1, 1); // blue
}

void RangeTool::RelOriElevationPieSliceGraphic::render(osg::Geode* geode, State& state)
{
  osg::Vec3d startVecENU;
  // The RelOriAzimuthPieSliceGraphic endVec is used as the startVec for this graphic
  {
    RelOriAzimuthMeasurement m;
    const double relOriAzim = m.value(state);
    const simCore::Vec3& rotatedOri = simCore::rotateEulerAngle(state.beginEntity_.ypr_, simCore::Vec3(relOriAzim, 0., 0.));
    startVecENU = calcYprVector(rotatedOri);
  }


  const double relOriElev = measuredValue_;
  if ((state.beginEntity_.node_->type() == simData::PLATFORM) &&
      (state.endEntity_.node_->type() == simData::PLATFORM))
  {
    createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, state.coord(State::COORD_OBJ_1), relOriElev, geode, state);
  }
  else
  {
    // calc the endVec from the RAE endpoint's orientation
    const osg::Vec3d& endVecENU = calcYprVector(state.endEntity_.ypr_);
    createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, endVecENU, relOriElev, geode, state);
  }
}

//----------------------------------------------------------------------------

RangeTool::RelOriCompositeAnglePieSliceGraphic::RelOriCompositeAnglePieSliceGraphic()
  : PieSliceGraphic("Rel Ori Composite Angle")
{
  options_.pieColor_.set(.5, .5, .5, 1); // gray
}

void RangeTool::RelOriCompositeAnglePieSliceGraphic::render(osg::Geode* geode, State& state)
{
  const osg::Vec3d& startVecENU = calcYprVector(state.beginEntity_.ypr_);

  if ((state.beginEntity_.node_->type() == simData::PLATFORM) &&
    (state.endEntity_.node_->type() == simData::PLATFORM))
  {
    createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, state.coord(State::COORD_OBJ_1), measuredValue_, geode, state);
  }
  else
  {
    const osg::Vec3d& endVecENU = calcYprVector(state.endEntity_.ypr_);
    createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, endVecENU, measuredValue_, geode, state);
  }
}

//----------------------------------------------------------------------------

RangeTool::RelAspectAnglePieSliceGraphic::RelAspectAnglePieSliceGraphic()
  : PieSliceGraphic("Rel Aspect Angle")
{
  options_.pieColor_.set(.5, .5, .5, 1); // gray
}

void RangeTool::RelAspectAnglePieSliceGraphic::render(osg::Geode* geode, State& state)
{
  const double angle = measuredValue_;
  const osg::Vec3d& endVecENU = calcYprVector(state.endEntity_.ypr_);
  const osg::Vec3d startVec = state.coord(State::COORD_OBJ_0) - state.coord(State::COORD_OBJ_1);
  createGeometry(state.coord(State::COORD_OBJ_1), startVec, endVecENU, angle, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::RelVelAzimuthPieSliceGraphic::RelVelAzimuthPieSliceGraphic()
  : PieSliceGraphic("Rel Vel Azimuth")
{ }

void RangeTool::RelVelAzimuthPieSliceGraphic::render(osg::Geode* geode, State& state)
{
  // relvel measurement is not meaningful when vel is zero
  if (state.beginEntity_.vel_ == simCore::Vec3())
    return;

  const double relVelAzim = measuredValue_;
  simCore::Vec3 fpa;
  const simCore::Vec3& vel = state.beginEntity_.vel_;
  simCore::calculateFlightPathAngles(vel, fpa);
  const simCore::Vec3& rotatedOri = simCore::rotateEulerAngle(fpa, simCore::Vec3(relVelAzim, 0., 0.));
  const osg::Vec3d& endVecENU = calcYprVector(rotatedOri);
  const osg::Vec3d startVecENU(vel.x(), vel.y(), vel.z());
  createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, endVecENU, relVelAzim, geode, state);
}

//----------------------------------------------------------------------------

RangeTool::RelVelElevationPieSliceGraphic::RelVelElevationPieSliceGraphic()
  : PieSliceGraphic("Rel Vel Elevation")
{
  options_.pieColor_.set(.5, .5, 1, 1); // blue
}

void RangeTool::RelVelElevationPieSliceGraphic::render(osg::Geode* geode, State& state)
{
  // relvel measurement is not meaningful when vel is zero
  if (state.beginEntity_.vel_ == simCore::Vec3())
    return;

  // The RelVelAzimuthPieSliceGraphic endVec is used as the startVec for the graphic.
  osg::Vec3d startVecENU;
  {
    RelVelAzimuthMeasurement m;
    const double relVelAzim = m.value(state);
    simCore::Vec3 fpa;
    simCore::calculateFlightPathAngles(state.beginEntity_.vel_, fpa);
    const simCore::Vec3& rotatedOri = simCore::rotateEulerAngle(fpa, simCore::Vec3(relVelAzim, 0., 0.));
    startVecENU = calcYprVector(rotatedOri);
  }

  const double relVelElev = measuredValue_;
  if (state.endEntity_.node_->type() == simData::PLATFORM)
  {
    createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, state.coord(State::COORD_OBJ_1), relVelElev, geode, state);
  }
  else
  {
    // calc the endVec from the RAE endpoint's orientation
    const osg::Vec3d& endVecENU = calcYprVector(state.endEntity_.ypr_);
    createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, endVecENU, relVelElev, geode, state);
  }
}

//----------------------------------------------------------------------------

RangeTool::RelVelCompositeAnglePieSliceGraphic::RelVelCompositeAnglePieSliceGraphic()
  : PieSliceGraphic("Rel Vel Composite Angle")
{
  options_.pieColor_.set(.5, .5, .5, 1); // gray
}

void RangeTool::RelVelCompositeAnglePieSliceGraphic::render(osg::Geode* geode, State& state)
{
  // relvel measurement is not meaningful when vel is zero
  if (state.beginEntity_.vel_ == simCore::Vec3())
    return;

  const simCore::Vec3& vel = state.beginEntity_.vel_;
  const osg::Vec3d startVecENU(vel.x(), vel.y(), vel.z());
  const double relVelComposite = measuredValue_;
  if (state.endEntity_.node_->type() == simData::PLATFORM)
  {
    createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, state.coord(State::COORD_OBJ_1), relVelComposite, geode, state);
  }
  else
  {
    // calc the endVec from the RAE endpoint's orientation
    const osg::Vec3d& endVecENU = calcYprVector(state.endEntity_.ypr_);
    createGeometry(state.coord(State::COORD_OBJ_0), startVecENU, endVecENU, relVelComposite, geode, state);
  }
}

//----------------------------------------------------------------------------

RangeTool::GroundDistanceMeasurement::GroundDistanceMeasurement()
  : Measurement("Ground Rng", "Dist", osgEarth::Units::METERS)
{ }

double RangeTool::GroundDistanceMeasurement::value(State& state) const
{
  return simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
}

bool RangeTool::GroundDistanceMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::SlantDistanceMeasurement::SlantDistanceMeasurement()
  : Measurement("Slant Rng", "Rng", osgEarth::Units::METERS)
{ }

double RangeTool::SlantDistanceMeasurement::value(State& state) const
{
  return simCore::calculateSlant(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
}

bool RangeTool::SlantDistanceMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::AltitudeDeltaMeasurement::AltitudeDeltaMeasurement()
  : Measurement("Altitude", "Alt", osgEarth::Units::METERS)
{ }

double RangeTool::AltitudeDeltaMeasurement::value(State& state) const
{
  return simCore::calculateAltitude(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
}

bool RangeTool::AltitudeDeltaMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::BeamGroundDistanceMeasurement::BeamGroundDistanceMeasurement()
  : Measurement("Beam Ground Rng", "Dist(B)", osgEarth::Units::METERS)
{ }

double RangeTool::BeamGroundDistanceMeasurement::value(State& state) const
{
  simCore::Vec3 from = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_0));
  simCore::Vec3 to  = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_1));
  return simCore::calculateGroundDist(from, to, state.earthModel_, &state.coordConv_);
}

bool RangeTool::BeamGroundDistanceMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToNonBeamAssociation_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::BeamSlantDistanceMeasurement::BeamSlantDistanceMeasurement()
  : Measurement("Beam Slant Rng", "Rng(B)", osgEarth::Units::METERS)
{ }

double RangeTool::BeamSlantDistanceMeasurement::value(State& state) const
{
  simCore::Vec3 from = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_0));
  simCore::Vec3 to  = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_1));
  return simCore::calculateSlant(from, to, state.earthModel_, &state.coordConv_);
}

bool RangeTool::BeamSlantDistanceMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToNonBeamAssociation_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::BeamAltitudeDeltaMeasurement::BeamAltitudeDeltaMeasurement()
  : Measurement("Beam Altitude", "Alt(B)", osgEarth::Units::METERS)
{ }

double RangeTool::BeamAltitudeDeltaMeasurement::value(State& state) const
{
  simCore::Vec3 from = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_0));
  simCore::Vec3 to  = state.osg2simCore(state.coord(State::COORD_BEAM_LLA_1));
  return simCore::calculateAltitude(from, to, state.earthModel_, &state.coordConv_);
}

bool RangeTool::BeamAltitudeDeltaMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToNonBeamAssociation_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::DownRangeMeasurement::DownRangeMeasurement()
  : Measurement("Downrange", "DR", osgEarth::Units::METERS)
{ }

double RangeTool::DownRangeMeasurement::value(State& state) const
{
  double dr;
  simCore::calculateDRCRDownValue(state.beginEntity_.lla_, state.beginEntity_.ypr_.x(), state.endEntity_.lla_, state.earthModel_, &state.coordConv_, &dr, NULL, NULL);
  return dr;
}

bool RangeTool::DownRangeMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::CrossRangeMeasurement::CrossRangeMeasurement()
  : Measurement("Crossrange", "CR", osgEarth::Units::METERS)
{ }

double RangeTool::CrossRangeMeasurement::value(State& state) const
{
  double cr;
  simCore::calculateDRCRDownValue(state.beginEntity_.lla_, state.beginEntity_.ypr_.x(), state.endEntity_.lla_, state.earthModel_, &state.coordConv_, NULL, &cr, NULL);
  return cr;
}

bool RangeTool::CrossRangeMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::DownRangeCrossRangeDownValueMeasurement::DownRangeCrossRangeDownValueMeasurement()
  : Measurement("Down Value", "DV", osgEarth::Units::METERS)
{ }

double RangeTool::DownRangeCrossRangeDownValueMeasurement::value(State& state) const
{
  double dv;
  simCore::calculateDRCRDownValue(state.beginEntity_.lla_, state.beginEntity_.ypr_.x(), state.endEntity_.lla_, state.earthModel_, &state.coordConv_, NULL, NULL, &dv);
  return dv;
}

bool RangeTool::DownRangeCrossRangeDownValueMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::GeoDownRangeMeasurement::GeoDownRangeMeasurement()
  : Measurement("Geo Downrange", "DR(g)", osgEarth::Units::METERS)
{ }

double RangeTool::GeoDownRangeMeasurement::value(State& state) const
{
  double dr;
  simCore::calculateGeodesicDRCR(state.beginEntity_.lla_, state.beginEntity_.ypr_.x(), state.endEntity_.lla_, &dr, NULL);
  return dr;
}

bool RangeTool::GeoDownRangeMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::GeoCrossRangeMeasurement::GeoCrossRangeMeasurement()
  : Measurement("Geo Crossrange", "CR(g)", osgEarth::Units::METERS)
{ }

double RangeTool::GeoCrossRangeMeasurement::value(State& state) const
{
  double cr;
  simCore::calculateGeodesicDRCR(state.beginEntity_.lla_, state.beginEntity_.ypr_.x(), state.endEntity_.lla_, NULL, &cr);
  return cr;
}

bool RangeTool::GeoCrossRangeMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::TrueAzimuthMeasurement::TrueAzimuthMeasurement()
  : Measurement("True Azim", "Az(T)", osgEarth::Units::RADIANS)
{ }

double RangeTool::TrueAzimuthMeasurement::value(State& state) const
{
  double az;
  calculateTrueAngles_(state, &az, NULL, NULL);
  return az;
}

bool RangeTool::TrueAzimuthMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.platformHostId_, state.endEntity_.node_->type(), state.endEntity_.platformHostId_);
}

//----------------------------------------------------------------------------

RangeTool::TrueElevationMeasurement::TrueElevationMeasurement()
  : Measurement("True Elev", "El", osgEarth::Units::RADIANS)
{ }

double RangeTool::TrueElevationMeasurement::value(State& state) const
{
  double el;
  calculateTrueAngles_(state, NULL, &el, NULL);
  return el;
}

bool RangeTool::TrueElevationMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.platformHostId_, state.endEntity_.node_->type(), state.endEntity_.platformHostId_);
}

//----------------------------------------------------------------------------

RangeTool::TrueCompositeAngleMeasurement::TrueCompositeAngleMeasurement()
  : Measurement("True Composite", "Cmp(T)", osgEarth::Units::RADIANS)
{ }

double RangeTool::TrueCompositeAngleMeasurement::value(State& state) const
{
  double cmp;
  calculateTrueAngles_(state, NULL, NULL, &cmp);
  return cmp;
}

bool RangeTool::TrueCompositeAngleMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.platformHostId_, state.endEntity_.node_->type(), state.endEntity_.platformHostId_);
}

//----------------------------------------------------------------------------

RangeTool::MagneticAzimuthMeasurement::MagneticAzimuthMeasurement(std::shared_ptr<simCore::DatumConvert> datumConvert)
  : Measurement("Mag Azim", "Az(M)", osgEarth::Units::RADIANS),
  datumConvert_(datumConvert)
{
}

double RangeTool::MagneticAzimuthMeasurement::value(State& state) const
{
  double az;
  calculateTrueAngles_(state, &az, NULL, NULL);
  az = datumConvert_->convertMagneticDatum(state.beginEntity_.lla_, state.timeStamp_, az, simCore::COORD_SYS_LLA, simCore::MAGVAR_TRUE, simCore::MAGVAR_WMM, 0.0);
  return az;
}

bool RangeTool::MagneticAzimuthMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.platformHostId_, state.endEntity_.node_->type(), state.endEntity_.platformHostId_);
}

//----------------------------------------------------------------------------

void RangeTool::RelOriMeasurement::getAngles(double* az, double* el, double* cmp, State& state) const
{
  bool raeBgnEntity = isRaeObject_(state.beginEntity_.node_->type());
  bool raeEndEntity = isRaeObject_(state.endEntity_.node_->type());
  if (raeBgnEntity && raeEndEntity && (state.beginEntity_.platformHostId_ == state.endEntity_.platformHostId_))
  {
    // handle cases where calculations are between RAE based objects with the same host platform
    if (az)
      *az = state.endEntity_.ypr_.yaw() - state.beginEntity_.ypr_.yaw();
    if (el)
      *el = state.endEntity_.ypr_.pitch() - state.beginEntity_.ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(state.beginEntity_.ypr_.yaw(), state.beginEntity_.ypr_.pitch(), state.endEntity_.ypr_.yaw(), state.endEntity_.ypr_.pitch());
  }
  else if ((raeBgnEntity && (state.endEntity_.node_->type() == simData::PLATFORM) && (state.beginEntity_.platformHostId_ == state.endEntity_.platformHostId_)) ||
           (raeEndEntity && (state.beginEntity_.node_->type() == simData::PLATFORM) && (state.beginEntity_.platformHostId_ == state.endEntity_.platformHostId_)))
  {
    // handle cases where calculations are between RAE based objects their own host platform
    if (az)
      *az = state.endEntity_.ypr_.yaw() - state.beginEntity_.ypr_.yaw();
    if (el)
      *el = state.endEntity_.ypr_.pitch() - state.beginEntity_.ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(state.beginEntity_.ypr_.yaw(), state.beginEntity_.ypr_.pitch(), state.endEntity_.ypr_.yaw(), state.endEntity_.ypr_.pitch());
  }
  else
  {
    simCore::calculateRelAzEl(state.beginEntity_.lla_, state.beginEntity_.ypr_, state.endEntity_.lla_, az, el, cmp, state.earthModel_, &state.coordConv_);
  }
}

//----------------------------------------------------------------------------

RangeTool::RelOriAzimuthMeasurement::RelOriAzimuthMeasurement()
  : RelOriMeasurement("Rel Azim", "Az(r)", osgEarth::Units::RADIANS)
{ }

double RangeTool::RelOriAzimuthMeasurement::value(State& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  az = simCore::angFixPI(az);
  return az;
}

bool RangeTool::RelOriAzimuthMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.platformHostId_, state.endEntity_.node_->type(), state.endEntity_.platformHostId_);
}

//----------------------------------------------------------------------------

RangeTool::RelOriElevationMeasurement::RelOriElevationMeasurement()
  : RelOriMeasurement("Rel Elev", "El(r)", osgEarth::Units::RADIANS)
{ }

double RangeTool::RelOriElevationMeasurement::value(State& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return el;
}

bool RangeTool::RelOriElevationMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.platformHostId_, state.endEntity_.node_->type(), state.endEntity_.platformHostId_);
}

//----------------------------------------------------------------------------

RangeTool::RelOriCompositeAngleMeasurement::RelOriCompositeAngleMeasurement()
  : RelOriMeasurement("Rel Composite", "Cmp(r)", osgEarth::Units::RADIANS)
{ }

double RangeTool::RelOriCompositeAngleMeasurement::value(State& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return cmp;
}

bool RangeTool::RelOriCompositeAngleMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.platformHostId_, state.endEntity_.node_->type(), state.endEntity_.platformHostId_);
}

//----------------------------------------------------------------------------

void RangeTool::RelVelMeasurement::getAngles(double* az, double* el, double* cmp, State& state) const
{
  simCore::Vec3 fpaVec;
  simCore::calculateFlightPathAngles(state.beginEntity_.vel_, fpaVec);

  bool raeEndEntity = isRaeObject_(state.endEntity_.node_->type());
  if (raeEndEntity && (state.beginEntity_.node_->type() == simData::PLATFORM) && (state.beginEntity_.platformHostId_ == state.endEntity_.platformHostId_))
  {
    // handle case where calculation is between host platform and its RAE based objects
    if (az)
      *az = state.endEntity_.ypr_.yaw() - fpaVec.yaw();
    if (el)
      *el = state.endEntity_.ypr_.pitch() - fpaVec.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(fpaVec.yaw(), fpaVec.pitch(), state.endEntity_.ypr_.yaw(), state.endEntity_.ypr_.pitch());
  }
  else
  {
    simCore::calculateRelAzEl(state.beginEntity_.lla_, fpaVec, state.endEntity_.lla_, az, el, cmp, state.earthModel_, &state.coordConv_);
  }
}

//----------------------------------------------------------------------------

RangeTool::RelVelAzimuthMeasurement::RelVelAzimuthMeasurement()
  : RelVelMeasurement("Rel Vel Azim", "Az(v)", osgEarth::Units::RADIANS)
{ }

double RangeTool::RelVelAzimuthMeasurement::value(State& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return az;
}

bool RangeTool::RelVelAzimuthMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isVelocityAngle_(state.beginEntity_.node_->type(), state.beginEntity_.platformHostId_, state.endEntity_.node_->type(), state.endEntity_.platformHostId_);
}

//----------------------------------------------------------------------------

RangeTool::RelVelElevationMeasurement::RelVelElevationMeasurement()
  : RelVelMeasurement("Rel Vel Elev", "El(v)", osgEarth::Units::RADIANS)
{ }

double RangeTool::RelVelElevationMeasurement::value(State& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return el;
}

bool RangeTool::RelVelElevationMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isVelocityAngle_(state.beginEntity_.node_->type(), state.beginEntity_.platformHostId_, state.endEntity_.node_->type(), state.endEntity_.platformHostId_);
}

//----------------------------------------------------------------------------

RangeTool::RelVelCompositeAngleMeasurement::RelVelCompositeAngleMeasurement()
  : RelVelMeasurement("Rel Vel Composite", "Cmp(v)", osgEarth::Units::RADIANS)
{ }

double RangeTool::RelVelCompositeAngleMeasurement::value(State& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return cmp;
}

bool RangeTool::RelVelCompositeAngleMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isVelocityAngle_(state.beginEntity_.node_->type(), state.beginEntity_.platformHostId_, state.endEntity_.node_->type(), state.endEntity_.platformHostId_);
}
//----------------------------------------------------------------------------

RangeTool::ClosingVelocityMeasurement::ClosingVelocityMeasurement()
  : Measurement("Closing Vel", "V(c)", osgEarth::Units::METERS_PER_SECOND)
{ }

double RangeTool::ClosingVelocityMeasurement::value(State& state) const
{
  return simCore::calculateClosingVelocity(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_, state.beginEntity_.vel_, state.endEntity_.vel_);
}

bool RangeTool::ClosingVelocityMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isPlatformToPlatform_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::SeparationVelocityMeasurement::SeparationVelocityMeasurement()
  : Measurement("Separation Vel", "V(s)", osgEarth::Units::METERS_PER_SECOND)
{ }

double RangeTool::SeparationVelocityMeasurement::value(State& state) const
{
  return -simCore::calculateClosingVelocity(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_, state.beginEntity_.vel_, state.endEntity_.vel_);
}

bool RangeTool::SeparationVelocityMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isPlatformToPlatform_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::VelocityDeltaMeasurement::VelocityDeltaMeasurement()
  : Measurement("Vel Delta", "V(d)", osgEarth::Units::METERS_PER_SECOND)
{ }

double RangeTool::VelocityDeltaMeasurement::value(State& state) const
{
  return simCore::calculateVelocityDelta(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_, state.beginEntity_.vel_, state.endEntity_.vel_);
}

bool RangeTool::VelocityDeltaMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isPlatformToPlatform_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::VelAzimDownRangeMeasurement::VelAzimDownRangeMeasurement()
  : Measurement("Vel Azim Down Range", "DR(v)", osgEarth::Units::METERS)
{ }

double RangeTool::VelAzimDownRangeMeasurement::value(State& state) const
{
  double downRng=0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(state.beginEntity_.vel_, fpa);
  simCore::calculateDRCRDownValue(state.beginEntity_.lla_, fpa[0], state.endEntity_.lla_, state.earthModel_, &state.coordConv_, &downRng, NULL, NULL);
  return downRng;
}

bool RangeTool::VelAzimDownRangeMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return (state.beginEntity_.node_->type() == simData::PLATFORM);
}

//----------------------------------------------------------------------------

RangeTool::VelAzimCrossRangeMeasurement::VelAzimCrossRangeMeasurement()
  : Measurement("Vel Azim Cross Range", "CR(v)", osgEarth::Units::METERS)
{ }

double RangeTool::VelAzimCrossRangeMeasurement::value(State& state) const
{
  double crossRng=0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(state.beginEntity_.vel_, fpa);
  simCore::calculateDRCRDownValue(state.beginEntity_.lla_, fpa[0], state.endEntity_.lla_, state.earthModel_, &state.coordConv_, NULL, &crossRng, NULL);
  return crossRng;
}

bool RangeTool::VelAzimCrossRangeMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return (state.beginEntity_.node_->type() == simData::PLATFORM);
}

//----------------------------------------------------------------------------

RangeTool::VelAzimGeoDownRangeMeasurement::VelAzimGeoDownRangeMeasurement()
  : Measurement("Vel Azim Geo Down Range", "DR(gv)", osgEarth::Units::METERS)
{ }

double RangeTool::VelAzimGeoDownRangeMeasurement::value(State& state) const
{
  double downRng=0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(state.beginEntity_.vel_, fpa);
  simCore::calculateGeodesicDRCR(state.beginEntity_.lla_, fpa[0], state.endEntity_.lla_, &downRng, NULL);
  return downRng;
}

bool RangeTool::VelAzimGeoDownRangeMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return (state.beginEntity_.node_->type() == simData::PLATFORM);
}

//----------------------------------------------------------------------------

RangeTool::VelAzimGeoCrossRangeMeasurement::VelAzimGeoCrossRangeMeasurement()
  : Measurement("Vel Azim Geo Cross Range", "CR(gv)", osgEarth::Units::METERS)
{ }

double RangeTool::VelAzimGeoCrossRangeMeasurement::value(State& state) const
{
  double crossRng=0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(state.beginEntity_.vel_, fpa);
  simCore::calculateGeodesicDRCR(state.beginEntity_.lla_, fpa[0], state.endEntity_.lla_, NULL, &crossRng);
  return crossRng;
}

bool RangeTool::VelAzimGeoCrossRangeMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return (state.beginEntity_.node_->type() == simData::PLATFORM);
}

//----------------------------------------------------------------------------

RangeTool::AspectAngleMeasurement::AspectAngleMeasurement()
  : Measurement("Aspect Angle", "Asp(r)", osgEarth::Units::RADIANS)
{ }

double RangeTool::AspectAngleMeasurement::value(State& state) const
{
  return simCore::calculateAspectAngle(state.beginEntity_.lla_, state.endEntity_.lla_, state.endEntity_.ypr_);
}

bool RangeTool::AspectAngleMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isPlatformToPlatform_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RangeTool::RfMeasurement::RfMeasurement(const std::string& name, const std::string& abbr, const osgEarth::Units& units)
  : RelOriMeasurement(name, abbr, units)
{

}

void RangeTool::RfMeasurement::getRfParameters_(State& state, double *azAbs, double *elAbs, double *hgtMeters, double* xmtGaindB, double* rcvGaindB, double* rcs, bool useDb) const
{
  if (azAbs != NULL || elAbs != NULL)
  {
    double azAbsLocal;
    double elAbsLocal;
    calculateTrueAngles_(state, &azAbsLocal, &elAbsLocal, NULL);
    if (azAbs != NULL)
      *azAbs = azAbsLocal;

    if (elAbs != NULL)
      *elAbs = elAbsLocal;
  }

  if (hgtMeters != NULL)
  {
    const simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;
    if (rf != NULL)
      *hgtMeters = rf->antennaHeight();
    else
      *hgtMeters = 0.0;
  }

  // Do NOT set RF parameter values from RFPropagationFacade, in order to match the behavior of SIMDIS 9

  if (xmtGaindB != NULL || rcvGaindB != NULL)
  {
    double xmtGaindBLocal = simCore::DEFAULT_ANTENNA_GAIN;
    double rcvGaindBLocal = simCore::DEFAULT_ANTENNA_GAIN;
    const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(state.beginEntity_.node_.get());
    if (beam)
    {
      double azRelLocal;
      double elRelLocal;
      RelOriMeasurement::getAngles(&azRelLocal, &elRelLocal, NULL, state);
      xmtGaindBLocal = beam->gain(azRelLocal, elRelLocal);
      rcvGaindBLocal = xmtGaindBLocal;
    }
    if (xmtGaindB != NULL)
      *xmtGaindB = xmtGaindBLocal;

    if (rcvGaindB != NULL)
      *rcvGaindB = rcvGaindBLocal;
  }

  if (rcs != NULL)
  {
    double rcsLocal = useDb ? simCore::SMALL_DB_VAL : simCore::SMALL_RCS_SM;
    // To match SIMDIS 9, the end entity must be a platform.
    if (state.endEntity_.node_->type() == simData::PLATFORM)
    {
      simCore::RadarCrossSectionPtr rcsPtr = state.endEntity_.platformHostNode_->getRcs();
      if (rcsPtr != NULL)
      {
        // need the angles from the target to the beam source to get the correct rcs values
        double azTarget;
        double elTarget;
        const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(state.beginEntity_.node_.get());
        simCore::PolarityType type = (beam != NULL) ? beam->polarity() : simCore::POLARITY_UNKNOWN;
        const double frequency = simCore::DEFAULT_FREQUENCY;
        simCore::calculateRelAzEl(state.endEntity_.lla_, state.endEntity_.ypr_, state.beginEntity_.lla_, &azTarget, &elTarget, NULL, state.earthModel_, &state.coordConv_);
        if (useDb)
          rcsLocal = rcsPtr->RCSdB(frequency, azTarget, elTarget, type);
        else
          rcsLocal = rcsPtr->RCSsm(frequency, azTarget, elTarget, type);
      }
    }
    *rcs = rcsLocal;
  }
}

//----------------------------------------------------------------------------

RangeTool::RFGainMeasurement::RFGainMeasurement()
  : RfMeasurement("Gain", "Gain", LOG10)
{ }

double RangeTool::RFGainMeasurement::value(State& state) const
{
  const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(state.beginEntity_.node_.get());
  if (beam)
  {
    double azRelLocal;
    double elRelLocal;
    getAngles(&azRelLocal, &elRelLocal, NULL, state);
    return beam->gain(azRelLocal, elRelLocal);
  }
  return 0.0;
}

bool RangeTool::RFGainMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}


//----------------------------------------------------------------------------

RangeTool::RFPowerMeasurement::RFPowerMeasurement()
  : RfMeasurement("Power", "Pwr", RF_POWER)
{ }

double RangeTool::RFPowerMeasurement::value(State& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;
  if (rf == NULL)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  double hgtMeters;
  double xmtGaindB;
  double rcvGaindB;
  double rcsSqm;

  getRfParameters_(state, &az, NULL, &hgtMeters, &xmtGaindB, &rcvGaindB, &rcsSqm, false);
  double slantRngMeters = simCore::calculateSlant(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  if (rcsSqm == simCore::SMALL_RCS_SM)
  {
    // no valid rcs data found; use default 1.0 sqm as documented in SIMDIS User Manual
    rcsSqm = 1.0;
  }
  return rf->getReceivedPower(az, slantRngMeters, hgtMeters, xmtGaindB, rcvGaindB, rcsSqm, gndRngMeters);
}

bool RangeTool::RFPowerMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
         (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

RangeTool::RFOneWayPowerMeasurement::RFOneWayPowerMeasurement()
  : RfMeasurement("One Way Power", "Pwr(1)", RF_POWER)
{ }

double RangeTool::RFOneWayPowerMeasurement::value(State& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;
  if (rf == NULL)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  double hgtMeters;
  double xmtGaindB;
  double rcvGaindB;

  getRfParameters_(state, &az, NULL, &hgtMeters, &xmtGaindB, &rcvGaindB, NULL, false);
  double slantRngMeters = simCore::calculateSlant(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);

  return rf->getOneWayPower(az, slantRngMeters, hgtMeters, xmtGaindB, gndRngMeters, rcvGaindB);
}

bool RangeTool::RFOneWayPowerMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
         (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

RangeTool::HorizonMeasurement::HorizonMeasurement(const std::string &typeName, const std::string &typeAbbr, const osgEarth::Units &units)
  : Measurement(typeName, typeAbbr, units),
  opticalEffectiveRadius_(DEFAULT_OPTICAL_RADIUS),
  rfEffectiveRadius_(DEFAULT_RF_RADIUS)
{
  // Override the default formatter
  formatter_ = new RangeTool::HorizonFormatter;
}

bool RangeTool::HorizonMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

void RangeTool::HorizonMeasurement::setEffectiveRadius(double opticalRadius, double rfRadius)
{
  opticalEffectiveRadius_ = opticalRadius;
  rfEffectiveRadius_ = rfRadius;
}

double RangeTool::HorizonMeasurement::calcAboveHorizon_(State& state, simCore::HorizonCalculations horizon) const
{
  // TODO: SDK-52 Add terrain blocking

  double maxRng = simCore::calculateSlant(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  double losRng = simCore::calculateHorizonDist(state.beginEntity_.lla_, horizon, opticalEffectiveRadius_, rfEffectiveRadius_) +
    simCore::calculateHorizonDist(state.endEntity_.lla_, horizon, opticalEffectiveRadius_, rfEffectiveRadius_);
  return (maxRng <= losRng) ? 1 : 0;
}

//----------------------------------------------------------------------------

RangeTool::RadioHorizonMeasurement::RadioHorizonMeasurement()
  : HorizonMeasurement("Radio Horizon", "Hor(r)", UNITLESS)
{ }

double RangeTool::RadioHorizonMeasurement::value(State& state) const
{
  return calcAboveHorizon_(state, simCore::RADAR_HORIZON);
}

//----------------------------------------------------------------------------

RangeTool::OpticalHorizonMeasurement::OpticalHorizonMeasurement()
  : HorizonMeasurement("Optical Horizon", "Hor(o)", UNITLESS)
{ }

double RangeTool::OpticalHorizonMeasurement::value(State& state) const
{
  return calcAboveHorizon_(state, simCore::OPTICAL_HORIZON);
}

//----------------------------------------------------------------------------

RangeTool::PodMeasurement::PodMeasurement()
  : RfMeasurement("POD", "POD", PERCENTAGE)
{ }

double RangeTool::PodMeasurement::value(State& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;

  if (rf == NULL)
    return 0.0;

  double az;
  getRfParameters_(state, &az, NULL, NULL, NULL, NULL, NULL, false);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);

  return rf->getPOD(az, gndRngMeters, state.endEntity_.lla_.alt());
}

bool RangeTool::PodMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
         (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

RangeTool::LossMeasurement::LossMeasurement()
  : RfMeasurement("Loss", "Loss", LOG10)
{ }

double RangeTool::LossMeasurement::value(State& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;

  if (rf == NULL)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  getRfParameters_(state, &az, NULL, NULL, NULL, NULL, NULL, false);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);

  return rf->getLoss(az, gndRngMeters, state.endEntity_.lla_.alt());
}

bool RangeTool::LossMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
         (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

RangeTool::PpfMeasurement::PpfMeasurement()
  : RfMeasurement("PPF", "PPF", LOG10)
{ }

double RangeTool::PpfMeasurement::value(State& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;

  if (rf == NULL)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  getRfParameters_(state, &az, NULL, NULL, NULL, NULL, NULL, false);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);

  return rf->getPPF(az, gndRngMeters, state.endEntity_.lla_.alt());
}

bool RangeTool::PpfMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
        (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

RangeTool::SnrMeasurement::SnrMeasurement()
  : RfMeasurement("SNR", "SNR", LOG10)
{ }

double RangeTool::SnrMeasurement::value(State& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;

  if (rf == NULL)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  double xmtGaindB;
  double rcvGaindB;
  double rcsSqm;

  getRfParameters_(state, &az, NULL, NULL, &xmtGaindB, &rcvGaindB, &rcsSqm, false);
  double slantRngMeters = simCore::calculateSlant(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  double altitude = state.endEntity_.lla_.alt();
  if (rcsSqm == simCore::SMALL_RCS_SM)
  {
    // no valid rcs data found; use default 1.0 sqm as documented in SIMDIS User Manual
    rcsSqm = 1.0;
  }
  return rf->getSNR(az, slantRngMeters, altitude, xmtGaindB, rcvGaindB, rcsSqm, gndRngMeters);
}

bool RangeTool::SnrMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
         (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

RangeTool::CnrMeasurement::CnrMeasurement()
  : RfMeasurement("CNR", "CNR", LOG10)
{ }

double RangeTool::CnrMeasurement::value(State& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;

  if (rf == NULL)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  getRfParameters_(state, &az, NULL, NULL, NULL, NULL, NULL, false);
  //unlike other RF - related calculations, CNR doesn't have a height component
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);

  return rf->getCNR(az, gndRngMeters);
}

bool RangeTool::CnrMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
         (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

RangeTool::RcsMeasurement::RcsMeasurement()
  : RfMeasurement("RCS", "RCS", RF_POWER_SM)
{ }

double RangeTool::RcsMeasurement::value(State& state) const
{
  //RCS is a measure of the electrical or reflective area of a target, it is usually expressed in square meters or dBsm.
  double rcsDb;
  getRfParameters_(state, NULL, NULL, NULL, NULL, NULL, &rcsDb, true);

  return rcsDb;
}

bool RangeTool::RcsMeasurement::willAccept(const simVis::RangeTool::State& state) const
{
  return (state.endEntity_.node_->type() == simData::PLATFORM) &&
    (state.endEntity_.node_->getId() == state.endEntity_.platformHostNode_->getId()) &&
    (state.endEntity_.platformHostNode_->getRcs() != NULL);
}
