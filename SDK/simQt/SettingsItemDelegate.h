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
#ifndef SIMQT_SETTINGSITEMDELEGATE_H
#define SIMQT_SETTINGSITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QValidator>
#include "simCore/Common/Export.h"

namespace simQt
{

/** Delegate helper explicitly used for COLOR items in simQt::Settings */
class SDKQT_EXPORT SettingsColorItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT;
public:
  /** Constructor */
  SettingsColorItemDelegate(QObject* parent = nullptr);

  /** Paints a box with the color against a black and a white background. */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Creates a color editor window */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Sets the color editor window's color data */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  /** Updates the data model provided with the editor's data */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  /** Update the editor's geometry */
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private slots:
  /** Called on acceptance of the color GUI */
  void commitAndCloseEditor_();
  /** Called on rejection of the color GUI */
  void cancelEditor_();

private:
  /** Gets a color from the data model provided (by the index) using the role provided */
  QColor getColor_(const QModelIndex& index, int role=Qt::DisplayRole) const;
  /** Paints the background of the list item; useful as a backdrop for custom drawing. */
  void paintItemBackground_(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Paints the colored square onto the list item */
  void paintColoredSquare_(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

/** Delegate helper explicitly used for DIRECTORY items in simQt::Settings */
class SDKQT_EXPORT SettingsDirectorySelectorDelegate : public QStyledItemDelegate
{
  Q_OBJECT;
public:
  /** Constructor */
  SettingsDirectorySelectorDelegate(QObject* parent = nullptr);
  virtual ~SettingsDirectorySelectorDelegate();

  /** Creates a color editor window */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Sets the color editor window's color data */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  /** Updates the data model provided with the editor's data */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  /** Update the editor's geometry */
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private slots:
  /** Called on acceptance of the pop up file GUI */
  void commitEditor_();
};

/** Delegate helper for INTEGER items, using a spin box as the text editor */
class SDKQT_EXPORT SettingsIntegerSpinBoxDelegate : public QStyledItemDelegate
{
public:
  /** Constructor */
  SettingsIntegerSpinBoxDelegate(QObject* parent = nullptr);

  /** Creates an integer spin box editor window */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Sets the integer spin box editor window's numeric data */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  /** Updates the data model provided with the editor's data */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
};

/** Delegate helper for DOUBLE items, using a QDoubleSpinBox as the text editor */
class SDKQT_EXPORT SettingsDoubleSpinBoxDelegate : public QStyledItemDelegate
{
 public:
   /** Constructor */
  SettingsDoubleSpinBoxDelegate(QObject* parent = nullptr);

  /** Creates a double spin box editor window */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Sets the double spin box editor window's numeric data */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  /** Updates the data model provided with the editor's data */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
};

/** Delegate helper for FILENAME items, using a FileSelectorWidget as the text editor */
class SDKQT_EXPORT SettingsFileSelectorDelegate : public QStyledItemDelegate
{
  Q_OBJECT;
 public:
   /** Constructor */
  SettingsFileSelectorDelegate(QObject* parent = nullptr);
  virtual ~SettingsFileSelectorDelegate();

  /** Creates a file selector editor window */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Sets the file selector editor window's filename data */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  /** Updates the data model provided with the editor's data */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  /** Update the editor's geometry */
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private slots:
  /** Called on acceptance of the pop up file GUI */
  void commitEditor_();
};

/** Delegate helper for ENUMERATION items, using a QComboBox as the text editor */
class SDKQT_EXPORT SettingsEnumerationDelegate : public QStyledItemDelegate
{
  Q_OBJECT;
 public:
   /** Constructor */
  SettingsEnumerationDelegate(QObject* parent = nullptr);

  /** Creates a file selector editor window */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Sets the file selector editor window's combo box data */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  /** Updates the data model provided with the editor's data */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  /** Update the editor's geometry */
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Override paint() to correct the text display. */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

/** Delegate helper for FONT items, using a FontWidget as the text editor. Only provides option to edit font file */
class SDKQT_EXPORT SettingsFontSelectorDelegate : public QStyledItemDelegate
{
  Q_OBJECT;
 public:
   /** Constructor */
  SettingsFontSelectorDelegate(QObject* parent = nullptr);
  virtual ~SettingsFontSelectorDelegate();

  /** Creates a file selector editor window */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Sets the file selector editor window's filename data */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  /** Updates the data model provided with the editor's data */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  /** Update the editor's geometry */
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private slots:
  /** Called on acceptance of the pop up file GUI */
  void commitEditor_();
};

/** Delegate helper for QFONT items, using a QFontDialog as the editor */
class SDKQT_EXPORT SettingsQFontSelectorDelegate : public QStyledItemDelegate
{
  Q_OBJECT;
public:
  /** Constructor */
  SettingsQFontSelectorDelegate(QObject* parent = nullptr);
  virtual ~SettingsQFontSelectorDelegate();

  /** Creates a file selector editor window */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Sets the file selector editor window's data */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  /** Updates the data model provided with the editor's data */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  /** Update the editor's geometry */
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private slots:
  /** Called on acceptance of the GUI */
  void commitAndCloseEditor_();
  /** Called on rejection of the GUI */
  void cancelEditor_();
};

/** Delegate helper for HEX items, using a line edit as the text editor */
class SDKQT_EXPORT SettingsHexEditDelegate : public QStyledItemDelegate
{
public:
  /** Constructor */
  SettingsHexEditDelegate(QObject* parent = nullptr);

  /** Creates an hex formatted line edit editor window */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Sets the line edit's hex data */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  /** Updates the data model provided with the editor's data */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  /** Override paint() to correct the text display. */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
  int maxCharCount_(const QModelIndex& index) const;
};

/** Validator for the SettingsHexEditDelegate line edit */
class SettingsHexEditValidator : public QValidator
{
public:
  SettingsHexEditValidator(unsigned int min, unsigned int max, QObject* parent=nullptr);
  virtual ~SettingsHexEditValidator();
  virtual void fixup(QString& input) const;
  virtual QValidator::State validate(QString& input, int& pos) const;
private:
  unsigned int min_;
  unsigned int max_;
};

/** Generic multiplexer for simQt::Settings data display and editing. */
class SDKQT_EXPORT SettingsItemDelegate : public QStyledItemDelegate
{
public:
  /** Constructor */
  SettingsItemDelegate(QObject* parent = nullptr);
  virtual ~SettingsItemDelegate();

  /** Overrides QStyledItemDelegate::paint() to delegate to proper data type delegate. */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Overrides QStyledItemDelegate::createEditor() to delegate to proper data type delegate. */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Overrides QStyledItemDelegate::setEditorData() to delegate to proper data type delegate. */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  /** Overrides QStyledItemDelegate::setModelData() to delegate to proper data type delegate. */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  /** Overrides QStyledItemDelegate::updateEditorGeometry() to delegate to proper data type delegate. */
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /** Overrides QStyledItemDelegate::eventFilter() to ignore window-hiding as a cue for saving data */
  virtual bool eventFilter(QObject *object, QEvent *event);

private:
  /** Returns the appropriate delegate, or nullptr if none */
  const QStyledItemDelegate* findDelegate_(const QModelIndex& index) const;

  /** Delegate for COLOR values */
  SettingsColorItemDelegate colorDelegate_;
  /** Delegate for INTEGER values */
  SettingsIntegerSpinBoxDelegate integerDelegate_;
  /** Delegate for DOUBLE values */
  SettingsDoubleSpinBoxDelegate doubleDelegate_;
  /** Delegate for FILENAME values */
  SettingsFileSelectorDelegate filenameDelegate_;
  /** */
  SettingsDirectorySelectorDelegate directoryDelegate_;
  /** Delegate for ENUMERATION values */
  SettingsEnumerationDelegate enumerationDelegate_;
  /** Delegate for FONT values */
  SettingsFontSelectorDelegate fontDelegate_;
  /** Delegate for QFONT values */
  SettingsQFontSelectorDelegate qFontDelegate_;
  /** Delegate for HEX values */
  SettingsHexEditDelegate hexDelegate_;
};

}

#endif /* SIMQT_SETTINGSITEMDELEGATE_H */
