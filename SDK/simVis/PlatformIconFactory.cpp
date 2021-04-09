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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cassert>
#include <mutex>
#include "osg/ComputeBoundsVisitor"
#include "osg/CullFace"
#include "osg/Depth"
#include "osg/MatrixTransform"
#include "osgEarth/Registry"
#include "simCore/String/Format.h"
#include "simData/DataTypes.h"
#include "simVis/BillboardAutoTransform.h"
#include "simVis/Constants.h"
#include "simVis/OverrideColor.h"
#include "simVis/Registry.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/PlatformIconFactory.h"

namespace simVis {

/**
 * Helper class that maintains various prefs values that will affect the 2-D icon optimization.
 * This caches values that get used when creating the prototype Node instance.  If all values
 * in this class match, then two different platforms should generate identical Node instances.
 * This class provides an operator<() for the purpose of sorting into a std::map.
 *
 * Not all platform prefs are represented here because they might not impact the icon rendering.
 * For example, the label prefs don't apply here, and neither do extrapolation settings.
 *
 * Failure to include a preference here means two different platforms that vary only in that one
 * preference will still share the same icon (SimpleBinnedIconNode).  If that preference impacts
 * the actual display, one of those two platforms will have an incorrect display.
 */
class MergeSettings
{
public:
  /** Default constructor */
  MergeSettings()
  {
  }

  /** Construct matching the values from the prefs. */
  explicit MergeSettings(const simData::PlatformPrefs& prefs)
  {
    set(prefs);
  }

  /** Update the values to match the prefs passed in. */
  void set(const simData::PlatformPrefs& prefs)
  {
    const auto& posOffset = prefs.platpositionoffset();
    platPositionOffset_ = osg::Vec3d(posOffset.x(), posOffset.y(), posOffset.z());
    const auto& oriOffset = prefs.orientationoffset();
    orientationOffset_ = osg::Vec3d(oriOffset.yaw(), oriOffset.pitch(), oriOffset.yaw());
    icon_ = simVis::Registry::instance()->findModelFile(prefs.icon());
    iconAlignment_ = prefs.iconalignment();
    const bool useOverride = prefs.commonprefs().has_useoverridecolor() && prefs.commonprefs().useoverridecolor() &&
      ((prefs.commonprefs().overridecolor() & 0xFF) != 0);
    overrideColor_ = useOverride ? simVis::Color(prefs.commonprefs().overridecolor(), simVis::Color::RGBA) : osg::Vec4f(1.f, 1.f, 1.f, 1.f);
    noDepthIcons_ = prefs.nodepthicons();
    useCullFace_ = prefs.usecullface();
    cullFace_ = osg::CullFace::FRONT_AND_BACK;
    if (useCullFace_)
    {
      if (prefs.cullface() == simData::FRONT)
        cullFace_ = osg::CullFace::FRONT;
      else if (prefs.cullface() == simData::BACK)
        cullFace_ = osg::CullFace::BACK;
    }
    brightness_ = prefs.brightness();
  }

  /** Retrieves the icon field; useful to avoid a double findModelFile(). */
  std::string icon() const
  {
    return icon_;
  }

  /**
   * Provide a comparison operator so this class can be used as a key in a std::map.
   * Failure to include one of the settings captured in the set() here has the same impact
   * as not capturing them.
   */
  bool operator<(const MergeSettings& rhs) const
  {
    // std::tie<> provides a convenient variadic-oriented operator< we can use
    auto asTuple = [](const MergeSettings& rhs) {
      return std::tie(rhs.platPositionOffset_, rhs.orientationOffset_,
        rhs.icon_, rhs.iconAlignment_, rhs.overrideColor_, rhs.noDepthIcons_,
        rhs.useCullFace_, rhs.cullFace_, rhs.brightness_);
    };
    return asTuple(*this) < asTuple(rhs);
  }

private:
  osg::Vec3d platPositionOffset_;
  osg::Vec3d orientationOffset_;
  std::string icon_;
  simData::TextAlignment iconAlignment_;
  osg::Vec4f overrideColor_;
  bool noDepthIcons_ = true;
  bool useCullFace_ = false;
  osg::CullFace::Mode cullFace_ = osg::CullFace::FRONT_AND_BACK;
  int brightness_ = 36;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Simplified node path for image icons that meet certain criteria.  This is an optimized
 * container for image icons that includes performance improvements to reduce state changes.
 *
 * Intended to be used with a single factory for all platforms.  The factory groups up
 * identical icons, assigning a unique ID to each icon.  The icons rely on a nested render
 * bin approach to force icons with similar statesets to render in sequence, rather than
 * randomly, reducing GL stateset changes and drastically improving performance.
 *
 * If 500 entities have the same prefs and can be represented in this node (as per
 * PlatformIconFactory::canApply_()), then only 1 SimpleBinnedIconNode is created for
 * that scene, and it is reused 500 times.
 *
 * If another 300 entities are added to the same scene with different prefs, you'll have
 * two instantiations of SimpleBinnedIconNode, reused another 300 times.  When the scene
 * goes to render, it will (by virtue of render bins) render the 500 platforms first all
 * together, then the second set of 300 icons second, all together.  This reduces state
 * changes and simplifies the draw portion of the OSG pipeline.
 */
class SimpleBinnedIconNode : public osg::MatrixTransform
{
public:
  explicit SimpleBinnedIconNode(const MergeSettings& mergeSettings)
    : mergeSettings_(mergeSettings)
  {
  }

