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
#ifndef SIMQT_GANTTCHARTVIEW_H
#define SIMQT_GANTTCHARTVIEW_H

#include <QAbstractItemView>
#include "simCore/Common/Common.h"

namespace simQt
{

/**
 * Class which constructs a Gantt chart from a Qt item model.
 * Items are drawn using start and end "time" to determine where they start and end horizontally.
 * The units of "time" can be anything as long as all items use the same unit system.
 * Each top level row of the data model is treated as one horizontal level of the Gantt chart.
 * Each child row of those top level rows is an item in that level of the chart.
 * Depending on whether levels are set to collapse, these children will either be drawn each on their own level or all on the same level.
 * Foreground color, toolTip, and icon of each item are taken from the foregroundRole, toolTipRole,
 * and decorationRole of the first column of that item's row.  Column and role of begin and end times
 * can be changed with the set(Begin/End)TimeRole and set(Begin/End)TimeColumn methods, but they must be
 * in the item's row.
 */
class SDKQT_EXPORT GanttChartView : public QAbstractItemView
{
  Q_OBJECT;
  Q_PROPERTY(double referenceLineSpacing READ referenceLineSpacing WRITE setReferenceLineSpacing);
  Q_PROPERTY(bool drawReferenceLines READ drawReferenceLines WRITE setDrawReferenceLines);
  Q_PROPERTY(double iconSize READ iconSize WRITE setIconSize);
  Q_PROPERTY(bool collapseLevels READ collapseLevels WRITE setCollapseLevels);

public:
  /** Constructor */
  explicit GanttChartView(QWidget* parent = NULL);
  /** Destructor */
  virtual ~GanttChartView();

  /** Pure virtuals from QAbstractItemView */
  virtual QModelIndex indexAt(const QPoint &point) const;
  virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
  virtual QRect visualRect(const QModelIndex &index) const;

  /** Zoom factor for increasing draw size of items */
  double zoom() const;
  /** Set zoom factor.  Increasing zoom results in larger items.  Must be greater than or equal to 1.  Multiplies size of items e.g. Zoom of 2 doubles size of all items */
  void setZoom(double newZoom);

  /** Space between dashed vertical background lines.  Same units as time */
  double referenceLineSpacing() const;
  /** Set space between reference lines.  Same units as time */
  void setReferenceLineSpacing(double newSpacing);

  /** Whether to draw vertical reference lines */
  bool drawReferenceLines() const;
  /** Set whether to draw vertical reference lines */
  void setDrawReferenceLines(bool draw);

  /** Size of icons, drawn to the right of the rightmost edges of the items.  Units are pixels across for square icons */
  double iconSize() const;
  /** Set icon size.  Units are pixels across for square icons */
  void setIconSize(double newSize);

  /** Role to use when searching for begin time of each item */
  Qt::ItemDataRole beginTimeRole() const;
  /** Set role to use when searching for begin time of each item */
  void setBeginTimeRole(Qt::ItemDataRole role);

  /** Role to use when searching for end time of each item */
  Qt::ItemDataRole endTimeRole() const;
  /** Set role to use when searching for end time of each item */
  void setEndTimeRole(Qt::ItemDataRole role);

  /** Column to search in for begin time of each item */
  int beginTimeColumn() const;
  /** Set column to search in for begin time of each item */
  void setBeginTimeColumn(int col);

  /** Column to search in for end time of each item */
  int endTimeColumn() const;
  /** Set column to search in for end time of each item */
  void setEndTimeColumn(int col);

  /** Whether to use parent item as level or place each item on its own level.  True for parent item, false for own level */
  bool collapseLevels() const;
  /** Set whether to use parent item as level or place each item on its own level.  True for parent item, false for own level */
  void setCollapseLevels(bool collapse);

  /** Value used to draw the current time indicator line */
  double currentTime() const;
  /** Set time to draw current time indicator at */
  void setCurrentTime(double newTime);

  /** Value to use as start time if bounds are not calculated to fit contents */
  double customStart() const;
  /** Set value to use as start time if bounds are not calculated to fit contents */
  void setCustomStart(double newStart);

