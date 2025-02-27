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
#ifndef SIMCORE_CALC_LUT_LUT2_H
#define SIMCORE_CALC_LUT_LUT2_H

#include <algorithm>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

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
      /** set noDataValue */
      void setNoDataValue(Value noDataValue) { noDataValue_ = noDataValue; }
      /** @return noDataValue */
      const std::optional<Value>& noDataValue() const { return noDataValue_; }
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
      double minX_ = 0.;          /**< minimum X value of the LUT */
      double minY_ = 0.;          /**< minimum Y value of the LUT */
      double maxX_ = 0.;          /**< maximum X value of the LUT */
      double maxY_ = 0.;          /**< maximum Y value of the LUT */
      double stepX_ = 0.;         /**< X dimension step size of the LUT */
      double stepY_ = 0.;         /**< Y dimension step size of the LUT */
      size_t numX_ = 0;           /**< number of X dimension values in the LUT */
      size_t numY_ = 0;           /**< number of Y dimension values in the LUT */
      std::vector<std::vector<Value> > array_;  /**< STL storage container for LUT; inner vector holds y data */
      std::optional<Value> noDataValue_;
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
    * Performs interpolation of the LUT using the specified values
    * @param[in ] lut2 Lookup table to find index and perform interpolation
    * @param[in ] exactX value to find closest x dimension index
    * @param[in ] exactY value to find closest y dimension index
    * @param[in ] func Bilinear interpolation function to use
    * @return interpolated LUT value based on specified value
    */
    template <class Value, class Function>
    inline std::optional<Value> interpolateWithNoDataValue(const LUT2<Value>& lut2, double exactX, double exactY, Function func)
    {
      if (!lut2.noDataValue())
        return interpolate(lut2, exactX, exactY, func);

      const std::pair<double, double>& rv = index(lut2, exactX, exactY);

      if (rv.first < 0 || rv.second < 0)
        throw std::out_of_range("simCore::LUT::interpolate");

      size_t lowx = static_cast<size_t>(rv.first);
      size_t lowy = static_cast<size_t>(rv.second);
      const double stepX = lut2.stepX();
      const double stepY = lut2.stepY();
      if (lowx == lut2.numX() - 1)
        --lowx;
      if (lowy == lut2.numY() - 1)
        --lowy;
      const double minX = lut2.minX() + stepX * lowx;
      const double minY = lut2.minY() + stepY * lowy;

      // an exactX,exactY input picks out a 2x2 table for interpolation; convention for bilinear interpolator is
      // 
      // UL (xmin, ymax)    UR (xmax, ymax)
      // LL (xmin, ymin)    LR (xmax, ymin)
      // 
      auto LL = lut2(lowx, lowy);
      auto LR = lut2(lowx + 1, lowy);
      auto UL = lut2(lowx, lowy + 1);
      auto UR = lut2(lowx + 1, lowy + 1);

      // detect noData cases
      const auto noDataValue = lut2.noDataValue().value();
      const bool noDataUL = (UL == noDataValue);
      const bool noDataUR = (UR == noDataValue);
      const bool noDataLL = (LL == noDataValue);
      const bool noDataLR = (LR == noDataValue);
      const unsigned noDataCount = (noDataLL ? 1 : 0) + (noDataLR ? 1 : 0) + (noDataUR ? 1 : 0) + (noDataUL ? 1 : 0);
      if (noDataCount == 4)
        return {};
      if (noDataCount >= 1)
      {
        // implement a closeness criterion, to prevent replacement of noData with values that distort interpolation (and would be managed by scale factor weighting in a normal interpolation)
        // given
        // x{1,2};
        // y{10,20};
        // and
        // data
        // x=1: 100,200
        // x=2: 300,600
        // interpolating for (1, 12)
        // will produce an answer of 120
        // 
        // interpolating for (1.5, 12) with noData values (without the closeness criteria)
        // 100,-99
        // -99,600
        // will produce an answer of 200
        // (and will return 200 for any choice of x)
        //
        //

        // determine if specified exactX is close to minX or maxX, using 10% as the criterion, as this can impact interpolation decisions
        const bool exactXcloseToMin = (exactX <= (minX + .1 * stepX));
        const bool exactXcloseToMax = (exactX >= (minX + .9 * stepX));
        // determine if specified exactY is close to minY or maxY
        const bool exactYcloseToMin = (exactY <= (minY + .1 * stepY));
        const bool exactYcloseToMax = (exactY >= (minY + .9 * stepY));

        // disallow interpolation when exact index is close to min (or max) index and both min (or max) values are noData
        if ((exactXcloseToMin && noDataLL && noDataUL) ||
          (exactXcloseToMax && noDataLR && noDataUR) ||
          (exactYcloseToMin && noDataLL && noDataLR) ||
          (exactYcloseToMax && noDataUL && noDataUR))
          return {};

        if (noDataCount == 3)
        {
          // some cases with closeness criteria may have been rejected above as not-interpolatable
          if (noDataLR && noDataUR && noDataUL)
            return LL;
          if (noDataLL && noDataLR && noDataUR)
            return UL;
          if (noDataLL && noDataLR && noDataUL)
            return UR;
          if (noDataLL && noDataUR && noDataUL)
            return LR;
          // logic above exhausts all possibilities
          assert(0);
        }

        if (noDataCount == 2)
        {
          // all combinations; noting that some special cases were processed above
          if (noDataUL)
          {
            if (noDataUR)
            {
              UL = LL;
              UR = LR;
            }
            else if (noDataLL)
            {
              UL = UR;
              LL = LR;
            }
            else if (noDataLR)
            {
              UL = exactYcloseToMax ? UR : (exactXcloseToMin ? LL : UR);
              LR = exactYcloseToMin ? LL : (exactXcloseToMax ? UR : LL);
            }
          }
          else if (noDataUR)
          {
            if (noDataLL)
            {
              UR = exactYcloseToMax ? UL : (exactXcloseToMax ? LR : UL);
              LL = exactYcloseToMin ? LR : (exactXcloseToMin ? UL : LR);
            }
            else if (noDataLR)
            {
              UR = UL;
              LR = LL;
            }
          }
          else if (noDataLL)
          {
            // all other combinations have been handled.
            assert(noDataLR);
            LL = UL;
            LR = UR;
          }
          else
          {
            // logic above should exhaust all cases
            assert(0);
            return {};
          }

          // fallthrough to func call
        }
        else if (noDataCount == 1)
        {
          // use closeness criteria to choose value to replace the noData value, prioritising closeness-in-y over closeness-in-x; fall back to value that has same y, different x if no closeness criteria applies
          if (noDataUL)
            UL = exactYcloseToMax ? UR : (exactXcloseToMin ? LL : UR);
          else if (noDataUR)
            UR = exactYcloseToMax ? UL : (exactXcloseToMax ? LR : UL);
          else if (noDataLL)
            LL = exactYcloseToMin ? LR : (exactXcloseToMin ? UL : LR);
          else if (noDataLR)
            LR = exactYcloseToMin ? LL : (exactXcloseToMax ? UR : LL);
          else
          {
            // logic above should exhaust all cases
            assert(0);
            return {};
          }
          // fallthrough to func call
        }
      }

      // logic above guarantees that any nodataValue is replaced by a good data value
      assert(LL != noDataValue && LR != noDataValue && UL != noDataValue && UR != noDataValue);

      return func(LL, LR, UR, UL,
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
