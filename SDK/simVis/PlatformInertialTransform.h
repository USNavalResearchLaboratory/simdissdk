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
#ifndef SIMVIS_PLATFORMINERTIALTRANSFORM_H
#define SIMVIS_PLATFORMINERTIALTRANSFORM_H

#include "osg/observer_ptr"
#include "osg/Transform"
#include "simCore/Common/Common.h"

namespace simVis
{
  struct LocatorCallback;

/**
 * Transform that reverses the orientation transform of a Platform.  Useful as a child
 * or scaled child of a PlatformModel.  Will align with the entity's inertial axis rather
 * than the body axis.  Correctly accounts for icon billboarding.  For example:
 *
 * PlatformInertialTransform* inertialXform = new PlatformInertialTransform;
 * platform->addScaledChild(inertialXform);
 * inertialXform->setLocator(platform->getLocator());
 * AxisVector* inertialAxis = new AxisVector;
 * inertialXform->addChild(inertialAxis);
 */
class SDKVIS_EXPORT PlatformInertialTransform : public osg::Transform
{
public:
  META_Node(simVis, PlatformInertialTransform);
  /** Constructor */
  PlatformInertialTransform();
  /** Copy constructor using CopyOp to manage deep vs shallow copy. */
  PlatformInertialTransform(const PlatformInertialTransform& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

  virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor*) const;
  virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor*) const;

  /** Changes the locator used to determine the inverse rotation required for correct orientation */
  void setLocator(simVis::Locator* locator);

  /** Syncs with the locator, adjusting the orientation of the node */
  virtual void syncWithLocator();

protected:
  virtual ~PlatformInertialTransform();
  /** Override childInserted() to call syncWithLocator() when needed */
  virtual void childInserted(unsigned int pos);

private:
  /** Computed entity rotation matrix for the locator, inverted to back out the rotation */
  osg::Quat entityRotationInverse_;

  osg::observer_ptr<Locator> locator_;
  osg::ref_ptr<LocatorCallback> callback_;
};

}

#endif /* SIMVIS_PLATFORMINERTIALTRANSFORM_H */
