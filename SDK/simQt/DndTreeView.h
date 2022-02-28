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

#ifndef SIMQT_DND_TREE_VIEW_H
#define SIMQT_DND_TREE_VIEW_H

#include <QTreeView>
#include "simCore/Common/Export.h"

namespace simQt {

/**
  * Wrapper class to circumvent a Drag and Drop bug with QTreeView.
  * For details see: https://bugreports.qt.io/browse/QTBUG-76418
  * and https://bugreports.qt.io/browse/QTBUG-44939
  */
class SDKQT_EXPORT DndTreeView : public QTreeView
{
  Q_OBJECT;

public:
  /** Constructor */
  explicit DndTreeView(QWidget* parent = nullptr);

  /** Destructor */
  virtual ~DndTreeView();

protected:
  /** Override to circumvent a Drag and Drop bug */
  virtual void dragEnterEvent(QDragEnterEvent* event);
};

}

#endif
