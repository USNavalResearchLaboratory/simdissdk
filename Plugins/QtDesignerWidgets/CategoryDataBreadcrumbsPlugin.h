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
#ifndef CATEGORY_DATA_BREADCRUMBS_PLUGIN_H
#define CATEGORY_DATA_BREADCRUMBS_PLUGIN_H

#include <QDesignerCustomWidgetInterface>

namespace simData { class DataStore; }

// Wrapper class for the CategoryDataBreadcrumbs to provide QDesignerCustomWidgetInterface
class CategoryDataBreadcrumbsPlugin : public QObject, public QDesignerCustomWidgetInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  explicit CategoryDataBreadcrumbsPlugin(QObject *parent = 0);
  virtual ~CategoryDataBreadcrumbsPlugin();

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

private:
  simData::DataStore* dataStore_;
};

#endif // CATEGORY_DATA_BREADCRUMBS_PLUGIN_H
