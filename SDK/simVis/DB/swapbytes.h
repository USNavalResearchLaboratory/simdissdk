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
#ifndef SIMVIS_DB_SWAPBYTES_H
#define SIMVIS_DB_SWAPBYTES_H

#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "simCore/Common/Common.h"

#if !defined(SIM_LITTLE_ENDIAN) && !defined(SIM_BIG_ENDIAN)
#if defined(X86) || defined(ALPHA) || defined(__x86_64__) || defined(_WIN64) || defined(WIN32)
#define SIM_LITTLE_ENDIAN
#endif
#endif

#ifndef BYTESWAPSHIFT

namespace simVis_db
{
  template <class T>
  inline
  void swap_bytes(T *const value, const size_t nItems = 1)
  {
  //   using namespace std;
    for (size_t i = 0; i < nItems; ++i)
    {
      char *ptr = (char*)&value[i]; // Treat value as an array of bytes
      size_t size = sizeof(T);
      size_t halfSize = size >> 1;
      size_t end = size - 1;
      for (size_t i = 0; i < halfSize; ++i)
        std::swap(ptr[end - i], ptr[i]);
    }
  }

  template <class T>
  inline
  T swap_bytes_return(const T &value, const size_t nItems = 1)
  {
    T temp = value;
    swap_bytes(&temp);
    return temp;
  }

#ifdef SIM_LITTLE_ENDIAN

  template <class T>
  inline
  void make_little_endian_(T *const value)
  {
  }

  template <class T>
  inline
  void make_big_endian_(T *const value)
  {
    swap_bytes(value);
  }

#else

  template <class T>
  inline
  void make_little_endian_(T *const value)
  {
    swap_bytes(value);
  }

  template <class T>
  inline
  void make_big_endian_(T *const value)
  {
  }

#endif

  inline
  void make_big_endian(char *const val) { make_big_endian_(val); }
  inline
  void make_big_endian(int8_t *const val) { make_big_endian_(val); }
  inline
  void make_big_endian(uint8_t *const val) { make_big_endian_(val); }
  inline
  void make_big_endian(int16_t *const val) { make_big_endian_(val); }
  inline
  void make_big_endian(uint16_t *const val) { make_big_endian_(val); }
  inline
  void make_big_endian(int32_t *const val) { make_big_endian_(val); }
  inline
  void make_big_endian(uint32_t *const val) { make_big_endian_(val); }
  inline
  void make_big_endian(int64_t *const val) { make_big_endian_(val); }
  inline
  void make_big_endian(uint64_t *const val) { make_big_endian_(val); }
  inline
  void make_big_endian(float *const val) { make_big_endian_(val); }
  inline
  void make_big_endian(double *const val) { make_big_endian_(val); }

  inline
  void make_little_endian(char *const val) { make_little_endian_(val); }
  inline
  void make_little_endian(int8_t *const val) { make_little_endian_(val); }
  inline
  void make_little_endian(uint8_t *const val) { make_little_endian_(val); }
  inline
  void make_little_endian(int16_t *const val) { make_little_endian_(val); }
  inline
  void make_little_endian(uint16_t *const val) { make_little_endian_(val); }
  inline
  void make_little_endian(int32_t *const val) { make_little_endian_(val); }
  inline
  void make_little_endian(uint32_t *const val) { make_little_endian_(val); }
  inline
  void make_little_endian(int64_t *const val) { make_little_endian_(val); }
  inline
  void make_little_endian(uint64_t *const val) { make_little_endian_(val); }
  inline
  void make_little_endian(float *const val) { make_little_endian_(val); }
  inline
  void make_little_endian(double *const val) { make_little_endian_(val); }

  template <class T>
  inline
  void make_big_endian(T *const value, const size_t nItems)
  {
    for (size_t i = 0; i < nItems; ++i)
    {
      make_big_endian(&value[i]);
    }
  }

  template <class T>
  inline
  void make_little_endian(T *const value, const size_t nItems)
  {
    for (size_t i = 0; i < nItems; ++i)
    {
      make_little_endian(&value[i]);
    }
  }

