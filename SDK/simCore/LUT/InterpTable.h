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
#ifndef SIMCORE_CALC_LUT_INTERP_TABLE_H
#define SIMCORE_CALC_LUT_INTERP_TABLE_H

#include "simCore/Calc/Interpolation.h"
#include "LUT2.h"

namespace
{
  /**
  * @brief Helper class for performing bilinear interpolation using () operator
  */
  template <class Type>
  class BilinearInterpolate
  {
  public:
    /**
    * @brief Performs bilinear interpolation between a set of bounded values
    *
    * Performs bilinear interpolation between two sets of bounded values at the specified values
    * @param[in ] ll Lower left bound
    * @param[in ] lr Lower right bound
    * @param[in ] ur Upper right bound
    * @param[in ] ul Upper left bound
    * @param[in ] xLow Low value of X bound
    * @param[in ] xVal value to perform interpolation along X axis
    * @param[in ] xHigh High value of X bound
    * @param[in ] yLow Low value of Y bound
    * @param[in ] yVal value to perform interpolation along Y axis
    * @param[in ] yHigh High value of Y bound
    * @return bilinear interpolated value
    */
    Type operator()(Type ll, Type lr, Type ur, Type ul, double xLow, double xVal, double xHigh, double yLow, double yVal, double yHigh)
    {
      return simCore::bilinearInterpolate(ll, lr, ur, ul, xLow, xVal, xHigh, yLow, yVal, yHigh);
    }
  };
}

namespace simCore
{
  /**
  * @brief Exception handler for look up table interpolation
  */
  class InterpTableException : public std::exception
  {
  public:
    /**
    * InterpTableException constructor
    * @param[in] err error condition
    */
    InterpTableException(const std::string &err) : error_(err) {}
    /** InterpTableException destructor */
    virtual ~InterpTableException() throw() {}

    /** @return InterpTableException error condition */
    virtual const char *what() const throw() { return error_.c_str(); }
  private:
    std::string error_;   /**< error condition */
  };

  /**
  * @brief Exception handler for bounds checking of look up table interpolation
  */
  template <class Type>
  class InterpTableLimitException : public InterpTableException
  {
  public:
    /**
    * InterpTableLimitException constructor
    * @param[in] error error condition
    * @param[in] x requested X dimension index
    * @param[in] y requested Y dimension index
    * @param[in] value storage value type
    */
    InterpTableLimitException(const std::string &error, int x, int y, const Type &value)
      : InterpTableException(error), x_(x), y_(y), value_(value) {}
    /** InterpTableLimitException destructor */
    virtual ~InterpTableLimitException() throw() {}

  private:
    int x_;       /**< requested X dimension index */
    int y_;       /**< requested Y dimension index */
    Type value_;  /**< storage value type */
  };

  /**
  * @brief Utility class for storing a two dimensional look up table to be used for interpolation
  */
  template <class Type>
  class InterpTable
  {
  public:
    /**
    * @brief Initializes size of internal two dimensional look up table
    *
    * @param[in ] minX Minimum x value
    * @param[in ] maxX Maximum x value
    * @param[in ] numX Number of x values
    * @param[in ] minY Minimum y value
    * @param[in ] maxY Maximum y value
    * @param[in ] numY Number of y values
    */
    void initialize(double minX, double maxX, size_t numX, double minY, double maxY, size_t numY)
    {
      lut_.initialize(minX, maxX, numX, minY, maxY, numY);
    }

    /**
    * @return Const reference to the internal two dimensional look up table
    */
    const LUT::LUT2<Type> &lut() const { return lut_; }

    /**
    * @brief Performs look up of requested indices
    *
    * @param[in ] xIndex X index value
    * @param[in ] yIndex Y index value
    * @return Const reference to the value found in the look up table
    */
    const Type &operator()(size_t xIndex, size_t yIndex) const { return lut_(xIndex, yIndex); }

    /**
    * @brief Performs look up of requested indices
    *
    * @param[in ] xIndex X index value
    * @param[in ] yIndex Y index value
    * @return Reference to the value found in the look up table
    */
    Type &operator()(size_t xIndex, size_t yIndex) { return lut_(xIndex, yIndex); }
  protected:

  private:
    LUT::LUT2<Type> lut_; /**< two dimensional look up table */
  };

  /**
  * @brief Performs bilinear interpolation of a two dimensional look up table
  *
  * @param[in ] table Table to perform bilinear interpolation
  * @param[in ] x X value to look up
  * @param[in ] y Y value to look up
  * @return interpolated value
  * @throw InterpTableLimitException
  */
  template <class Type>
  inline Type BilinearLookup(const InterpTable<Type> &table, double x, double y)
  {
    int xError = 0;
    int yError = 0;
    const LUT::LUT2< Type > &lut = table.lut();
    BilinearInterpolate<Type > bil;
    if (x > lut.maxX())
    {
      x = lut.maxX();
      xError = 1;
    }
    else if (x < lut.minX())
    {
      x = lut.minX();
      xError = -1;
    }

    if (y > lut.maxY())
    {
      y = lut.maxY();
      yError = 1;
    }
    else if (y < lut.minY())
    {
      y = lut.minY();
      yError = -1;
    }
    if (xError || yError)
    {
      Type rv = LUT::interpolate(lut, x, y, bil);
      throw InterpTableLimitException<Type>("", xError, yError, rv);
    }
    return LUT::interpolate(lut, x, y, bil);
  }

  /**
  * @brief Performs bilinear interpolation of a two dimensional look up table
  *
  * @param[in ] table Table to perform bilinear interpolation
  * @param[in ] x X value to look up
  * @param[in ] y Y value to look up
  * @return interpolated value
  */
  template <class Type>
  inline Type BilinearLookupNoException(const InterpTable<Type> &table, double x, double y)
  {
    const LUT::LUT2< Type > &lut = table.lut();
    BilinearInterpolate< Type > bil;
    if (x > lut.maxX())
      x = lut.maxX();
    else if (x < lut.minX())
      x = lut.minX();

    if (y > lut.maxY())
      y = lut.maxY();
    else if (y < lut.minY())
      y = lut.minY();

    return LUT::interpolate(lut, x, y, bil);
  }

  /**
  * @brief Performs nearest neighbor of a two dimensional look up table
  *
  * @param[in ] table Table to perform nearest neighbor interpolation
  * @param[in ] x X value to look up
  * @param[in ] y Y value to look up
  * @return interpolated value
  * @throw InterpTableLimitException
  */
  template <class Type>
  inline Type NearestLookup(const InterpTable<Type> &table, double x, double y)
  {
    int xError = 0;
    int yError = 0;
    const LUT::LUT2<Type > &lut = table.lut();
    if (x > lut.maxX())
    {
      x = lut.maxX();
      xError = 1;
    }
    else if (x < lut.minX())
    {
      x = lut.minX();
      xError = -1;
    }

    if (y > lut.maxY())
    {
      y = lut.maxY();
      yError = 1;
    }
    else if (y < lut.minY())
    {
      y = lut.minY();
      yError = -1;
    }
    if (xError || yError)
    {
      Type rv = nearValue(lut, x, y);
      throw InterpTableLimitException<Type>("", xError, yError, rv);
    }
    return nearValue(lut, x, y);
  }

} // end of namespace

#endif /* SIMCORE_CALC_LUT_INTERP_TABLE_H */