  virtual ~SimpleBinnedIconNode()
  {
  }

  /**
   * Called by PlatformIconFactory to set the image icon node.  The order value is a strictly
   * increasing value from PlatformIconFactory, intended to provide a hint to help with binning.
   */
  void setNode(osg::Node* iconNode, uint64_t order)
  {
    if (getNumChildren())
      removeChildren(0, getNumChildren());
    addChild(iconNode);

    // Determine a nested render bin for the icon.  The render bin needs to be globally unique to
    // other global SDK bins, and should have a wide enough range to have a low chance of conflict
    // with other icons created from PlatformIconFactory.  If there is a conflict, it simply means
    // the stateset processing could arbitrarily flip, reducing performance (but not causing incorrect
    // graphics).  This nested render bin approach is essential to the performance improvement
    // provided by PlatformIconFactory.
    const int binNumber = 1000 + (order % 1000);

    // Inner node is two-pass, outer is sorted
    iconNode->getOrCreateStateSet()->setRenderBinDetails(simVis::BIN_PLATFORM_IMAGE, simVis::BIN_TWO_PASS_ALPHA, osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
    getOrCreateStateSet()->setRenderBinDetails(binNumber, simVis::BIN_GLOBAL_SIMSDK, osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
  }

  /**
   * Applies the prefs to the node.  This is a parallel path to prefs updates in simVis::PlatformModel,
   * and is necessary in order to reduce stateset changing during Draw.
   */
  void updatePrefs(const simData::PlatformPrefs& prefs)
  {
    // Assertion failure means setNode() was not called first.  Assertable because this class is
    // defined entirely inside this .cpp file and we can control execution order.  We need this
    // node to determine the image size.  Failure here means setNode() wasn't called.
    assert(getNumChildren() == 1);

    // Apply platform position offset, orientation offset
    osg::Matrix m;
    if (prefs.has_platpositionoffset())
    {
      const simData::Position& pos = prefs.platpositionoffset();
      // x/y order change and sign change needed to match the behavior of SIMDIS 9
      m.makeTranslate(osg::Vec3(-pos.y(), pos.x(), pos.z()));
    }

    // Do a translation for icon alignment, which is really just a special case of platform offset
    if (getNumChildren() == 1)
    {
      osg::ComputeBoundsVisitor cb;
      getChild(0)->accept(cb);
      const osg::BoundingBox& bounds = cb.getBoundingBox();
      const osg::Vec2f iconDims(bounds.xMax() - bounds.xMin(), bounds.yMax() - bounds.yMin());
      osg::Vec2f xyOffset;
      simVis::iconAlignmentToOffsets(prefs.iconalignment(), iconDims, xyOffset);
      m.preMultTranslate(osg::Vec3f(xyOffset, 0.f));
    }

    // Offset the orientation
    if (prefs.has_orientationoffset())
    {
      const simData::BodyOrientation& ori = prefs.orientationoffset();
      if (ori.yaw() != 0.0 || ori.pitch() != 0.0 || ori.roll() != 0.0)
      {
        const osg::Quat& qrot = Math::eulerRadToQuat(ori.yaw(), ori.pitch(), ori.roll());
        m.preMultRotate(qrot);
      }
    }
    setMatrix(m);

    osg::StateSet* stateSet = getOrCreateStateSet();

    // Apply depth testing
    if (!prefs.has_nodepthicons() || prefs.nodepthicons())
    {
      osg::ref_ptr<osg::Depth> depth = new osg::Depth(osg::Depth::ALWAYS, 0, 1, true);
      stateSet->setAttributeAndModes(depth.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    }
    else
    {
      osg::ref_ptr<osg::Depth> depth = new osg::Depth(osg::Depth::LESS, 0, 1, true);
      stateSet->setAttributeAndModes(depth.get(), osg::StateAttribute::ON);
    }

    // Avoid creating overrideColor_ to avoid state changes
    const bool useOverride = prefs.commonprefs().useoverridecolor();
    if (!overrideColor_.valid() && useOverride)
      overrideColor_ = new simVis::OverrideColor(stateSet);
    if (overrideColor_.valid())
    {
      overrideColor_->setColor(simVis::Color(prefs.commonprefs().overridecolor(), simVis::Color::RGBA));
      overrideColor_->setCombineMode(prefs.commonprefs().useoverridecolor() ?
        OverrideColor::MULTIPLY_COLOR : OverrideColor::OFF);
    }

    // Apply cull face
    if (!prefs.usecullface())
      stateSet->removeAttribute(osg::StateAttribute::CULLFACE);
    else
    {
      static const std::map<simData::PolygonFace, osg::CullFace::Mode> s_FACETOMODE = {
        {simData::FRONT, osg::CullFace::FRONT},
        {simData::BACK, osg::CullFace::BACK},
        {simData::FRONT_AND_BACK, osg::CullFace::FRONT_AND_BACK},
      };
      auto iter = s_FACETOMODE.find(prefs.cullface());
      // assert fail means an invalid face was specified
      assert(iter != s_FACETOMODE.end());
      if (iter != s_FACETOMODE.end())
        stateSet->setAttributeAndModes(new osg::CullFace(iter->second), osg::StateAttribute::ON);
    }

    // Brightness is a uniform on the ambient light contribution
    const float brightnessMagnitude = prefs.brightness() * BRIGHTNESS_TO_AMBIENT;
    auto* brightnessUniform = new osg::Uniform(LIGHT0_AMBIENT_COLOR.c_str(), osg::Vec4f(brightnessMagnitude, brightnessMagnitude, brightnessMagnitude, 1.f));
    stateSet->addUniform(brightnessUniform);
  }

  const MergeSettings& mergeSettings() const
  {
    return mergeSettings_;
  }

private:
  osg::ref_ptr<OverrideColor> overrideColor_;
  MergeSettings mergeSettings_;
};

///////////////////////////////////////////////////////////////////////////////

/** Simple container/wrapper around a std map, used to isolate/envelope external dependencies. */
struct PlatformIconFactory::IconContainer
{
  std::map<MergeSettings, osg::observer_ptr<SimpleBinnedIconNode> > map;
};

///////////////////////////////////////////////////////////////////////////////

class PlatformIconFactory::RemoveNotifier : public osg::Observer
{
public:
  RemoveNotifier(PlatformIconFactory& factory)
    : factory_(factory)
  {
  }

  virtual void objectDeleted(void* ptr) override
  {
    factory_.notifyRemove_(static_cast<osg::Node*>(ptr));
  }

private:
  PlatformIconFactory& factory_;
};

///////////////////////////////////////////////////////////////////////////////

PlatformIconFactory::PlatformIconFactory()
  : icons_(new IconContainer),
  nextOrder_(0),
  enabled_(true)
{
  removeNotifier_.reset(new RemoveNotifier(*this));
}

PlatformIconFactory::~PlatformIconFactory()
{
}

PlatformIconFactory* PlatformIconFactory::instance()
{
  static std::mutex s_InstanceMutex;
  static std::unique_ptr<PlatformIconFactory> s_Instance;
  if (!s_Instance)
  {
    std::lock_guard lock(s_InstanceMutex);
    // Double check pattern
    if (!s_Instance)
      s_Instance.reset(new PlatformIconFactory);
  }
  return s_Instance.get();
}

void PlatformIconFactory::notifyRemove_(osg::Node* old)
{
  // Only care about orphaned nodes
  SimpleBinnedIconNode* oldIcon = dynamic_cast<SimpleBinnedIconNode*>(old);
  if (old == nullptr || old->getNumParents() != 0)
  {
    // Should not be possible -- we should only get this right before the old node is deleted.
    // This means either a parent messed up a ref_ptr, or we're getting extra notifications for
    // nodes that are not SimpleBinnedIconNode instances.
    assert(0);
    return;
  }

  auto& iconMap = icons_->map;
  auto iter = iconMap.find(oldIcon->mergeSettings());
  if (iter != iconMap.end())
    iconMap.erase(iter);
}

osg::Node* PlatformIconFactory::getOrCreate(const simData::PlatformPrefs& prefs)
{
  // If we can't apply based on a settings in the prefs, then exit early
  if (!canApply_(prefs))
    return nullptr;

  // Determine if there's already an icon with the same settings
  const MergeSettings mergeSettings(prefs);
  auto& iconMap = icons_->map;
  auto iter = iconMap.find(mergeSettings);
  if (iter != iconMap.end())
  {
    // Assertion failure means notifyRemove_() failed or didn't get called, ditto with removeNotifier_
    assert(iter->second.valid());
    if (iter->second.valid())
      return iter->second.get();
  }

  // Attempt to load the model node from registry
  bool isImage = false;
  // We can only optimize image icons in this way
  osg::ref_ptr<osg::Node> modelNode = simVis::Registry::instance()->getOrCreateIconModel(mergeSettings.icon(), &isImage);
  if (!isImage || !modelNode)
    return nullptr;

  // Create a copy, because we need its state set
  osg::ref_ptr<SimpleBinnedIconNode> newIcon(new SimpleBinnedIconNode(mergeSettings));
  newIcon->addObserver(removeNotifier_.get());
  newIcon->setName("Binned Transform");

  // Avoid cloning tmd and lst files, which are known to edit textures on the fly.  Cloning them
  // will result in textures not getting updated.
  const std::string& extension = simCore::getExtension(prefs.icon());
  if (extension == ".tmd" || extension == ".lst")
    newIcon->setNode(modelNode, ++nextOrder_);
  else
    newIcon->setNode(osg::clone(modelNode.get(), osg::CopyOp::DEEP_COPY_ALL), ++nextOrder_);
  newIcon->updatePrefs(prefs);

  // Save the icon with its unique mergeSettings
  iconMap.emplace(mergeSettings, newIcon);
  return newIcon.release();
}

bool PlatformIconFactory::canApply_(const simData::PlatformPrefs& prefs) const
{
  // Cannot apply if not enabled
  if (!enabled_)
    return false;

  // Box mode, rendering a box instead of an icon, is unsupported
  if (prefs.drawbox())
    return false;

  // Alpha volume is not supported, but has no application to image icons

  // No circle highlight, nothing in the scaled inertial transform is supported
  if (prefs.drawcirclehilight())
    return false;

  // No ephemeris icons (scaled inertial transform)
  if (prefs.drawbodyaxis() || prefs.drawinertialaxis() || prefs.drawsunvec() || prefs.drawmoonvec())
    return false;

  return true;
}

bool PlatformIconFactory::hasRelevantChanges(const simData::PlatformPrefs& oldPrefs, const simData::PlatformPrefs& newPrefs) const
{
  return
    // Fields that invalidate the index and fields that alter whether return of canApply()
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, icon) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, iconalignment) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, platpositionoffset) ||
    PB_SUBFIELD_CHANGED(&oldPrefs, &newPrefs, orientationoffset, pitch) ||
    PB_SUBFIELD_CHANGED(&oldPrefs, &newPrefs, orientationoffset, yaw) ||
    PB_SUBFIELD_CHANGED(&oldPrefs, &newPrefs, orientationoffset, roll) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, drawbox) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, drawcirclehilight) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, drawbodyaxis) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, drawinertialaxis) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, drawsunvec) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, drawmoonvec) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, brightness) ||
    PB_SUBFIELD_CHANGED(&oldPrefs, &newPrefs, commonprefs, useoverridecolor) ||
    PB_SUBFIELD_CHANGED(&oldPrefs, &newPrefs, commonprefs, overridecolor) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, nodepthicons) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, usecullface) ||
    PB_FIELD_CHANGED(&oldPrefs, &newPrefs, cullface);
}

void PlatformIconFactory::setEnabled(bool enabled)
{
  enabled_ = enabled;
}

bool PlatformIconFactory::isEnabled() const
{
  return enabled_;
}

}
