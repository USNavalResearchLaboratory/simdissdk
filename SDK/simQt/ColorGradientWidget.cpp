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
#include <cassert>
#include <QColorDialog>
#include <QGroupBox>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include "simCore/Calc/Math.h"
#include "simQt/ColorWidget.h"
#include "simQt/ColorWidgetDelegate.h"
#include "simQt/QtConversion.h"
#include "simQt/QtFormatting.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/ColorGradientWidget.h"
#include "ui_ColorGradientWidget.h"

namespace simQt {

static const QString VALUE_TOOLTIP = QObject::tr("Value of the color stop, in the range [0,1].");
static const QString COLOR_TOOLTIP = QObject::tr("Color of the stop, interpolated with adjacent stops to create gradient.");

/** Width/height of color stop handles, in pixels */
static const int HANDLE_SIZE_PX = 10;
static const int HALF_HANDLE_PX = (HANDLE_SIZE_PX / 2);
/** Line thickness of color stop handles, in pixels */
static const int HANDLE_THICKNESS_PX = 2;
/** Line thickness of color stop handles including an outline, in pixels */
static const int OUTLINE_THICKNESS_PX = HANDLE_THICKNESS_PX + 2;
/** Tolerance for the mouse to grab a stop handle, in pixels. Float for proper division */
static const float HANDLE_TOLERANCE_PX = HALF_HANDLE_PX + OUTLINE_THICKNESS_PX;

static const QColor OUTLINE_COLOR = Qt::darkGray;
static const QColor HANDLE_COLOR = Qt::lightGray;
static const QColor HANDLE_PICK_COLOR = Qt::white;

static const QString GRAD_WIDGET_TOOLTIP = QObject::tr("Left-click and drag to move a color stop, changing its value.<p>Double-click to add or edit a stop.<p>Right-click to remove a stop.");

////////////////////////////////////////////////////

/**
 * QAbstractTableModel that represents a customizable
 * color gradient with values in the range [0,1].
 */
class ColorGradientWidget::ColorGradientModel : public QAbstractTableModel
{
public:
  explicit ColorGradientModel(QObject* parent = NULL)
    : QAbstractTableModel(parent)
  {}

  enum Column
  {
    COL_VALUE = 0,
    COL_COLOR,
    COL_LAST
  };

  enum Role
  {
    /// Indicates that the value is already in the [0,1] range and not a percent string
    DECIMAL_VALUE_ROLE = Qt::UserRole
  };

  // Overridden from QAbstractTableModel
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
  {
    // Flat table, no parents
    if (parent.isValid())
      return 0;
    return static_cast<int>(colorStops_.size());
  }

  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
  {
    // Flat table, no parents
    if (parent.isValid())
      return 0;
    return COL_LAST; // Value, Color
  }

  virtual Qt::ItemFlags flags(const QModelIndex& index) const override
  {
    if (!index.isValid())
      return Qt::NoItemFlags;

    return (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
  }

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
  {
    if (orientation != Qt::Horizontal || section >= columnCount())
      return QVariant();

    if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
      return QVariant();

    switch (static_cast<Column>(section))
    {
    case COL_VALUE:
      return (role == Qt::DisplayRole ? tr("Value") : VALUE_TOOLTIP);
    case COL_COLOR:
      return (role == Qt::DisplayRole ? tr("Color") : COLOR_TOOLTIP);
    default:
      assert(0); // Invalid column received
    }

    return QVariant();
  }

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= rowCount() || index.column() >= COL_LAST)
    {
      assert(0); // Received invalid index
      return false;
    }

    if (role == Qt::ToolTipRole)
    {
      switch (static_cast<Column>(index.column()))
      {
      case COL_VALUE:
        return VALUE_TOOLTIP;
      case COL_COLOR:
        return COLOR_TOOLTIP;
      default:
        assert(0); // Invalid column received
      }
      return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == DECIMAL_VALUE_ROLE)
    {
      auto iter = colorStops_.begin();
      std::advance(iter, index.row());

      switch (static_cast<Column>(index.column()))
      {
      case COL_VALUE:
      {
        if (role == DECIMAL_VALUE_ROLE)
          return iter->first;
        else if (role == Qt::EditRole)
          return iter->first * 100.f;
        return QString::number(static_cast<int>(iter->first * 100.f), 'f', 0) + QString("%");
      }
      case COL_COLOR:
        return iter->second;
      default:
        assert(0); // Invalid column received
      }

      return QVariant();
    }

