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
#ifndef SIMQT_MRULIST_H
#define SIMQT_MRULIST_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QPointer>
#include "simCore/Common/Export.h"

class QAction;
class QString;
class QMenu;

namespace simQt
{

/**
 * MruList is a collection of most recently used items, associated with a QMenu.
 * When you insertMru() into a menu, it creates a series of actions that represent
 * recently used items (e.g. recent files).  There is a maximum size to the MRU,
 * and newly added files are always added to the top of the list.  Adding an item
 * that already exists will bump it to the top of the list.  When an item is selected
 * from the list, the MruList emits the fileSelected() signal.
 * Note that filenames added with relative paths will be resolved to full path in the MruList,
 * so all filenames returned in getters or emitted in signals will be full path
 */
class SDKQT_EXPORT MruList : public QObject
{
  Q_OBJECT;
public:
  /** Constructor */
  explicit MruList(int maxFiles, QObject* parent=nullptr);
  virtual ~MruList();

  /** Retrieves the maximum number of entries in this list */
  int maximumSize() const;

  /** List of all files in the MRU */
  QStringList files() const;
  /** Ordered list of all MRU actions (0th element is top in list).  Includes actions that may be hidden. */
  QList<QAction*> actions() const;
  /** Action for clearing the list (added in insertMru() if includeClear is true) */
  QAction* clearAction() const;

  /**
   * Inserts the MRU list and optional separators into the menu provided.  Separators
   * added will hide automatically.
   * @param menu Menu into which to add our MRU
   * @param beforeAction Insert MRU before this action; if nullptr, MRU is appended to list
   * @param separatorBefore If true, include a menu separator before the MRU.  Note that the
   *   separator is hidden automatically when the MRU is empty.
   * @param separatorAfter If true, include a menu separator after the MRU.  Note that the
   *   separator is hidden automatically when the MRU is empty.
   * @param includeClear If true, clear button is included; automatically hidden as needed.
   */
  void insertMru(QMenu& menu, QAction* beforeAction, bool separatorBefore=true, bool separatorAfter=true,
    bool includeClear=true);

  /** Returns true if the MRU is enabled. Enabling only impacts the MRU actions that load files. */
  bool isEnabled() const;

public Q_SLOTS:
  /** Sets the list of files. */
  void setFiles(const QStringList& files);
  /** Adds the file to the most recently used list, at the very top.  Reorders list as needed. */
  void addFile(const QString& filename);
  /** Removes the file from the MRU, returning 0 on success and non-zero on error. */
  int removeFile(const QString& filename);
  /** Clears the MRU file list */
  void clear();

  /** Changes the is-enabled field for the MRU, enabling or disabling all load-file actions */
  void setEnabled(bool enabled);

Q_SIGNALS:
  /** Emitted when the user selects one of the recently used item menu items. */
  void fileSelected(const QString& filename);
  /** Emitted when the list of files maintained in the MRU changes. */
  void mruListChanged(const QStringList& files);

private Q_SLOTS:
  /** Called when the user selects on of the MRU actions.  Fires off fileSelected(). */
  void openRecentFile_();
  /** Hides/shows actions and separators, and sets the filenames as appropriate. */
  void fixActions_();

private:
  /** The list of all actions we created for MRU */
  QList<QAction*> mruActions_;
  /** Separators list is not owned (created by the menu they're in), but need removal on destruction */
  QList<QPointer<QAction> > separators_;
  /** Clear button */
  QAction* clearAction_ = nullptr;
  /** Current list of files in the MRU */
  QStringList filenames_;

  /** Flags whether actions are enabled or disabled. */
  bool isEnabled_ = true;
};

}

#endif /* SIMQT_MRULIST_H */
