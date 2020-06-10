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
#include <limits>
#include "simCore/Calc/Math.h"
#include "simCore/Calc/SquareMatrix.h"

namespace simCore
{

SquareMatrix::SquareMatrix(unsigned int dimension)
  : dimension_(dimension)
{
  if (dimension_ > 1)
    matrix_.assign(dimension_ * dimension_, 0.0);
}

SquareMatrix::SquareMatrix()
  : dimension_(3)
{
  matrix_.assign(dimension_ * dimension_, 0.0);
}

SquareMatrix::SquareMatrix(const SquareMatrix& rhs)
{
  dimension_ = rhs.dimension_;
  matrix_ = rhs.matrix_;
}

SquareMatrix::~SquareMatrix()
{
}

unsigned int SquareMatrix::dimension() const
{
  return dimension_;
}

SquareMatrix& SquareMatrix::operator= (const SquareMatrix &m)
{
  dimension_ = m.dimension_;
  matrix_ = m.matrix_;

  return *this;
}

int SquareMatrix::makeIdentity()
{
  if (dimension_ < 2)
    return 1;

  matrix_.assign(dimension_ * dimension_, 0.0);
  for (unsigned int ii = 0; ii < dimension_; ++ii)
    set(ii, ii, 1.0);

  return 0;
}

int SquareMatrix::makeZero()
{
  if (dimension_ < 2)
    return 1;

  matrix_.assign(dimension_ * dimension_, 0.0);

  return 0;
}

int SquareMatrix::transpose()
{
  if (dimension_ < 2)
    return 1;

  for (unsigned int row = 1; row < dimension_; ++row)
  {
    for (unsigned int col = 0; col < row; ++col)
    {
      double temp = get_(row, col);
      set(row, col, get_(col, row));
      set(col, row, temp);
    }
  }

  return 0;
}

int SquareMatrix::set(unsigned int row, unsigned int col, double value)
{
  if ((row >= dimension_) || (col >= dimension_))
    return 1;

  matrix_[row * dimension_ + col] = value;
  return 0;
}

double SquareMatrix::get(unsigned int row, unsigned int col) const
{
  if ((row >= dimension_) || (col >= dimension_))
    return std::numeric_limits<float>::quiet_NaN();

  return get_(row, col);
}

std::vector<double> SquareMatrix::row(unsigned int row) const
{
  std::vector<double> rv;
  if (row >= dimension_)
    return rv;

  for (unsigned int col = 0; col < dimension_; ++col)
    rv.push_back(get_(row, col));

  return rv;
}

std::vector<double> SquareMatrix::column(unsigned int col) const
{
  std::vector<double> rv;
  if (col >= dimension_)
    return rv;

  for (unsigned int row = 0; row < dimension_; ++row)
    rv.push_back(get_(row, col));

  return rv;
}

const double* SquareMatrix::data() const
{
  if (dimension_ < 2)
    return NULL;

  return matrix_.data();
}

int SquareMatrix::scale(double scaleValue)
{
  if (dimension_ < 2)
    return 1;

  for (auto it = matrix_.begin(); it != matrix_.end(); ++it)
    *it = scaleValue * *it;

  return 0;
}

int SquareMatrix::add(const SquareMatrix& m)
{
  if (dimension_ < 2)
    return 1;

  if (m.dimension_ != dimension_)
    return 1;

  for (unsigned int row = 0; row < dimension_; ++row)
  {
    for (unsigned int col = 0; col < dimension_; ++col)
      set(row, col, get_(row, col) + m.get_(row, col));
  }

  return 0;
}

int SquareMatrix::postMultiply(const SquareMatrix& m)
{
  if (dimension_ < 2)
    return 1;

  if (m.dimension_ != dimension_)
    return 1;

  SquareMatrix results(dimension_);
  for (unsigned int row = 0; row < dimension_; ++row)
  {
    for (unsigned int col = 0; col < dimension_; ++col)
    {
      double total = 0.0;
      for (unsigned int vec = 0; vec < dimension_; ++vec)
        total = total + get_(row, vec) * m.get_(vec, col);
      results.set(row, col, total);
    }
  }

  matrix_ = results.matrix_;
  return 0;
}

int SquareMatrix::preMultiply(const SquareMatrix& m)
{
  if (dimension_ < 2)
    return 1;

  if (m.dimension_ != dimension_)
    return 1;

  SquareMatrix results(dimension_);
  for (unsigned int row = 0; row < dimension_; ++row)
  {
    for (unsigned int col = 0; col < dimension_; ++col)
    {
      double total = 0.0;
      for (unsigned int vec = 0; vec < dimension_; ++vec)
        total = total + m.get_(row, vec) * get_(vec, col);
      results.set(row, col, total);
    }
  }

  matrix_ = results.matrix_;
  return 0;
}

double SquareMatrix::get_(unsigned int row, unsigned int col) const
{
  // Caller should prevent out of bounds call
  assert((row < dimension_) && (col < dimension_));
  if ((row >= dimension_) || (col >= dimension_))
    return 0.0;

  return matrix_[row * dimension_ + col];
}

bool areEqual(const SquareMatrix& m1, const SquareMatrix& m2, double t)
{
  if (m1.dimension() != m2.dimension())
    return false;

  for (unsigned int row = 0; row < m1.dimension(); ++row)
  {
    for (unsigned int col = 0; col < m1.dimension(); ++col)
    {
      if (!simCore::areEqual(m1.get(row, col), m2.get(row, col), t))
        return false;
    }
  }

  return true;
}

}
