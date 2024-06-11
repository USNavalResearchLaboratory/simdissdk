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
#include "osgEarth/GeoMath"
#include "osgEarth/LabelNode"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simVis/Types.h"
#include "simVis/CustomRendering.h"
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
static const simVis::Color DEFAULT_LINECOLOR = simVis::Color::Yellow;
static const simVis::Color DEFAULT_TEXTCOLOR = simVis::Color::White;
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
  osg::ref_ptr<osgEarth::TextSymbol> text = labelStyle_.getOrCreate<osgEarth::TextSymbol>();
  text->fill()->color() = DEFAULT_TEXTCOLOR;
  text->halo()->color() = simVis::Color::Black;
  text->alignment() = osgEarth::TextSymbol::ALIGN_CENTER_CENTER;
  text->haloOffset() = simVis::outlineThickness(simData::TO_THICK);
  osg::ref_ptr<osgEarth::RenderSymbol> render = labelStyle_.getOrCreate<osgEarth::RenderSymbol>();
  render->lighting() = false;
  text->size() = simVis::osgFontSize(DEFAULT_FONTSIZE);
  text->font() = simVis::Registry::instance()->findFontFile(DEFAULT_FONT);
  // Don't declutter -- always show this label
  text->declutter() = false;

  // Create the label node itself
  label_ = new osgEarth::LabelNode();
  label_->setMapNode(mapNode);
  label_->setStyle(labelStyle_);
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
  if (origin != nullptr && destination != nullptr && origin->isValid() && destination->isValid())
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
      label_->setText("");
    }
    else
    {
      // Figure out the label position
      double labelLat = 0;
      double labelLon = 0;
      osgEarth::GeoMath::midpoint(originLLA.lat(), originLLA.lon(),
        destinationLLA.lat(), destinationLLA.lon(), labelLat, labelLon);
      label_->setPosition(osgEarth::GeoPoint(wgs84Srs_.get(), labelLon * simCore::RAD2DEG, labelLat * simCore::RAD2DEG, (originLLA.alt() + destinationLLA.alt()) / 2.0));
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
  label_->setNodeMask((draw && !label_->text().empty()) ? displayMask_ : simVis::DISPLAY_MASK_NONE);
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
  labelStyle_.getOrCreate<osgEarth::TextSymbol>()->fill()->color() = color;
  label_->setStyle(labelStyle_);
}

void LineGraphic::setFont(const std::string& fontName)
{
  labelStyle_.getOrCreate<osgEarth::TextSymbol>()->font() = simVis::Registry::instance()->findFontFile(fontName);
  label_->setStyle(labelStyle_);
}

void LineGraphic::setFontSize(float fontSize)
{
  labelStyle_.getOrCreate<osgEarth::TextSymbol>()->size() = simVis::osgFontSize(fontSize);
  label_->setStyle(labelStyle_);
}

simVis::AnimatedLineNode* LineGraphic::animatedLine() const
{
  return animatedLine_.get();
}

osgEarth::LabelNode* LineGraphic::label() const
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

/** Position based off a node's locator LLA coordinate location */
EntityNodePosition::EntityNodePosition(simVis::EntityNode* node)
  : node_(node)
{
}

EntityNodePosition::~EntityNodePosition()
{
}

bool EntityNodePosition::isValid() const
{
  if (node_ == nullptr)
    return false;

  return node_->getNodeMask() != 0;
}

const simCore::Vec3& EntityNodePosition::lla() const
{
  if (node_ != nullptr)
    node_->getPosition(&lla_, simCore::COORD_SYS_LLA);

  return lla_;
}

simData::ObjectId EntityNodePosition::id() const
{
  if (node_ == nullptr)
    return 0;

  return node_->getId();
}

std::string EntityNodePosition::entityName() const
{
  if (node_ == nullptr)
    return "";

  return node_->getEntityName(simVis::EntityNode::DISPLAY_NAME);
}

bool EntityNodePosition::operator==(const Position& other) const
{
  const EntityNodePosition* pp = dynamic_cast<const EntityNodePosition*>(&other);
  return (pp != nullptr && (pp->id() == this->id()));
}

bool EntityNodePosition::operator!=(const Position& other) const
{
  return !operator==(other);
}


}
