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
#ifndef SIMQT_CATEGORYDATABREADCRUMBS_H
#define SIMQT_CATEGORYDATABREADCRUMBS_H

#include <map>
#include <QPen>
#include <QStyledItemDelegate>
#include <QWidget>
#include "simCore/Common/Common.h"

class QListWidget;
class QListWidgetItem;
namespace simData {
  class CategoryFilter;
  class CategoryNameManager;
}

namespace simQt {

/**
 * Styled item delegate used by CategoryDataBreadcrumbs that draws a list item using a rounded
 * rectangle and a close button.  When the user clicks on the close button, the delegate will
 * emit the closeClicked() signal.  Settings for the visualization are controlled by the nested
 * Style structure.  For close behavior to correctly work, the hosting QListView needs to
 * have mouse tracking enabled with setMouseTracking(true).
 */
class SDKQT_EXPORT CloseableItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT;
public:
  /** Contains various style settings for what the item looks like. */
  struct Style
  {
    /// Radius values for rounded rectangle drawing
    qreal rectangleRadiusX;
    qreal rectangleRadiusY;

    /// Pen and colors for the rectangle and its outline
    QPen outlinePen;
    QColor fillColor;
    QColor textColor;

    /// Margin around the drawn part of the item for consecutively placed items
    QMargins itemMargins;
    /// Inside padding around the textual region
    QMargins textPadding;
    /// Inside padding around the icon
    QMargins iconPadding;
    /// Icon and target size
    QIcon icon;
    QSize iconSize;

    Style();
  };

  /** Constructor and destructor */
  explicit CloseableItemDelegate(QObject* parent=NULL);
  virtual ~CloseableItemDelegate();

  /** Overrides from QStyledItemDelegate */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual bool editorEvent(QEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);

  /** Non-const accessor to the style */
  Style& style();
  /** Const accessor to the style */
  const Style& style() const;

signals:
  /** End user clicked on the close button for the given index */
  void closeClicked(const QModelIndex& index);

private:
  /** Calculates the drawn rectangle area for the close icon */
  QRectF calcIconRect_(const QStyleOptionViewItem& option) const;

  /** Drawing style */
  Style style_;
  /** Is true when the mouse is detected over the Close button */
  bool hovering_;
};

/**
 * Widget that will display the contents of a category filter in an easy-to-understand
 * flow similar to a breadcrumb display on a website.  Each contributing factor to the
 * filter is displayed as an item (a breadcrumb) along the horizontal.  Each breadcrumb
 * can be removed individually, causing the filter to be edited.  There is no guarantee
 * on the order of the breadcrumbs.
 *
 * The breadcrumb is intended primarily to be a horizontal widget.  It will wrap extra
 * filter entries on new lines, like a word-wrapped label will do.  The height of the
 * widget is automatically adjusted for the width.  For example:
 *
 * <pre>
 *  [ Friendly  X ]  [ Hostile  X ]  [ Red Force  X ]  [ Blue Force  X ] [ Remote  X ]
 *  [ Local  X ]
 * </pre>
 *
 * Use the setFilter() method to initialize the display to a particular filter.  When the
 * user edits the filter by removing breadcrumbs, the updated filter is emitted using
 * the filterEdited() signal.  The current state can be retrieved with getFilter().
 */
class SDKQT_EXPORT CategoryDataBreadcrumbs : public QWidget
{
  Q_OBJECT;
  Q_PROPERTY(qreal rectangleRadiusX READ rectangleRadiusX WRITE setRectangleRadiusX);
  Q_PROPERTY(qreal rectangleRadiusY READ rectangleRadiusY WRITE setRectangleRadiusY);
  Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor);
  Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor);
  Q_PROPERTY(QColor outlineColor READ outlineColor WRITE setOutlineColor);
  Q_PROPERTY(qreal outlineWidth READ outlineWidth WRITE setOutlineWidth);
  Q_PROPERTY(QMargins itemMargins READ itemMargins WRITE setItemMargins);
  Q_PROPERTY(QMargins textPadding READ textPadding WRITE setTextPadding);
  Q_PROPERTY(QMargins iconPadding READ iconPadding WRITE setIconPadding);
  Q_PROPERTY(QIcon closeIcon READ closeIcon WRITE setCloseIcon);
  Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize);
  Q_PROPERTY(int minimumGroupSize READ minimumGroupSize WRITE setMinimumGroupSize);
  Q_PROPERTY(QString noActiveFilterText READ noActiveFilterText WRITE setNoActiveFilterText);

