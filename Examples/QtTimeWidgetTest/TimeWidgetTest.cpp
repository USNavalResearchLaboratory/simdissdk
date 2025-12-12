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
#include <QApplication>
#include <QStyleHints>
#include "simCore/System/Utils.h"
#include "MainWindow.h"

int main(int argc, char* argv[])
{
  simCore::initializeSimdisEnvironmentVariables();
  QApplication app(argc, argv);

  // Force light mode for now until we fully support dark mode
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
  app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
#endif

  MainWindow* window = new MainWindow(nullptr);
  window->show();

  int rv = app.exec();
  delete window;
  return rv;
}

