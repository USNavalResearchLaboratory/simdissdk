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
#include <algorithm>
#include <cassert>
#include <limits>
#include <QHelpEvent>
#include <QToolTip>
#include <QPainter>
#include <QScrollBar>

#include "GanttChartView.h"

/// Amount of space between bar and icon
const double ICON_MARGIN = 5;

/// Amount to change box color by when drawing borders
const double LIGHT_FACTOR = 300;
const double DARK_FACTOR = 400;

namespace simQt
{

GanttChartView::GanttChartView(QWidget* parent)
  : QAbstractItemView(parent),
    range_(0),
    firstBegin_(std::numeric_limits<double>::max()),
    scale_(1),
    zoom_(1),
    referenceLineSpacing_(100),
    drawReferenceLines_(true),
    iconSize_(8),
    leftMouseDown_(false),
    beginTimeColumn_(1),
    beginTimeRole_(Qt::DisplayRole),
    endTimeColumn_(2),
    endTimeRole_(Qt::DisplayRole),
    collapseLevels_(false),
    currentTime_(-std::numeric_limits<double>::max())
{
}

GanttChartView::~GanttChartView()
{
}

QModelIndex GanttChartView::indexAt(const QPoint &point) const
{
  if (!model())
    return QModelIndex();

  int numLayers = 0;
  if (collapseLevels_)
    numLayers = model()->rowCount(rootIndex());
  else
  {
    for (int layer = 0; layer < model()->rowCount(rootIndex()); layer++)
    {
      numLayers += model()->rowCount(model()->index(layer, 0, rootIndex()));
    }
  }

  int itemHeight = (numLayers != 0) ? (viewport()->height() / numLayers) : 0;
  int itemNum = 0;
  int layer = 0;

  for (int parent = 0; parent < numLayers; parent++)
  {
    QModelIndex layerIndex = model()->index(parent, 0, rootIndex());

    if (collapseLevels_)
      layer = parent;

    for (int itemInLayer = 0; itemInLayer < model()->rowCount(layerIndex); itemInLayer++)
    {
      if (!collapseLevels_)
      {
        layer = itemNum;
        itemNum++;
      }

      QModelIndex beginIndex = model()->index(itemInLayer, beginTimeColumn_, layerIndex);
      double begin = model()->data(beginIndex, beginTimeRole_).toDouble(0);

      QModelIndex endIndex = model()->index(itemInLayer, endTimeColumn_, layerIndex);
      double end = model()->data(endIndex, endTimeRole_).toDouble(0);

      // Handle cases where the beginning is after the end
      if (begin > end)
      {
        double temp = begin;
        begin = end;
        end = temp;
      }

      QRect rect(((begin - firstBegin_) * (scale_ * zoom_)) - horizontalScrollBar()->value(), itemHeight * layer, (end - begin) * (scale_ * zoom_), itemHeight);
      if (rect.contains(point))
      {
        return model()->index(itemInLayer, 0, layerIndex);
      }
    }
  }
  return QModelIndex();
}

void GanttChartView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
  // Not implemented
}

QRect GanttChartView::visualRect(const QModelIndex &index) const
{
  // Not implemented
  return QRect();
}

double GanttChartView::zoom() const
{
  return zoom_;
}

void GanttChartView::setZoom(double newZoom)
{
  // Don't accept zoom less than 1, don't bother updating if zoom is the same
  if (newZoom <= 0 || newZoom == zoom_)
    return;

  zoom_ = newZoom;
  return;

  viewport()->update();
}

double GanttChartView::referenceLineSpacing() const
{
  return referenceLineSpacing_;
}

void GanttChartView::setReferenceLineSpacing(double newSpacing)
{
  if (newSpacing == referenceLineSpacing_)
    return;

  referenceLineSpacing_ = (newSpacing > 0) ? newSpacing : referenceLineSpacing_;

  viewport()->update();
}

bool GanttChartView::drawReferenceLines() const
{
  return drawReferenceLines_;
}