  template <class T>
  inline
  size_t beread(FILE *stream, T *const val, const size_t nItems = 1)
  {
    size_t nItemsRead = fread(val, sizeof(T), nItems, stream);
    make_big_endian(val, nItemsRead);
    return nItemsRead;
  }

  template <class T>
  inline
  size_t bewrite(FILE *stream, const T *const val, const size_t nItems = 1)
  {
    size_t nItemsWrote = 0;
    for (size_t i = 0; i < nItems; ++i)
    {
      T temp = val[i];
      make_big_endian(&temp);
      if (fwrite(&temp, sizeof(T), 1, stream) != 1)
        break;
      ++nItemsWrote;
    }
    return nItemsWrote;
  }

  template <class T>
  inline
  size_t leread(FILE *stream, T *const val, const size_t nItems = 1)
  {
    size_t nItemsRead = fread(val, sizeof(T), nItems, stream);
    make_little_endian(val, nItemsRead);
    return nItemsRead;
  }

  template <class T>
  inline
  size_t lewrite(FILE *stream, const T *const val, const size_t nItems = 1)
  {
    size_t nItemsWrote = 0;
    for (size_t i = 0; i < nItems; ++i)
    {
      T temp = val[i];
      make_little_endian(&temp);
      if (fwrite(&temp, sizeof(T), 1, stream) != 1)
        break;
      ++nItemsWrote;
    }
    return nItemsWrote;
  }

  template <class T>
  inline
  size_t beread(std::istream &stream, T *const val, const std::streamsize nItems = 1)
  {
    stream.read((char *)val, nItems * sizeof(T));
    size_t nItemsRead = stream.gcount() / sizeof(T);
    make_big_endian(val, nItemsRead);
    return nItemsRead;
  }

  template <class T>
  inline
  size_t bewrite(std::ostream &stream, const T *const val, const size_t nItems = 1)
  {
    size_t nItemsWrote = 0;
    for (size_t i = 0; i < nItems; ++i)
    {
      T temp = val[i];
      make_big_endian(&temp);
      stream.write((const char *)&temp, sizeof(T));
      if (stream.bad())
        break;
      ++nItemsWrote;
    }
    return nItemsWrote;
  }

  template <class T>
  inline
  size_t leread(std::istream &stream, T *const val, const size_t nItems = 1)
  {
    stream.read((char *)val, nItems * sizeof(T));
    size_t nItemsRead = stream.gcount() / sizeof(T);
    make_little_endian(val, nItemsRead);
    return nItemsRead;
  }

  template <class T>
  inline
  size_t lewrite(std::ostream &stream, const T *const val, const size_t nItems = 1)
  {
    size_t nItemsWrote = 0;
    for (size_t i = 0; i < nItems; ++i)
    {
      T temp = val[i];
      make_little_endian(&temp);
      stream.write((const char *)&temp, sizeof(T));
      if (stream.bad())
        break;
      ++nItemsWrote;
    }
    return nItemsWrote;
  }

  template <class T>
  inline
  size_t beread(const void *const stream, T *const val, const size_t nItems = 1)
  {
    memcpy(val, stream, nItems * sizeof(T));
    make_big_endian(val, nItems);
    return nItems;
  }

  template <class T>
  inline
  size_t bewrite(void *const stream, const T *const val, const size_t nItems = 1)
  {
    memcpy(stream, val, nItems * sizeof(T));
    make_big_endian((T *)stream, nItems);
    return nItems;
  }

  template <class T>
  inline
  size_t leread(const void *const stream, T *const val, const size_t nItems = 1)
  {
    memcpy(val, stream, nItems * sizeof(T));
    make_little_endian(val, nItems);
    return nItems;
  }

