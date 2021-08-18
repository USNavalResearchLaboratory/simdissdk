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
#ifndef SIMUTIL_DATUMCONVERT_H
#define SIMUTIL_DATUMCONVERT_H

#include "osg/ref_ptr"
#include "simCore/Common/Common.h"
#include "simCore/Calc/DatumConvert.h"

#ifdef _MSC_VER // [
#pragma warning(push)
// Disable C4275: non-DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
#pragma warning(disable : 4275)
#endif // _MSC_VER ]


namespace osgEarth { class VerticalDatum; }

namespace simUtil {

/** Fully featured datum convert that uses WMM code and osgEarth vertical datum conversions */
class SDKUTIL_EXPORT DatumConvert : public simCore::DatumConvert
{
public:
  /** Initializes the datum providers */
  DatumConvert();
  virtual ~DatumConvert();

  /// Converts Magnetic Datum
  virtual double convertMagneticDatum(const simCore::Vec3& lla, const simCore::TimeStamp& timeStamp, double bearingRad,
    simCore::CoordinateSystem coordSystem, simCore::MagneticVariance inputDatum, simCore::MagneticVariance outputDatum,
    double userOffset) const;

  /// Converts Vertical Datum
  virtual double convertVerticalDatum(const simCore::Vec3& lla, const simCore::TimeStamp& timeStamp, simCore::CoordinateSystem coordSystem,
    simCore::VerticalDatum inputDatum, simCore::VerticalDatum outputDatum, double userOffset);

  /**
   * Pre-loads all vertical datum libraries.  This may take a few moments as the data is converted into
   * a height field.  This can be useful to avoid hiccups when loading data on demand.  This is an optional
   * call, and if not called, the individual vertical datum plugins will be loaded on demand.
   * @return 0 if all vertical datum are correctly loaded; non-zero means a vertical datum did not load
   *   correctly.
   */
  int preloadVerticalDatum();

private:
  /// Loads EGM 1984 data; returns 0 if successfully loaded
  int load84_();
  /// Loads EGM 1996 data; returns 0 if successfully loaded
  int load96_();
  /// Loads EGM 2008 data; returns 0 if successfully loaded
  int load2008_();

  simCore::WorldMagneticModel* wmm_;
  osg::ref_ptr<osgEarth::VerticalDatum> egm84_;
  bool loaded84_;
  osg::ref_ptr<osgEarth::VerticalDatum> egm96_;
  bool loaded96_;
  osg::ref_ptr<osgEarth::VerticalDatum> egm2008_;
  bool loaded2008_;
};

}

#ifdef _MSC_VER // [
#pragma warning(pop)
#endif // _MSC_VER ]

#endif /* SIMUTIL_DATUMCONVERT_H */
