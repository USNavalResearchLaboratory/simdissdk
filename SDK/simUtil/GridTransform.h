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
#ifndef SIMUTIL_GRIDTRANSFORM_H
#define SIMUTIL_GRIDTRANSFORM_H

#include "osg/MatrixTransform"
#include "simCore/Common/Export.h"

namespace simUtil {

/** Bit mask of various options on a grid cell */
enum GridOption {
  /** Width of cell gets expanded to fill available room in column. */
  GRID_FILL_X = 1 << 0,

  /** Height of cell gets expanded to fill available room in row. */
  GRID_FILL_Y = 1 << 1,

  /** If all cells in the column are stretching, then that column expands horizontally to fill available space in the GridTransform. */
  GRID_STRETCH_COLUMN = 1 << 2,

  /** If all cells in the row are stretching, then that row expands vertically to fill available space in the GridTransform. */
  GRID_STRETCH_ROW = 1 << 3,

  /** Convenience enum to fill both directions and stretch the cell in both directions */
  GRID_FILL = GRID_FILL_X | GRID_FILL_Y | GRID_STRETCH_ROW | GRID_STRETCH_COLUMN,

  /** Cell has a fixed width. */
  GRID_FIXED_WIDTH = 1 << 4,
  /** Cell has a fixed height. */
  GRID_FIXED_HEIGHT = 1 << 5,

