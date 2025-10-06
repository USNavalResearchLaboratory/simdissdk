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
#ifndef SIMQT_CATEGORYDATABREADCRUMBS_H
#define SIMQT_CATEGORYDATABREADCRUMBS_H

#include <map>
#include <memory>
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

class EntityCategoryFilter;

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
    qreal rectangleRadiusX = 4.0;
    qreal rectangleRadiusY = 4.0;

    /// Pen and colors for the rectangle and its outline
    QPen outlinePen;
    QColor fillColor;
    QColor altFillColor;
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
  explicit CloseableItemDelegate(QObject* parent=nullptr);
  virtual ~CloseableItemDelegate();

  /** Overrides from QStyledItemDelegate */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual bool editorEvent(QEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);

  /** Non-const accessor to the style */
  Style& style();
  /** Const accessor to the style */
  const Style& style() const;

Q_SIGNALS:
  /** End user clicked on the close button for the given index */
  void closeClicked(const QModelIndex& index);

private:
  /** Calculates the drawn rectangle area for the close icon */
  QRectF calcIconRect_(const QStyleOptionViewItem& option) const;

  /** Called by editorEvent() on mouse press events.  Returns true if update() required. */
  bool mousePressEvent_(QMouseEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
  /** Called by editorEvent() on mouse release events.  Returns true if update() required. */
  bool mouseReleaseEvent_(QMouseEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
  /** Called by editorEvent() on mouse move events.  Returns true if update() required. */
  bool mouseMoveEvent_(QMouseEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);

  /** Drawing style */
  Style style_;
  /** Currently hovered index from last move mouse event. */
  QModelIndex hoverIndex_;
  /** Currently pressed index from last press mouse event; cleared on release. */
  QModelIndex pressedIndex_;
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
  Q_PROPERTY(QColor altFillColor READ altFillColor WRITE setAltFillColor);
  Q_PROPERTY(QColor outlineColor READ outlineColor WRITE setOutlineColor);
  Q_PROPERTY(qreal outlineWidth READ outlineWidth WRITE setOutlineWidth);
  Q_PROPERTY(QMargins itemMargins READ itemMargins WRITE setItemMargins);
  Q_PROPERTY(QMargins textPadding READ textPadding WRITE setTextPadding);
  Q_PROPERTY(QMargins iconPadding READ iconPadding WRITE setIconPadding);
  Q_PROPERTY(QIcon closeIcon READ closeIcon WRITE setCloseIcon);
  Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize);
  Q_PROPERTY(int minimumGroupSize READ minimumGroupSize WRITE setMinimumGroupSize);
  Q_PROPERTY(bool hideWhenEmpty READ hideWhenEmpty WRITE setHideWhenEmpty);
  Q_PROPERTY(QString emptyText READ emptyText WRITE setEmptyText);

public:
  explicit CategoryDataBreadcrumbs(QWidget* parent=nullptr);
  virtual ~CategoryDataBreadcrumbs();

  /** Override to return a reasonable minimum height based on content */
  virtual QSize minimumSizeHint() const;
  virtual QSize sizeHint() const;

  // Property accessors
  qreal rectangleRadiusX() const;
  qreal rectangleRadiusY() const;
  QColor fillColor() const;
  QColor altFillColor() const;
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
  bool hideWhenEmpty() const;
  QString emptyText() const;

  /** Changes the radius on the rounded rectangle in the X coordinate for buttons. */
  void setRectangleRadiusX(qreal value);
  /** Changes the radius on the rounded rectangle in the Y coordinate for buttons. */
  void setRectangleRadiusY(qreal value);
  /** Changes the background fill color for the buttons. */
  void setFillColor(const QColor& value);
  /** Changes the alternate background fill color for the buttons. */
  void setAltFillColor(const QColor& value);
  /** Changes the text color for the buttons. */
  void setTextColor(const QColor& value);
  /** Changes the pen used for drawing the button outline.  Encapsulates the outline color and width. */
  void setOutlinePen(const QPen& value);
  /** Changes the outline color for the buttons. */
  void setOutlineColor(const QColor& value);
  /** Changes the outline width for the buttons. */
  void setOutlineWidth(qreal value);
  /** Changes the margin around each individual button. */
  void setItemMargins(const QMargins& value);
  /** Changes the padding inside the button, around the text. */
  void setTextPadding(const QMargins& value);
  /** Changes the padding inside the button, around the close icon. */
  void setIconPadding(const QMargins& value);
  /** Changes the icon to use for the close button icon. */
  void setCloseIcon(const QIcon& value);
  /** Changes the desired icon size. */
  void setIconSize(const QSize& value);

  /** Sets value to that of the EntityCategoryFilter and keeps both widgets in sync. */
  void bindTo(simQt::EntityCategoryFilter* categoryFilter);

public Q_SLOTS:
  /** Change the minimum number of items required to form a 'group' for a category name. */
  void setMinimumGroupSize(int value);
  /** Change whether the widget shows the empty text when empty, or is hidden. */
  void setHideWhenEmpty(bool value);
  /** Change the text shown when empty; only if hide-when-empty is false. */
  void setEmptyText(const QString& value);

  /** Clears the current filter. */
  void clearFilter();
  /** Changes the current filter. */
  void setFilter(const simData::CategoryFilter& filter);

Q_SIGNALS:
  /** End user changed the filter.  Note that this is a simplified filter and may not exactly match input. */
  void filterEdited(const simData::CategoryFilter& filter);

private Q_SLOTS:
  /** Responds to the QListWidget item having a close button clicked */
  void removeFilter_(const QModelIndex& index);
  /** Synchronize the current state to the state in the sender() EntityCategoryFilter */
  void synchronizeToSenderFilter_();

protected:
  /** Override resize event to recalculate the flow and adjust minimum height */
  virtual void resizeEvent(QResizeEvent* evt);

private:
  /** Adds items to the breadcrumb list for the given name */
  void addNameToList_(int nameIndex, bool useAltFillColor);
  /** Clears and redraws the entire list to reflect the state of the filter. */
  void rebuildList_();
  /** If the list is empty, adds the no-valid-item item */
  void addNoValidItemIfEmptyList_();

  /** Convenience method to append a list widget item for the given category */
  QListWidgetItem* addNameItem_(const QString& categoryName, int nameInt, bool useAltFillColor);
  /** Convenience method to append a list widget item for a specific value in a given category */
  QListWidgetItem* addValueItem_(const QString& text, const QString& name, int nameInt, int valueInt, bool isChecked, bool useAltFillColor);

  /** Creates a list for tooltips that uses <li> items for each check in the list */
  QString buildValuesHtmlList_(const simData::CategoryNameManager& nameManager, const std::map<int, bool>& checks, size_t maxItems) const;

  /** Listens to CategoryNameManager to know when the category filter is cleared */
  class FilterClearListener;
  std::shared_ptr<FilterClearListener> listener_;

  /** Widget that displays the filters */
  QListWidget* listWidget_ = nullptr;
  /** Delegate that draws the filter items */
  CloseableItemDelegate* itemDelegate_ = nullptr;
  /** Delegate to use when the list has no items (default delegate) */
  QStyledItemDelegate* plainDelegate_ = nullptr;
  /** Copy of the current filter */
  simData::CategoryFilter* filter_ = nullptr;

  /** Minimum number of items in a category before grouping */
  int minimumGroupSize_ = 3;
  /** Hide the widget with a size of 0 when it is empty */
  bool hideWhenEmpty_ = true;
  /** Text to display when there are no items in the list */
  QString emptyText_;

  // Size hints calculated in minimumSizeHint(); mutable, following style from QLabel
  mutable QSize minimumSizeHint_;
  mutable QSize sizeHint_;
  mutable bool validHints_ = false;
};

}

#endif /* SIMQT_CATEGORYDATABREADCRUMBS_H */
