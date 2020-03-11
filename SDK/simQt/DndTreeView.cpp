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

#include <QDragEnterEvent>
#include "simQt/DndTreeView.h"

namespace simQt {

DndTreeView::DndTreeView(QWidget* parent)
  : QTreeView(parent)
{
}

DndTreeView::~DndTreeView()
{
}

void DndTreeView::dragEnterEvent(QDragEnterEvent *event)
{
  QTreeView::dragEnterEvent(event);
  event->acceptProposedAction();
}

}

