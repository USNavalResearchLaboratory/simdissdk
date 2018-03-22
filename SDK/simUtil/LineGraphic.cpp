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
#include "osgEarth/GeoMath"
#include "osgEarthSymbology/Color"
#include "osgEarthAnnotation/LabelNode"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simVis/Constants.h"
#include "simVis/Utils.h"
#include "simVis/AnimatedLine.h"
#include "simVis/Registry.h"
#include "LineGraphic.h"

namespace simUtil
{

static const int GRAPHIC_MASK_RULERLINE = simVis::DISPLAY_MASK_GOG;
static const float DEFAULT_LINEWIDTH = 2.0f;
static const unsigned short DEFAULT_STIPPLE = 0xf00f;
static const simVis::Color DEFAULT_LINECOLOR = osgEarth::Symbology::Color::Yellow;
static const simVis::Color DEFAULT_TEXTCOLOR = osgEarth::Symbology::Color::White;
static const std::string DEFAULT_FONT = "arialbd.ttf";
static const float DEFAULT_FONTSIZE = 14;

LineGraphic::LineGraphic(osg::Group* scene, osgEarth::MapNode* mapNode)
  : scene_(scene),
    wgs84Srs_(osgEarth::SpatialReference::create("wgs84")),
    animatedLine_(new simVis::AnimatedLineNode(DEFAULT_LINEWIDTH, false)),
    displayMask_(GRAPHIC_MASK_RULERLINE)
{
  // Configure default settings of the animated line
  animatedLine_->setStipple1(DEFAULT_STIPPLE);
  animatedLine_->setStipple2(0);
  animatedLine_->setColor1(DEFAULT_LINECOLOR);
  animatedLine_->setShiftsPerSecond(0.0);
  // Apply the clip plane on the line
  animatedLine_->getOrCreateStateSet()->setMode(simVis::CLIPPLANE_VISIBLE_HORIZON_GL_MODE, 1);
  animatedLine_->getOrCreateStateSet()->setRenderBinDetails(simVis::BIN_ANIMATEDLINE, simVis::BIN_GLOBAL_SIMSDK);

  // Set up the label node default style
  osg::ref_ptr<osgEarth::Symbology::TextSymbol> text = labelStyle_.getOrCreate<osgEarth::Symbology::TextSymbol>();
  text->fill()->color() = DEFAULT_TEXTCOLOR;
  text->halo()->color() = osgEarth::Symbology::Color::Black;
  text->alignment() = osgEarth::Symbology::TextSymbol::ALIGN_CENTER_CENTER;
  text->haloOffset() = simVis::outlineThickness(simData::TO_THICK);
  osg::ref_ptr<osgEarth::Symbology::RenderSymbol> render = labelStyle_.getOrCreate<osgEarth::Symbology::RenderSymbol>();
  render->lighting() = false;
  text->size() = simVis::osgFontSize(DEFAULT_FONTSIZE);
  text->font() = simVis::Registry::instance()->findFontFile(DEFAULT_FONT);
  // Don't declutter -- always show this label
  text->declutter() = false;

  // Create the label node itself
  label_ = new osgEarth::Annotation::LabelNode(mapNode, labelStyle_);
  label_->setDynamic(true);

  // Hide the line and label until we need them
  animatedLine_->setNodeMask(0);
  label_->setNodeMask(0);

  scene_->addChild(animatedLine_);
  scene_->addChild(label_);
}

LineGraphic::~LineGraphic()
{
  scene_->removeChild(label_);
  scene_->removeChild(animatedLine_);
}

void LineGraphic::set(const simUtil::Position* origin, const simUtil::Position* destination, const std::string& labelString)
{
  // Update the positions
  if (origin != NULL && destination != NULL && origin->isValid() && destination->isValid())
  {
    set(origin->lla(), destination->lla(), labelString);
  }
  else
  {
    // Turn off the node
    animatedLine_->setNodeMask(simVis::DISPLAY_MASK_NONE);
    label_->setNodeMask(simVis::DISPLAY_MASK_NONE);
  }
}

void LineGraphic::set(const simCore::Vec3& originLLA, const simCore::Vec3& destinationLLA, const std::string& labelString)
{
  if (originLLA != destinationLLA)
  {
    simCore::Coordinate coord1(simCore::COORD_SYS_LLA, originLLA);
    simCore::Coordinate coord2(simCore::COORD_SYS_LLA, destinationLLA);
    animatedLine_->setEndPoints(coord1, coord2);
    // Turn on the node
    animatedLine_->setNodeMask(displayMask_);

    if (labelString.empty())
    {
      // Turn off label if no text provided
      label_->setNodeMask(simVis::DISPLAY_MASK_NONE);
    }
    else
    {
      // Figure out the label position
      double labelLat = 0;
      double labelLon = 0;
      osgEarth::GeoMath::midpoint(originLLA.lat(), originLLA.lon(),
        destinationLLA.lat(), destinationLLA.lon(), labelLat, labelLon);
      label_->setPosition(osgEarth::GeoPoint(wgs84Srs_.get(), labelLon * simCore::RAD2DEG, labelLat * simCore::RAD2DEG));
      label_->setText(labelString);
      label_->setNodeMask(displayMask_);
    }
  }
  else
  {
    // Turn off the node
    animatedLine_->setNodeMask(simVis::DISPLAY_MASK_NONE);
    label_->setNodeMask(simVis::DISPLAY_MASK_NONE);
  }
}

void LineGraphic::setLineWidth(float lineWidth)
{
  animatedLine_->setLineWidth(lineWidth);
}

bool LineGraphic::isDrawn() const
{
  // Currently label and line cannot have different NodeMasks. This method will need updating if that changes
  return animatedLine_->getNodeMask() != 0;
}

void LineGraphic::setDraw(bool draw)
{
  animatedLine_->setNodeMask(draw ? displayMask_ : simVis::DISPLAY_MASK_NONE);
  label_->setNodeMask(draw ? displayMask_ : simVis::DISPLAY_MASK_NONE);
}

void LineGraphic::setDisplayMask(int displayMask)
{
  if (displayMask == displayMask_)
    return;

  displayMask_ = displayMask;
  // Use setDraw to appropriately reset the nodeMasks
  setDraw(isDrawn());
}

void LineGraphic::setStipplePattern(unsigned short stipple)
{
  animatedLine_->setStipple1(stipple);
}

void LineGraphic::setLineColor(const simVis::Color& color)
{
  animatedLine_->setColor1(color);
}

void LineGraphic::setTextColor(const simVis::Color& color)
{
  labelStyle_.getOrCreate<osgEarth::Symbology::TextSymbol>()->fill()->color() = color;
  label_->setStyle(labelStyle_);
}

void LineGraphic::setFont(const std::string& fontName)
{
  labelStyle_.getOrCreate<osgEarth::Symbology::TextSymbol>()->font() = simVis::Registry::instance()->findFontFile(fontName);
  label_->setStyle(labelStyle_);
}

void LineGraphic::setFontSize(float fontSize)
{
  labelStyle_.getOrCreate<osgEarth::Symbology::TextSymbol>()->size() = simVis::osgFontSize(fontSize);
  label_->setStyle(labelStyle_);
}

simVis::AnimatedLineNode* LineGraphic::animatedLine() const
{
  return animatedLine_.get();
}

osgEarth::Annotation::LabelNode* LineGraphic::label() const
{
  return label_.get();
}

////////////////////////////////////////////////////////////

bool Position::positionEquals_(const Position& other) const
{
  return isValid() == other.isValid() && lla() == other.lla();
}

///////////////////////////////////////////////////////////////////////

StaticPosition::StaticPosition()
  : valid_(false)
{
}

StaticPosition::StaticPosition(const simCore::Vec3& lla)
  : valid_(true),
  lla_(lla)
{
}

StaticPosition::StaticPosition(const Position& copy)
  : valid_(copy.isValid()),
  lla_(copy.lla())
{
}

StaticPosition::~StaticPosition()
{
}

void StaticPosition::clear()
{
  valid_ = false;
}

void StaticPosition::setLla(const simCore::Vec3& lla)
{
  valid_ = true;
  lla_ = lla;
}

bool StaticPosition::isValid() const
{
  return valid_;
}

const simCore::Vec3& StaticPosition::lla() const
{
  return lla_;
}

bool StaticPosition::operator==(const Position& other) const
{
  return positionEquals_(other);
}

bool StaticPosition::operator!=(const Position& other) const
{
  return !positionEquals_(other);
}

///////////////////////////////////////////////////////////////////////

/** Position based off a platform's location */
PlatformPosition::PlatformPosition(const simData::DataStore& dataStore, simData::ObjectId platformId)
  : dataStore_(dataStore),
  platformId_(platformId)
{
}

PlatformPosition::~PlatformPosition()
{
}

bool PlatformPosition::isValid() const
{
  const simData::PlatformUpdateSlice* slice = dataStore_.platformUpdateSlice(platformId_);
  if (slice == NULL)
    return false;
  return slice->current() != NULL;
}

const simCore::Vec3& PlatformPosition::lla() const
{
  pullFromDataStore_(lla_);
  return lla_;
}

simData::ObjectId PlatformPosition::platformId() const
{
  return platformId_;
}

int PlatformPosition::pullFromDataStore_(simCore::Vec3& outLla) const
{
  // Get the current item out of the slice
  const simData::PlatformUpdateSlice* slice = dataStore_.platformUpdateSlice(platformId_);
  if (slice == NULL)
    return 1;
  const simData::PlatformUpdate* current = slice->current();
  if (current == NULL)
    return 1;
  // Pull out and convert position
  simCore::Vec3 ecefV3;
  current->position(ecefV3);
  simCore::CoordinateConverter::convertEcefToGeodeticPos(ecefV3, outLla);
  return 0;
}

bool PlatformPosition::operator==(const Position& other) const
{
  const PlatformPosition* pp = dynamic_cast<const PlatformPosition*>(&other);
  return pp != NULL &&
    (pp->platformId_ == this->platformId_) &&
    (&pp->dataStore_ == &this->dataStore_);
}

bool PlatformPosition::operator!=(const Position& other) const
{
  return !operator==(other);
}


}
