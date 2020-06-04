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
#include <QPainter>
#include <QSortFilterProxyModel>
#include "simQt/QtConversion.h"
#include "simQt/QtFormatting.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/ColorGradientWidget2.h"
#include "ui_ColorGradientWidget2.h"

namespace simQt {

static const QString VALUE_TOOLTIP = QObject::tr("Value of the color stop, in the range [0,1].");
static const QString COLOR_TOOLTIP = QObject::tr("Color of the stop, interpolated with adjacent stops to create gradient.");

/** Width/height of color stop handles, in pixels */
static const int STOP_SIZE_PX = 10;
/** Line thickness of color stop handles, in pixels */
static const int STOP_THICKNESS_PX = 2;

////////////////////////////////////////////////////

/** QAbstractTableModel that represents a customizable color gradient */
class ColorGradientWidget2::ColorGradientModel : public QAbstractTableModel
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

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      auto iter = colorStops_.begin();
      std::advance(iter, index.row());

      switch (static_cast<Column>(index.column()))
      {
      case COL_VALUE:
        return iter->first;
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

    auto iter = colorStops_.begin();
    std::advance(iter, index.row());

    switch (static_cast<Column>(index.column()))
    {
    case COL_VALUE:
    {
      iter->first = value.toFloat();
      emit dataChanged(createIndex(index.row(), COL_VALUE), createIndex(index.row(), COL_LAST - 1));
      return true;
    }
    case COL_COLOR:
    {
      iter->second = value.value<QColor>();
      emit dataChanged(createIndex(index.row(), COL_VALUE), createIndex(index.row(), COL_LAST - 1));
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

private:
  /** Unordered vector pairing values with corresponding colors */
  std::vector<std::pair<float, QColor> > colorStops_;
};

////////////////////////////////////////////////////

/**
 * Widget that displays the gradient defined in the
 * assigned model and allows the user to modify it.
 */
class ColorGradientWidget2::GradientDisplayWidget : public QWidget
{
public:
  explicit GradientDisplayWidget(ColorGradientModel& model, QWidget* parent = NULL)
    : QWidget(parent),
    model_(model)
  {
  }

  virtual void paintEvent(QPaintEvent* event)
  {
    QPainter painter(this);
    auto width = painter.device()->width();
    auto height = painter.device()->height();

    auto gradient = QLinearGradient(0, 0, width, 0);

    for (int i = 0; i < model_.rowCount(); ++i)
    {
      auto index = model_.index(i, ColorGradientModel::COL_VALUE);
      const float value = index.data().toFloat();
      const QColor color = index.sibling(i, ColorGradientModel::COL_COLOR).data().value<QColor>();

      gradient.setColorAt(value, color);
    }

    // Have to paint the gradient before drawing our stops
    painter.fillRect(0, 0, width, height, gradient);

    for (int i = 0; i < model_.rowCount(); ++i)
    {
      auto index = model_.index(i, ColorGradientModel::COL_VALUE);
      const float value = index.data().toFloat();

      drawStopRect_(painter, value, width, height);
    }
  }

private:
  /** Draws the rectangular handle used to control a color stop */
  void drawStopRect_(QPainter& painter, float value, int width, int height)
  {
    painter.save();
    QPen handlePen(Qt::white);
    handlePen.setWidth(STOP_THICKNESS_PX);
    painter.setPen(handlePen);

    const int x = (value * width) - (STOP_SIZE_PX / 2);
    const int y = (height / 2) - (STOP_SIZE_PX / 2);

    painter.drawRect(x, y, STOP_SIZE_PX, STOP_SIZE_PX);
    painter.restore();
  }

  ColorGradientModel& model_;
};

////////////////////////////////////////////////////

ColorGradientWidget2::ColorGradientWidget2(QWidget* parent)
  : QWidget(parent),
  ui_(new Ui_ColorGradientWidget2)
{
  model_ = new ColorGradientModel(this);
  proxyModel_ = new QSortFilterProxyModel(this);

  ui_->setupUi(this);

  proxyModel_->setSourceModel(model_);
  ui_->treeView->setModel(proxyModel_);
  ui_->treeView->sortByColumn(ColorGradientModel::COL_VALUE, Qt::AscendingOrder);

  auto display = new GradientDisplayWidget(*model_);
  QSizePolicy policy;
  policy.setHorizontalPolicy(QSizePolicy::Expanding);
  policy.setVerticalPolicy(QSizePolicy::Expanding);
  policy.setHorizontalStretch(10); // Arbitrary number larger than defaults of other items
  display->setSizePolicy(policy);

  ui_->gridLayout->addWidget(display, 0, 1);

  // Configure using a default gradient
  setColorGradient(ColorGradient::newDefaultGradient());
}

ColorGradientWidget2::~ColorGradientWidget2()
{
}

void ColorGradientWidget2::setColorGradient(const ColorGradient& gradient)
{
  if (gradient == getColorGradient())
    return;

  model_->setColorGradient(gradient);
}

ColorGradient ColorGradientWidget2::getColorGradient() const
{
  return model_->getColorGradient();
}

void ColorGradientWidget2::clear()
{
  model_->clear();
}

void ColorGradientWidget2::showTable(bool show)
{
  ui_->stopsTableGroup->setVisible(show);
}

}
