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
#ifndef SIMQT_ENTITY_FILTER_LINE_EDIT_H
#define SIMQT_ENTITY_FILTER_LINE_EDIT_H

#include <QLineEdit>
#include <QActionGroup>
#include <QRegExp>
#include "simCore/Common/Export.h"

namespace simQt {

/// Adds a right mouse click menu to a QLineEdit for selecting filtering options.
class SDKQT_EXPORT EntityFilterLineEdit : public QLineEdit
{
  Q_OBJECT

public:
  /// Constructor takes the parent widget
  EntityFilterLineEdit(QWidget  *parent = 0);
  virtual ~EntityFilterLineEdit();

  /// Set options
  void configure(const QString& filter, Qt::CaseSensitivity caseSensitive, QRegExp::PatternSyntax expression);

  /// Regex-only will set to case insensitive and regex, hiding other options.  Turning off will unhide them.
  void setRegexOnly(bool regexOnly);

signals:
  /// Let the outside know that a filter option has changed.
  void changed(QString filter, Qt::CaseSensitivity caseSensitive, QRegExp::PatternSyntax expression);

public slots:
  /// The text for the filtering was changed by the user
  void textFilterChanged();
  /// The case sensitive option was changed by the user
  void caseSensitive();
  /// The user wants a regular expression filter
  void regularExpression();
  /// The user wants a wildcard filter
  void wildcard();
  /// The user wants a fixed string filter
  void fixedString();

protected:
  /// Displays the right mouse click menu
  virtual void contextMenuEvent(QContextMenuEvent * event);

  Qt::CaseSensitivity caseSensitive_; ///< current case sensitivity
  QRegExp::PatternSyntax expression_; ///< how the pattern is interpreted

  QAction* caseSensitiveAction_; ///< connect GUI to data
  QAction* regularAction_; ///< connect GUI to data
  QAction* wildcardAction_; ///< connect GUI to data
  QAction* fixedAction_; ///< connect GUI to data
  QList<QAction*> standardClickMenu_; ///< standard right-click options
  QMenu* rightMouseClickMenu_; ///< our context menu

  /// Flags that we're in regex-only mode, hiding other options
  bool regexOnly_;
};

}
#endif

