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
#ifndef SIMQT_COLORWIDGETDELEGATE_H
#define SIMQT_COLORWIDGETDELEGATE_H

#include <QStyledItemDelegate>
#include "simCore/Common/Export.h"

namespace simQt {

/**
  * ItemDelegate class which provides the user with a simQt::ColorWidget
  * to edit the color field of a ColorMapping threshold
  */
class SDKQT_EXPORT ColorWidgetDelegate : public QStyledItemDelegate
{
  Q_OBJECT;
public:
  explicit ColorWidgetDelegate(bool showAlpha, QObject* parent = nullptr);

  /** Creates the ColorWidget the user can interact with to change the color */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /** Sets the current value of the ColorWidget based on the selected index's data */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;

  /**
    * Renders the QStyledItemDelegate. We need to override this method so that
    * the ColorWidget is always shown, as opposed to overriding createEditor()
    * which only shows after the user has clicked into the column.
    */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /** User has made a change and now the value needs to be passed to data model */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

  /** Resizes the widget based on the size of cell */
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private slots:
  /** Triggered when editing is finished in the ColorWidget's dialog. Forces the model to update */
  void commitAndCloseEditor_();

private:
  /** Paints the background of the list item; useful as a backdrop for custom drawing. */
  void paintItemBackground_(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /** Determines if the alpha value will be editable in displayed editors */
  bool showAlpha_;
};

}
#endif /* SIMQT_COLORWIDGETDELEGATE_H */
