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
#include "MouseManipulator.h"

namespace simUtil {

MouseManipulatorProxy::MouseManipulatorProxy()
{
}

MouseManipulatorProxy::MouseManipulatorProxy(const MouseManipulatorPtr& realManipulator)
  : manipulator_(realManipulator)
{
}

int MouseManipulatorProxy::push(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (manipulator_ == NULL)
    return 0;
  return manipulator_->push(ea, aa);
}

int MouseManipulatorProxy::release(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (manipulator_ == NULL)
    return 0;
  return manipulator_->release(ea, aa);
}

int MouseManipulatorProxy::move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (manipulator_ == NULL)
    return 0;
  return manipulator_->move(ea, aa);
}

int MouseManipulatorProxy::drag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (manipulator_ == NULL)
    return 0;
  return manipulator_->drag(ea, aa);
}

int MouseManipulatorProxy::doubleClick(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (manipulator_ == NULL)
    return 0;
  return manipulator_->doubleClick(ea, aa);
}

int MouseManipulatorProxy::scroll(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (manipulator_ == NULL)
    return 0;
  return manipulator_->scroll(ea, aa);
}

int MouseManipulatorProxy::frame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (manipulator_ == NULL)
    return 0;
  return manipulator_->frame(ea, aa);
}

void MouseManipulatorProxy::activate()
{
  if (manipulator_)
    manipulator_->activate();
}

void MouseManipulatorProxy::deactivate()
{
  if (manipulator_)
    manipulator_->deactivate();
}

MouseManipulatorPtr MouseManipulatorProxy::subject() const
{
  return manipulator_;
}

void MouseManipulatorProxy::setSubject(MouseManipulatorPtr manipulator)
{
  manipulator_ = manipulator;
}

}