    return QVariant();
  }

  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
  {
    if (!index.isValid() || !value.isValid())
      return false;

    if (index.row() >= rowCount() || index.column() >= COL_LAST)
    {
      assert(0); // Received invalid index
      return false;
    }

    auto iter = colorStops_.begin() + index.row();
    switch (static_cast<Column>(index.column()))
    {
    case COL_VALUE:
    {
      float val;
      if (role == DECIMAL_VALUE_ROLE)
        val = value.toFloat();
      else
        val = (value.toString().replace("%", "").toFloat() / 100.f);

      // Block invalid or duplicate values
      if (val < 0.f || val > 1.f || hasStop_(val))
        return false;
      iter->first = value.toFloat();
      emit dataChanged(createIndex(index.row(), COL_VALUE), createIndex(index.row(), COL_VALUE));
      return true;
    }
    case COL_COLOR:
    {
      iter->second = value.value<QColor>();
      emit dataChanged(createIndex(index.row(), COL_COLOR), createIndex(index.row(), COL_COLOR));
      return true;
    }
    default:
      assert(0); // Invalid column received
    }

    return false;
  }

  /** Resets the model with the given color gradient */
  void setColorGradient(const ColorGradient& gradient)
  {
    emit beginResetModel();
    colorStops_.clear();

    for (const auto& colorVal : gradient.colors())
      colorStops_.push_back(std::make_pair(colorVal.first, colorVal.second));

    emit endResetModel();
  }

  /** Retrieves the current color gradient from the model */
  ColorGradient getColorGradient() const
  {
    ColorGradient grad;
    grad.clearColors();

    for (const auto& colorStop : colorStops_)
      grad.setColor(colorStop.first, colorStop.second);

    return grad;
  }

  /** Removes all color stops from the model */
  void clear()
  {
    if (colorStops_.empty())
      return;

    emit beginResetModel();
    colorStops_.clear();
    emit endResetModel();
  }

  /** Removes the color stop indicated by the given index */
  void removeStop(const QModelIndex& index)
  {
    if (!index.isValid() || index.row() >= rowCount())
      return;

    if (rowCount() <= 2)
    {
      assert(0); // Dev error, minimum of two stops required
      return;
    }

    auto iter = colorStops_.begin() + index.row();
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    colorStops_.erase(iter);
    endRemoveRows();
  }

  /** Adds a new color stop with the given value, generating an appropriate color */
  QModelIndex addStop(float value)
  {
    // Ignore invalid values
    if (value < 0.f || value > 1.f)
      return QModelIndex();

    return addStop_(value, guessColor_(value));
  }

  /** Sets or creates the stop at the given value with the given color */
  void setColor(float value, const QColor& color)
  {
    for (auto stopIter = colorStops_.begin(); stopIter != colorStops_.end(); ++stopIter)
    {
      if (stopIter->first == value)
      {
        if (stopIter->second != color)
        {
          stopIter->second = color;
          const int row = std::distance(colorStops_.begin(), stopIter);
          const QModelIndex topLeft = index(row, 0);
          const QModelIndex bottomRight = index(row, COL_LAST - 1);
          emit dataChanged(topLeft, bottomRight);
        }
        return;
      }
    }

    addStop_(value, color);
  }

  /**
   * Updates the given persistent index to point to the stop closest
   * to the given value within the defined range, if one exists.
   * Returns true if a stop was found.
   */
  bool indexForValue(float trueValue, QPersistentModelIndex& stopIdx, float tolerance = 0.1f) const
  {
    stopIdx = QModelIndex();
    bool stopFound = false;
    for (auto stopIter = colorStops_.begin(); stopIter != colorStops_.end(); ++stopIter)
    {
      const float delta = fabs(stopIter->first - trueValue);
      if (delta <= tolerance)
      {
        tolerance = delta;
        stopIdx = index(std::distance(colorStops_.begin(), stopIter), COL_VALUE);
        stopFound = true;
        // Later stop could be closer, keep searching
      }
    }

    return stopFound;
  }

private:
  /** Convenience method to add a stop with proper signalling */
  QModelIndex addStop_(float value, const QColor& color)
  {
    const int rowIdx = colorStops_.size();
    emit beginInsertRows(QModelIndex(), rowIdx, rowIdx);
    colorStops_.push_back(std::make_pair(value, color));
    emit endInsertRows();

    return index(rowIdx, COL_VALUE);
  }

