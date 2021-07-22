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
#ifndef SIMVIS_MODKEYHANDLER_H
#define SIMVIS_MODKEYHANDLER_H

#include <utility>
#include <vector>
#include "simCore/Common/Export.h"

namespace simVis
{

/**
* Class manages handling mod key masks to account for variations in how the mod key masks for left and right options are
* specified. Some setups provide both the left and right mod keys in the event handler's mod key mask, but others will provide
* only the single activated key, left or right. This class is useful when dealing with multiple mod key combinations, to verify
* that all required mod keys were activated.
*
* For example, if left or right CTRL key is required, set mod key mask to MODKEY_CTRL, which is defined in osgGA/GUIEventAdapter.h as
* (MODKEY_LEFT_CTRL|MODKEY_RIGHT_CTRL). Some systems will pass MODKEY_CTRL in the event handler's mod key mask, but other will simply
* pass MODKEY_LEFT_CTRL or MODKEY_RIGHT_CTRL depending on which actual key was pressed. This class ensures that all three of those
* options will pass when MODKEY_CTRL is set as the required mod keys.
*/
class SDKVIS_EXPORT ModKeyHandler
{
public:
  ModKeyHandler();
  virtual ~ModKeyHandler();

  /**
  * Indicate if the specified mod key mask contains only the required mod keys
  * @param modKeyMask mask of mod keys to test
  * @return true if specified mod key mask contains only the required mod keys
  */
  bool pass(int modKeyMask) const;

  /**
  * Set the mod keys required to pass this handler
  * @param modKeyMask mask of all required mod keys
  */
  void setModKeys(int modKeyMask);

private:
  /// list of all mod keys, left and right components
  std::vector<std::pair<int, int> > modKeys_;
};

}

#endif
