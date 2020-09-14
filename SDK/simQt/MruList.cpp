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
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 */
#include <cassert>
#include <QAction>
#include <QFileInfo>
#include <QMenu>
#include "simQt/MruList.h"

namespace simQt
{

MruList::MruList(int maxFiles, QObject* parent)
  : QObject(parent)
{
  // Assertion failure implies the developer is doing something funny they should protect against.
  assert(maxFiles > 0);

  // Set up the clear button
  clearAction_ = new QAction("Clear Recent Files", this);
  clearAction_->setVisible(false);
  clearAction_->setEnabled(false);
  clearAction_->setStatusTip("Removes all items from the Most Recently Used list.");
  clearAction_->setToolTip(clearAction_->statusTip());
  connect(clearAction_, SIGNAL(triggered()), this, SLOT(clear()));

  // Set up the list of all MRU items (up to maxFiles)
  for (int k = 0; k < maxFiles; ++k)
  {
    QAction* action = new QAction(this);
    action->setVisible(false);
    action->setEnabled(false);
    connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile_()));
    mruActions_.push_back(action);
  }
}

MruList::~MruList()
{
  // separators are owned by the generating menu, not by us
  for (auto it = separators_.begin(); it != separators_.end(); ++it)
    delete *it;
}

int MruList::maximumSize() const
{
  return mruActions_.size();
}

QStringList MruList::files() const
{
  return filenames_;
}

QList<QAction*> MruList::actions() const
{
  return mruActions_;
}

void MruList::addFile(const QString& filename)
{
  QStringList originalFiles = filenames_;
  QString fileFullPath = QFileInfo(filename).absoluteFilePath();

  // Move the entry to the top
  filenames_.removeAll(fileFullPath);
  filenames_.push_front(fileFullPath);
  // Limit the size of QStringList to maximumSize()
  if (filenames_.size() > maximumSize())
    filenames_.erase(filenames_.begin() + maximumSize(), filenames_.end());
  fixActions_();

  // It's possible that the list of files didn't change, if user selected #1 entry.
  // Note that selecting #1 entry is not enough to determine that list didn't change.
  if (originalFiles != filenames_)
    emit(mruListChanged(filenames_));
}

void MruList::setFiles(const QStringList& files)
{
  if (filenames_ == files)
    return;
  filenames_ = files;

  // Limit the size of QStringList to maximumSize()
  if (filenames_.size() > maximumSize())
    filenames_.erase(filenames_.begin() + maximumSize(), filenames_.end());
  fixActions_();
  emit(mruListChanged(filenames_));
}

int MruList::removeFile(const QString& filename)
{
  if (filenames_.removeAll(filename) > 0)
  {
    fixActions_();
    emit(mruListChanged(filenames_));
    return 0;
  }
  return 1;
}

void MruList::clear()
{
  if (filenames_.empty())
    return;
  filenames_.clear();
  fixActions_();
  emit(mruListChanged(filenames_));
}

void MruList::openRecentFile_()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (action != nullptr)
    emit(fileSelected(action->data().toString()));
}

void MruList::fixActions_()
{
  // Assertion failure means that we have more files than actions, which should be protected
  // against in other functions (e.g. addFile()).
  assert(filenames_.size() <= mruActions_.size());
  int numFilenames = qMin(filenames_.size(), mruActions_.size());

  // Set the filename and text for all the good actions
  bool showSeparators = false;
  for (int idx = 0; idx < numFilenames; ++idx)
  {
    QAction* action = mruActions_[idx];
    QString filename = filenames_[idx];
    QFileInfo fi(filename);
    action->setData(filename);
    action->setToolTip(filename);
    action->setStatusTip(filename);

    // Note that actions are disabled so they cannot be activated by hotkey, and
    // hidden because their text name may be incorrect.
    action->setVisible(true);
    action->setEnabled(true);

    // Text looks like: "&3 file.asi"; can only specify shortcuts (&) for items numbered < 10.
    if (idx < 9)
      action->setText(tr("&%1 %2").arg(idx + 1).arg(fi.fileName()));
    else
      action->setText(tr("%1 %2").arg(idx + 1).arg(fi.fileName()));

    // We're showing an action, so we want to show the separators
    showSeparators = true;
  }

  // Hide all the other actions
  for (int idx = numFilenames; idx < mruActions_.size(); ++idx)
  {
    QAction* action = mruActions_[idx];
    action->setVisible(false);
    action->setEnabled(false);
  }

  // Get rid of nullptrs that might be introduced by deleted menus
  separators_.removeAll(nullptr);
  // Hide or show separators based on whether any actions are shown
  for (auto it = separators_.begin(); it != separators_.end(); ++it)
    (*it)->setVisible(showSeparators);

  // Clear action gets the same treatment -- either shown or not shown based on number of entries
  clearAction_->setVisible(showSeparators);
  clearAction_->setEnabled(showSeparators);
}

void MruList::insertMru(QMenu& menu, QAction* beforeAction, bool separatorBefore, bool separatorAfter, bool includeClear)
{
  if (separatorBefore)
    separators_.push_back(menu.insertSeparator(beforeAction));
  menu.insertActions(beforeAction, mruActions_);
  if (includeClear)
    menu.insertAction(beforeAction, clearAction_);
  if (separatorAfter)
    separators_.push_back(menu.insertSeparator(beforeAction));

  // Separators might need to be shown or hidden
  fixActions_();
}

QAction* MruList::clearAction() const
{
  return clearAction_;
}

}