void GanttChartView::setDrawReferenceLines(bool draw)
{
  if (draw == drawReferenceLines_)
    return;

  drawReferenceLines_ = draw;

  viewport()->update();
}

double GanttChartView::iconSize() const
{
  return iconSize_;
}

void GanttChartView::setIconSize(double newSize)
{
  if (iconSize_ == newSize)
    return;

  iconSize_ = newSize;

  viewport()->update();
}

Qt::ItemDataRole GanttChartView::beginTimeRole() const
{
  return beginTimeRole_;
}

void GanttChartView::setBeginTimeRole(Qt::ItemDataRole role)
{
  if (beginTimeRole_ == role)
    return;

  beginTimeRole_ = role;

  viewport()->update();
}

Qt::ItemDataRole GanttChartView::endTimeRole() const
{
  return endTimeRole_;
}

void GanttChartView::setEndTimeRole(Qt::ItemDataRole role)
{
  if (endTimeRole_ == role)
    return;

  endTimeRole_ = role;

  viewport()->update();
}

int GanttChartView::beginTimeColumn() const
{
  return beginTimeColumn_;
}

void GanttChartView::setBeginTimeColumn(int col)
{
  if (beginTimeColumn_ == col)
    return;

  beginTimeColumn_ = col;

  viewport()->update();
}

int GanttChartView::endTimeColumn() const
{
  return endTimeColumn_;
}

void GanttChartView::setEndTimeColumn(int col)
{
  if (endTimeColumn_ == col)
    return;

  endTimeColumn_ = col;

  viewport()->update();
}

bool GanttChartView::collapseLevels() const
{
  return collapseLevels_;
}

void GanttChartView::setCollapseLevels(bool collapse)
{
  if (collapseLevels_ == collapse)
    return;
  collapseLevels_ = collapse;
  viewport()->update();
}

double GanttChartView::currentTime() const
{
  return currentTime_;
}

void GanttChartView::setCurrentTime(double newTime)
{
  if (currentTime_ == newTime)
    return;
  currentTime_ = newTime;
  viewport()->update();
}

void GanttChartView::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
  viewport()->update();
}

bool GanttChartView::viewportEvent(QEvent *event)
{
  // Don't even try if we have no model
  if (!model())
    return QAbstractScrollArea::viewportEvent(event);

  if (event->type() == QEvent::ToolTip)
  {
    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
    QModelIndex indx = indexAt(helpEvent->pos());

    QToolTip::showText(helpEvent->globalPos(), model()->data(indx, Qt::ToolTipRole).toString());
    return true;
  }
  if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)
  {
    mousePressEvent(static_cast<QMouseEvent*>(event));
    return true;
  }

  if (event->type() == QEvent::Wheel)
  {
    wheelEvent(static_cast<QWheelEvent*>(event));
    return true;
  }
  if (event->type() == QEvent::Resize)
  {
    resizeEvent(static_cast<QResizeEvent*>(event));
    return true;
  }

  return QAbstractScrollArea::viewportEvent(event);
}

