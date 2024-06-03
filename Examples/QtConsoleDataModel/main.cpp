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
#include <iostream>
#include <QApplication>
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simQt/ConsoleDataModel.h"
#include "simQt/ConsoleChannel.h"
#include "simQt/StdStreamConsoleChannel.h"
#include "simQt/ResourceInitializer.h"
#include "Console.h"

namespace
{

int showConsoleWindow(QApplication& app)
{
  // Set up the console data model
  simQt::ConsoleDataModel consoleDataModel;
  // Capture stdout and stderr too, for consistency
  simQt::StdStreamConsoleChannel stdStreamChannel;
  stdStreamChannel.bindTo(consoleDataModel);

  // Send all the SIM_* notify stuff to a console data model channel
  simQt::ChannelNotifyHandler* notifyHandler = new simQt::ChannelNotifyHandler;
  notifyHandler->setChannel(consoleDataModel.registerChannel("Notifications"));
  notifyHandler->setUsePrefix(false); // omit the SIM_* prefix
  // Tell simCore Notify subsystem about the new handler
  simNotify::setNotifyHandlers(simNotify::NotifyHandlerPtr(notifyHandler));
  // Decrease the notification level so we see more messages
  simNotify::setNotifyLevel(simNotify::NOTIFY_INFO);

  // At this point, all stdout and stderr is captured to the console data model,
  // and so is all of the SIM_NOTIFY messages.

  // Print some sample messages, before console is created (as a demonstration)
  SIM_INFO << "Sample SIM_INFO notification\n";
  std::cerr << "Sample cerr statement (writing to stderr)\n";

  // Create a GUI and show the console
  Console console(consoleDataModel);
  console.show();

  return app.exec();
}

}

int main(int argc, char* argv[])
{
  simCore::checkVersionThrow();

  QApplication app(argc, argv);
  simQt::ResourceInitializer::initialize();

  // Dedicate a new scope here so that all of the handlers and background work
  // done for capturing the console is deallocated before the QApplication.
  // Without this, we'd either get some minor warning messages on exit, or we'd
  // need to use dynamic memory and careful deallocation to avoid the warnings.
  return showConsoleWindow(app);
}
