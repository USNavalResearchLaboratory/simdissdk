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
#ifndef SIMDATA_PROTOBUF_DESCRIPTOR_H
#define SIMDATA_PROTOBUF_DESCRIPTOR_H

/** Utility header to silence common MSVC warnings for protobuf includes. */

#ifdef _MSC_VER // [
#pragma warning( push )
// Disable C4244: "conversion from __int64 to int, possible loss of data"
#pragma warning( disable : 4244 )
#endif // _MSC_VER ]

#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"

#endif /* SIMDATA_PROTOBUF_DESCRIPTOR_H */
