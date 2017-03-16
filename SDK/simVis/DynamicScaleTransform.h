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
#ifndef SIMVIS_DYNAMICSCALETRANSFORM_H
#define SIMVIS_DYNAMICSCALETRANSFORM_H

#include "osg/Transform"
#include "osg/observer_ptr"
#include "osg/Quat"
#include "simCore/Common/Export.h"

namespace osg {
  class NodeVisitor;
  class Camera;
}

namespace simVis
{

/**
 * Handles dynamic scaling of entities.
 *
 * Dynamic scaling will apply a scale factor transform to improve the visibility of the
 * icon.  The scaling is based on the icon dimensions, the distance from the eye, and
 * user-provided scaling values.  Each inset view resizes independently.
 *
 * Dynamic scaling can be enabled or disabled.  When disabled, the icon is not dynamically
 * scaled, and a typical scaling of (1,1,1) is applied.  When enabled, the scaling factor
 * is calculated based on the input parameters.  The following user-provided values impact
 * the scaling of a dynamically scaled icon:
 *
 * Static Scalar - Icon scale to be applied to the icon only when the eye is closer than
 *   the distance at which dynamic scaling begins.
 *
 * Dynamic Scalar - Increases the distance at which the icon locks into place size-wise.  In
 *   other words, larger values create smaller icons, but smaller values create larger icons.
 *   Larger values create smaller icons because the lock-in location for the eye distance
 *   becomes farther, meaning the icon is smaller from "natural" scaling before the dynamic
 *   portion kicks in.  This is a multiplicative factor based on icon size.
 *
 * Scale Offset - Increases the distance at which the icon locks into place size-wise.  In
 *   other words, larger values create smaller icons, but smaller values create larger icons.
 *   Larger values create smaller icons because the lock-in location for the eye distance
 *   becomes farther, meaning the icon is smaller from "natural" scaling before the dynamic
 *   portion kicks in.  This is an additive factor and is compared directly to eye distance.
 */
class SDKVIS_EXPORT DynamicScaleTransform : public osg::Transform
{
public:
  DynamicScaleTransform();
  /** Copy constructor using CopyOp to manage deep vs shallow copy. */
  DynamicScaleTransform(const DynamicScaleTransform&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

  virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor*) const;
  virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor*) const;

  /** Sets the node to use for determining scaling factor; defaults to first child if not set. */
  void setSizingNode(osg::Node* node);
  /** Recomputes the bounds from the sizing node; use this when sizing node changes size; automatically called on setSizingNode() */
  void recomputeBounds();

  /** Turns on or off the dynamic scaling.  When off, no scaling is done. */
  void setDynamicScalingEnabled(bool enabled);
  /** Returns whether dynamic scaling is activated */
  bool isDynamicScalingEnabled() const;

  /** Changes the static scaling (smaller value is smaller icon); combines with dynamic */
  void setStaticScalar(double scalar);
  /** Retrieves the static scale factor (smaller value is smaller icon) */
  double staticScalar() const;

  /** Changes the scale factor for dynamic scaling (smaller value is bigger icon); combines with static */
  void setDynamicScalar(double scalar);
  /** Retrieves the dynamic scale factor (smaller value is bigger icon) */
  double dynamicScalar() const;

  /** Adds an offset to the distance at which scaling begins */
  void setScaleOffset(double scaleOffset);
  /** Retrieves the scale offset */
  double scaleOffset() const;

  /** When set, the override scale will force a scale on each axis, skipping dynamic and other static scaling */
  void setOverrideScale(const osg::Vec3& scaleXyz);
  /** Returns the current scale override */
  const osg::Vec3& overrideScale() const;
  /** Returns true if there is a valid scale override */
  bool hasOverrideScale() const;
  /** Clears out the scale override */
  void clearOverrideScale();

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "DynamicScaleTransform"; }

  /** Override accept() to compute per-view bounds */
  virtual void accept(osg::NodeVisitor& nv);

  /**
   * Given a camera, iterates through the scene and recalculates the bounding spheres on
   * all DynamicScaleTransform in the scene.  This is useful before doing an intersection
   * test in cases where there are multiple insets.  Failure to use this before an
   * intersection test means that the bounds on the dynamic scale node may not be appropriate
   * for the given intersection.  This is only done on active nodes.
   * @param camera Visited to rescale all dynamic scale transforms.
   */
  static void recalculateAllDynamicScaleBounds(osg::Camera& camera);

protected:
  /** Protected destructor to force use of osg::ref_ptr */
  virtual ~DynamicScaleTransform();

private:
  /// Helper class to recalculate bounds
  class RecalculateScaleVisitor;

  /** Returns first child if sizingNode_ is unset */
  osg::Node* getSizingNode_();
  /** Computes the dynamic scale; requires valid sizing node and valid icon scale factor */
  osg::Vec3f computeDynamicScale_(double range);
  /** Recalculates the bounds if in dynamic scale mode, called by the recalculateAllDynamicScaleBounds() */
  void recalculate_(double range);

  /// Sizing node that is used for appropriate scaling based on eye distance
  osg::observer_ptr<osg::Node> sizingNode_;
  /// When false, the scale factor is not applied (scale of 1.0)
  bool dynamicEnabled_;
  /// Starting static scale.  Larger values increase size
  double staticScalar_;
  /// Multiplier on the size.  Smaller values increase size
  double dynamicScalar_;
  /// Offset on the size, applied after multiplier
  double scaleOffset_;

  /// Validity flag for overrideScale_
  bool overrideScaleSet_;
  /// Override scale value; only valid if overrideScaleSet_
  osg::Vec3f overrideScale_;

  /// 3D scaling applied in Transform
  osg::Vec3f cachedScale_;

  /// Computed icon scaling factor, based on bounding box of sizing node
  double iconScaleFactor_;
};

}

#endif /* SIMVIS_DYNAMICSCALETRANSFORM_H */
