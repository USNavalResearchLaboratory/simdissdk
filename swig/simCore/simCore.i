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
%ignore simCore::CoordinateConverter::operator=;
%ignore simCore::SquareMatrix::operator=;

// Note, order of inclusion matters

////////////////////////////////////////////////
// simCore/Common
%include "simCore/Common/Exception.h"
%include "simCore/Common/Time.h"

////////////////////////////////////////////////
// simCore/Calc
%include "simCore/Calc/Vec3.h"
%include "simCore/Calc/Angle.h"

// simCore::coordinateSystemFromString()
%apply int& OUTPUT { simCore::CoordinateSystem& outSystem };

%include "simCore/Calc/CoordinateSystem.h"
%include "simCore/Calc/Coordinate.h"

// Handling output parameter for toScientific() from simCore/calc/Math.cpp
%apply int* OUTPUT { int* exp };

// v3Scale() needs a custom wrapper for proper return value
%rename("wrap_v3Scale") simCore::v3Scale;
%include "simCore/Calc/Math.h"

%pythoncode %{
def v3Scale(scalar, inVec):
  outVec = Vec3()
  wrap_v3Scale(scalar, inVec, outVec)
  return outVec
%}

%rename("wrap_convert") simCore::CoordinateConverter::convert;
%include "simCore/Calc/CoordinateConverter.h"
%pythoncode %{
def CoordConvert_convert(self, inCoord, outSystem):
        outCoord = Coordinate()
        rv = self.wrap_convert(inCoord, outCoord, outSystem)
        return rv, outCoord
CoordinateConverter.convert = CoordConvert_convert
%}

// isValidGars()
%apply std::string* OUTPUT { std::string* err };
%apply int* OUTPUT { int* lonBand };
%apply int* OUTPUT { int* latPrimaryIdx };
%apply int* OUTPUT { int* latSecondaryIdx };
%apply int* OUTPUT { int* quad15 };
%apply int* OUTPUT { int* key5 };
// convertGarsToGeodetic()
%apply double& OUTPUT { double& latRad };
%apply double& OUTPUT { double& lonRad };
// convertGeodeticToGars()
%apply std::string* OUTPUT { std::string* garsOut };
%extend simCore::Gars {
  // Fixes ordering of parameters for SWIG
  static int convertGeodeticToGars(double latRad, double lonRad, simCore::Gars::Level level, std::string* garsOut, std::string* err) {
    return simCore::Gars::convertGeodeticToGars(latRad, lonRad, *garsOut, level, err);
  }
  static int convertGeodeticToGars(double latRad, double lonRad, std::string* garsOut, std::string* err) {
    return simCore::Gars::convertGeodeticToGars(latRad, lonRad, *garsOut, simCore::Gars::GARS_5, err);
  }
};
%ignore simCore::Gars::convertGeodeticToGars; // Ignore the C++ header file version
%warnfilter(509) simCore::Gars; // Ignore the overload warnings since they're treated as output parameters
%include "simCore/Calc/Gars.h"

%include "simCore/Calc/Geometry.h"
%include "simCore/Calc/Interpolation.h"

// simCore::WorldMagneticModel::calculateMagneticVariance()
%apply double& OUTPUT { double& varianceRad };
%include "simCore/Calc/MagneticVariance.h"

%warnfilter(509) simCore::Mgrs; // Ignore the overload warnings since they're treated as output parameters
// convertMgrsToGeodetic()
%apply double& OUTPUT { double& lat };
%apply double& OUTPUT { double& lon };
// breakMgrsString()
%apply int& OUTPUT { int& zone };
%apply std::string& OUTPUT { std::string& gzdLetters };
%apply double& OUTPUT { double& easting };
%apply double& OUTPUT { double& northing };
// TODO: These two methods do not wrap cleanly due to boolean reference
%ignore simCore::Mgrs::convertMgrsToUtm;
%ignore simCore::Mgrs::convertMgrsToUps;
%include "simCore/Calc/Mgrs.h"

%include "simCore/Calc/MultiFrameCoordinate.h"

// simCore::BisectionSearch::searchX()
%apply double& INOUT { double& x};
%apply double& INOUT { double& xlo, double& xhi };
// Required for double[3] in newtonInterp()
%include "carrays.i"
%array_class(double, DoubleArray);
// newtonInterp()
%apply double& OUTPUT { double& funcAtT0 };
// invLinearInterp()
%apply double& OUTPUT { double& t0 };
%include "simCore/Calc/NumericalAnalysis.h"

