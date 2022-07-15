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
#include "osg/ComputeBoundsVisitor"
#include "osgUtil/CullVisitor"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simVis/Constants.h"
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "simVis/DynamicScaleTransform.h"

#undef LC
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

///////////////////////////////////////////////////////////////////////

/**
 * Visitor that will touch every DynamicScaleTransform and call the recalculate_() method
 * with the range from the camera to the transform, so that the transform can figure out
 * the dynamic scaling aspect, which will directly impact the bounds of the node.
 *
 * This is required during intersection tests with anything that might involve a dynamic
 * scale node because the bounds must be correct before the intersection visitor comes
 * through, else the node's traverse() will not even be called.
 */
class DynamicScaleTransform::RecalculateScaleVisitor : public osg::NodeVisitor
{
public:
  explicit RecalculateScaleVisitor(TraversalMode tm=osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
    : NodeVisitor(tm)
  {
  }

  // Build up a list of transforms along the node path
  virtual void apply(osg::Transform& xform)
  {
    // Presumption/Optimization: We only fix the top DST in the node path
    simVis::DynamicScaleTransform* dst = dynamic_cast<simVis::DynamicScaleTransform*>(&xform);
    if (dst)
    {
      // getTrans().length() returns the distance from center to the eye
      if (!matrices_.empty())
        dst->recalculate_(matrices_.back()->getTrans().length(), nullptr);
      return;
    }

    // Fill out the matrix to match the last one, then transform via this xform
    osg::ref_ptr<osg::RefMatrix> matrix = matrices_.empty() ? new osg::RefMatrix() : new osg::RefMatrix(*matrices_.back());
    xform.computeLocalToWorldMatrix(*matrix,this);

    // We want to ignore the view matrix if the transform is an absolute reference
    if (xform.getReferenceFrame() != osg::Transform::RELATIVE_RF)
      matrices_.push_back(new osg::RefMatrix());

    matrices_.push_back(matrix);
    traverse(xform);
    matrices_.pop_back();
    // Take off the empty matrix if needed
    if (xform.getReferenceFrame() != osg::Transform::RELATIVE_RF)
      matrices_.pop_back();
  }

private:
  /// Deque of all matrices as we traverse the scene
  std::deque<osg::ref_ptr<osg::RefMatrix> > matrices_;
};

///////////////////////////////////////////////////////////////////////

DynamicScaleTransform::DynamicScaleTransform()
  : osg::Transform(),
    dynamicEnabled_(true),
    staticScalar_(1.0),
    dynamicScalar_(1.0),
    scaleOffset_(0.0),
    dynamicScalePixel_(false),
    overrideScaleSet_(false),
    overrideScale_(NO_SCALE),
    cachedScale_(NO_SCALE),
    iconScaleFactor_(INVALID_SCALE_FACTOR),
    fixedIconScaleFactor_(INVALID_SCALE_FACTOR)
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
    dynamicScalePixel_(rhs.dynamicScalePixel_),
    overrideScaleSet_(rhs.overrideScaleSet_),
    overrideScale_(rhs.overrideScale_),
    cachedScale_(rhs.cachedScale_),
    iconScaleFactor_(INVALID_SCALE_FACTOR),
    fixedIconScaleFactor_(INVALID_SCALE_FACTOR)
{
}

DynamicScaleTransform::~DynamicScaleTransform()
{
}

bool DynamicScaleTransform::computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
  // SIM-13244 / SIMDIS-3633: Do not apply the scaling to the model if node visitor is null.
  // One of the few cases where this is null is during a compute-bounds case. And in that case we
  // don't know the viewport, so we shouldn't be doing anything with dynamic scaling. If we were
  // to apply the dynamic scaling, the value could be stale (from another viewport). And if the
  // model doesn't include (0,0,0) in its bounding sphere, then the model could incorrectly "grow"
  // farther from the origin and end up being reported (incorrectly) as outside view frustum.
  // However, this only really matters even for the culling case, if there is no geometry at the
  // origin. To avoid the inverse of the problem (model not big enough to be inside cull volume),
  // we only apply this fix for models that have no geometry at the origin.

