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
#ifndef SIMCORE_CALC_MATH_CONSTANTS_H
#define SIMCORE_CALC_MATH_CONSTANTS_H

// use <math.h> and not <cmath> here to get the Microsoft #defines consistently.
#include <math.h>

#ifdef WIN32
#if !defined(_MATH_DEFINES_DEFINED)

/* Define useful mathematical constants, when they are not defined by Windows.
 * Defining _USE_MATH_DEFINES tells Windows to define them;
 * Windows defines _MATH_DEFINES_DEFINED to indicate that it has defined them.
 */

#ifndef M_E
#define M_E 2.71828182845904523536   /* mathematical constant e */
#endif
#ifndef M_LOG2E
#define M_LOG2E 1.44269504088896340736   /* log2(e) */
#endif
#ifndef M_LOG10E
#define M_LOG10E 0.434294481903251827651  /* log10(e) */
#endif
#ifndef M_LN2
#define M_LN2 0.693147180559945309417  /* ln(2) */
#endif
#ifndef M_LN10
#define M_LN10 2.30258509299404568402  /* ln(10) */
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846  /* mathematical constant pi */
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923  /* pi/2 */
#endif
#ifndef M_PI_4
#define M_PI_4 0.78539816339744830962  /* pi/4 */
#endif
#ifndef M_1_PI
#define M_1_PI 0.31830988618379067154  /* 1/pi */
#endif
#ifndef M_2_PI
#define M_2_PI 0.63661977236758134308  /* 2/pi */
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI 1.12837916709551257390  /* 2/sqrt(pi) */
#endif
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880  /* sqrt(2) */
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2 0.707106781186547524401  /* sqrt(1/2) */
#endif

#endif /* _MATH_DEFINES_DEFINED */
#endif /* WIN32 */


/* Support useful mathematical constants are not defined under windows
 * but are supported under Linux
 */
#ifndef M_LOG102
#define M_LOG102        0.30102999566398119521  /* log10(2) */
#endif
#ifndef M_TWOPI
#define M_TWOPI         6.28318530717958647692  /* 2*pi */
#endif
#ifndef M_3PI_4
#define M_3PI_4         2.3561944901923448370E0 /* 3/4*pi */
#endif
#ifndef M_SQRTPI
#define M_SQRTPI        1.77245385090551602792981 /* sqrt(pi) */
#endif
#ifndef M_LN2LO
#define M_LN2LO         1.9082149292705877000E-10 /* lower bits of log e2 (ln(2) */
#endif
#ifndef M_LN2HI
#define M_LN2HI         6.9314718036912381649E-1  /* ln(2) */
#endif
#ifndef M_SQRT3
#define M_SQRT3         1.73205080756887719000  /* sqrt(3) */
#endif
#ifndef M_IVLN10
#define M_IVLN10        0.43429448190325182765  /* 1/log(10) */
#endif
#ifndef M_LOG2_E
#define M_LOG2_E        0.693147180559945309417 /* ln(2) */
#endif
#ifndef M_INVLN2
#define M_INVLN2        1.4426950408889633870E0 /* 1/ln(2) */
#endif

#endif /* SIMCORE_CALC_MATH_CONSTANTS_H */
