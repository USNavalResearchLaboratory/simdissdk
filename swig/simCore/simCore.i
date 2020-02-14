%module(directors="1") simCore

%feature("autodoc", "3");

%{
#include "simCore.h"
%}


#ifdef _WIN32
%include "windows.i"
#endif

// Include STL support
%include "std_map.i"
%include "std_string.i"
%include "std_vector.i"

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;

// Windows MSVC uses long long for 64 bit ints.  Linux g++ uses long
#ifdef MSVC
typedef unsigned long long uint64_t;
typedef long long int64_t;
#else
typedef unsigned long uint64_t;
typedef long int64_t;
#endif


#define SDKCORE_EXPORT

%ignore simCore::Vec3::operator=;
%ignore simCore::Vec3::operator[];
%ignore simCore::Coordinate::operator=;

// Note, order of inclusion matters

////////////////////////////////////////////////
// simCore/Common
%include "simCore/Common/Exception.h"
%include "simCore/Common/FileSearch.h"
%include "simCore/Common/HighPerformanceGraphics.h"
%include "simCore/Common/Time.h"

////////////////////////////////////////////////
// simCore/Calc
%include "simCore/Calc/Vec3.h"
%include "simCore/Calc/Angle.h"
%include "simCore/Calc/CoordinateSystem.h"
%include "simCore/Calc/Coordinate.h"

// TODO: Add these and test them as you add them
/*
%include "simCore/Calc/Calculations.h"
%include "simCore/Calc/CoordinateConverter.h"
%include "simCore/Calc/DatumConvert.h"
%include "simCore/Calc/Gars.h"
%include "simCore/Calc/Geometry.h"
%include "simCore/Calc/GogToGeoFence.h"
%include "simCore/Calc/Interpolation.h"
%include "simCore/Calc/MagneticVariance.h"
%include "simCore/Calc/MathConstants.h"
%include "simCore/Calc/Math.h"
%include "simCore/Calc/Mgrs.h"
%include "simCore/Calc/MultiFrameCoordinate.h"
%include "simCore/Calc/NumericalAnalysis.h"
%include "simCore/Calc/Random.h"
%include "simCore/Calc/SquareMatrix.h"
%include "simCore/Calc/UnitContext.h"
%include "simCore/Calc/Units.h"
%include "simCore/Calc/VerticalDatum.h"

////////////////////////////////////////////////
// simCore/EM
%include "simCore/EM/AntennaPattern.h"
%include "simCore/EM/Constants.h"
%include "simCore/EM/Decibel.h"
%include "simCore/EM/ElectroMagRange.h"
%include "simCore/EM/Propagation.h"
%include "simCore/EM/RadarCrossSection.h"

////////////////////////////////////////////////
// simCore/LUT
%include "simCore/LUT/InterpTable.h"
%include "simCore/LUT/LUT1.h"
%include "simCore/LUT/LUT2.h"

////////////////////////////////////////////////
// simCore/String
%include "simCore/String/Angle.h"
%include "simCore/String/Constants.h"
%include "simCore/String/FilePatterns.h"
%include "simCore/String/Format.h"
%include "simCore/String/TextFormatter.h"
%include "simCore/String/TextReplacer.h"
%include "simCore/String/Tokenizer.h"
%include "simCore/String/UnitContextFormatter.h"
%include "simCore/String/Utils.h"
%include "simCore/String/ValidNumber.h"
%include "simCore/Time/Clock.h"
%include "simCore/Time/ClockImpl.h"
%include "simCore/Time/Constants.h"
%include "simCore/Time/CountDown.h"
%include "simCore/Time/DeprecatedStrings.h"
%include "simCore/Time/Exception.h"
%include "simCore/Time/Julian.h"
%include "simCore/Time/String.h"
%include "simCore/Time/TimeClass.h"
%include "simCore/Time/TimeClock.h"
%include "simCore/Time/Utils.h"
*/
