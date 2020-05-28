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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cstdlib>
#include "simNotify/Notify.h"
#include "simQt/MonospaceItemDelegate.h"
#include "simQt/ConsoleDataModel.h"
#include "ui_Console.h"
#include "Console.h"

Console::Console(simQt::ConsoleDataModel& dataModel, QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_Console)
{
  ui_->setupUi(this);
  ui_->consoleView->setModel(&dataModel);

  // Set up the monospace font for the text column for correct alignment
  ui_->consoleView->setItemDelegateForColumn(3, new simQt::MonospaceItemDelegate(this));

  // These commands must come after the setModel()
  ui_->consoleView->header()->setStretchLastSection(false);
#if(QT_VERSION >= QT_VERSION_CHECK(5,0,0))
  ui_->consoleView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
  ui_->consoleView->header()->setResizeMode(QHeaderView::ResizeToContents);
#endif

  // Sync the GUI settings to the state of the data model
  ui_->colorTextCheck->setChecked(dataModel.colorizeText());
  ui_->newestOnTopCheck->setChecked(dataModel.newestOnTop());
  ui_->numLinesSpin->setValue(dataModel.numLines());
  ui_->spamSpin->setValue(dataModel.spamFilterTimeout());

  // When GUI options change, update the data model
  connect(ui_->colorTextCheck, SIGNAL(toggled(bool)), &dataModel, SLOT(setColorizeText(bool)));
  connect(ui_->newestOnTopCheck, SIGNAL(toggled(bool)), &dataModel, SLOT(setNewestOnTop(bool)));
  connect(ui_->numLinesSpin, SIGNAL(valueChanged(int)), &dataModel, SLOT(setNumLines(int)));
  connect(ui_->spamSpin, SIGNAL(valueChanged(double)), &dataModel, SLOT(setSpamFilterTimeout(double)));
  connect(ui_->minSeverityCombo, SIGNAL(currentIndexChanged(int)), &dataModel, SLOT(setMinimumSeverity(int)));

  // In a real application, you may need signals to update the GUI if the underlying
  // data model values change through another mechanism (another GUI for example)
  connect(ui_->generateButton, SIGNAL(clicked()), this, SLOT(generateEntries_()));

  // Generate the entries for the minimum severity combo
  populateMinSeverity_();

  // Set up the flood simulation
  connect(ui_->floodRateSpin, SIGNAL(valueChanged(int)), this, SLOT(setFloodRate_(int)));
  setFloodRate_(ui_->floodRateSpin->value());
  connect(ui_->floodTestingCheck, SIGNAL(toggled(bool)), this, SLOT(toggleFloodTimer_(bool)));
  connect(&floodTimer_, SIGNAL(timeout()), this, SLOT(generateEntries_()));
}

Console::~Console()
{
  delete ui_;
}

void Console::generateEntries_()
{
  // Provide a wait cursor, just in case of long operation
  if (!ui_->floodTestingCheck->isChecked())
    QApplication::setOverrideCursor(Qt::WaitCursor);

  // Generate the number of entries based on countSpin's value
  for (int k = 0; k < ui_->countSpin->value(); ++k)
  {
    // Get the severity.  Called in loop (instead of outside) so that the random works
    simNotify::NotifySeverity severity = severity_();

    // "Defeat" flood/spam protection by generating a unique message ID in our output
    SIM_NOTIFY(severity) << "Generated Message " << (k + 1) << "\n";
  }

  // Remove the wait cursor
  if (!ui_->floodTestingCheck->isChecked())
    QApplication::restoreOverrideCursor();
}

simNotify::NotifySeverity Console::severity_() const
{
  switch (ui_->severityCombo->currentIndex())
  {
  case 0: // Info
    return simNotify::NOTIFY_INFO;
  case 1: // Warning
    return simNotify::NOTIFY_WARN;
  case 2: // Error
    return simNotify::NOTIFY_ERROR;
  case 3: // Random
  default:
    break;
  }

  // Generate a random value, which might be below the notification level
  double value0to1 = static_cast<double>(rand()) / RAND_MAX;
  // Cast it to an int value from 0 to max (NOTIFY_DEBUG_FP)
  int intValue = static_cast<int>(value0to1 * static_cast<int>(simNotify::NOTIFY_DEBUG_FP));
  return static_cast<simNotify::NotifySeverity>(intValue);
}

void Console::setFloodRate_(int hz)
{
  ui_->floodRateSpin->setValue(hz);
  floodTimer_.setInterval(1000 / hz);
}

void Console::toggleFloodTimer_(bool turnOn)
{
  if (turnOn)
    floodTimer_.start();
  else
    floodTimer_.stop();
}

void Console::populateMinSeverity_()
{
  for (int k = 0; k <= simNotify::NOTIFY_DEBUG_FP; ++k)
  {
    ui_->minSeverityCombo->addItem(QString::fromStdString(simNotify::severityToString(static_cast<simNotify::NotifySeverity>(k))));
  }
  ui_->minSeverityCombo->setCurrentIndex(simNotify::NOTIFY_INFO);
}