  /** Default initialization of grid cell options. */
  DEFAULT_GRID_OPTIONS = GRID_FILL
};

/** Listener interface used to announce when the layout has changed. */
class SDKUTIL_EXPORT GridLayoutListener : public osg::Referenced
{
public:
  /** Called after the layout has changed. */
  virtual void postLayoutChange() = 0;

protected:
  /** Protect osg::Referenced-derived destructor */
  virtual ~GridLayoutListener() {}
};

/**
 * Represents a single cell in the GridTransform.  All children of GridTransform should
 * be instances of GridCell.  Positions and sizes are typically in pixels, but the system
 * is completely relative and could represent percentages or any other system.  For this
 * reason, parameters and member variables are not explicitly called out as pixels or
 * any other unit type.
 */
class SDKUTIL_EXPORT GridCell : public osg::MatrixTransform
{
public:
  /** Default Constructor */
  GridCell();
  /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
  explicit GridCell(const GridCell& rhs, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
  /** Define the default node parameters */
  META_Node(simUtil, GridCell);

  /** Set options.  See simUtil::GridOption. */
  void setOptions(unsigned int opts);
  /** Retrieve all currently set options.  See simUtil::GridOption. */
  unsigned int options() const;
  /** Turns on a single option, convenience method. */
  void setOption(GridOption option);
  /** Turns off a single option, convenience method. */
  void unsetOption(GridOption option);
  /** Returns true if option is set; convenience method. */
  bool testOption(GridOption option) const;

  /** Returns true if the fixed width flag is on */
  bool fixedWidth() const;
  /** Returns true if the fixed height flag is on */
  bool fixedHeight() const;
  /** Returns true if the stretched row flag is on */
  bool stretchRow() const;
  /** Returns true if the stretched column flag is on */
  bool stretchColumn() const;
  /** Returns true if the fill-X flag is on */
  bool fillX() const;
  /** Returns true if the fill-Y flag is on */
  bool fillY() const;

  /** Turns on fixed width option and saves the given width. */
  void setFixedWidth(float width);
  /** Turns on fixed height option and saves the given height. */
  void setFixedHeight(float height);

  /** Returns the X position (grows to right) relative to parent. */
  float x() const;
  /** Returns the Y position (grows upward) relative to parent. */
  float y() const;
  /** Returns the drawn width of the cell. */
  float width() const;
  /** Returns the drawn height of the cell. */
  float height() const;

  /** Returns the default width of the child(ren).  Columns are proportionally sized based on their default width. */
  virtual float defaultWidth() const;
  /** Returns the default height of the child(ren).  Rows are proportionally sized based on their default height. */
  virtual float defaultHeight() const;
  /**
   * Change the default size.  This is the size used for ratio calculations in the GridTransform,
   * in order to properly assign ratio sizes to the child cells.  When stretching, the remaining
   * space is divided up proportionally based on default size.  When not stretching, size is assigned
   * based on the values of the default size.  By default, each cell has a default size of 1.0.
   */
  void setDefaultSize(float width, float height);

  /** Called by the layout in order to set current position */
  void setPosition(float x, float y, float width, float height);

protected:
  /** Protect from osg::ref_ptr double delete common issue */
  virtual ~GridCell();

  /**
   * Override this method if you want to do custom positioning.  Default implementation
   * adjusts matrix transform by scaling and translating.
   */
  virtual void setPositionImpl_();

private:
  float x_;
  float y_;
  float width_;
  float height_;
  unsigned int options_;
  float defaultWidth_;
  float defaultHeight_;
};

////////////////////////////////////////////////////////////////

/**
 * Transform that arranges children in a grid layout.  The 0th child is always the upper-left
 * item in the matrix.  The matrix can either have a fixed number of rows, or a fixed number
 * of columns, similar to FOX Toolkit's matrix class.  The children are stored in the
 * osg::Group's children vector.  The class is only tested against GridCell children.
 *
 * When fixed by columns, the size parameter specifies the number of columns in the grid.
 * The first #size items fill up the first row horizontally, then a new row is created on
 * the (size + 1)'th item.
 *
 * When not fixed by columns, i.e. when fixed by rows, the size parameter specifies the number
 * of rows in the grid.  The first #size items fill up the first column vertically, then a new
 * column is created on the (size + 1)'th item.
 */
class SDKUTIL_EXPORT GridTransform : public osg::MatrixTransform
{
public:
  /** Creates a 1-column vertically expanding grid.  Equivalent to GridTransform(1, true). */
  GridTransform();
  /** Constructor with specific size */
  GridTransform(int size, bool fixedByColumns);
  /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
  explicit GridTransform(const GridTransform& rhs, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
  /** Define the default node parameters */
  META_Node(simUtil, GridTransform);

  /** Set a GridLayoutListener. Any previously set listener will be overwritten. */
  void setLayoutListener(GridLayoutListener* listener);

  /** Changes the horizontal and vertical spacing. */
  void setSpacing(float spacing);
  /** Changes the horizontal spacing. */
  void setHorizontalSpacing(float spacing);
  /** Changes the vertical spacing. */
  void setVerticalSpacing(float spacing);
  /** Retrieve the horizontal spacing. */
  float horizontalSpacing() const;
  /** Retrieve the vertical spacing. */
  float verticalSpacing() const;

  /** Changes whether the columns are packed with uniform width. */
  void setPackUniformWidth(bool packUniform);
  /** Changes whether the rows are packed with uniform height. */
  void setPackUniformHeight(bool packUniform);
  /** Returns true if each column should be proportioned to a uniform width. */
  bool packUniformWidth() const;
  /** Returns true if each row should be proportioned to a uniform height. */
  bool packUniformHeight() const;

  /** Changes the internal padding.  Array indices are left, right, top, bottom. */
  void setPadding(const osg::Vec4f& padding);
  /** Retrieves the padding values */
  osg::Vec4f padding() const;

  /** Indicate that the layout is dirty and need to reposition children.  This queues a layout for next update. */
  void recalc();

  /** Returns the current number of columns. */
  int numColumns() const;
  /** Returns the current number of rows. */
  int numRows() const;

  /**
   * Returns true if the number of columns is fixed by the user value.  If true, new children
   * expand to the right, wrapping around once the columns are filled.  If false, then new
   * children are added to the bottom, wrapping around to the top when row is filled.
   */
  bool isFixedByColumns() const;
  /** Changes the fixed-by-columns flag.  Forces a re-layout to rearrange children. */
  void setFixedByColumns(bool fixedByColumns);

  /** Only applicable when fixed by columns is true. */
  void setNumColumns(int cols);
  /** Only applicable when fixed by columns is false. */
  void setNumRows(int rows);

  /** Returns -1 if child not found */
  int rowOfChild(const GridCell* item) const;
  /** Returns -1 if child not found */
  int columnOfChild(const GridCell* item) const;
  /** Returns child at index, or NULL if none */
  const GridCell* childAt(int row, int column) const;

  /** Override to update layout */
  virtual void traverse(osg::NodeVisitor& nv);

  /** Set the size of the container; children will be resized to fill the space as per configuration. */
  void setSize(float width, float height);
  /** Returns the configured width. */
  float width() const;
  /** Returns the configured height. */
  float height() const;

  /** Calculates the width of the children, accounting for spacing and padding. */
  int getDefaultWidth() const;
  /** Calculates the height of the children, accounting for spacing and padding. */
  int getDefaultHeight() const;

  /** Get the actual width of the specified column, valid only after a call to doLayout_(). */
  float getColumnWidth(int column) const;
  /** Get the actual height of the specified row, valid only after a call to doLayout_(). */
  float getRowHeight(int row) const;

protected:
  /** Protect from osg::ref_ptr double delete common issue */
  virtual ~GridTransform();

  /** Override from osg::Group to dirty layout */
  virtual void childRemoved(unsigned int pos, unsigned int numChildrenToRemove);
  /** Override from osg::Group to dirty layout */
  virtual void childInserted(unsigned int pos);

private:
  /** Returns the width of the widest child. Useful for packing by uniform width. */
  float maxChildWidth_() const;
  /** Returns the height of the tallest child. Useful for packing by uniform height. */
  float maxChildHeight_() const;

  /** Clears the dirty flag, fixing update traversals as needed. */
  void unsetLayoutDirtyFlag_();
  /** Workhorse method that positions children based on settings. */
  void doLayout_();

  /** Pointer to the GridLayoutListener, if any. */
  osg::ref_ptr<GridLayoutListener> listener_;

  /** Spacing between each consecutive column, horizontally. */
  float hSpacing_;
  /** Spacing between each consecutive row, vertically. */
  float vSpacing_;
  /** Indicates each column should be the same width. */
  bool packUniformWidth_;
  /** Indicates each row should be the same height. */
  bool packUniformHeight_;

  /** User-supplied value for number of rows or number of columns. */
  int userNum_;
  /** If true, then userNum_ is number of columns.  If false, then userNum_ is number of rows. */
  bool fixedByColumns_;

  /**
   * Internal padding of the table.  Uses Box Model, padding takes up room inside designated area.
   * Order is left, right, top, and bottom.
   */
  osg::Vec4f padding_;

  /** User-provided width */
  float width_;
  /** User-provided height */
  float height_;

  /** Indicates that the layout needs to re-run on next update traversal */
  bool layoutDirty_;

  /** Cache of the actual column widths being used, updated via doLayout_(). */
  std::vector<float> columnWidths_;
  /** Cache of the actual row heights being used, updated via doLayout_(). */
  std::vector<float> rowHeights_;
};


}

#endif /* SIMUTIL_GRIDTRANSFORM_H */

