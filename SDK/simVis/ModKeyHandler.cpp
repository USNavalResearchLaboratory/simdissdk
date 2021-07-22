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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#include "osgGA/GUIEventAdapter"
#include "ModKeyHandler.h"

namespace simVis
{

ModKeyHandler::ModKeyHandler()
{}

ModKeyHandler::~ModKeyHandler()
{}

bool ModKeyHandler::pass(int modKeyMask) const
{
  for (auto iter = modKeys_.begin(); iter != modKeys_.end(); ++iter)
  {
    // check that mod key mask contains left or right mod key
    if (!(modKeyMask & (iter->first | iter->second)))
      return false;
    // remove both left and right mod key masks now that they've been verified, if they exist
    if (iter->first & modKeyMask)
      modKeyMask ^= iter->first;
    if (iter->second & modKeyMask)
      modKeyMask ^= iter->second;
  }
  return modKeyMask == 0;
}

void ModKeyHandler::setModKeys(int modKeyMask)
{
  modKeys_.clear();
  if (modKeyMask & osgGA::GUIEventAdapter::MODKEY_CTRL)
    modKeys_.push_back(std::make_pair(osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL, osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL));
  if (modKeyMask & osgGA::GUIEventAdapter::MODKEY_SHIFT)
    modKeys_.push_back(std::make_pair(osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT, osgGA::GUIEventAdapter::MODKEY_RIGHT_SHIFT));
  if (modKeyMask & osgGA::GUIEventAdapter::MODKEY_ALT)
    modKeys_.push_back(std::make_pair(osgGA::GUIEventAdapter::MODKEY_LEFT_ALT, osgGA::GUIEventAdapter::MODKEY_RIGHT_ALT));
  if (modKeyMask & osgGA::GUIEventAdapter::MODKEY_META)
    modKeys_.push_back(std::make_pair(osgGA::GUIEventAdapter::MODKEY_LEFT_META, osgGA::GUIEventAdapter::MODKEY_RIGHT_META));
  if (modKeyMask & osgGA::GUIEventAdapter::MODKEY_SUPER)
    modKeys_.push_back(std::make_pair(osgGA::GUIEventAdapter::MODKEY_LEFT_SUPER, osgGA::GUIEventAdapter::MODKEY_RIGHT_SUPER));
  if (modKeyMask & osgGA::GUIEventAdapter::MODKEY_HYPER)
    modKeys_.push_back(std::make_pair(osgGA::GUIEventAdapter::MODKEY_LEFT_HYPER, osgGA::GUIEventAdapter::MODKEY_RIGHT_HYPER));
}

}