  template <class T>
  inline
  size_t lewrite(void *const stream, const T *const val, const size_t nItems = 1)
  {
    memcpy(stream, val, nItems * sizeof(T));
    make_little_endian((T *)stream, nItems);
    return nItems;
  }

#else

// Swap macros.
#define SWAP16(val)((((val)>>8)&0xff)|(((val)<<8)&0xff00))
#define SWAP32(val)((((val)>>24)&0xff)|(((val)>>8)&0xff00)|(((val)<<8)&0xff0000)|(((val)<<24)&0xff000000))
#ifdef __GNUC__
#define SWAP64(val)((((val)>>56)&0xffLL)|(((val)>>40)&0xff00LL)|(((val)>>24)&0xff0000LL)|(((val)>>8)&0xff000000LL)| \
  (((val)<<8)&0xff00000000LL)|(((val)<<24)&0xff0000000000LL)|(((val)<<40)&0xff000000000000LL)|(((val)<<56)&0xff00000000000000LL))
#else
#define SWAP64(val)((((val)>>56)&0xffL)|(((val)>>40)&0xff00L)|(((val)>>24)&0xff0000L)|(((val)>>8)&0xff000000L)| \
  (((val)<<8)&0xff00000000L)|(((val)<<24)&0xff0000000000L)|(((val)<<40)&0xff000000000000L)|(((val)<<56)&0xff00000000000000L))
#endif

  // Generic byte swapping routines.
  template<class T> inline void swap_bytes(T *const value)
  {
    char *ptr=(char*)(void*)value; // Treat value as an array of bytes
    size_t size = sizeof(T);
    register size_t halfSize = size >> 1;
    register size_t end = size - 1;
    for (register size_t i = 0; i < halfSize; ++i)
      std::swap(ptr[end - i], ptr[i]);
  }

  template<class T> inline void swap_bytes(T *const value, register const size_t nItems)
  {
    for (register size_t i=0;i<nItems;++i)
    {
      char *ptr=(char*)(void*)&value[i]; // Treat value as an array of bytes
      size_t size = sizeof(T);
      register size_t halfSize = size >> 1;
      register size_t end = size - 1;
      for (register size_t i = 0; i < halfSize; ++i)
        std::swap(ptr[end - i], ptr[i]);
    }
  }


  // Specialized routines.

  // NULL routines.
  template<> inline void swap_bytes(char *const value) {}
  template<> inline void swap_bytes(char *const value, register const size_t nItems) {}
  template<> inline void swap_bytes(int8_t *const value) {}
  template<> inline void swap_bytes(uint8_t *const value) {}
  template<> inline void swap_bytes(int8_t *const value, register const size_t nItems) {}
  template<> inline void swap_bytes(uint8_t *const value, register const size_t nItems) {}

  // Single item routines.
  template<> inline void swap_bytes(int16_t *const value) { *value=SWAP16(*value); }
  template<> inline void swap_bytes(uint16_t *const value) { *value=SWAP16(*value); }
  template<> inline void swap_bytes(int32_t *const value) { *value=SWAP32(*value); }
  template<> inline void swap_bytes(uint32_t *const value) { *value=SWAP32(*value); }
  template<> inline void swap_bytes(int64_t *const value) { *value=SWAP64(*value); }
  template<> inline void swap_bytes(uint64_t *const value) { *value=SWAP64(*value); }

  template<> inline void swap_bytes(float *const value)
  {
    int32_t *const pseudo=(int32_t*)(void*)value;
    *pseudo=SWAP32(*pseudo);
  }

  template<> inline void swap_bytes(double *const value)
  {
    int64_t *const pseudo=(int64_t*)(void*)value;
    *pseudo=SWAP64(*pseudo);
  }

  #ifdef SIM_LITTLE_ENDIAN
  template<class T> inline void make_little_endian(T *const value) {}

  // Generic swap.  Has a '_' prefix to prevent it from accidently being used with structs, etc.
  template<class T> inline void make_little_endian_(T *const value) {}
  template<class T> inline void make_big_endian_(T *const value) { swap_bytes(value); }

  inline void make_big_endian(char *const value) {}
  inline void make_big_endian(int8_t *const value) {}
  inline void make_big_endian(uint8_t *const value) {}
  inline void make_big_endian(int16_t *const value) { *value=SWAP16(*value); }
  inline void make_big_endian(uint16_t *const value) { *value=SWAP16(*value); }
  inline void make_big_endian(int32_t *const value) { *value=SWAP32(*value); }
  inline void make_big_endian(uint32_t *const value) { *value=SWAP32(*value); }
  inline void make_big_endian(int64_t *const value) { *value=SWAP64(*value); }
  inline void make_big_endian(uint64_t *const value) { *value=SWAP64(*value); }

  inline void make_big_endian(float *const value)
  {
    int32_t *const pseudo=(int32_t*)(void*)value;
    *pseudo=SWAP32(*pseudo);
  }

  inline void make_big_endian(double *const value)
  {
    int64_t *const pseudo=(int64_t*)(void*)value;
    *pseudo=SWAP64(*pseudo);
  }
