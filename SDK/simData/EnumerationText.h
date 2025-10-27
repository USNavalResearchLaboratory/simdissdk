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

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "simCore/Common/Export.h"

namespace simData
{
/**
 * Class for converting enumeration value into a text string.
 * To make an object first call insert_() follow any number of append_().
 * Call insert_() to skip over numbers.
 *
 * Values are the enumeration value while indexes starts at zero and counts
 * up for each enumeration value.
 */
class SDKDATA_EXPORT EnumerationText
{
public:
  EnumerationText();
  virtual ~EnumerationText();

  /** Return the text for the given value; use visit() to get valid values */
  std::string text(size_t value) const;

  /** Converts an enumeration value into an index; returns std::numeric_limits<size_t>::max() on error */
  size_t valueToIndex(size_t value) const;

  /** Converts a index into an enumeration value; returns std::numeric_limits<size_t>::max() on error */
  size_t indexToValue(size_t index) const;

  /** Call for each value/text pair */
  using VisitorFn = std::function<void(size_t value, const std::string& text)>;

  /** Visit each value/text pair */
  void visit(VisitorFn fn) const;

  static std::unique_ptr<EnumerationText> makeAngleUnitsName();
  static std::unique_ptr<EnumerationText> makeAnimatedLineBendName();
  static std::unique_ptr<EnumerationText> makeAntennaPatternAlgorithmName();
  static std::unique_ptr<EnumerationText> makeAntennaPatternFileFormatName();
  static std::unique_ptr<EnumerationText> makeAntennaPatternTypeName();
  static std::unique_ptr<EnumerationText> makeBackdropImplementationName();
  static std::unique_ptr<EnumerationText> makeBackdropTypeName();
  static std::unique_ptr<EnumerationText> makeBeamDrawModeName();
  static std::unique_ptr<EnumerationText> makeBeamDrawTypeName();
  static std::unique_ptr<EnumerationText> makeBeamTypeName();
  static std::unique_ptr<EnumerationText> makeCircleHilightShapeName();
  static std::unique_ptr<EnumerationText> makeCoordinateSystemName();
  static std::unique_ptr<EnumerationText> makeDistanceUnitsName();
  static std::unique_ptr<EnumerationText> makeDynamicScaleAlgorithmName();
  static std::unique_ptr<EnumerationText> makeElapsedTimeFormatName();
  static std::unique_ptr<EnumerationText> makeFragmentEffectName();
  static std::unique_ptr<EnumerationText> makeGateDrawModeName();
  static std::unique_ptr<EnumerationText> makeGateFillPatternName();
  static std::unique_ptr<EnumerationText> makeGateTypeName();
  static std::unique_ptr<EnumerationText> makeGeodeticUnitsName();
  static std::unique_ptr<EnumerationText> makeIconRotationName();
  static std::unique_ptr<EnumerationText> makeLifespanModeName();
  static std::unique_ptr<EnumerationText> makeLocalGridTypeName();
  static std::unique_ptr<EnumerationText> makeMagneticVarianceName();
  static std::unique_ptr<EnumerationText> makeModelDrawModeName();
  static std::unique_ptr<EnumerationText> makeOverrideColorCombineModeName();
  static std::unique_ptr<EnumerationText> makePlatformDrawOffBehaviorName();
  static std::unique_ptr<EnumerationText> makePolarityName();
  static std::unique_ptr<EnumerationText> makeVolumeTypeName();
  static std::unique_ptr<EnumerationText> makePolygonFaceName();
  static std::unique_ptr<EnumerationText> makePolygonModeName();
  static std::unique_ptr<EnumerationText> makeSpeedUnitsName();
  static std::unique_ptr<EnumerationText> makeTextAlignmentName();
  static std::unique_ptr<EnumerationText> makeTextOutlineName();
  static std::unique_ptr<EnumerationText> makeTimeTickDrawStyleName();
  static std::unique_ptr<EnumerationText> makeTrackModeName();
  static std::unique_ptr<EnumerationText> makeUseValueName();
  static std::unique_ptr<EnumerationText> makeVerticalDatumName();

private:
  /** Insert the given text at the given value; the insert_() method must be called before the first call to append_(). */
  void insert_(size_t value, const std::string& text);

  /** Append text, the value is generated by incrementing the highest value by one; call insert_() to skip indexes. */
  void append_(const std::string& text);

  std::map<size_t, std::string> text_;
  std::vector<size_t> values_;
};

}

#endif