  /** Returns true if there is a stop with the given value */
  bool hasStop_(float value) const
  {
    for (const auto& colorStop : colorStops_)
    {
      if (colorStop.first == value)
        return true;
    }
    return false;
  }

  /** Guesses at a default color for a new stop at the given value */
  QColor guessColor_(float value) const
  {
    // Skip color guessing if we're empty
    if (colorStops_.empty())
      return Qt::black;

    // Can't interpolate from one value
    if (rowCount() == 1)
      return colorStops_.begin()->second;

    // Get the value less than new value
    auto leftIter = colorStops_.rbegin();
    while (leftIter != colorStops_.rend() && leftIter->first >= value)
      ++leftIter;

    // New value is new lowest, use color of previous lowest
    if (leftIter == colorStops_.rend())
    {
      --leftIter;
      return leftIter->second;
    }

    // Get the value greater than or equal to new value
    auto rightIter = colorStops_.begin();
    while (rightIter != colorStops_.end() && rightIter->first < value)
      ++rightIter;

    // New value is new highest, use color of previous highest
    if (rightIter == colorStops_.end())
    {
      --rightIter;
      return rightIter->second;
    }

    // Don't try to add duplicate values
    if (rightIter->first == value)
    {
      assert(0); // Testing, shouldn't be able to duplicate values
      return Qt::black;
    }

    // Get the interpolated color
    return ColorGradient::interpolate(leftIter->second, rightIter->second, leftIter->first, value, rightIter->first);
  }

  /** Unordered vector pairing values with corresponding colors */
  std::vector<std::pair<float, QColor> > colorStops_;
};

////////////////////////////////////////////////////

/**
 * Widget that displays the gradient defined in the
 * assigned model and allows the user to modify it.
 */
class ColorGradientWidget::GradientDisplayWidget : public QWidget
{
public:
  explicit GradientDisplayWidget(ColorGradientModel& model, QWidget* parent = NULL)
    : QWidget(parent),
    model_(model),
    dragIndex_(QModelIndex())
  {
    setMinimumHeight(HANDLE_SIZE_PX + HANDLE_THICKNESS_PX + OUTLINE_THICKNESS_PX);
    // Enable mouse tracking so we get move events with no buttons pressed
    setMouseTracking(true);
    connect(&model_, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(update()));
    connect(&model_, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(update()));
    connect(&model_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(update()));
    connect(&model_, SIGNAL(modelReset()), this, SLOT(update()));
  }

  virtual void paintEvent(QPaintEvent* event) override
  {
    QPainter painter(this);
    auto width = painter.device()->width();
    auto height = painter.device()->height();

    auto gradient = QLinearGradient(0, 0, width, 0);

    for (int i = 0; i < model_.rowCount(); ++i)
    {
      auto index = model_.index(i, ColorGradientModel::COL_VALUE);
      const float value = index.data(ColorGradientModel::DECIMAL_VALUE_ROLE).toFloat();
      const QColor color = index.sibling(i, ColorGradientModel::COL_COLOR).data().value<QColor>();

      gradient.setColorAt(value, color);
    }

    // Have to paint the gradient before drawing our stops
    painter.fillRect(0, 0, width, height, gradient);

    const int y = (height / 2) - HALF_HANDLE_PX;
    for (int i = 0; i < model_.rowCount(); ++i)
    {
      auto index = model_.index(i, ColorGradientModel::COL_VALUE);
      const float value = index.data(ColorGradientModel::DECIMAL_VALUE_ROLE).toFloat();
      const int x = (value * width) - HALF_HANDLE_PX;
      bool highlight = false;
      if (dragIndex_.isValid())
        highlight = (index == dragIndex_);
      else
        highlight = (index == pickIndex_);

      drawStopRect_(painter, x, y, highlight);
    }
  }

  virtual void mousePressEvent(QMouseEvent* evt) override
  {
    if (evt->button() != Qt::RightButton
      && evt->button() != Qt::LeftButton)
      return;

    // Use our cached pick index if we have one, else try to pick
    if (!pickIndex_.isValid() && !findStopForEvent_(evt, pickIndex_))
      return;

    if (evt->button() == Qt::RightButton)
    {
      if (model_.rowCount() > 2)
        model_.removeStop(pickIndex_);
      pickIndex_ = QModelIndex();
    }
    // Left click has the index set, so it can handle drag
    dragIndex_ = pickIndex_;
  }

