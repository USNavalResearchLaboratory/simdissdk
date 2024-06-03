%module(directors="1") simCore

// Add a date here to trigger forced regeneration: 02/13/2024

// Do not include deprecated methods in the SWIG output; defined to noop
#define SDK_DEPRECATE(function, deprecationMessage)
// Disable copy/move macros, SWIG does not understand
#define SDK_DISABLE_COPY(k)
#define SDK_DISABLE_MOVE(k)
#define SDK_DISABLE_COPY_MOVE(k)

%ignore simCore::v3Length;
%ignore simCore::v3Scale;
%ignore simCore::v3Norm;
%ignore simCore::v3Add;
%ignore simCore::v3Subtract;
%ignore simCore::v3Dot;
%ignore simCore::v3Cross;
%ignore simCore::v3Negate;


%feature("autodoc", "3");

%{

#include "simCore/Common/Exception.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/DatumConvert.h"
#include "simCore/Calc/Gars.h"
#include "simCore/Calc/Geometry.h"
#include "simCore/Calc/GeoFence.h"
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
#include "simCore/Time/Exception.h"
#include "simCore/Time/Julian.h"
#include "simCore/Time/String.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/TimeClock.h"
#include "simCore/Time/Utils.h"

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


%include "simCoreCommon.i"
%include "simCoreCalc.i"
%include "simCoreEM.i"
%include "simCoreLUT.i"
%include "simCoreString.i"
%include "simCoreTime.i"
