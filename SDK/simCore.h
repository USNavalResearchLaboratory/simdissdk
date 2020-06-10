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
#ifndef SIMDISSDK_SIMCORE_H
#define SIMDISSDK_SIMCORE_H

#ifdef _MSC_VER
#pragma message( __FILE__ ": warning <DEPR>: File is deprecated and will be removed in a future release." )
#else
#warning File is deprecated and will be removed in a future release.
#endif

#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/DatumConvert.h"
#include "simCore/Calc/Gars.h"
#include "simCore/Calc/Geometry.h"
#include "simCore/Calc/GogToGeoFence.h"
#include "simCore/Calc/Interpolation.h"
#include "simCore/Calc/MagneticVariance.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/MathConstants.h"
#include "simCore/Calc/Mgrs.h"
#include "simCore/Calc/MultiFrameCoordinate.h"
#include "simCore/Calc/NumericalAnalysis.h"
#include "simCore/Calc/Random.h"
#include "simCore/Calc/SquareMatrix.h"
#include "simCore/Calc/UnitContext.h"
#include "simCore/Calc/Units.h"
#include "simCore/Calc/Vec3.h"
#include "simCore/Calc/VerticalDatum.h"
#include "simCore/Common/Common.h"
#include "simCore/Common/Exception.h"
#include "simCore/Common/Export.h"
#include "simCore/Common/FileSearch.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Time.h"
#include "simCore/Common/Version.h"
#include "simCore/EM/AntennaPattern.h"
#include "simCore/EM/Constants.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/ElectroMagRange.h"
#include "simCore/EM/Propagation.h"
#include "simCore/EM/RadarCrossSection.h"
#include "simCore/LUT/InterpTable.h"
#include "simCore/LUT/LUT1.h"
#include "simCore/LUT/LUT2.h"
#include "simCore/String/Angle.h"
#include "simCore/String/Constants.h"
#include "simCore/String/CsvReader.h"
#include "simCore/String/FilePatterns.h"
#include "simCore/String/Format.h"
#include "simCore/String/TextFormatter.h"
#include "simCore/String/TextReplacer.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/UnitContextFormatter.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Time/Clock.h"
#include "simCore/Time/ClockImpl.h"
#include "simCore/Time/Constants.h"
#include "simCore/Time/CountDown.h"
#include "simCore/Time/DeprecatedStrings.h"
#include "simCore/Time/Exception.h"
#include "simCore/Time/Julian.h"
#include "simCore/Time/String.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/TimeClock.h"
#include "simCore/Time/Utils.h"

#endif /* SIMDISSDK_SIMCORE_H */
