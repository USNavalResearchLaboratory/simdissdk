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
#include "GogManager.h"

namespace simUtil
{

void GogObject::addGogObjectObserver(GogObjectObserverPtr observer)
{
  observers_.push_back(observer);
}

void GogObject::removeGogObjectObserver(GogObjectObserverPtr observer)
{
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

void GogObject::firePropertyChanged_() const
{
  for (std::vector<GogObjectObserverPtr>::const_iterator iter = observers_.begin(); iter != observers_.end(); ++iter)
  {
    (*iter)->propertyChanged(*this);
  }
}

void GogObject::fireDrawChanged_() const
{
  for (std::vector<GogObjectObserverPtr>::const_iterator iter = observers_.begin(); iter != observers_.end(); ++iter)
  {
    (*iter)->drawChanged(*this);
  }
}

}
