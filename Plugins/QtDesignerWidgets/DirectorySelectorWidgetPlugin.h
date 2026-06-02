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
#ifndef DIRECTORY_SELECTOR_WIDGET_PLUGIN_H
#define DIRECTORY_SELECTOR_WIDGET_PLUGIN_H

#include <QDesignerCustomWidgetInterface>

// Wrapper class for the DirectoyrSelectorWidget to provide QDesignerCustomWidgetInterface
class DirectorySelectorWidgetPlugin : public QObject, public QDesignerCustomWidgetInterface
{
  Q_OBJECT;
  Q_INTERFACES(QDesignerCustomWidgetInterface);

public:
  explicit DirectorySelectorWidgetPlugin(QObject *parent = nullptr);

  bool isContainer() const override;
  bool isInitialized() const override;
  QIcon icon() const override;
  QString domXml() const override;
  QString group() const override;
  QString includeFile() const override;
  QString name() const override;
  QString toolTip() const override;
  QString whatsThis() const override;
  QWidget *createWidget(QWidget *parent) override;
  void initialize(QDesignerFormEditorInterface *core) override;

private:
  bool initialized_;
};

#endif