#else
  template<class T> inline void make_big_endian(T *const value) {}
  template<class T> inline void make_big_endian_(T *const value) {}
  template<class T> inline void make_little_endian_(T *const value) { swap_bytes(value); }

  inline void make_little_endian(char *const value) {}
  inline void make_little_endian(int8_t *const value) {}
  inline void make_little_endian(uint8_t *const value) {}
  inline void make_little_endian(int16_t *const value) { *value=SWAP16(*value); }
  inline void make_little_endian(uint16_t *const value) { *value=SWAP16(*value); }
  inline void make_little_endian(int32_t *const value) { *value=SWAP32(*value); }
  inline void make_little_endian(uint32_t *const value) { *value=SWAP32(*value); }
  inline void make_little_endian(int64_t *const value) { *value=SWAP64(*value); }
  inline void make_little_endian(uint64_t *const value) { *value=SWAP64(*value); }

  inline void make_little_endian(float *const value)
  {
    int32_t *const pseudo=(int32_t*)(void*)value;
    *pseudo=SWAP32(*pseudo);
  }

  inline void make_little_endian(double *const value)
  {
    int64_t *const pseudo=(int64_t*)(void*)value;
    *pseudo=SWAP64(*pseudo);
  }
#endif

  // Multi item routines.
  template<> inline void swap_bytes(int16_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP16(value[i]);
  }

  template<> inline void swap_bytes(uint16_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP16(value[i]);
  }

  template<> inline void swap_bytes(int32_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP32(value[i]);
  }

  template<> inline void swap_bytes(uint32_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP32(value[i]);
  }

  template<> inline void swap_bytes(int64_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP64(value[i]);
  }

  template<> inline void swap_bytes(uint64_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP64(value[i]);
  }

  template<> inline void swap_bytes(float *const value, register const size_t nItems)
  {
    int32_t *const pseudo=(int32_t*)(void*)value;
    for (register size_t i = 0; i < nItems; ++i) pseudo[i] = SWAP32(pseudo[i]);
  }

  template<> inline void swap_bytes(double *const value, register const size_t nItems)
  {
    int64_t *const pseudo=(int64_t*)(void*)value;
    for (register size_t i = 0; i < nItems; ++i) pseudo[i] = SWAP64(pseudo[i]);
  }

#ifdef SIM_LITTLE_ENDIAN
  template<class T> inline void make_little_endian(T *const value, register const size_t nItems) {}
  template<class T> inline void make_little_endian_(T *const value, register const size_t nItems) {}
  template<class T> inline void make_big_endian_(T *const value, register const size_t nItems) { swap_bytes(value, nItems); }

  inline void make_big_endian(char *const value, register const size_t nItems) {}
  inline void make_big_endian(int8_t *const value, register const size_t nItems) {}
  inline void make_big_endian(uint8_t *const value, register const size_t nItems) {}

  inline void make_big_endian(int16_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP16(value[i]);
  }

  inline void make_big_endian(uint16_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP16(value[i]);
  }

  inline void make_big_endian(int32_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP32(value[i]);
  }

  inline void make_big_endian(uint32_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP32(value[i]);
  }

  inline void make_big_endian(int64_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP64(value[i]);
  }

  inline void make_big_endian(uint64_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP64(value[i]);
  }

  inline void make_big_endian(float *const value, register const size_t nItems)
  {
    int32_t *const pseudo = (int32_t*)(void*)value;
    for (register size_t i = 0; i < nItems; ++i) pseudo[i] = SWAP32(pseudo[i]);
  }

  inline void make_big_endian(double *const value, register const size_t nItems)
  {
    int64_t *const pseudo = (int64_t*)(void*)value;
    for (register size_t i = 0; i < nItems; ++i) pseudo[i] = SWAP64(pseudo[i]);
  }
