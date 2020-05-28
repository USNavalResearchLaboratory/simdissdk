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
#include <QWidget>
#include "simQt/StartupLayoutTask.h"

namespace simQt
{

StartupLayoutTask::StartupLayoutTask()
  : QObject(),
    widget_(NULL)
{
}

StartupLayoutTask::StartupLayoutTask(QObject* receiver, const char* method)
  : QObject(),
    widget_(NULL)
{
  if (receiver != NULL && method != NULL)
    connect(this, SIGNAL(executed()), receiver, method);
}

StartupLayoutTask::~StartupLayoutTask()
{
}

bool StartupLayoutTask::shouldExecuteOnNextStartup() const
{
  return widget_ != NULL && widget_->isVisible();
}

void StartupLayoutTask::execute()
{
  emit(executed());
}

void StartupLayoutTask::setWidget(QWidget* widget)
{
  if (widget_ == widget)
    return;
  clearWidget();
  widget_ = widget;
  // Tie into the destroyed() signal so we never get into an invalid state
  if (widget_ != NULL)
  {
    connect(widget_, SIGNAL(destroyed()), this, SLOT(clearWidget()));
  }
}

void StartupLayoutTask::clearWidget()
{
  if (widget_ != NULL)
    disconnect(widget_, SIGNAL(destroyed()), this, SLOT(clearWidget()));
  widget_ = NULL;
}

}
