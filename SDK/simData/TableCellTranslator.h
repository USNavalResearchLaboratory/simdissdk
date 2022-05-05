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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMDATA_TABLECELLTRANSLATOR_H
#define SIMDATA_TABLECELLTRANSLATOR_H

#include <string>
#include <sstream>
#include <cstdlib>
#include "simCore/Common/Common.h"

namespace simData
{

/**
 * Helper class to easily convert between one data type and another data type using the
 * cast() function.  For example:
 *
 * <code>
 * double value = 5;
 * std::string strValue;
 * TableCellTranslator::cast(value, strValue);
 * std::cout << strValue << " should equal '5'" << std::endl;
 * </code>
 *
 * The usefulness of this class becomes apparent when mixing it with template types:
 *
 * <code>
 * template <typename OutType, typename InType)
 * void saveValue(const InType& in) {
 *   OutType out;
 *   TableCellTranslator::cast(in, out);
 *   internalVec_.push_back(out);
 * }
 * </code>
 *
 */
class TableCellTranslator
{
public:
  /** Responsible for converting string to uint8_t */
  static void cast(const std::string& a, uint8_t& b) {b = (uint8_t)atoi(a.c_str());}
  /** Responsible for converting string to int8_t */
  static void cast(const std::string& a, int8_t& b) {b = (int8_t)atoi(a.c_str());}
  /** Responsible for converting string to uint16_t */
  static void cast(const std::string& a, uint16_t& b) {TableCellTranslator::fromString_(a, &b);}
  /** Responsible for converting string to int16_t */
  static void cast(const std::string& a, int16_t& b) {TableCellTranslator::fromString_(a, &b);}
  /** Responsible for converting string to uint32_t */
  static void cast(const std::string& a, uint32_t& b) {TableCellTranslator::fromString_(a, &b);}
  /** Responsible for converting string to int32_t */
  static void cast(const std::string& a, int32_t& b) {TableCellTranslator::fromString_(a, &b);}
  /** Responsible for converting string to uint64_t */
  static void cast(const std::string& a, uint64_t& b) {TableCellTranslator::fromString_(a, &b);}
  /** Responsible for converting string to int64_t */
  static void cast(const std::string& a, int64_t& b) {TableCellTranslator::fromString_(a, &b);}
  /** Responsible for converting string to float */
  static void cast(const std::string& a, float& b) {TableCellTranslator::fromString_(a, &b);}
  /** Responsible for converting string to double */
  static void cast(const std::string& a, double& b) {TableCellTranslator::fromString_(a, &b);}

  /** Responsible for converting uint8_t to string */
  static void cast(uint8_t a, std::string& b) {TableCellTranslator::toString_(static_cast<int>(a), &b);}
  /** Responsible for converting int8_t to string */
  static void cast(int8_t a, std::string& b) {TableCellTranslator::toString_(static_cast<int>(a), &b);}
  /** Responsible for converting uint16_t to string */
  static void cast(uint16_t a, std::string& b) {TableCellTranslator::toString_(a, &b);}
  /** Responsible for converting int16_t to string */
  static void cast(int16_t a, std::string& b) {TableCellTranslator::toString_(a, &b);}
  /** Responsible for converting uint32_t to string */
  static void cast(uint32_t a, std::string& b) {TableCellTranslator::toString_(a, &b);}
  /** Responsible for converting int32_t to string */
  static void cast(int32_t a, std::string& b) {TableCellTranslator::toString_(a, &b);}
  /** Responsible for converting uint64_t to string */
  static void cast(uint64_t a, std::string& b) {TableCellTranslator::toString_(a, &b);}
  /** Responsible for converting int64_t to string */
  static void cast(int64_t a, std::string& b) {TableCellTranslator::toString_(a, &b);}
  /** Responsible for converting float to string */
  static void cast(float a, std::string& b) {TableCellTranslator::toString_(a, &b);}
  /** Responsible for converting double to string */
  static void cast(double a, std::string& b) {TableCellTranslator::toString_(a, &b);}

  /** Template method for doing all other numeric conversions between AType and BType */
  template <class AType, class BType>
  static void cast(const AType& a, BType& b)
  {
    b = static_cast<BType>(a);
  }

private:
  /** Helper function to use stringstream to convert string to value */
  template<typename T>
  static void fromString_(const std::string& str, T* val)
  {
    *val = 0; // Covers string without value case
    std::stringstream ss(str);
    ss >> *val;
  }

  static void toString_(uint8_t val, std::string* str) { *str = std::to_string(val); }
  static void toString_(int8_t val, std::string* str) { *str = std::to_string(val); }
  static void toString_(uint16_t val, std::string* str) { *str = std::to_string(val); }
  static void toString_(int16_t val, std::string* str) { *str = std::to_string(val); }
  static void toString_(uint32_t val, std::string* str) { *str = std::to_string(val); }
  static void toString_(int32_t val, std::string* str) { *str = std::to_string(val); }
  static void toString_(uint64_t val, std::string* str) { *str = std::to_string(val); }
  static void toString_(int64_t val, std::string* str) { *str = std::to_string(val); }
  static void toString_(float val, std::string* str) { toStringFloat_(val, str); }
  static void toString_(double val, std::string* str) { toStringFloat_(val, str); }
  static void toString_(const std::string& val, std::string* str) { *str = val; }

  /** Helper function to use stringstream to convert value to string */
  template<typename T>
  static void toStringFloat_(T val, std::string* str)
  {
    std::stringstream ss;
    ss.setf(std::ios::fixed, std::ios::floatfield);
    ss << val;
    *str = ss.str();
  }
};

}

#endif /* SIMDATA_TABLECELLTRANSLATOR_H */
