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
#ifndef SIMQT_QTUTILS_HH
#define SIMQT_QTUTILS_HH

#include "simCore/Common/Export.h"

class QWidget;

namespace simQt {

/**
 * Provides various utility methods for handling Qt widgets
 */
class SDKQT_EXPORT QtUtils
{
public:

  /**
  * Moves the specified widget so that it is centered on the specified parent widget. If the parent widget's center is not onscreen, will
  * center on the parent's screen. If the parent widget does not exist, will center on the main screen. If centering obscures the widget's
  * top and left area, adjust position to ensure that top and left area are visible.
  * @param widget will move to centered point
  * @param parent to center upon
  */
  static void centerWidgetOnParent(QWidget& widget, const QWidget* parent);

  /**
  * Returns the available geometry for the parent widget's screen if it exists, otherwise returns the available geometry of the main screen.
  * Per Qt documentation, on X11 window manager with more than 1 screen, this may return total geometry rather than available geometry.
  * @param widget required to find screen in some versions of Qt
  * @param parent target screen to find
  */
  static QRect getAvailableScreenGeometry(const QWidget& widget, const QWidget* parent);

};

}

#endif