void GanttChartView::paintEvent(QPaintEvent* event)
{
  updateGeometries_();

  QPainter painter(viewport());
  // Move painter's coordinate system relative to the viewport
  painter.translate(-horizontalScrollBar()->value(), 0);

  // Draw reference lines
  painter.setPen(Qt::DashLine);

  const double spacingPixels = (referenceLineSpacing_ * (scale_ * zoom_));

  // If spacingPixels <= 1, the reference lines are drawn on every pixel of the viewport.  This defeats the purpose.
  if (drawReferenceLines_ && spacingPixels > 1)
  {
    for (double x = 0; x < viewport()->width(); x += spacingPixels)
    {
      painter.drawLine(x, 0, x, viewport()->height() - 1);
    }
  }

  if (!model())
    return;

  int numLayers = 0;
  if (collapseLevels_)
    numLayers = model()->rowCount(rootIndex());
  else
  {
    for (int layer = 0; layer < model()->rowCount(rootIndex()); layer++)
    {
      numLayers += model()->rowCount(model()->index(layer, 0, rootIndex()));
    }
  }

  int itemHeight = (numLayers != 0) ? (viewport()->height() / numLayers) : 0;
  int itemNum = 0;
  int layer = 0;

  painter.setPen(Qt::SolidLine);
  // Each child of the root item
  for (int parent = 0; parent < model()->rowCount(rootIndex()); parent++)
  {
    QModelIndex parentIndex = model()->index(parent, 0, rootIndex());

    if (collapseLevels_)
      layer = parent;

    // Each item in the row
    for (int itemInLayer = 0; itemInLayer < model()->rowCount(parentIndex); itemInLayer++)
    {
      if (!collapseLevels_)
      {
        layer = itemNum;
        itemNum++;
      }

      QModelIndex itemIndex = model()->index(itemInLayer, 0, parentIndex);
      QColor color = model()->data(itemIndex, Qt::ForegroundRole).value<QColor>();
      QIcon icon = model()->data(itemIndex, Qt::DecorationRole).value<QIcon>();

      QModelIndex beginIndex = model()->index(itemInLayer, beginTimeColumn_, parentIndex);
      double begin = model()->data(beginIndex, beginTimeRole_).toDouble(0);

      QModelIndex endIndex = model()->index(itemInLayer, endTimeColumn_, parentIndex);
      double end = model()->data(endIndex, endTimeRole_).toDouble(0);

      // Handle cases where the beginning is after the end
      if (begin > end)
      {
        std::swap(begin, end);
      }

      painter.fillRect((begin - firstBegin_) * (scale_ * zoom_), itemHeight * layer, (end - begin) * (scale_ * zoom_), itemHeight, color);

      // Draw a border to give depth
      painter.setPen(color.lighter(LIGHT_FACTOR));
      painter.drawLine((begin - firstBegin_) * (scale_ * zoom_), itemHeight * layer, (begin - firstBegin_) * (scale_ * zoom_), itemHeight * (layer + 1));
      painter.drawLine((begin - firstBegin_) * (scale_ * zoom_), itemHeight * layer, (end - firstBegin_) * (scale_ * zoom_), itemHeight * layer);

      painter.setPen(color.darker(DARK_FACTOR));
      painter.drawLine((end - firstBegin_) * (scale_ * zoom_), itemHeight * layer, (end - firstBegin_) * (scale_ * zoom_), itemHeight * (layer + 1));
      painter.drawLine((begin - firstBegin_) * (scale_ * zoom_), itemHeight * (layer + 1), (end - firstBegin_) * (scale_ * zoom_), itemHeight * (layer + 1));

      // Draw the icon to the right of the item
      double centerY = ((itemHeight * layer) + itemHeight / 2);

      icon.paint(&painter, QRect((end - firstBegin_) * (scale_ * zoom_) + ICON_MARGIN, centerY - (iconSize_ / 2), iconSize_, iconSize_));
    }
  }

  double currTimeLineX = (currentTime_ - firstBegin_) * (scale_ * zoom_);
  painter.drawLine(currTimeLineX, 0, currTimeLineX, viewport()->height() - 1);
}

void GanttChartView::mouseDoubleClickEvent(QMouseEvent* event)
{
  QModelIndex index = indexAt(event->pos());
  if (index != QModelIndex())
    emit(doubleClicked(index));
}

void GanttChartView::mousePressEvent(QMouseEvent* event)
{
  if ((event->buttons() & Qt::LeftButton) == 0)
  {
    if (leftMouseDown_)
    {
      leftMouseDown_ = false;

      if (!isEmpty_())
        return;

      emit clicked(indexAt(event->pos()));

      // Zero scale not allowed and will crash here
      assert((scale_ * zoom_) != 0);

      double timeAtCursor = firstBegin_ + (horizontalScrollBar()->value() + event->pos().x()) / (scale_ * zoom_);
      emit timeValueAtPositionClicked(timeAtCursor);
      viewport()->update();
    }
  }
  else
    leftMouseDown_ = true;
}