// simCore::calculateRelAzEl()
%apply double* OUTPUT { double* azim, double* elev, double* cmp };
// simCore::calculateGeodesicDRCR()
%apply double* OUTPUT { double* downRng, double* crossRng };
// simCore::calculateRelAng() and simCore::calculateRelAngToTrueAzEl()
%apply double* OUTPUT { double* azim, double* elev, double* cmp };
// TODO: calculateYawPitchFromBodyUnitX()
// TODO: sodanoDirect()
// TODO: sodanoInverse()
%include "simCore/Calc/Calculations.h"

%include "simCore/Calc/Random.h"
%include "simCore/Calc/SquareMatrix.h"

%include "simCore/Calc/Units.h"
%include "simCore/Calc/VerticalDatum.h"

// TODO: Implement after installing simCore/Time
/*
%include "simCore/Calc/DatumConvert.h"
%include "simCore/Calc/UnitContext.h"
*/

%template(intSdkMax) simCore::sdkMax<int>;
%template(intSdkMin) simCore::sdkMin<int>;
%template(intSquare) simCore::square<int>;
%template(intSign) simCore::sign<int>;
%template(doubleSdkMax) simCore::sdkMax<double>;
%template(doubleSdkMin) simCore::sdkMin<double>;
%template(doubleSquare) simCore::square<double>;
%template(doubleSign) simCore::sign<double>;

%template(Vec3LinearInterpolate) simCore::linearInterpolate<simCore::Vec3>;
%template(DoubleLinearInterpolate) simCore::linearInterpolate<double>;
%template(intBilinearInterpolate) simCore::bilinearInterpolate<int>;
%template(doubleBilinearInterpolate) simCore::bilinearInterpolate<double>;

////////////////////////////////////////////////
// simCore/EM
// TODO: Implement
/*
%include "simCore/EM/Constants.h"

// AntennaPattern::minMaxGain()
%apply float* OUTPUT { float* min, float* max };
%include "simCore/EM/AntennaPattern.h"


%include "simCore/EM/Decibel.h"
%include "simCore/EM/ElectroMagRange.h"
%include "simCore/EM/Propagation.h"
%include "simCore/EM/RadarCrossSection.h"
*/
////////////////////////////////////////////////
// simCore/LUT
%include "simCore/LUT/LUT1.h"
%include "simCore/LUT/LUT2.h"
%include "simCore/LUT/InterpTable.h"

%template(intLUT1) simCore::LUT::LUT1<int>;
%template(doubleLUT1) simCore::LUT::LUT1<double>;
%template(intIndex) simCore::LUT::index<int>;
%template(doubleIndex) simCore::LUT::index<double>;
%template(intLowValue) simCore::LUT::lowValue<int>;
%template(doubleLowValue) simCore::LUT::lowValue<double>;
// TODO: interpolate()

%template(intLUT2) simCore::LUT::LUT2<int>;
%template(doubleLUT2) simCore::LUT::LUT2<double>;
// TODO: index()
// TODO: nearValue()
// TODO: interpolate()

// TODO: BilinearInterpolate class
%template(intInterpTableLimitException) simCore::InterpTableLimitException<int>;
%template(doubleInterpTableLimitException) simCore::InterpTableLimitException<double>;
%template(intInterpTable) simCore::InterpTable<int>;
%template(doubleInterpTable) simCore::InterpTable<double>;
// TODO: NearestLookup()
// TODO: BilinearLookup()
// TODO: BilinearLookupNoException()

////////////////////////////////////////////////
// simCore/String
%apply double& OUTPUT { double& ang };
%include "simCore/String/Angle.h"

%include "simCore/String/Constants.h"
%include "simCore/String/FilePatterns.h"
%include "simCore/String/Format.h"

%apply size_t& OUTPUT { size_t& endWordPos };
%apply std::string& OUTPUT { std::string& tokenName, std::string& tokenValue };
%include "simCore/String/Tokenizer.h"

%include "simCore/String/Utils.h"

// Format.h
// TODO: join()
// TODO: getStrippedLine()

// Tokenizer.h
// TODO: stringTokenizer()
// TODO: getTokens()
// TODO: removeQuotes()
// TODO: tokenizeWithQuotes()
// TODO: quoteTokenizer()
// TODO: removeCommentTokens()
// TODO: quoteCommentTokenizer()
// TODO: escapeTokenize()

// TODO: Add these and test them as you add them
/*
%include "simCore/String/TextFormatter.h"
%include "simCore/String/TextReplacer.h"
%include "simCore/String/UnitContextFormatter.h"
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