#else
  template<class T> inline void make_big_endian(T *const value, register const size_t nItems) {}
  template<class T> inline void make_big_endian_(T *const value, register const size_t nItems) {}
  template<class T> inline void make_little_endian_(T *const value, register const size_t nItems) { swap_bytes(value, nItems); }

  inline void make_little_endian(char *const value, register const size_t nItems) {}
  inline void make_little_endian(int8_t *const value, register const size_t nItems) {}
  inline void make_little_endian(uint8_t *const value, register const size_t nItems) {}

  inline void make_little_endian(int16_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP16(value[i]);
  }

  inline void make_little_endian(uint16_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP16(value[i]);
  }

  inline void make_little_endian(int32_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP32(value[i]);
  }

  inline void make_little_endian(uint32_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP32(value[i]);
  }

  inline void make_little_endian(int64_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP64(value[i]);
  }

  inline void make_little_endian(uint64_t *const value, register const size_t nItems)
  {
    for (register size_t i = 0; i < nItems; ++i) value[i] = SWAP64(value[i]);
  }

  inline void make_little_endian(float *const value, register const size_t nItems)
  {
    int32_t *const pseudo = (int32_t*)(void*)value;
    for (register size_t i = 0; i < nItems; ++i) pseudo[i] = SWAP32(pseudo[i]);
  }

  inline void make_little_endian(double *const value, register const size_t nItems)
  {
    int64_t *const pseudo = (int64_t*)(void*)value;
    for (register size_t i = 0; i < nItems; ++i) pseudo[i] = SWAP64(pseudo[i]);
  }
  #endif


  // Swapped I\O
  template <class T> inline size_t beread(FILE *stream, T *const val)
  {
    size_t nItemsRead = fread(val, sizeof(T), 1, stream);
    make_big_endian(val);
    return nItemsRead;
  }

  template <class T> inline size_t beread(FILE *stream, T *const val, register const size_t nItems)
  {
    size_t nItemsRead = fread(val, sizeof(T), nItems, stream);
    make_big_endian(val, nItemsRead);
    return nItemsRead;
  }

  template <class T> inline size_t bewrite(FILE *stream, const T *const val)
  {
    T temp = *val;
    make_big_endian(&temp);
    return fwrite(&temp, sizeof(T), 1, stream);
  }

  template <class T> inline size_t bewrite(FILE *stream, const T *const val, register const size_t nItems)
  {
    register size_t nItemsWrit = 0;
    while (nItemsWrit < nItems)
    {
      T temp = val[nItemsWrit++];
      make_big_endian(&temp);
      if (fwrite(&temp, sizeof(T), 1, stream)!=1)
        break;
    }
    return nItemsWrit;
  }

  template <class T> inline size_t leread(FILE *stream, T *const val)
  {
    size_t nItemsRead = fread(val, sizeof(T), 1, stream);
    make_little_endian(val);
    return nItemsRead;
  }

  template <class T> inline size_t leread(FILE *stream, T *const val, register const size_t nItems)
  {
    size_t nItemsRead = fread(val, sizeof(T), nItems, stream);
    make_little_endian(val, nItemsRead);
    return nItemsRead;
  }

  template <class T> inline size_t lewrite(FILE *stream, const T *const val)
  {
    T temp = *val;
    make_little_endian(&temp);
    return fwrite(&temp, sizeof(T), 1, stream);
  }

  template <class T> inline size_t lewrite(FILE *stream, const T *const val, register const size_t nItems)
  {
    register size_t nItemsWrit = 0;
    while (nItemsWrit < nItems)
    {
      T temp = val[nItemsWrit++];
      make_little_endian(&temp);
      if (fwrite(&temp, sizeof(T), 1, stream) != 1)
        break;
    }
    return nItemsWrit;
  }

  template <class T> inline size_t beread(std::istream &stream, T *const val)
  {
    stream.read((char *)val, sizeof(T));
    size_t nItemsRead = stream.gcount() / sizeof(T);
    make_big_endian(val);
    return nItemsRead;
  }

  template <class T> inline size_t beread(std::istream &stream, T *const val, register const size_t nItems)
  {
    stream.read((char *)val, nItems * sizeof(T));
    size_t nItemsRead = stream.gcount() / sizeof(T);
    make_big_endian(val, nItemsRead);
    return nItemsRead;
  }

  template <class T> inline size_t bewrite(std::ostream &stream, const T *const val)
  {
    T temp = *val;
    make_big_endian(&temp);
    stream.write((const char *)&temp, sizeof(T));
    return (stream.bad()) ? 0 : 1;
  }

  template <class T> inline size_t bewrite(std::ostream &stream, const T *const val, register const size_t nItems)
  {
    register size_t nItemsWrit = 0;
    while (nItemsWrit < nItems)
    {
      T temp = val[nItemsWrit++];
      make_big_endian(&temp);
      stream.write((const char *)&temp, sizeof(T));
      if (stream.bad())
        break;
    }
    return nItemsWrit;
  }

  template <class T> inline size_t leread(std::istream &stream, T *const val)
  {
    stream.read((char *)val, sizeof(T));
    size_t nItemsRead = stream.gcount()/sizeof(T);
    make_little_endian(val);
    return nItemsRead;
  }

  template <class T> inline size_t leread(std::istream &stream, T *const val, register const size_t nItems)
  {
    stream.read((char *)val, nItems * sizeof(T));
    size_t nItemsRead = stream.gcount() / sizeof(T);
    make_little_endian(val, nItemsRead);
    return nItemsRead;
  }

  template <class T> inline size_t lewrite(std::ostream &stream, const T *const val)
  {
    T temp = *val;
    make_little_endian(&temp);
    stream.write((const char *)&temp, sizeof(T));
    return (stream.bad()) ? 0 : 1;
  }

  template <class T> inline size_t lewrite(std::ostream &stream, const T *const val, register const size_t nItems)
  {
    size_t nItemsWrit = 0;
    while (nItemsWrit < nItems)
    {
      T temp = val[nItemsWrit++];
      make_little_endian(&temp);
      stream.write((const char *)&temp, sizeof(T));
      if (stream.bad())
        break;
    }
    return nItemsWrit;
  }

  template <class T> inline size_t beread(const void *const stream, T *const val)
  {
    memcpy(val, stream, sizeof(T));
    make_big_endian(val);
    return 1;
  }

  template <class T> inline size_t beread(const void *const stream, T *const val, register const size_t nItems)
  {
    memcpy(val, stream, nItems * sizeof(T));
    make_big_endian(val, nItems);
    return nItems;
  }

  template <class T> inline size_t bewrite(void *const stream, const T *const val)
  {
    memcpy(stream, val, sizeof(T));
    make_big_endian((T *)stream);
    return 1;
  }

  template <class T> inline size_t bewrite(void *const stream, const T *const val, register const size_t nItems)
  {
    memcpy(stream, val, nItems * sizeof(T));
    make_big_endian((T *)stream, nItems);
    return nItems;
  }

  template <class T> inline size_t leread(const void *const stream, T *const val)
  {
    memcpy(val, stream, sizeof(T));
    make_little_endian(val);
    return 1;
  }

  template <class T> inline size_t leread(const void *const stream, T *const val, register const size_t nItems)
  {
    memcpy(val, stream, nItems * sizeof(T));
    make_little_endian(val, nItems);
    return nItems;
  }

  template <class T> inline size_t lewrite(void *const stream, const T *const val)
  {
    memcpy(stream, val, sizeof(T));
    make_little_endian((T *)stream);
    return 1;
  }

  template <class T> inline size_t lewrite(void *const stream, const T *const val, register const size_t nItems)
  {
    memcpy(stream, val, nItems * sizeof(T));
    make_little_endian((T *)stream, nItems);
    return nItems;
  }

#endif

} // namespace simVis_db

#endif /* SIMVIS_DB_SWAPBYTES_H */
