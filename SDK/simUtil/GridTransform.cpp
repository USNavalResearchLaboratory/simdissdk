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
#include "osgEarth/NodeUtils"
#include "simCore/Calc/Math.h"
#include "simUtil/GridTransform.h"

namespace simUtil {

GridCell::GridCell()
  : MatrixTransform(),
    x_(0.f),
    y_(0.f),
    width_(1.f),
    height_(1.f),
    options_(DEFAULT_GRID_OPTIONS),
    defaultWidth_(1.f),
    defaultHeight_(1.f)
{
}

GridCell::GridCell(const GridCell& rhs, const osg::CopyOp& copyop)
  : MatrixTransform(rhs, copyop),
    x_(rhs.x_),
    y_(rhs.y_),
    width_(rhs.width_),
    height_(rhs.height_),
    options_(rhs.options_),
    defaultWidth_(rhs.defaultWidth_),
    defaultHeight_(rhs.defaultHeight_)
{
}

GridCell::~GridCell()
{
}

void GridCell::setOptions(unsigned int opts)
{
  options_ = opts;
}

void GridCell::setOption(GridOption option)
{
  options_ |= option;
}

void GridCell::unsetOption(GridOption option)
{
  options_ = (options_ & ~option);
}

bool GridCell::testOption(GridOption option) const
{
  return (options_ & static_cast<unsigned int>(option)) == static_cast<unsigned int>(option);
}

unsigned int GridCell::options() const
{
  return options_;
}

void GridCell::setFixedWidth(float width)
{
  width_ = width;
  setOption(GRID_FIXED_WIDTH);
}

void GridCell::setFixedHeight(float height)
{
  height_ = height;
  setOption(GRID_FIXED_HEIGHT);
}

float GridCell::x() const
{
  return x_;
}

float GridCell::y() const
{
  return y_;
}

float GridCell::width() const
{
  return width_;
}

float GridCell::height() const
{
  return height_;
}

float GridCell::defaultWidth() const
{
  return defaultWidth_;
}

float GridCell::defaultHeight() const
{
  return defaultHeight_;
}

void GridCell::setDefaultSize(float width, float height)
{
  defaultWidth_ = width;
  defaultHeight_ = height;
}

bool GridCell::fixedWidth() const
{
  return testOption(GRID_FIXED_WIDTH);
}

bool GridCell::fixedHeight() const
{
  return testOption(GRID_FIXED_HEIGHT);
}

bool GridCell::stretchRow() const
{
  return testOption(GRID_STRETCH_ROW);
}

bool GridCell::stretchColumn() const
{
  return testOption(GRID_STRETCH_COLUMN);
}

bool GridCell::fillX() const
{
  return testOption(GRID_FILL_X);
}

bool GridCell::fillY() const
{
  return testOption(GRID_FILL_Y);
}

void GridCell::setPosition(float x, float y, float width, float height)
{
  // Always save the width/height/x/y values
  width_ = width;
  height_ = height;
  x_ = x;
  y_ = y;

  // Call into the impl to adjust the position (this can be overridden)
  setPositionImpl_();
}

void GridCell::setPositionImpl_()
{
  osg::Matrix m;
  m.postMult(osg::Matrix::scale(osg::Vec3f(width(), height(), 1.f)));
  m.postMult(osg::Matrix::translate(osg::Vec3f(x(), y(), 0.f)));
  setMatrix(m);
}

////////////////////////////////////////////////////////////////

GridTransform::GridTransform()
  : hSpacing_(0.f),
    vSpacing_(0.f),
    packUniformWidth_(false),
    packUniformHeight_(false),
    userNum_(1),
    fixedByColumns_(true),
    width_(1.f),
    height_(1.f),
    layoutDirty_(false)
{
}

GridTransform::GridTransform(int size, bool fixedByColumns)
  : hSpacing_(0.f),
    vSpacing_(0.f),
    packUniformWidth_(false),
    packUniformHeight_(false),
    userNum_(simCore::sdkMax(1, size)),
    fixedByColumns_(fixedByColumns),
    width_(1.f),
    height_(1.f),
    layoutDirty_(false)
{
}

GridTransform::GridTransform(const GridTransform& rhs, const osg::CopyOp& copyop)
  : hSpacing_(rhs.hSpacing_),
    vSpacing_(rhs.vSpacing_),
    packUniformWidth_(rhs.packUniformWidth_),
    packUniformHeight_(rhs.packUniformHeight_),
    userNum_(rhs.userNum_),
    fixedByColumns_(rhs.fixedByColumns_),
    padding_(rhs.padding_),
    width_(rhs.width_),
    height_(rhs.height_),
    layoutDirty_(rhs.layoutDirty_)
{
}

GridTransform::~GridTransform()
{
}

void GridTransform::setLayoutListener(GridLayoutListener* listener)
{
  listener_ = listener;
}

void GridTransform::setSpacing(float spacing)
{
  if (hSpacing_ == spacing && vSpacing_ == spacing)
    return;
  hSpacing_ = spacing;
  vSpacing_ = spacing;
  recalc();
}

void GridTransform::setHorizontalSpacing(float spacing)
{
  if (hSpacing_ == spacing)
    return;
  hSpacing_ = spacing;
  recalc();
}

void GridTransform::setVerticalSpacing(float spacing)
{
  if (vSpacing_ == spacing)
    return;
  vSpacing_ = spacing;
  recalc();
}

float GridTransform::horizontalSpacing() const
{
  return hSpacing_;
}

float GridTransform::verticalSpacing() const
{
  return vSpacing_;
}

void GridTransform::setPadding(const osg::Vec4f& padding)
{
  if (padding_ == padding)
    return;

  padding_ = padding;
  recalc();
}

osg::Vec4f GridTransform::padding() const
{
  return padding_;
}

void GridTransform::setPackUniformWidth(bool packUniform)
{
  if (packUniformWidth_ == packUniform)
    return;
  packUniformWidth_ = packUniform;
  recalc();
}

void GridTransform::setPackUniformHeight(bool packUniform)
{
  if (packUniformHeight_ == packUniform)
    return;
  packUniformHeight_ = packUniform;
  recalc();
}

bool GridTransform::packUniformWidth() const
{
  return packUniformWidth_;
}

bool GridTransform::packUniformHeight() const
{
  return packUniformHeight_;
}

void GridTransform::recalc()
{
  if (layoutDirty_)
    return;

  // Add an update visitor to recalculate the layout.  doLayout_() removes this.
  ADJUST_UPDATE_TRAV_COUNT(this, 1);
  layoutDirty_ = true;
}

int GridTransform::numColumns() const
{
  if (fixedByColumns_)
    return userNum_;
  return (getNumChildren() + userNum_ - 1) / userNum_;
}

int GridTransform::numRows() const
{
  if (!fixedByColumns_)
    return userNum_;
  return (getNumChildren() + userNum_ - 1) / userNum_;
}

bool GridTransform::isFixedByColumns() const
{
  return fixedByColumns_;
}

void GridTransform::setFixedByColumns(bool fixedByColumns)
{
  if (fixedByColumns == fixedByColumns_)
    return;
  fixedByColumns_ = fixedByColumns;
  recalc();
}

void GridTransform::setNumColumns(int cols)
{
  if (cols < 1)
    return;
  // Only adjust if we're in columns mode
  if (!fixedByColumns_ || cols == userNum_)
    return;
  userNum_ = cols;
  recalc();
}

void GridTransform::setNumRows(int rows)
{
  if (rows < 1)
    return;
  // Only adjust if we're in rows mode
  if (fixedByColumns_ || rows == userNum_)
    return;
  userNum_ = rows;
  recalc();
}

int GridTransform::rowOfChild(const GridCell* item) const
{
  assert(userNum_ > 0); // guaranteed by setNumRows / setNumColumns
  const unsigned int idx = getChildIndex(item);
  if (idx >= getNumChildren())
    return -1;
  return fixedByColumns_ ? (idx / userNum_) : (idx % userNum_);
}

int GridTransform::columnOfChild(const GridCell* item) const
{
  assert(userNum_ > 0); // guaranteed by setNumRows / setNumColumns
  const unsigned int idx = getChildIndex(item);
  if (idx >= getNumChildren())
    return -1;
  return fixedByColumns_ ? (idx % userNum_) : (idx / userNum_);
}

const GridCell* GridTransform::childAt(int row, int column) const
{
  if (fixedByColumns_)
  {
    if (column < 0 || column >= userNum_)
      return NULL;
    return dynamic_cast<const GridCell*>(getChild(column + userNum_ * row));
  }
  if (row < 0 || row >= userNum_)
    return NULL;
  return dynamic_cast<const GridCell*>(getChild(row + userNum_ * column));
}

void GridTransform::traverse(osg::NodeVisitor& nv)
{
  // Ideally the layout management occurs in the update traversal; that's what it's for.
  // But there are cases where the layout can change between the update traversal and the
  // cull traversal.  Because of this, double check the layout during cull traversal too.
  if (layoutDirty_ && (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR ||
    nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR))
  {
    doLayout_();
    // doLayout_() is responsible for clearing the dirty flag, and also for dealing
    // with the update traversal count.
    assert(!layoutDirty_);
  }

  osg::MatrixTransform::traverse(nv);
}

void GridTransform::setSize(float width, float height)
{
  if (width_ == width && height_ == height)
    return;
  width_ = width;
  height_ = height;
  recalc();
}

float GridTransform::width() const
{
  return width_;
}

float GridTransform::height() const
{
  return height_;
}

int GridTransform::getDefaultWidth() const
{
  assert(userNum_ > 0); // guaranteed by setNumRows / setNumColumns

  // Remember the width if we're packing uniform width
  const float maxWidth = (packUniformWidth_ ? maxChildWidth_() : 0.f);
  // Store the total width and the width of each column
  const int numColumns = this->numColumns();
  std::vector<float> columnWidths(numColumns, 0.f);
  float totalWidth = 0.f;

  // Loop through the children
  const unsigned int numChildren = getNumChildren();
  for (unsigned int idx = 0; idx < numChildren; ++idx)
  {
    // Skip child if it's not visible
    const GridCell* node = dynamic_cast<const GridCell*>(getChild(idx));
    if (node == NULL || node->getNodeMask() == 0)
      continue;

    // Get the current cell width
    float width = 0.f;
    if (node->fixedWidth())
      width = node->width();
    else if (packUniformWidth_)
      width = maxWidth;
    else
      width = node->defaultWidth();

    // Which column are we in?
    const int column = (fixedByColumns_ ? (idx % userNum_) : (idx / userNum_));
    float& lastColumnWidth = columnWidths[column];
    if (width > lastColumnWidth)
    { // Increase the total by the delta from old value
      totalWidth += (width - lastColumnWidth);
      lastColumnWidth = width;
    }
  }

  // Adjust for internal spacing and padding
  if (numColumns > 0)
    totalWidth += (numColumns - 1) * hSpacing_;
  return totalWidth + padding_[0] + padding_[1];
}

int GridTransform::getDefaultHeight() const
{
  assert(userNum_ > 0); // guaranteed by setNumRows / setNumColumns

  // Remember the height if we're packing uniform height
  const float maxHeight = (packUniformHeight_ ? maxChildHeight_() : 0.f);
  // Store the total height and the height of each row
  const int numRows = this->numRows();
  std::vector<float> rowHeights(numRows, 0.f);
  float totalHeight = 0.f;

  // Loop through the children
  const unsigned int numChildren = getNumChildren();
  for (unsigned int idx = 0; idx < numChildren; ++idx)
  {
    // Skip child if it's not visible
    const GridCell* node = dynamic_cast<const GridCell*>(getChild(idx));
    if (node == NULL || node->getNodeMask() == 0)
      continue;

    // Get the current cell height
    float height = 0.f;
    if (node->fixedHeight())
      height = node->height();
    else if (packUniformHeight_)
      height = maxHeight;
    else
      height = node->defaultHeight();

    // Which row are we in?
    const int row = (fixedByColumns_ ? (idx / userNum_) : (idx % userNum_));
    float& lastRowHeight = rowHeights[row];
    if (height > lastRowHeight)
    { // Increase the total by the delta from old value
      totalHeight += (height - lastRowHeight);
      lastRowHeight = height;
    }
  }

  // Adjust for internal spacing and padding
  if (numRows > 0)
    totalHeight += (numRows - 1) * vSpacing_;
  return totalHeight + padding_[2] + padding_[3];
}

float GridTransform::getColumnWidth(int column) const
{
  if (column < 0 || column >= static_cast<int>(columnWidths_.size()))
    return 0.f;

  return columnWidths_[column];
}

float GridTransform::getRowHeight(int row) const
{
  if (row < 0 || row >= static_cast<int>(rowHeights_.size()))
    return 0.f;

  return rowHeights_[row];
}

void GridTransform::childRemoved(unsigned int pos, unsigned int numChildrenToRemove)
{
  recalc();
}

void GridTransform::childInserted(unsigned int pos)
{
  // Assertion failure means that the developer failed to use GridCell appropriately.
  // This means that the GridTransform will be unable to place the child.
  assert(dynamic_cast<GridCell*>(getChild(pos)));
  recalc();
}

float GridTransform::maxChildWidth_() const
{
  float width = 0.f;
  const unsigned int numChildren = getNumChildren();
  for (unsigned int idx = 0; idx < numChildren; ++idx)
  {
    // Skip child if it's not visible
    const GridCell* node = dynamic_cast<const GridCell*>(getChild(idx));
    if (node == NULL || node->getNodeMask() == 0)
      continue;
    if (node->fixedWidth())
      width = simCore::sdkMax(node->width(), width);
    else
      width = simCore::sdkMax(node->defaultWidth(), width);
  }
  return width;
}

float GridTransform::maxChildHeight_() const
{
  float height = 0.f;
  const unsigned int numChildren = getNumChildren();
  for (unsigned int idx = 0; idx < numChildren; ++idx)
  {
    // Skip child if it's not visible
    const GridCell* node = dynamic_cast<const GridCell*>(getChild(idx));
    if (node == NULL || node->getNodeMask() == 0)
      continue;
    if (node->fixedHeight())
      height = simCore::sdkMax(node->height(), height);
    else
      height = simCore::sdkMax(node->defaultHeight(), height);
  }
  return height;
}

void GridTransform::unsetLayoutDirtyFlag_()
{
  if (layoutDirty_)
  {
    ADJUST_UPDATE_TRAV_COUNT(this, -1);
    layoutDirty_ = false;
  }
}

void GridTransform::doLayout_()
{
  /*
   * This is a multi-step algorithm intended to do proper layout for the matrix.  We must first
   * accumulate data about each cell, then do column-based calculations, then apply the calculated
   * cell position to each cell in the matrix.  The algorithm is as follows:
   *
   * 1) Figure out which rows and which columns contribute to stretching.  Figure out the
   *      size of each row and column using the maximum width/height of each child.
   *
   * 2) Accumulate total size of the stretched columns.  We'll use that later.  For non-
   *      stretched columns, go ahead and reserve horizontal space from "hRemain".
   *
   * 3) Just like pass 2, but for rows.  Accumulates total size of rows, and reserves vertical
   *      space for fixed height rows from "vRemain".
   *
   * 4) For each column that is stretched, redistribute the remaining horizontal space.
   *
   * 5) For each row that is stretched, redistribute the remaining vertical space.
   *
   * 6) Position each individual cell, calling setPosition() with correct size parameters.
   */

  assert(userNum_ > 0); // guaranteed by setNumRows / setNumColumns

  // Initialize some useful constants and the remaining space we're eating away at.  Note
  // that this uses the Box Model, in which the padding is internal to the reserved space.
  const float left = padding_[0];
  const float right = width_ - padding_[1];
  const float top = height_ - padding_[2];
  const float bottom = padding_[3];
  float hRemain = right - left;
  float vRemain = top - bottom;

  // Set up vectors for storing column widths
  const int numRows = this->numRows();
  const int numColumns = this->numColumns();
  const float maxWidth = maxChildWidth_();
  const float maxHeight = maxChildHeight_();
  std::vector<float> columnWidths(numColumns, 0.f);
  std::vector<bool> stretchColumns(numColumns, true);
  std::vector<float> rowHeights(numRows, 0.f);
  std::vector<bool> stretchRows(numRows, true);
  columnWidths_.assign(numColumns, 0.f);
  rowHeights_.assign(numRows, 0.f);

  // Early exit if no children
  const unsigned int numChildren = getNumChildren();
  if (numChildren == 0)
  {
    unsetLayoutDirtyFlag_();
    return;
  }

  // Loop through each child
  for (unsigned int idx = 0; idx < numChildren; ++idx)
  {
    // Skip child if it's not visible
    GridCell* node = dynamic_cast<GridCell*>(getChild(idx));
    if (node == NULL || node->getNodeMask() == 0)
      continue;

    // Figure out what our row/column is
    int row = idx / userNum_;
    int column = idx % userNum_;
    if (!fixedByColumns_)
      std::swap(row, column);

    // Calculate the width of the cell
    float childWidth = 0.f;
    if (node->fixedWidth())
      childWidth = node->width();
    else if (packUniformWidth_)
      childWidth = maxWidth;
    else
      childWidth = node->defaultWidth();
    // Ditto on the height
    float childHeight = 0.f;
    if (node->fixedHeight())
      childHeight = node->height();
    else if (packUniformHeight_)
      childHeight = maxHeight;
    else
      childHeight = node->defaultHeight();

    // Save the width and height
    columnWidths[column] = simCore::sdkMax(columnWidths[column], childWidth);
    rowHeights[row] = simCore::sdkMax(rowHeights[row], childHeight);
    // Clear out the column/row stretch if needed
    if (!node->stretchColumn())
      stretchColumns[column] = false;
    if (!node->stretchRow())
      stretchRows[row] = false;
  }

  // Figure out column stretching
  float hTotalStretchedSize = 0.f; // "default width" of all stretched columns
  int hNumStretched = 0; // number of columns that actually stretch
  for (int column = 0; column < numColumns; ++column)
  {
    const float width = columnWidths[column];
    if (width > 0.f)
    {
      if (stretchColumns[column])
      {
        ++hNumStretched;
        hTotalStretchedSize += width;
      }
      else
        hRemain -= width;
    }
  }

  // Figure out row stretching
  float vTotalStretchedSize = 0.f; // "default height" of all stretched rows
  int vNumStretched = 0; // number of columns that actually stretch
  for (int row = 0; row < numRows; ++row)
  {
    const float height = rowHeights[row];
    if (height > 0.f)
    {
      if (stretchRows[row])
      {
        ++vNumStretched;
        vTotalStretchedSize += height;
      }
      else
        vRemain -= height;
    }
  }

  // Reserve the spacing
  hRemain -= (numColumns - 1) * hSpacing_;
  vRemain -= (numRows - 1) * vSpacing_;

  // Second pass on stretching passes out the horizontal stretch
  std::vector<float> xPositions(numColumns + 1, 0.f);
  float currentX = left;
  for (int column = 0; column < numColumns; ++column)
  {
    float width = columnWidths[column];
    xPositions[column] = currentX;
    // Adjust the width for stretched columns
    if (stretchColumns[column])
    {
      // Guaranteed by the ++hNumStretched when stretchColumns flagged true
      assert(hNumStretched > 0);

      if (hTotalStretchedSize == 0.f)
      { // Divide it up equally; avoid divide-by-zero in invalid layouts
        if (hNumStretched != 0)
          width = hRemain / hNumStretched;
      }
      else
      { // Divide by proportion of sum
        width = (width * hRemain) / hTotalStretchedSize;
      }
    }
    currentX += width + hSpacing_;
  }
  // Save the final X as start of next (non-existent) column, for easier calcs later
  xPositions[numColumns] = currentX;

  // Second pass on stretching passes out the vertical stretch
  std::vector<float> yTopPositions(numRows + 1, 0.f);
  float currentY = top;
  for (int row = 0; row < numRows; ++row)
  {
    float height = rowHeights[row];
    yTopPositions[row] = currentY;
    // Adjust the height for stretched rows
    if (stretchRows[row])
    {
      // Guaranteed by the ++vNumStretched when stretchRows flagged true
      assert(vNumStretched > 0);

      if (vTotalStretchedSize == 0.f)
      { // Divide it up equally; avoid divide-by-zero in invalid layouts
        if (vNumStretched != 0)
          height = vRemain / vNumStretched;
      }
      else
      { // Divide by proportion of sum
        height = (height * vRemain) / vTotalStretchedSize;
      }
    }
    currentY -= height + vSpacing_;
  }
  // Save the final Y as start of next (non-existent) row, for easier calcs later
  yTopPositions[numRows] = currentY;

  // Finally, do the positioning for each child
  for (unsigned int idx = 0; idx < numChildren; ++idx)
  {
    // Skip child if it's not visible
    GridCell* node = dynamic_cast<GridCell*>(getChild(idx));
    if (node == NULL || node->getNodeMask() == 0)
      continue;

    // Figure out what our row/column is
    int row = idx / userNum_;
    int column = idx % userNum_;
    if (!fixedByColumns_)
      std::swap(row, column);

    const float xPosition = xPositions[column];
    const float columnWidth = xPositions[column + 1] - xPosition - hSpacing_;

    // Figure out the width to pass down to child
    float width = 0.f;
    if (node->fixedWidth())
      width = node->width();
    else if (node->fillX())
      width = columnWidth;
    else if (packUniformWidth_)
      width = maxWidth;
    else
      width = node->defaultWidth();

    const float yBottomPosition = yTopPositions[row + 1] + vSpacing_;
    const float columnHeight = yTopPositions[row] - yBottomPosition;
    // Figure out the height to pass down to child
    float height = 0.f;
    if (node->fixedHeight())
      height = node->height();
    else if (node->fillY())
      height = columnHeight;
    else if (packUniformHeight_)
      height = maxHeight;
    else
      height = node->defaultHeight();

    // We could align here: left, right, center; top, bottom, center
    node->setPosition(xPosition, yBottomPosition, width, height);
    columnWidths_[column] = width;
    rowHeights_[row] = height;
  }

  // Clear the dirty flag since we just did layout
  unsetLayoutDirtyFlag_();

  if (listener_)
    listener_->postLayoutChange();
}

}
