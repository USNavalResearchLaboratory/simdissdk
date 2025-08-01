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
#include <limits>
#include "osg/Depth"
#include "osgEarth/LabelNode"
#include "simCore/Calc/Math.h"
#include "simVis/AlphaTest.h"
#include "simVis/Constants.h"
#include "simVis/Locator.h"
#include "simVis/LocatorNode.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/OverheadMode.h"
#include "simVis/EntityLabel.h"

namespace simVis
{
/** Reject pixels with an alpha equal or less than this value.  Useful for blending text correctly against SilverLining. */
static const float ALPHA_THRESHOLD = 0.05f;

EntityLabelNode::EntityLabelNode()
  : hasLastPrefs_(false)
{
  // entity labels are off until prefs turn them on
  setNodeMask(DISPLAY_MASK_NONE);
}

EntityLabelNode::EntityLabelNode(simVis::Locator* locator)
  : hasLastPrefs_(false)
{
  locatorNode_ = new LocatorNode(locator);
  locatorNode_->setNodeMask(DISPLAY_MASK_NONE);
  addChild(locatorNode_.get());
  // entity labels are off until prefs turn them on
  setNodeMask(DISPLAY_MASK_NONE);
}

EntityLabelNode::~EntityLabelNode()
{
}

/// Update the label with the given preferences and text
void EntityLabelNode::update(const simData::CommonPrefs& commonPrefs, const std::string& text, float zOffset)
{
  const simData::LabelPrefs& labelPrefs = commonPrefs.labelprefs();

  // whether to draw the label at all:
  const bool draw = commonPrefs.draw() && labelPrefs.draw();

  // Only make the label when needed
  if (!draw && !label_.valid())
  {
    // entity labels are off until prefs turn them on
    assert(getNodeMask() == DISPLAY_MASK_NONE);
    return;
  }

  bool forceStyle = false;
  if (!label_.valid())
  {
    // create a label node for the first time
    osgEarth::Style style;
    style.getOrCreate<osgEarth::TextSymbol>()->alignment() = static_cast<osgEarth::TextSymbol::Alignment>(labelPrefs.alignment());
    style.getOrCreate<osgEarth::TextSymbol>()->pixelOffset() = osg::Vec2s(labelPrefs.offsetx(), labelPrefs.offsety());
    style.getOrCreate<osgEarth::TextSymbol>()->encoding() = osgEarth::TextSymbol::ENCODING_UTF8;

    label_ = new osgEarth::LabelNode("", style);
    label_->setDynamic(true);
    label_->setNodeMask(simVis::DISPLAY_MASK_LABEL);
    label_->setHorizonCulling(false);
    label_->setOcclusionCulling(false);
    label_->setText(lastText_);

    // Note that labels are not flattened (by default) in overhead mode

    // Set various states in order to make rendering text look better against SilverLining
    osg::StateSet* stateSet = label_->getOrCreateStateSet();

    // Always write to the depth buffer, overriding the osgEarth internal settings
    stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, true), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    AlphaTest::setValues(stateSet, ALPHA_THRESHOLD, osg::StateAttribute::ON);

    // Note: no need to clamp the label's geo transform in overhead mode, since the Locator
    // will take care of that for us. -gw
    if (locatorNode_.valid())
      locatorNode_->addChild(label_.get());
    else
      addChild(label_.get());
    forceStyle = true;
  }

  // Detect label changes in the last update and apply those changes
  if (label_.valid())
  {
    setNodeMask(draw ? DISPLAY_MASK_LABEL : DISPLAY_MASK_NONE);
    if (locatorNode_.valid())
    {
      locatorNode_->setNodeMask(draw ? DISPLAY_MASK_LABEL : DISPLAY_MASK_NONE);

      // if label was just enabled with this prefs change, force our locator node to sync with its locator
      if (draw && (!hasLastPrefs_ || !lastCommonPrefs_.draw() || !lastCommonPrefs_.labelprefs().draw()))
        locatorNode_->syncWithLocator();
    }
    // For priority pref, 0 is least likely to show, higher values more likely to show, negative values are always shown
    // FLT_MAX means always show to osgEarth
    const float actualPriority = (labelPrefs.priority() >= 0.0) ? labelPrefs.priority() : std::numeric_limits<float>::max();
    // Change the priority to user value, or FLT_MAX if always-show
    if (!simCore::areEqual(label_->getPriority(), actualPriority))
      label_->setPriority(actualPriority);

    // Preferences that change how the text is displayed and not the content of the text
    const simData::LabelPrefs& lastLabelPrefs = lastCommonPrefs_.labelprefs();
    const bool labelStylePrefsChanged =
      !hasLastPrefs_ ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, color) ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, offsetx) ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, offsety) ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, outlinecolor) ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, overlayfontname) ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, overlayfontpointsize) ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, textoutline) ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, backdroptype) ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, alignment) ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, priority) ||
      PB_FIELD_CHANGED(&lastLabelPrefs, &labelPrefs, backdropimplementation);

    // update the style:
    if (labelStylePrefsChanged || forceStyle)
    {
      osgEarth::Style style;
      osgEarth::TextSymbol* ts = style.getOrCreate<osgEarth::TextSymbol>();
      ts->alignment() = static_cast<osgEarth::TextSymbol::Alignment>(labelPrefs.alignment());
      ts->pixelOffset() = osg::Vec2s(labelPrefs.offsetx(), labelPrefs.offsety());
      ts->encoding() = osgEarth::TextSymbol::ENCODING_UTF8;

      // Declutter is off if priority is negative
      ts->declutter() = (labelPrefs.priority() >= 0);

      // text color:
      osg::Vec4 color = ColorUtils::RgbaToVec4(labelPrefs.color());
      ts->fill() = osgEarth::Fill(color.r(), color.g(), color.b(), color.a());

      // outline:
      if (labelPrefs.textoutline() != simData::TextOutline::TO_NONE && labelPrefs.backdroptype() != simData::BackdropType::BDT_NONE && color.a() != 0)
      {
        ts->halo()->color() = ColorUtils::RgbaToVec4(labelPrefs.outlinecolor());
        ts->haloOffset() = simVis::outlineThickness(labelPrefs.textoutline());
        ts->haloBackdropType() = backdropType(labelPrefs.backdroptype());
        ts->haloImplementation() = backdropImplementation(labelPrefs.backdropimplementation());
      }
      else
      {
        ts->halo()->color() = osg::Vec4();
        ts->haloOffset() = 0.f;
        ts->haloBackdropType() = osgText::Text::NONE;
      }

      // font:
      if (!labelPrefs.overlayfontname().empty())
      {
        std::string fileFullPath = simVis::Registry::instance()->findFontFile(labelPrefs.overlayfontname());
        if (!fileFullPath.empty()) // only set if font file found, uses default OS font otherwise
          ts->font() = fileFullPath;
      }

      ts->size() = simVis::osgFontSize(static_cast<float>(labelPrefs.overlayfontpointsize()));

      label_->setStyle(style);
    }

    // apply the local altitude offset passed in
    const osg::Vec3d labelOffset(0.0, 0.0, zOffset);
    if (label_->getLocalOffset() != labelOffset)
      label_->setLocalOffset(labelOffset);

    // check for an update:
    if (text != lastText_)
    {
      label_->setText(text);
      lastText_ = text;
    }
  }

  lastCommonPrefs_ = commonPrefs;
  hasLastPrefs_ = true;
}

}

