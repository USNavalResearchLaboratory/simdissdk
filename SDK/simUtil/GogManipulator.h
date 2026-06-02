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
#pragma once

#include <functional>
#include <memory>
#include <vector>
#include "osg/Group"
#include "osgEarth/MapNode"
#include "simCore/Common/Common.h"

namespace osgEarth { class GeoPoint; }
namespace simVis { class AnimatedLineNode; }
namespace simVis::GOG { class GogNodeInterface; }

namespace simUtil {

class IconDragger;

/**
 * Represents a standalone, reusable manipulator for GOGs. Can be attached to a single GOG shape,
 * and that target GOG can be changed. Includes in-scene handles for translation, scale, and
 * rotation. Can only edit position for relative GOGs, by changing the reference location.
 * Changes take effect immediately, and when the user lets go an Edit-Finished callback is
 * optionally fired off. This class works with GogNodeInterface, e.g. single GOG Shapes. A
 * single GOG file might contain one or more shapes.
 */
class SDKUTIL_EXPORT GogManipulator : public osg::Group
{
public:
  /** Create a GOG Manipulator for the given map node. */
  explicit GogManipulator(osgEarth::MapNode* mapNode);
  virtual ~GogManipulator();
  SDK_DISABLE_COPY_MOVE(GogManipulator);

  /** Bind the manipulator to a specific GOG */
  void setTarget(std::shared_ptr<simVis::GOG::GogNodeInterface> gog);
  /** Hide the manipulator and clear the active target */
  void clearTarget();
  /** Return the currently bound GOG target */
  std::shared_ptr<simVis::GOG::GogNodeInterface> target() const;
  /** Returns true if the target GOG is set and is editable */
  bool hasTarget() const;

  /** Returns true if the GOG is "structurally" uneditable, e.g. absolute, attached, or not supported. These don't change. */
  static bool canEdit(const simVis::GOG::GogNodeInterface& gog);

  /** Callback fired when the user releases the mouse after dragging a handle. */
  using EditFinishedCallback = std::function<void(std::shared_ptr<simVis::GOG::GogNodeInterface>)>;
  /** Attach a listener to be notified when a drag operation completes */
  void setEditFinishedCallback(EditFinishedCallback cb);

private:
  class SynchronizeCallback;

  void syncDraggersToGog_();
  void handleTranslation_(const osgEarth::GeoPoint& newPos);
  void handleRotation_(const osgEarth::GeoPoint& handlePos);
  void handleScale_(const osgEarth::GeoPoint& handlePos);

  std::shared_ptr<simVis::GOG::GogNodeInterface> activeGog_;
  osg::observer_ptr<osgEarth::MapNode> mapNode_;
  osg::ref_ptr<osg::Group> handlesGroup_;
  osg::ref_ptr<simUtil::IconDragger> transDragger_;
  osg::ref_ptr<simUtil::IconDragger> rotDragger_;
  osg::ref_ptr<simUtil::IconDragger> scaleDragger_;

  /** User-provided function to notify when mouse is released from an edit. */
  EditFinishedCallback editFinishedCallback_;

  /** Distance from the GOG origin (0,0) to the rotation handle; set on setTarget() */
  double rotationDistanceM_ = 10000.;
  /** Distance for 100% scale, for the rotation handle (at 45 degree angle) */
  double baseScaleDistanceM_ = 14142.;

  /** Track whether we're dragging, so we know when to send out the edit-finished callback. */
  bool isDragging_ = false;

  /** Holds the visual guides for bounding box and line to rotation icon */
  osg::ref_ptr<osg::Group> guideGroup_;
  /** Draws a line from center of shape into the 0 degree north location of the rotation icon */
  osg::ref_ptr<simVis::AnimatedLineNode> centerToRotLine_;
  /** Draws a box around the bounding box to help identify the scale and rotation locations */
  std::vector<osg::ref_ptr<simVis::AnimatedLineNode>> boxLines_;
};

}