  /** Value to use as end time if bounds are not calculated to fit contents */
  double customEnd() const;
  /** Set value to use as end time if bounds are not calculated to fit contents */
  void setCustomEnd(double newEnd);

  /** True if using custom start and end times as bounds, false if bounds are calculated to fit contents */
  bool usingCustomBounds() const;
  /** Set true to use custom start and end times as bounds, false to calculate bounds to fit contents */
  void setUseCustomBounds(bool useCustomBounds);

signals:
  /** Emits value in time of x-coordinate clicked */
  void timeValueAtPositionClicked(double timeValue);

protected slots:
  /** Redraw when data changes */
  virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
  /** Redraw when data changes */
  virtual void rowsInserted(const QModelIndex &parent, int start, int end);
  /** Redraw when data changes */
  virtual void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

protected:
  /** Catch events sent to viewport widget when necessary */
  virtual bool viewportEvent(QEvent* event);
  /** Draw the chart*/
  virtual void paintEvent(QPaintEvent* event);
  /** Emit signal describing item that was double clicked */
  virtual void mouseDoubleClickEvent(QMouseEvent* event);
  /** Emit signals at mouse left click */
  virtual void mousePressEvent(QMouseEvent* event);
  /** Change zoom with mouse wheel */
  virtual void wheelEvent(QWheelEvent* event);
  /** Update horizontal scroll bar on resize */
  virtual void resizeEvent(QResizeEvent* event);

  /** Pure virtuals from QAbstractItemView */
  virtual int horizontalOffset() const;
  virtual bool isIndexHidden(const QModelIndex &index) const;
  virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
  virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags);
  virtual int verticalOffset() const;
  virtual QRegion visualRegionForSelection(const QItemSelection &selection) const;

private:
  /** Update the horizontal scroll bar's range */
  void updateGeometries_();
  /** Update range_ and firstBegin_ */
  void updateEndpoints_();
  /** Check to see if the chart is empty */
  bool isEmpty_() const;

  /** Draws a single item in the gantt chart */
  void drawItem_(int itemLayer, double layerHeight, int indexInLayer, const QModelIndex& parent, QPainter& painter) const;
  /** Draws an arrow indicating an item completely out of bounds before valid range of gantt chart */
  void drawArrowLeft_(int itemLayer, double layerHeight, const QColor& color, QPainter& painter) const;
  /** Draws an arrow indicating an item completely out of bounds after valid range of gantt chart */
  void drawArrowRight_(int itemLayer, double layerHeight, const QColor& color, QPainter& painter) const;

  /// difference between first and last endpoint
  double range_;
  /// first endpoint
  double firstBegin_;
  /// Scale factor when converting from time ranges to item size.  Scales items up or down to fill exact size of viewport horizontally
  double scale_;
  /// Zoom factor.  Default is 1, must be greater than or equal to 1.  Multiplies size of items e.g. Zoom of 2 doubles size of all items
  double zoom_;
  /// Space between reference lines in time units.  Must be positive
  double referenceLineSpacing_;
  /// Whether to draw reference lines
  bool drawReferenceLines_;
  /// Size of icon in pixels across.  Icons are square
  double iconSize_;
  /// Tell if left mouse button has been pressed on this widget
  bool leftMouseDown_;
  /// Column of the data model to search for begin times
  int beginTimeColumn_;
  /// Role to use when searching for begin times
  Qt::ItemDataRole beginTimeRole_;
  /// Column of the data model to search for end times
  int endTimeColumn_;
  /// Role to use when searching for end times
  Qt::ItemDataRole endTimeRole_;
  /// If true, all items with the same parent are drawn on the same level.  If false, items are organized by parent, but drawn on individual levels
  bool collapseLevels_;
  /// Position of vertical line indicating the current time
  double currentTime_;
  /// Value to use as start time if explicitly set bounds are used
  double customStart_;
  /// Value to use as end time if explicitly set bounds are used
  double customEnd_;
  /// Whether bounds should be calculated to fit entries or set explicitly.  False to calculate from entries, true to use explicit bounds
  bool useCustomBounds_;
};

}

#endif //SIMQT_GANTTCHARTWIDGET_H
