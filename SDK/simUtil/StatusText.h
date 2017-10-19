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
#ifndef SIMUTIL_STATUSTEXT_H
#define SIMUTIL_STATUSTEXT_H

#include <osg/ref_ptr>
#include "simCore/Common/Common.h"
#include "simCore/String/TextReplacer.h"

namespace simUtil {

class HudManager;
class HudColumnText;
class View;

/**
 * Class that manages status display info overlay on the specified view
 */
class SDKUTIL_EXPORT StatusText : public osg::Referenced
{
public:
  /// Enumeration of positions for status display
  enum Position
  {
    LEFT_BOTTOM,
    LEFT_CENTER,
    LEFT_TOP
  };


  /** Constructs a new StatusText */
  StatusText(simVis::View* view, simCore::TextReplacerPtr textReplacer, StatusText::Position pos=LEFT_BOTTOM);

  /**
  * Display the status as specified by statusSpec
  * @return 0 on success, !0 on error
  */
  int setStatusSpec(const std::string& statusSpec, const osg::Vec4f& color = osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f), double fontSize = 16.0, const std::string& font = "arial.ttf");

  /**
  * Remove the status display
  */
  void removeFromView();

protected:
  /** Destructor */
  virtual ~StatusText();

private:
  class FrameEventHandler;

  /** Build the status text object */
  void create_(const std::string& status, const osg::Vec4f& color, const std::string& font, double fontSize);

  /** Update the existing text object  */
  void update_();

private:
  osg::observer_ptr<simVis::View> view_;        /// Reference to the view in which the status will be displayed
  simUtil::HudManager* hudManager_;                   ///< Reference to the HudManager
  osg::observer_ptr<simUtil::HudColumnText> statusHudText_;   ///< Reference to the HudText that contains the status display
  osg::ref_ptr<FrameEventHandler> updateEventHandler_;  ///< Reference to the update event handler
  simCore::TextReplacerPtr textReplacer_;               ///< Reference to the replacer that processes status specification into status text
  Position position_;                                   ///< Position of status display
  std::string statusSpec_;                              ///< Current status specifier from which status display is generated
};

} // namespace simUtil

#endif // SIMUTIL_STATUSTEXT_H

