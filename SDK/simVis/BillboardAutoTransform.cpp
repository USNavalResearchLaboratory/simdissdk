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

#include "osgUtil/CullVisitor"
#include "simCore/Calc/MathConstants.h"
#include "simVis/BillboardAutoTransform.h"

#define LC "[BillboardAutoTransform]"

namespace simVis
{

BillboardAutoTransform::BillboardAutoTransform()
  : osg::AutoTransform(),
    dirty_(true),
    rotateInScreenSpace_(false),
    screenSpaceRotationRadians_(0.0)
{
  // deactivate culling for the first traversal. We will reactivate it later.
  setCullingActive(false);
  setMinimumScale(1.0);
}

void BillboardAutoTransform::setRotateInScreenSpace(bool value)
{
  rotateInScreenSpace_ = value;
}

bool BillboardAutoTransform::getRotateInScreenSpace() const
{
  return rotateInScreenSpace_;
}

void BillboardAutoTransform::setScreenSpaceRotation(double radians)
{
  screenSpaceRotationRadians_ = radians;
}

double BillboardAutoTransform::getScreenSpaceRotation() const
{
  return screenSpaceRotationRadians_;
}

void BillboardAutoTransform::accept(osg::NodeVisitor& nv)
{
  // optimization - don't bother with mathing if the node is hidden.
  // (this occurs in Node::accept, which we override here)
  if (!nv.validNodeMask(*this))
    return;

  bool resetLodScale = false;
  double oldLodScale = 1.0;
  if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
  {
    // re-activate culling now that the first cull traversal has taken place.
    this->setCullingActive(true);
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (cv)
    {
      if (rotateInScreenSpace_ == true)
      {
        osg::Vec3d translation, scale;
        osg::Quat  rotation, so;
        osg::RefMatrix& mvm = *(cv->getModelViewMatrix());

        mvm.decompose(translation, rotation, scale, so);

        // this will rotate the object into screen space.
        osg::Quat toScreen(rotation.inverse());

        // we need to compensate for the "heading" of the camera, so compute that.
        // From (http://goo.gl/9bjM4t).
        // GEOCENTRIC ONLY!

        const osg::Matrixd& view = cv->getCurrentCamera()->getViewMatrix();
        osg::Matrixd viewInverse;
        viewInverse.invert(view);

        osg::Vec3d N(0, 0, 6356752); // north pole, more or less
        osg::Vec3d b(-view(0, 2), -view(1, 2), -view(2, 2)); // look vector
        osg::Vec3d E = osg::Vec3d(0, 0, 0)*viewInverse;
        osg::Vec3d u = E; u.normalize();

        // account for looking straight downish
        if (osg::equivalent(b*u, -1.0, 1e-4))
        {
          // up vec becomes the look vec.
          b = osg::Matrixd::transform3x3(view, osg::Vec3f(0.0, 1.0, 0.0));
          b.normalize();
        }

        osg::Vec3d proj_d = b - u*(b*u);
        osg::Vec3d n = N - E;
        osg::Vec3d proj_n = n - u*(n*u);
        osg::Vec3d proj_e = proj_n^u;

        double cameraHeading = atan2(proj_e*proj_d, proj_n*proj_d);

        while (cameraHeading < 0.0)
          cameraHeading += M_2_PI;
        double objHeading = screenSpaceRotationRadians_;
        while (objHeading < 0.0)
          objHeading += M_2_PI;
        double finalRot = cameraHeading - objHeading;
        while (finalRot > M_PI)
          finalRot -= M_2_PI;

        osg::Quat toRotation(finalRot, osg::Vec3(0, 0, 1));

        setRotation(toRotation * toScreen);
      }

      else if (_autoRotateMode == ROTATE_TO_SCREEN)
      {
        osg::Vec3d translation;
        osg::Quat rotation;
        osg::Vec3d scale;
        osg::Quat so;

        cv->getModelViewMatrix()->decompose(translation, rotation, scale, so);

        setRotation(rotation.inverse());
      }

      dirty_ = false;

      // update the LOD Scale based on the auto-scale.
      const double xScale = getScale().x();
      if (xScale != 1.0 && xScale != 0.0)
      {
        oldLodScale = cv->getLODScale();
        resetLodScale = true;
        cv->setLODScale(1.0 / xScale);
      }

    } // if (cv)
  } // if is cull visitor

  // finally, skip AT's accept and do Transform.
  Transform::accept(nv);

  // Reset the LOD scale if we changed it
  if (resetLodScale)
  {
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (cv)
      cv->setLODScale(oldLodScale);
  }
}

void BillboardAutoTransform::dirty()
{
  dirty_ = true;
  setCullingActive(false);
}

}
