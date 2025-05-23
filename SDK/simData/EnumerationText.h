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
#ifndef SIMDATA_ENUMERATION_TEXT_H
#define SIMDATA_ENUMERATION_TEXT_H

#include <map>
#include <memory>
#include <string>
#include "simCore/Common/Export.h"

namespace simData
{
/**
 * Class for converting enumeration index into a text string.
 * To make an object first call insert_() follow any number of append_().
 * Call insert_() to skip over numbers.
 */
class SDKDATA_EXPORT EnumerationText
{
public:
  EnumerationText();
  virtual ~EnumerationText();

  /** Return the text for the given index where index starts at 1 */
  std::string text(size_t index) const;

  static std::unique_ptr<EnumerationText> makeBeamTypeName();
  static std::unique_ptr<EnumerationText> makeGateTypeName();
  static std::unique_ptr<EnumerationText> makeCoordinateSystemName();
  static std::unique_ptr<EnumerationText> makeMagneticVarianceName();
  static std::unique_ptr<EnumerationText> makeVerticalDatumName();

private:
  /** Insert the given text at the given index; the insert_() method must be called before the first call to append_(). */
  void insert_(size_t index, const std::string& text);

  /** Append text, the index is generated by incrementing the highest index by one; call insert_() to skip indexes. */
  void append_(const std::string& text);

  std::map<size_t, std::string> text_;
};

}

#endif