  virtual void mouseReleaseEvent(QMouseEvent* evt) override
  {
    dragIndex_ = QModelIndex();
  }

  virtual void mouseMoveEvent(QMouseEvent* evt) override
  {
    if (width() == 0)
      return;

    // If we aren't dragging, then pick the closest
    if (!dragIndex_.isValid())
    {
      QPersistentModelIndex newPick;
      findStopForEvent_(evt, newPick);
      if (newPick != pickIndex_)
      {
        pickIndex_.swap(newPick);
        update();
      }
      return;
    }
    // Clamp to [0,1] for tooltip purposes
    const float newVal = simCore::sdkMin(1.f, simCore::sdkMax((static_cast<float>(evt->x()) / width()), 0.f));
    assert(dragIndex_.column() == ColorGradientModel::COL_VALUE); // Dev Error: model should've given value index
    model_.setData(dragIndex_, newVal, ColorGradientModel::DECIMAL_VALUE_ROLE);

    QPoint ttPos = mapToGlobal(QPoint(evt->x(), y()));
    QToolTip::showText(ttPos,
      tr("Value: ") + QString::number(static_cast<int>(newVal * 100.f), 'f', 0) + QString("%"),
      this);
  }

  virtual void mouseDoubleClickEvent(QMouseEvent* evt) override
  {
    if (evt->button() != Qt::LeftButton || width() == 0)
      return;

    // Have to re-find index, since we received a release
    if (!findStopForEvent_(evt, dragIndex_))
    {
      // If we didn't doubleclick on a stop, create a new stop
      const float newVal = (static_cast<float>(evt->x()) / width());
      dragIndex_ = model_.addStop(newVal);
    }

    // Open color dialog to set the stop's color
    const QModelIndex colorIdx = dragIndex_.sibling(dragIndex_.row(), ColorGradientModel::COL_COLOR);
    QColor tempColor = model_.data(colorIdx).value<QColor>();
    if (showAlpha_)
      tempColor = QColorDialog::getColor(tempColor, this, tr("Gradient Stop Color"), COLOR_DIALOG_OPTIONS | QColorDialog::ShowAlphaChannel);
    else
      tempColor = QColorDialog::getColor(tempColor, this, tr("Gradient Stop Color"), COLOR_DIALOG_OPTIONS);
    if (tempColor.isValid())
      model_.setData(colorIdx, QVariant(tempColor));

    // Clear both, since the color dialog likely ate our release event
    dragIndex_ = QModelIndex();
    pickIndex_ = QModelIndex();
  }

private:
  /** Draws the rectangular handle used to control a color stop */
  void drawStopRect_(QPainter& painter, int x, int y, bool highlight)
  {
    painter.save();

    QPen outlinePen(OUTLINE_COLOR);
    outlinePen.setWidth(OUTLINE_THICKNESS_PX);
    painter.setPen(outlinePen);
    painter.drawRect(x, y, HANDLE_SIZE_PX, HANDLE_SIZE_PX);

    QPen handlePen(highlight ? HANDLE_PICK_COLOR : HANDLE_COLOR);
    handlePen.setWidth(HANDLE_THICKNESS_PX);
    painter.setPen(handlePen);
    painter.drawRect(x, y, HANDLE_SIZE_PX, HANDLE_SIZE_PX);
    painter.restore();
  }

  /**
   * Updates the given index to the closest stop to the mouse event.
   * Returns true if a stop was found in range, false otherwise.
   */
  bool findStopForEvent_(QMouseEvent* evt, QPersistentModelIndex& stopIdx) const
  {
    const int midY = (height() / 2);
    // Ignore events outside the vertical center
    if (width() == 0 || evt->y() < (midY - HANDLE_SIZE_PX)
      || evt->y() > (midY + HANDLE_SIZE_PX))
    {
      stopIdx = QModelIndex();
      return false;
    }

    const float trueVal = (static_cast<float>(evt->x()) / width());
    const float maxDelta = (HANDLE_TOLERANCE_PX / width());
    return model_.indexForValue(trueVal, stopIdx, maxDelta);
  }

  ColorGradientModel& model_;
  bool showAlpha_;
  QPersistentModelIndex dragIndex_;
  QPersistentModelIndex pickIndex_;
};

////////////////////////////////////////////////////

