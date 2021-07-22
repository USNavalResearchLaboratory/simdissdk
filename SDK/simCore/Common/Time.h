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
#ifndef SIMCORE_COMMON_TIME_H
#define SIMCORE_COMMON_TIME_H

#include <time.h>
#include <sys/timeb.h>
#include <cassert>

#ifndef WIN32

#include <sys/time.h>
#include <unistd.h>

#define timestruc_to_timespec(a, b) memcpy((a), (b), sizeof(*(a)))

#ifdef __cplusplus
extern "C" {
#endif

  /**
  * Suspends the calling program for a specified period of time
  * @param[in ] milliseconds Number of milliseconds to wait
  */
  inline void Sleep(unsigned int milliseconds)
  {
    usleep(milliseconds * 1000);
  }

#ifdef __cplusplus
}
#endif

#else /* WIN32 */

#include <winsock2.h>

/// winsock2.h defines timeval but not timezone or timespec

/**
 * Equivalent POSIX structure used to hold minimal information about the local time zone
 */
struct timezone
{
  int tz_minuteswest; /**< the number of minutes west of GMT */
  int tz_dsttime;     /**< If nonzero, daylight savings time applies during some part of the year */
};

/// If your software already defines timespec_t, define this macro in your CMake file
#if !defined(DEFINED_TIMESPEC_T)
/**
 * Equivalent POSIX structure that provides a decimal-based fixed-point data format for elapsed time
 */
typedef struct timespec
{
  time_t tv_sec;      /**< number of seconds */
  long   tv_nsec;     /**< number of nanoseconds */
} timespec_t;
#endif


#ifndef timespec_to_timeval
#define timespec_to_timeval(t, ts) \
  (t)->tv_sec = (ts)->tv_sec; \
  (t)->tv_usec = (ts)->tv_nsec / 1000;

#define timeval_to_timespec(ts, t) \
  (ts)->tv_sec = (t)->tv_sec; \
  (ts)->tv_nsec = (t)->tv_usec * 1000;
#endif

#define gettimeofday GetTimeOfDay

/**
* Gets the current date and time of day from host system
* @param[out] t Current time of day
* @param[out] z Current time zone
* @return 0:success, -1:failure
* @pre timeval param valid
*/
inline int GetTimeOfDay(struct timeval *t, struct timezone *z)
{
  assert(t);
  if (!t)
    return -1;

  struct _timeb _gtodtmp;
  _ftime(&_gtodtmp);
  (t)->tv_sec = static_cast<long>(_gtodtmp.time);
  (t)->tv_usec = _gtodtmp.millitm * 1000;

  if (z)
  {
    (z)->tz_minuteswest = _gtodtmp.timezone;
    (z)->tz_dsttime = _gtodtmp.dstflag;
  }

  return 0;
}

#endif /* WIN32 */

#endif /* SIMCORE_COMMON_TIME_H */
