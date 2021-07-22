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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_CALC_LUT_LUT1_H
#define SIMCORE_CALC_LUT_LUT1_H

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace simCore
{
  namespace LUT
  {
    /**
    * One dimensional look up table class
    */
    template <class Value = double>
    class LUT1
    {
    public:
      /**
      * @brief Initializes size of one dimensional look up table
      *
      * @param[in ] minX Minimum x value
      * @param[in ] maxX Maximum x value
      * @param[in ] numX Number of x values
      * @param[in ] value Storage value type
      * @throw std::invalid_argument
      */
      void initialize(double minX, double maxX, size_t numX, Value value = Value())
      {
        if (!numX || maxX <= minX)
          throw std::invalid_argument("simCore::LUT::LUT1::initialize");

        minX_ = minX;
        maxX_ = maxX;
        numX_ = numX;
        stepX_ = (maxX - minX) / (numX - 1);
        array_.resize(numX, value);
      }
      /** @return Minimum X dimension value */
      double minX() const { return minX_; }
      /** @return Maximum X dimension value */
      double maxX() const { return maxX_; }
      /** @return Step size of X dimension */
      double stepX() const { return stepX_; }
      /** @return Number of X dimension values */
      size_t numX() const { return numX_; }
      /**
      * Performs look up at specified index
      * @param[in ] index Location to perform look up
      * @return Const reference to value at specified index
      * @throw std::out_of_range
      */
      const Value &operator()(size_t index) const
      {
        if (index >= numX_)
          throw std::out_of_range("simCore::LUT::LUT1::operator(size_t index) const");

        return array_[index];
      }
      /**
      * Performs look up at specified index
      * @param[in ] index Location to perform look up
      * @return Reference to value at specified index
      * @throw std::out_of_range
      */
      Value &operator()(size_t index)
      {
        if (index >= numX_)
          throw std::out_of_range("simCore::LUT::LUT1::operator(size_t index)");

        return array_[index];
      }

    private:
      double minX_;               /**< minimum value of the LUT */
      double maxX_;               /**< maximum value of the LUT */
      double stepX_;              /**< step size of the LUT */
      size_t numX_;               /**< number of values in the LUT */
      std::vector<Value> array_;  /**< STL storage container for LUT */
    };


    /**
    * Determines closest value in LUT to specified value
    * @param[in ] min minimum value of the LUT
    * @param[in ] step step size of the LUT
    * @param[in ] exact value to find closest index
    * @return closest value in LUT to specified value
    */
    inline double index(double min, double step, double exact)
    { return (exact - min) / step; }

    /**
    * Determines closest index in LUT to specified value
    * @param[in ] lut1 Look up table to find index
    * @param[in ] exact value to find closest index
    * @return closest index to specified value
    */
    template <class Value>
    inline double index(const LUT1<Value> &lut1, double exact)
    { return index(lut1.minX(), lut1.stepX(), exact); }

    /**
    * Determines lowest index in LUT to specified value
    * @param[in ] lut1 Look up table to find index
    * @param[in ] exact value to find closest index
    * @return lowest index to specified value
    */
    template <class Value>
    inline Value lowValue(const LUT1<Value> &lut1, double exact)
    { return lut1(static_cast<size_t>(index(lut1, exact))); }

    /**
    * Determines highest index in LUT to specified value
    * @param[in ] lut1 Look up table to find index
    * @param[in ] exact value to find closest index
    * @return highest index to specified value
    */
    template <class Value>
    inline Value highValue(const LUT1<Value> &lut1, double exact)
    { return lut1(static_cast<size_t>(index(lut1, exact)) + 1); }

    /**
    * Determines nearest index in LUT to specified value
    * @param[in ] lut1 Look up table to find index
    * @param[in ] exact value to find closest index
    * @return nearest index to specified value
    */
    template <class Value>
    inline Value nearValue(const LUT1<Value> &lut1, double exact)
    { return lut1(static_cast<size_t>(index(lut1, exact) + 0.5)); }

    /**
    * Performs interpolation of the LUT using the specified value
    * @param[in ] lut1 Look up table to find index and perform interpolation
    * @param[in ] exact value to find closest index
    * @param[in ] func Linear interpolation function to use
    * @return interpolated LUT value based on specified value
    */
    template <class Value, class Function>
    inline Value interpolate(const LUT1<Value> &lut1, double exact, Function func)
    {
      double dlow = index(lut1, exact);
      if (dlow < 0)
        throw std::out_of_range("simCore::LUT::interpolate");

      size_t low = static_cast<size_t>(dlow);
      double stepX = lut1.stepX();
      if (low == lut1.numX() - 1)
        --low;
      double minX = lut1.minX() + stepX * low;
      return func(lut1(low), lut1(low + 1), minX, exact, minX + stepX);
    }

    /**
    * Writes the LUT to an output stream
    * @param[in ] out Output stream
    * @param[in ] lut1 Look up table to write to output stream
    */
    template <class Value>
    inline std::ostream &operator<<(std::ostream &out, const LUT1<Value> &lut1)
    {
      out << lut1.numX() << " " << lut1.minX() << " " << lut1.stepX() << " " << lut1.maxX() << std::endl;
      for (size_t i = 0; i < lut1.numX(); ++i)
      {
        out << i << " = " << lut1(i) << std::endl;
      }
      return out;
    }

  } // end of namespace LUT
} // end of namespace simCore

#endif /* SIMCORE_CALC_LUT_LUT1_H */
