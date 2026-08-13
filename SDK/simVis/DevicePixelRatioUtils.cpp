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
#include "osgGA/GUIEventHandler"
#include "osgText/TextBase"
#include "osgEarth/Registry"
#include "simCore/Calc/Math.h"
#include "simVis/Utils.h"
#include "simVis/DevicePixelRatioUtils.h"

namespace simVis {

double DevicePixelRatioUtils::getDpr()
{
  return osgEarth::Registry::instance()->getDevicePixelRatio();
}

double DevicePixelRatioUtils::toLogical(double physical)
{
  return physical / DevicePixelRatioUtils::getDpr();
}

double DevicePixelRatioUtils::toPhysical(double logical)
{
  return logical * DevicePixelRatioUtils::getDpr();
}

osg::Vec2d DevicePixelRatioUtils::getLogicalMousePosition(const osgGA::GUIEventAdapter& ea)
{
  const double dpr = DevicePixelRatioUtils::getDpr();
  return osg::Vec2d(ea.getX() / dpr, ea.getY() / dpr);
}

/////////////////////////////////////////////////////////////////////////

DpiTextScalingCallback::DpiTextScalingCallback(float logicalFontSize)
  : logicalFontSize_(logicalFontSize)
{
}

void DpiTextScalingCallback::setCharacterSize(float logicalFontSize)
{
  logicalFontSize_ = logicalFontSize;
}

void DpiTextScalingCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
  auto* textNode = dynamic_cast<osgText::Text*>(node);
  if (textNode)
    applyScaling_(*textNode);
  traverse(node, nv);
}

osg::ref_ptr<DpiTextScalingCallback> DpiTextScalingCallback::install(osgText::Text& textNode, float logicalFontSize)
{
  // Need dynamic variance to change font size at runtime
  textNode.setDataVariance(osg::Object::DYNAMIC);

  osg::ref_ptr<DpiTextScalingCallback> rv = new DpiTextScalingCallback(logicalFontSize);
  textNode.addUpdateCallback(rv.get());
  rv->applyScaling_(textNode);
  return rv;
}

void DpiTextScalingCallback::applyScaling_(osgText::Text& textNode) const
{
  const double currentDpr = simVis::DevicePixelRatioUtils::getDpr();

  // Calculate what the physical character size SHOULD be on this monitor
  const float expectedPhysicalSize = logicalFontSize_ * static_cast<float>(currentDpr);

  // If it doesn't match the current size (with tolerance for float drift), we moved screens!
  if (!simCore::areEqual(textNode.getCharacterHeight(), expectedPhysicalSize, 0.01f))
  {
    const unsigned int res = static_cast<unsigned int>(simCore::sdkMax(32.0, static_cast<double>(logicalFontSize_) * currentDpr));
    textNode.setFontResolution(res, res);
    textNode.setCharacterSize(expectedPhysicalSize);
  }
}


}
