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
#ifndef SIMQT_QTDESIGNER_PLUGINS_H
#define SIMQT_QTDESIGNER_PLUGINS_H

#include <QDesignerCustomWidgetCollectionInterface>
#include <QObject>

//  This class collects all the plug-in widgets for the Qt Designer; update constructor with new plug-in widgets
class simQtDesignerPlugins : public QObject, public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
  Q_PLUGIN_METADATA(IID "mil.navy.nrl.simdis.SIMDIS_SDK.simQtDesignerPlugins")

public:
  explicit simQtDesignerPlugins(QObject* parent = nullptr);

public: // QDesignerCustomWidgetCollectionInterface API
  virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
  QList<QDesignerCustomWidgetInterface*> widgetFactories_;
};

#endif