void GanttChartView::wheelEvent(QWheelEvent * event)
{
  // event->delta returns distance rotated in eighths of a degree
  double numDegrees = event->delta() / 8;
  // Qt documentation reports that most mice wheels use steps of 15 degrees: http://doc.qt.io/qt-4.8/qwheelevent.html#delta
  double numSteps = numDegrees / 15;

  // scale_ > 0 enforced in updateEndpoints(), zoom_ >= enforced in setZoom()
  assert(scale_ > 0 && zoom_ >= 1);

  double timeAtCursor = firstBegin_ + (horizontalScrollBar()->value() + event->pos().x()) / (scale_ * zoom_);
  if (numSteps < 0)
  {
    for (int i = 0; i > numSteps; i--)
    {
      zoom_ = zoom_ * .8;
    }
  }
  else
  {
    for (int i = 0; i < numSteps; i++)
    {
      zoom_ = zoom_ * 1.25;
    }
  }

  // Don't allow zoom less than 1.  Default to 1
  if (zoom_ <= 1)
    zoom_ = 1;

  updateGeometries_();

  // Keep the point under the cursor as close to the cursor as possible
  horizontalScrollBar()->setValue(((timeAtCursor - firstBegin_) * (scale_ * zoom_)) - event->pos().x());

  viewport()->update();
}

void GanttChartView::resizeEvent(QResizeEvent* event)
{
  updateGeometries_();
}

int GanttChartView::horizontalOffset() const
{
  return horizontalScrollBar()->value();
}

bool GanttChartView::isIndexHidden(const QModelIndex &index) const
{
  // Not implemented
  return false;
}

QModelIndex GanttChartView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
  // Not implemented
  return QModelIndex();
}

void GanttChartView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
  // Not implemented
}

int GanttChartView::verticalOffset() const
{
  // Not implemented
  return 0;
}

QRegion GanttChartView::visualRegionForSelection(const QItemSelection &selection) const
{
  // Not implemented
  return QRegion();
}

void GanttChartView::updateGeometries_()
{
  updateEndpoints_();

  // Full size of chart after scaling to fit the viewport, then zooming in
  double fullChartSize = range_ * scale_ * zoom_;

  horizontalScrollBar()->setPageStep(viewport()->width());
  horizontalScrollBar()->setRange(0, std::max(0.0, fullChartSize - viewport()->width()));
}

void GanttChartView::updateEndpoints_()
{
  firstBegin_ = std::numeric_limits<double>::max();
  double lastEnd = -std::numeric_limits<double>::max();
  int numLayers = 0;

  // If model is null, the for loop is skipped and we still get a reasonable default value for scale_
  if (model())
    numLayers = model()->rowCount(rootIndex());

  // Determine the bound of start and end points
  for (int layer = 0; layer < numLayers; layer++)
  {
    QModelIndex layerIndex = model()->index(layer, 0, rootIndex());

    for (int itemInLayer = 0; itemInLayer < model()->rowCount(layerIndex); itemInLayer++)
    {
      QModelIndex beginIndex = model()->index(itemInLayer, 1, layerIndex);
      double begin = model()->data(beginIndex, Qt::DisplayRole).toDouble(0);

      QModelIndex endIndex = model()->index(itemInLayer, 2, layerIndex);
      double end = model()->data(endIndex, Qt::DisplayRole).toDouble(0);

      // Handle cases where the beginning is after the end
      if (begin > end)
      {
        std::swap(begin, end);
      }

      firstBegin_ = (firstBegin_ > begin) ? begin : firstBegin_;
      lastEnd = (lastEnd < end) ? end : lastEnd;
    }
  }

  range_ = lastEnd - firstBegin_;
  if (range_ != 0 && firstBegin_ != std::numeric_limits<double>::max() && lastEnd != -std::numeric_limits<double>::max())
    scale_ = viewport()->width() / range_;
  else
    scale_ = 1;
}

bool GanttChartView::isEmpty_() const
{
  if (!model())
    return false;

  for (int i = 0; i < model()->rowCount(); i++)
  {
    QModelIndex layerIndex = model()->index(i, 0, rootIndex());

    if (model()->hasChildren(layerIndex))
    {
      return true;
    }
  }

  return false;
}

}