ColorGradientWidget::ColorGradientWidget(QWidget* parent)
  : QWidget(parent),
  ui_(new Ui_ColorGradientWidget),
  tableGroup_(NULL),
  treeView_(NULL),
  showTable_(true),
  showAlpha_(true),
  showHelp_(true)
{
  model_ = new ColorGradientModel(this);
  proxyModel_ = new QSortFilterProxyModel(this);

  ui_->setupUi(this);

  proxyModel_->setSourceModel(model_);
  // Sort by the edit role to avoid "string order"
  proxyModel_->setSortRole(Qt::EditRole);

  auto display = new GradientDisplayWidget(*model_);
  QSizePolicy policy;
  policy.setHorizontalPolicy(QSizePolicy::Expanding);
  policy.setVerticalPolicy(QSizePolicy::Expanding);
  policy.setHorizontalStretch(10); // Arbitrary number larger than defaults of other items
  display->setSizePolicy(policy);
  display->setToolTip(simQt::formatTooltip(tr("Color Gradient"), GRAD_WIDGET_TOOLTIP));

  ui_->gridLayout->addWidget(display, 0, 1);

  ui_->helpButton->setVisible(showHelp_);
  connect(ui_->helpButton, SIGNAL(clicked(bool)), this, SLOT(showHelpDialog_()));

  // Configure using a default gradient
  setColorGradient(ColorGradient::newDefaultGradient());
  // Setup our table
  showOrHideTable_();

  connect(model_, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(emitGradientChanged_()));
  connect(model_, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(emitGradientChanged_()));
  connect(model_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(emitGradientChanged_()));
}

ColorGradientWidget::~ColorGradientWidget()
{
}

void ColorGradientWidget::setColorGradient(const ColorGradient& gradient)
{
  if (gradient == getColorGradient())
    return;

  model_->setColorGradient(gradient);
}

ColorGradient ColorGradientWidget::getColorGradient() const
{
  return model_->getColorGradient();
}

void ColorGradientWidget::clear()
{
  model_->clear();
}

bool ColorGradientWidget::showTable() const
{
  return showTable_;
}

bool ColorGradientWidget::showAlpha() const
{
  return showAlpha_;
}

bool ColorGradientWidget::showHelp() const
{
  return showHelp_;
}

void ColorGradientWidget::setShowTable(bool show)
{
  if (show == showTable_)
    return;

  showTable_ = show;
  showOrHideTable_();
}

void ColorGradientWidget::setShowAlpha(bool show)
{
  if (show == showAlpha_)
    return;
  showAlpha_ = show;

  if (treeView_)
  {
    treeView_->itemDelegateForColumn(ColorGradientModel::COL_COLOR)->deleteLater();
    treeView_->setItemDelegateForColumn(ColorGradientModel::COL_COLOR, new ColorWidgetDelegate(showAlpha_, this));
  }
}

void ColorGradientWidget::setShowHelp(bool show)
{
  if (show == showHelp_)
    return;

  showHelp_ = show;
  ui_->helpButton->setVisible(showHelp_);
}

void ColorGradientWidget::emitGradientChanged_()
{
  emit gradientChanged(getColorGradient());
}

void ColorGradientWidget::showHelpDialog_()
{
  QMessageBox msg(QMessageBox::Question, tr("Color Gradient"),
    GRAD_WIDGET_TOOLTIP, QMessageBox::Close, this);
  msg.exec();
}

void ColorGradientWidget::showOrHideTable_()
{
  if (!showTable_)
  {
    delete tableGroup_;
    tableGroup_ = NULL;
    // treeView_ will be deleted by Qt parentage
    treeView_ = NULL;
    return;
  }

  assert(!tableGroup_ && !treeView_); // Dev error: Should not call this unless flag changes
  tableGroup_ = new QGroupBox(this);
  tableGroup_->setTitle(tr("Color Stops"));
  tableGroup_->setFlat(true);
  QVBoxLayout* groupLayout = new QVBoxLayout(tableGroup_);
  groupLayout->setContentsMargins(0, 9, 0, 0);

  treeView_ = new QTreeView(tableGroup_);
  treeView_->setRootIsDecorated(false);
  treeView_->setModel(proxyModel_);
  treeView_->setItemDelegateForColumn(ColorGradientModel::COL_COLOR, new ColorWidgetDelegate(showAlpha_, this));
  treeView_->sortByColumn(ColorGradientModel::COL_VALUE, Qt::AscendingOrder);

  groupLayout->addWidget(treeView_);
  tableGroup_->setLayout(groupLayout);
  ui_->verticalLayout->addWidget(tableGroup_);
}


}