public:
  explicit CategoryDataBreadcrumbs(QWidget* parent=NULL);
  virtual ~CategoryDataBreadcrumbs();

  /** Override to return a reasonable minimum height based on content */
  virtual QSize minimumSizeHint() const;
  virtual QSize sizeHint() const;

  /** Retrieve the current filter.  Note that this is a simplified filter and may not match setFilter()'s values unsimplified. */
  void getFilter(simData::CategoryFilter& filter);
  /** Clears the current filter. */
  void clearFilter();

  // Property accessors
  qreal rectangleRadiusX() const;
  qreal rectangleRadiusY() const;
  QColor fillColor() const;
  QColor textColor() const;
  QPen outlinePen() const;
  QColor outlineColor() const;
  qreal outlineWidth() const;
  QMargins itemMargins() const;
  QMargins textPadding() const;
  QMargins iconPadding() const;
  QIcon closeIcon() const;
  QSize iconSize() const;
  int minimumGroupSize() const;
  QString noActiveFilterText() const;
  void setRectangleRadiusX(qreal value);
  void setRectangleRadiusY(qreal value);
  void setFillColor(const QColor& value);
  void setTextColor(const QColor& value);
  void setOutlinePen(const QPen& value);
  void setOutlineColor(const QColor& value);
  void setOutlineWidth(qreal value);
  void setItemMargins(const QMargins& value);
  void setTextPadding(const QMargins& value);
  void setIconPadding(const QMargins& value);
  void setCloseIcon(const QIcon& value);
  void setIconSize(const QSize& value);
  void setMinimumGroupSize(int value);
  void setNoActiveFilterText(const QString& value);

public slots:
  /** Changes the current filter. */
  void setFilter(const simData::CategoryFilter& filter);

signals:
  /** End user changed the filter.  Note that this is a simplified filter and may not exactly match input. */
  void filterEdited(const simData::CategoryFilter& filter);

private slots:
  /** Responds to the QListWidget item having a close button clicked */
  void removeFilter_(const QModelIndex& index);

protected:
  /** Override resize event to recalculate the flow and adjust minimum height */
  virtual void resizeEvent(QResizeEvent* evt);

private:
  /** Adds items to the breadcrumb list for the given name */
  void addNameToList_(int nameIndex);
  /** Clears and redraws the entire list to reflect the state of the filter. */
  void rebuildList_();
  /** If the list is empty, adds the no-valid-item item */
  void addNoValidItemIfEmptyList_();

  /** Convenience method to append a list widget item for the given category */
  QListWidgetItem* addNameItem_(const QString& categoryName, int nameInt);
  /** Convenience method to append a list widget item for a specific value in a given category */
  QListWidgetItem* addValueItem_(const QString& text, const QString& name, int nameInt, int valueInt, bool isChecked);

  /** Creates a list for tooltips that uses <li> items for each check in the list */
  QString buildValuesHtmlList_(const simData::CategoryNameManager& nameManager, const std::map<int, bool>& checks, size_t maxItems) const;

  /** Widget that displays the filters */
  QListWidget* listWidget_;
  /** Delegate that draws the filter items */
  CloseableItemDelegate* itemDelegate_;
  /** Delegate to use when the list has no items (default delegate) */
  QStyledItemDelegate* plainDelegate_;
  /** Copy of the current filter */
  simData::CategoryFilter* filter_;

  /** Minimum number of items in a category before grouping */
  int minimumGroupSize_;
  /** Text to display when there are no items in the list */
  QString noActiveFilterText_;

  // Size hints calculated in minimumSizeHint(); mutable, following style from QLabel
  mutable QSize minimumSizeHint_;
  mutable QSize sizeHint_;
  mutable bool validHints_;
};

}

#endif /* SIMQT_CATEGORYDATABREADCRUMBS_H */
