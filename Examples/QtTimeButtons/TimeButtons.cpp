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
#include "simCore/Common/Version.h"
#include "simQt/ResourceInitializer.h"
#include "simUtil/ExampleResources.h"
#include "MainWindow.h"

/// Example linking the time buttons Qt widget to the scenario

int main(int argc, char **argv)
{
  simCore::checkVersionThrow();

  QApplication app(argc, argv);

  // set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  simQt::ResourceInitializer::initialize();

  MainWindow *win = new MainWindow;
  win->show();

  return app.exec();
}

