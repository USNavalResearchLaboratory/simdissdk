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
#include "osg/ComputeBoundsVisitor"
#include "osgUtil/CullVisitor"
#include "simNotify/Notify.h"
#include "simVis/Constants.h"
#include "simVis/Utils.h"
#include "simVis/DynamicScaleTransform.h"

#define LC "[DynamicScaleTransform] "

namespace simVis {

/**
 * Size scalar applies after the exponent relative to size.  Larger values mean that
 * the icon will be smaller once it "locks" for the dynamic scaling, so the lock occurs
 * further from the icon.  Smaller values mean that the icon will be larger once it
 * "locks" in the dynamic scaling, so lock occurs closer to icon.
 */
static const float DS_SIZE_SCALAR = 50.f;

/**
 * Exponential factor to apply to the bounding radius.  Larger values create smaller
 * icons, whereas smaller values create larger icons.  This is involved with the
 * normalization of dynamic scaled icons so that large entities and small entities
 * each scale to reasonable values.
 */
static const float DS_SIZE_EXPONENT = 0.85f;

/** Sentinel value for invalid scale factor (icon not valid) */
static const double INVALID_SCALE_FACTOR = 0.0;

/** Indicates a scaling of 1 (no scaling) */
static const osg::Vec3f NO_SCALE = osg::Vec3f(1.f, 1.f, 1.f);

DynamicScaleTransform::DynamicScaleTransform()
  : osg::Transform(),
    dynamicEnabled_(true),
    staticScalar_(1.0),
    dynamicScalar_(1.0),
    scaleOffset_(0.0),
    overrideScaleSet_(false),
    overrideScale_(NO_SCALE),
    cachedScale_(NO_SCALE),
    iconScaleFactor_(INVALID_SCALE_FACTOR)
{
  setName("DynamicScaleTransform");
}

DynamicScaleTransform::DynamicScaleTransform(const DynamicScaleTransform& rhs, const osg::CopyOp& copyop)
  : osg::Transform(rhs, copyop),
    sizingNode_(rhs.sizingNode_),
    dynamicEnabled_(rhs.dynamicEnabled_),
    staticScalar_(rhs.staticScalar_),
    dynamicScalar_(rhs.dynamicScalar_),
    scaleOffset_(rhs.scaleOffset_),
    overrideScaleSet_(rhs.overrideScaleSet_),
    overrideScale_(rhs.overrideScale_),
    cachedScale_(rhs.cachedScale_),
    iconScaleFactor_(INVALID_SCALE_FACTOR)
{
}

DynamicScaleTransform::~DynamicScaleTransform()
{
}

bool DynamicScaleTransform::computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
  matrix.preMultScale(cachedScale_);
  return true;
}

bool DynamicScaleTransform::computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
  if (cachedScale_.x() == 0 || cachedScale_.y() == 0 || cachedScale_.z() == 0)
    return false;
  matrix.postMultScale(osg::Vec3d(1.0/cachedScale_.x(), 1.0/cachedScale_.y(), 1.0/cachedScale_.z()));
  return true;
}

void DynamicScaleTransform::setDynamicScalingEnabled(bool enabled)
{
  dynamicEnabled_ = enabled;
  // Use the static scale if needed, but only if we're not overriding
  if (!overrideScaleSet_ && !dynamicEnabled_)
    cachedScale_.set(staticScalar_, staticScalar_, staticScalar_);
}

bool DynamicScaleTransform::isDynamicScalingEnabled() const
{
  return dynamicEnabled_;
}

osg::Node* DynamicScaleTransform::getSizingNode_()
{
  if (sizingNode_.valid())
    return sizingNode_.get();
  if (getNumChildren() > 0)
    return getChild(0);
  return NULL;
}

void DynamicScaleTransform::setSizingNode(osg::Node* node)
{
  sizingNode_ = node;
  recomputeBounds();
}

