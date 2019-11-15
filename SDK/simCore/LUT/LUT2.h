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
#ifndef SIMCORE_CALC_LUT_LUT2_H
#define SIMCORE_CALC_LUT_LUT2_H

#include <vector>
#include <functional>
#include <utility>
#include <algorithm>

#include "LUT1.h"

namespace simCore
{
  namespace LUT
  {
    /**
    * Two-dimensional lookup table class
    */
    template <class Value = double>
    class LUT2
    {
    public:
      /**
      * @brief Initializes size of internal two-dimensional lookup table
      *
      * @param[in ] minX Minimum x value
      * @param[in ] maxX Maximum x value
      * @param[in ] numX Number of x values
      * @param[in ] minY Minimum y value
      * @param[in ] maxY Maximum y value
      * @param[in ] numY Number of y values
      * @param[in ] value Storage value type
      * @throw std::invalid_argument
      */
      void initialize(double minX, double maxX, size_t numX, double minY, double maxY, size_t numY, Value value = Value())
      {
        if (!numX || !numY || maxX < minX || maxY <= minY)
          throw std::invalid_argument("simCore::LUT::LUT2::initialize");

        minX_ = minX;
        maxX_ = maxX;
        numX_ = numX;
        // when there is only a single x-value, support one-dim lookup table functionality
        stepX_ = (maxX != minX && numX > 1) ? (maxX - minX) / (numX - 1) : 0;
        array_.resize(numX);
        minY_ = minY;
        maxY_ = maxY;
        numY_ = numY;
        stepY_ = (maxY - minY) / (numY - 1);

        typename std::vector<std::vector<Value> >::iterator iter;
        for (iter = array_.begin(); iter != array_.end(); ++iter)
        {
          iter->resize(numY, value);
        }
      }
      /** @return Minimum X dimension value */
      double minX() const { return minX_; }
      /** @return Maximum X dimension value */
      double maxX() const { return maxX_; }
      /** @return Step size of X dimension */
      double stepX() const { return stepX_; }
      /** @return Number of X dimension values */
      size_t numX() const { return numX_; }
      /** @return Minimum Y dimension value */
      double minY() const { return minY_; }
      /** @return Maximum Y dimension value */
      double maxY() const { return maxY_; }
      /** @return Step size of Y dimension */
      double stepY() const { return stepY_; }
      /** @return Number of Y dimension values */
      size_t numY() const { return numY_; }
      /**
      * Performs lookup at specified indices
      * @param[in ] xIndex X location to perform lookup
      * @param[in ] yIndex Y location to perform lookup
      * @return Const reference to value at specified indices
      * @throw std::out_of_range
      */
      const Value &operator()(size_t xIndex, size_t yIndex) const
      {
        if (xIndex >= numX_ || yIndex >= numY_)
          throw std::out_of_range("simCore::LUT::LUT2::operator(size_t xIndex, size_t yIndex) const");

        return array_[xIndex][yIndex];
      }
      /**
      * Performs lookup at specified indices
      * @param[in ] xIndex X location to perform lookup
      * @param[in ] yIndex Y location to perform lookup
      * @return Reference to value at specified indices
      * @throw std::out_of_range
      */
      Value &operator()(size_t xIndex, size_t yIndex)
      {
        if (xIndex >= numX_ || yIndex >= numY_)
          throw std::out_of_range("simCore::LUT::LUT2::operator(size_t xIndex, size_t yIndex)");

        return array_[xIndex][yIndex];
      }

    private:
      double minX_;               /**< minimum X value of the LUT */
      double minY_;               /**< minimum Y value of the LUT */
      double maxX_;               /**< maximum X value of the LUT */
      double maxY_;               /**< maximum Y value of the LUT */
      double stepX_;              /**< X dimension step size of the LUT */
      double stepY_;              /**< Y dimension step size of the LUT */
      size_t numX_;               /**< number of X dimension values in the LUT */
      size_t numY_;               /**< number of Y dimension values in the LUT */
      std::vector<std::vector<Value> > array_;  /**< STL storage container for LUT */
    };

    /**
    * Determines closest values in LUT to specified values
    * @param[in ] lut2 Lookup table to find index
    * @param[in ] exactX value to find closest x dimension index
    * @param[in ] exactY value to find closest y dimension index
    * @return closest values to specified value
    */
    template <class Value>
    inline std::pair<double, double> index(const LUT2<Value> &lut2, double exactX, double exactY)
    {
      return std::pair<double, double>(index(lut2.minX(), lut2.stepX(), exactX), index(lut2.minY(), lut2.stepY(), exactY));
    }

    /**
    * Determines nearest indices in LUT to specified values
    * @param[in ] lut2 Lookup table to find index
    * @param[in ] exactX value to find closest x dimension index
    * @param[in ] exactY value to find closest y dimension index
    * @return nearest indices to specified values
    */
    template <class Value>
    inline Value nearValue(const LUT2<Value> &lut2, double exactX, double exactY)
    {
      const std::pair<double, double>& rv = index(lut2, exactX, exactY);
      return lut2(static_cast<size_t>(rv.first + 0.5), static_cast<size_t>(rv.second + 0.5));
    }

    /**
    * Performs interpolation of the LUT using the specified values
    * @param[in ] lut2 Lookup table to find index and perform interpolation
    * @param[in ] exactX value to find closest x dimension index
    * @param[in ] exactY value to find closest y dimension index
    * @param[in ] func Bilinear interpolation function to use
    * @return interpolated LUT value based on specified value
    */
    template <class Value, class Function>
    inline Value interpolate(const LUT2<Value> &lut2, double exactX, double exactY, Function func)
    {
      const std::pair<double, double>& rv = index(lut2, exactX, exactY);

      if (rv.first < 0 || rv.second < 0)
        throw std::out_of_range("simCore::LUT::interpolate");

      size_t lowx = size_t(rv.first);
      size_t lowy = size_t(rv.second);
      double stepX = lut2.stepX();
      double stepY = lut2.stepY();
      if (lowx == lut2.numX() - 1)
        --lowx;
      if (lowy == lut2.numY() - 1)
        --lowy;
      double minX = lut2.minX() + stepX * lowx;
      double minY = lut2.minY() + stepY * lowy;
      return func(lut2(lowx, lowy), lut2(lowx + 1, lowy),
        lut2(lowx + 1, lowy + 1), lut2(lowx, lowy + 1),
        minX, exactX, minX + stepX, minY, exactY, minY + stepY);
    }

    /**
    * Writes the LUT to an output stream
    * @param[in ] out Output stream
    * @param[in ] lut2 Lookup table to write to output stream
    */
    template <class Value>
    inline std::ostream &operator<<(std::ostream &out, const LUT2<Value> &lut2)
    {
      out << lut2.numX() << " " << lut2.minX() << " " << lut2.stepX() << " " << lut2.maxX() << std::endl;
      out << lut2.numY() << " " << lut2.minY() << " " << lut2.stepY() << " " << lut2.maxY() << std::endl;
      const std::vector<std::vector <Value> >& array = lut2.array_;
      for (size_t i = 0; i < array.size(); ++i)
      {
        const std::vector<Value>& vec = array[i];
        for (size_t j = 0; j < vec.size(); ++j)
        {
          out << i << " " << j << " = " << vec[j] << std::endl;
        }
      }
      return out;
    }

  } // end of namespace LUT
} // end of namespace simCore

#endif /* SIMCORE_CALC_LUT_LUT2_H */
