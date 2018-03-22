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
#ifndef CATEGORY_FILTER_WIDGET_PLUGIN_H
#define CATEGORY_FILTER_WIDGET_PLUGIN_H

#include <QDesignerCustomWidgetInterface>

namespace simData { class DataStore; }

// Wrapper class for the FileSelectorWidget to provide QDesignerCustomWidgetInterface
class CategoryFilterWidgetPlugin : public QObject, public QDesignerCustomWidgetInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  explicit CategoryFilterWidgetPlugin(QObject *parent = 0);
  virtual ~CategoryFilterWidgetPlugin();

  bool isContainer() const;
  bool isInitialized() const;
  QIcon icon() const;
  QString domXml() const;
  QString group() const;
  QString includeFile() const;
  QString name() const;
  QString toolTip() const;
  QString whatsThis() const;
  QWidget *createWidget(QWidget *parent);
  void initialize(QDesignerFormEditorInterface *core);

  /** Creates a set of category names and values for testing/display purposes */
  static void createDefaultCategories(simData::DataStore& dataStore);

private:
  simData::DataStore* dataStore_;
};

#endif // CATEGORY_FILTER_WIDGET_PLUGIN_H