  // Apply the scale if the node visitor is valid, or if we know nothing about bounds
  if (nv || _children.empty())
    matrix.preMultScale(cachedScale_);
  else
  {
    // If the bounding sphere is valid and contains (0,0,0) then also apply the cached scale,
    // to avoid edge-of-frustum-but-really-inside edge case.
    const osg::BoundingSphere& bounds = _children[0]->getBound();
    if (bounds.valid() && bounds.contains(osg::Vec3f(0.f, 0.f, 0.f)))
      matrix.preMultScale(cachedScale_);
  }
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

void DynamicScaleTransform::setDynamicScaleToPixels(bool dynamicScalePixel)
{
  dynamicScalePixel_ = dynamicScalePixel;
}

bool DynamicScaleTransform::dynamicScaleToPixels() const
{
  return dynamicScalePixel_;
}

void DynamicScaleTransform::setFixedSize(double meters)
{
  fixedIconScaleFactor_ = DS_SIZE_SCALAR * powf(meters, DS_SIZE_EXPONENT);
  iconScaleFactor_ = fixedIconScaleFactor_;
}

osg::Node* DynamicScaleTransform::getSizingNode_() const
{
  if (sizingNode_.valid())
    return sizingNode_.get();
  if (!_children.empty())
    return _children[0].get();
  return nullptr;
}

void DynamicScaleTransform::setSizingNode(osg::Node* node)
{
  sizingNode_ = node;
  recomputeBounds();
}

void DynamicScaleTransform::recomputeBounds()
{
  if (fixedIconScaleFactor_ != INVALID_SCALE_FACTOR)
  {
    iconScaleFactor_ = fixedIconScaleFactor_;
    return;
  }

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

double DynamicScaleTransform::getSimulatedOrthoRange_(osgUtil::CullVisitor* cv) const
{
  // Check for nullptr, such as invalid dynamic_cast<>
  if (!cv)
    return 0.0;

  // Need camera to get matrix
  const osg::Camera* camera = cv->getCurrentCamera();
  if (!camera)
    return 0.0;

  // Need the view to get the current FOV
  const simVis::View* view = dynamic_cast<const simVis::View*>(camera->getView());
  if (!view)
    return 0.0;

  // If the projection matrix is in perspective and not ortho, return 0
  const osg::Matrix& proj = camera->getProjectionMatrix();
  if (osg::equivalent(proj(3, 3), 0.0)) // not ortho mode (perspective mode)
    return 0.0;

  // Pull out the projection matrix
  double L, R, B, T, N, F;
  camera->getProjectionMatrixAsOrtho(L, R, B, T, N, F);
  const double height = T - B;
  const double tanFOV = tan(simCore::DEG2RAD * view->fovY() * 0.5);
  // Avoid divide-by-zero
  if (tanFOV == 0.0)
    return 0.0;
  return (height * 0.5) / tanFOV;
}

void DynamicScaleTransform::accept(osg::NodeVisitor& nv)
{
  // Optimize away if not visible, don't accept on children
  if (!nv.validNodeMask(*this))
    return;

  // Only care about cull visitor and intersection visitor
  if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR &&
    nv.getVisitorType() != osg::NodeVisitor::INTERSECTION_VISITOR)
  {
    Transform::accept(nv);
    return;
  }

  // Recompute the scalar if it is currently invalid
  if (iconScaleFactor_ == INVALID_SCALE_FACTOR)
    recomputeBounds();

  // Calculate a scalar for the cull visitor for ortho mode.  This is needed because platforms
  // in ortho appear closer than they are to the eye because ortho is not in perspective mode.
  // So in Ortho, you specify the left/right/top/bottom extents.  The eye range doesn't really
  // matter -- an object two meters away is as big as an object two hundred kilometers away because
  // of the projection.  In effect, in ortho mode the actual eye range along the eye vector has no
  // impact on display.  But osg::LODNode isn't smart enough to account for this.  So we do the
  // calculations here, by calculating the 'apparent' range (eye distance as if we were in
  // perspective), then calculating the actual range, and changing the LOD scalar based on the
  // ratio between the two.
  osgUtil::CullVisitor* cullVisitor = dynamic_cast<osgUtil::CullVisitor*>(&nv);
  // Note that ortho range will change per inset, but not necessarily per platform.
  const double orthoRange = getSimulatedOrthoRange_(cullVisitor);
  const double rangeToEye = nv.getEyePoint().length();

  bool resetLodScale = false;
  double oldLodScale = 0.0;
  // Rescale the LOD
  if (rangeToEye != 0.0 && orthoRange != 0.0 && cullVisitor)
  {
    oldLodScale = cullVisitor->getLODScale();
    resetLodScale  = true;
    cullVisitor->setLODScale(oldLodScale * orthoRange / rangeToEye);
  }

  // Figure out the scaling: either override, dynamic, or static
  osg::Vec3f newScale;
  if (hasOverrideScale())
    newScale = overrideScale_;
  else if (!isDynamicScalingEnabled())
    newScale.set(staticScalar_, staticScalar_, staticScalar_);
  else
  {
    const double range = (orthoRange == 0.0 ? rangeToEye : orthoRange);
    // Compute the dynamic scale based on the distance from the eye
    newScale = computeDynamicScale_(range, dynamic_cast<osg::CullStack*>(&nv));
  }

  // Dirty the bounding sphere and return the size
  if (cachedScale_ != newScale && newScale.x() > 0.0 && newScale.y() > 0.0 && newScale.z() > 0.0)
  {
    cachedScale_ = newScale;
    dirtyBound();
  }
  Transform::accept(nv);

  // Reset the LOD scale back to what it used to be
  if (cullVisitor && resetLodScale)
    cullVisitor->setLODScale(oldLodScale);
}

void DynamicScaleTransform::recalculate_(double range, osg::CullStack* cullStack)
{
  // noop; don't adjust bounds
  if (hasOverrideScale() || !isDynamicScalingEnabled() || range <= 0.0)
    return;

  const osg::Vec3f newScale = computeDynamicScale_(range, cullStack);
  // Dirty the bounding sphere
  if (cachedScale_ != newScale && newScale.x() > 0.0 && newScale.y() > 0.0 && newScale.z() > 0.0)
  {
    cachedScale_ = newScale;
    dirtyBound();
  }
}

osg::Vec3f DynamicScaleTransform::computeDynamicScale_(double range, osg::CullStack* cullStack) const
{
  // Pixel model dynamic scale algorithm (relatively new)
  if (dynamicScalePixel_ && cullStack)
  {
    // Note the use of 0.48f is about half a pixel, matches constant from osg/AutoTransform.cpp
    const float pixelSize = cullStack->pixelSize(osg::Vec3f(), 0.48f);
    const double safeDs = (dynamicScalar_ == 0. ? 1. : dynamicScalar_);
    const double scale = staticScalar_ * (pixelSize == 0.f ? 0. : (1.0 / pixelSize)) / safeDs;
    // Do not bother with dynamic scale offset; it only really makes sense in the context of the dynamic
    // scale algorithm, which helps to prevent viewport items from being too big based on eye range.
    return osg::Vec3f(scale, scale, scale);
  }

  // Traditional dynamic scale algorithm
  if (iconScaleFactor_ != INVALID_SCALE_FACTOR)
  {
    // Compute the distance at which scaling begins
    const float maxLen = iconScaleFactor_ * dynamicScalar_ + scaleOffset_;

    // Calculate the scale value
    float scale = staticScalar_;
    if (range > maxLen && maxLen != 0.f)
      scale = staticScalar_ * range / maxLen;
    return osg::Vec3f(scale, scale, scale);
  }
  return NO_SCALE;
}

void DynamicScaleTransform::recalculateAllDynamicScaleBounds(osg::Camera& camera)
{
  // Set up the visitor and have it go
  RecalculateScaleVisitor updateDynamicScaleBounds;
  camera.accept(updateDynamicScaleBounds);
}

}
