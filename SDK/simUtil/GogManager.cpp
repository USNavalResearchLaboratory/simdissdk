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
 *               EW Modeling and Simulation, Code 5770
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * For more information please send email to simdis@enews.nrl.navy.mil
 *
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * U.S. Naval Research Laboratory.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
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
