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
#ifndef SIMVIS_DB_SWAPBYTES_H
#define SIMVIS_DB_SWAPBYTES_H

#include <algorithm>
#include <cstring>
#include "simCore/Common/Common.h"

// Try to guess endianness
#if !defined(SIM_LITTLE_ENDIAN) && !defined(SIM_BIG_ENDIAN)
#if defined(X86) || defined(ALPHA) || defined(__x86_64__) || defined(_WIN64) || defined(WIN32)
#define SIM_LITTLE_ENDIAN
#endif
#endif

namespace simVis_db
{
  template <class T>
  inline
  void swapBytes(T *const value, const size_t nItems = 1)
  {
    for (size_t i = 0; i < nItems; ++i)
    {
      char *ptr = (char*)&value[i]; // Treat value as an array of bytes
      const size_t size = sizeof(T);
      const size_t halfSize = size >> 1;
      const size_t end = size - 1;
      for (size_t j = 0; j < halfSize; ++j)
        std::swap(ptr[end - j], ptr[j]);
    }
  }

#ifdef SIM_LITTLE_ENDIAN

  template <class T>
  inline
  void makeBigEndianImpl(T *const value)
  {
    swapBytes(value);
  }

#else

  template <class T>
  inline
  void makeBigEndianImpl(T *const value)
  {
  }

#endif

  inline
  void makeBigEndian(char *const val) { makeBigEndianImpl(val); }
  inline
  void makeBigEndian(int8_t *const val) { makeBigEndianImpl(val); }
  inline
  void makeBigEndian(uint8_t *const val) { makeBigEndianImpl(val); }
  inline
  void makeBigEndian(int16_t *const val) { makeBigEndianImpl(val); }
  inline
  void makeBigEndian(uint16_t *const val) { makeBigEndianImpl(val); }
  inline
  void makeBigEndian(int32_t *const val) { makeBigEndianImpl(val); }
  inline
  void makeBigEndian(uint32_t *const val) { makeBigEndianImpl(val); }
  inline
  void makeBigEndian(int64_t *const val) { makeBigEndianImpl(val); }
  inline
  void makeBigEndian(uint64_t *const val) { makeBigEndianImpl(val); }
  inline
  void makeBigEndian(float *const val) { makeBigEndianImpl(val); }
  inline
  void makeBigEndian(double *const val) { makeBigEndianImpl(val); }

  template <class T>
  inline
  void makeBigEndian(T *const value, const size_t nItems)
  {
    for (size_t i = 0; i < nItems; ++i)
    {
      makeBigEndian(&value[i]);
    }
  }

  template <class T>
  inline
  size_t beRead(const void *const stream, T *const val, const size_t nItems = 1)
  {
    memcpy(val, stream, nItems * sizeof(T));
    makeBigEndian(val, nItems);
    return nItems;
  }

  template <class T>
  inline
  size_t beWrite(void *const stream, const T *const val, const size_t nItems = 1)
  {
    memcpy(stream, val, nItems * sizeof(T));
    makeBigEndian((T *)stream, nItems);
    return nItems;
  }

} // namespace simVis_db

#endif /* SIMVIS_DB_SWAPBYTES_H */