void DynamicScaleTransform::recomputeBounds()
{
  // Compute the bounding box of the icon
  osg::ComputeBoundsVisitor cb;
  cb.setTraversalMask(cb.getTraversalMask() |~ simVis::DISPLAY_MASK_LABEL);
  osg::ref_ptr<osg::Node> sizingNode = getSizingNode_();
  if (!sizingNode.valid())
  {
    iconScaleFactor_ = INVALID_SCALE_FACTOR;
    return;
  }
  sizingNode->accept(cb);

  // Get the maximum dimension, used in the scalar operation
  const float maxDimension = VectorScaling::boundingBoxMaxDimension(cb.getBoundingBox());

  // Compute the icon scale factor using the SIMDIS 9 algorithm
  iconScaleFactor_ = DS_SIZE_SCALAR * powf(maxDimension, DS_SIZE_EXPONENT);
}

void DynamicScaleTransform::setStaticScalar(double scalar)
{
  if (staticScalar_ != scalar)
  {
    staticScalar_ = scalar;
    dirtyBound();
  }
}

double DynamicScaleTransform::staticScalar() const
{
  return staticScalar_;
}

void DynamicScaleTransform::setDynamicScalar(double scalar)
{
  if (dynamicScalar_ != scalar)
  {
    dynamicScalar_ = scalar;
    dirtyBound();
  }
}

double DynamicScaleTransform::dynamicScalar() const
{
  return dynamicScalar_;
}

void DynamicScaleTransform::setScaleOffset(double scaleOffset)
{
  if (scaleOffset_ != scaleOffset)
  {
    scaleOffset_ = scaleOffset;
    dirtyBound();
  }
}

double DynamicScaleTransform::scaleOffset() const
{
  return scaleOffset_;
}

void DynamicScaleTransform::setOverrideScale(const osg::Vec3& scaleXyz)
{
  if (!overrideScaleSet_ || overrideScale_ != scaleXyz)
  {
    overrideScaleSet_ = true;
    overrideScale_ = scaleXyz;
    cachedScale_ = scaleXyz;
    dirtyBound();
  }
}

const osg::Vec3& DynamicScaleTransform::overrideScale() const
{
  return overrideScale_;
}

bool DynamicScaleTransform::hasOverrideScale() const
{
  return overrideScaleSet_;
}

void DynamicScaleTransform::clearOverrideScale()
{
  if (overrideScaleSet_ || overrideScale_ != NO_SCALE)
  {
    overrideScaleSet_ = false;
    overrideScale_.set(NO_SCALE);
    dirtyBound();
  }
}

void DynamicScaleTransform::accept(osg::NodeVisitor& nv)
{
  // Optimize away if not visible, don't accept on children
  if (!nv.validNodeMask(*this))
    return;

  // Only care about cull visitor
  if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
  {
    Transform::accept(nv);
    return;
  }

  // Recompute the scalar if it is currently invalid
  if (iconScaleFactor_ == INVALID_SCALE_FACTOR)
    recomputeBounds();

  // Figure out the scaling: either override, dynamic, or static
  osg::Vec3f newScale;
  if (hasOverrideScale())
    newScale = overrideScale_;
  else if (!isDynamicScalingEnabled())
    newScale.set(staticScalar_, staticScalar_, staticScalar_);
  else
    newScale = computeDynamicScale_(dynamic_cast<osgUtil::CullVisitor*>(&nv));

  // Dirty the bounding sphere and return the size
  if (cachedScale_ != newScale && newScale.x() > 0.0 && newScale.y() > 0.0 && newScale.z() > 0.0)
  {
    cachedScale_ = newScale;
    dirtyBound();
  }
  Transform::accept(nv);
}

osg::Vec3f DynamicScaleTransform::computeDynamicScale_(const osgUtil::CullVisitor* cullVisitor)
{
  osg::ref_ptr<const osg::Node> sizeNode = getSizingNode_();
  if (cullVisitor && sizeNode.valid() && iconScaleFactor_ != INVALID_SCALE_FACTOR)
  {
    // Compute distance from entity to the eye
    const double distance = cullVisitor->getEyeLocal().length();

    // Compute the distance at which scaling begins
    const float maxLen = iconScaleFactor_ * dynamicScalar_ + scaleOffset_;

    // Calculate the scale value
    float scale = staticScalar_;
    if (distance > maxLen && maxLen != 0.f)
      scale = staticScalar_ * distance / maxLen;
    return osg::Vec3f(scale, scale, scale);
  }
  return NO_SCALE;
}

}
