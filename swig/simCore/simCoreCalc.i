// Add a date here to trigger forced regeneration: 02/09/2024
%ignore simCore::Vec3::Vec3(Vec3&&);
%ignore simCore::Vec3::operator=;
%ignore simCore::Vec3::operator[];
%ignore simCore::Coordinate::operator=;
%ignore simCore::CoordinateConverter::operator=;
%ignore simCore::SquareMatrix::operator=;

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

%include "simCore/Calc/Math.h"

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

%rename("wrap_tangentPlane2Sphere") simCore::tangentPlane2Sphere;
%rename("wrap_geodeticToSpherical") simCore::geodeticToSpherical;
%rename("wrap_calculateBodyUnitX") simCore::calculateBodyUnitX;
%rename("wrap_calculateBodyUnitY") simCore::calculateBodyUnitY;
%rename("wrap_calculateBodyUnitZ") simCore::calculateBodyUnitZ;
%rename("wrap_calculateVelFromGeodeticPos") simCore::calculateVelFromGeodeticPos;
%rename("wrap_calculateVelOriFromPos") simCore::calculateVelOriFromPos;
%rename("wrap_calculateGeodeticOriFromRelOri") simCore::calculateGeodeticOriFromRelOri;
%rename("wrap_calculateGeodeticOffsetPos") simCore::calculateGeodeticOffsetPos;
%rename("wrap_calculateGeodeticEndPoint") simCore::calculateGeodeticEndPoint;
%rename("wrap_calculateFlightPathAngles") simCore::calculateFlightPathAngles;
%rename("wrap_calculateVelocity") simCore::calculateVelocity;
%rename("wrap_getClosestPoint") simCore::getClosestPoint;

// simCore::calculateAoaSideslipTotalAoa()
%apply double* OUTPUT { double* aoa, double* ss, double* totalAoA };
// simCore::calculateYawPitchFromBodyUnitX()
%apply double& OUTPUT { double& yawOut, double& pitchOut };
// simCore::sodanoDirect()
%warnfilter(509) simCore::sodanoDirect;
%apply double* OUTPUT { double *latOut, double *lonOut, double *azbck };
// simCore::sodanoInverse()
%warnfilter(509) simCore::sodanoInverse;
%apply double* OUTPUT { double *azfwd, double *azbck };

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

// Various Python overrides
%pythoncode %{
def tangentPlane2Sphere(llaVec, tpVec):
  sphereVec = Vec3()
  sphereTpOrigin = Vec3()
  wrap_tangentPlane2Sphere(llaVec, tpVec, sphereVec, sphereTpOrigin)
  return sphereVec, sphereTpOrigin
def geodeticToSpherical(lat, lon, alt):
  point = Vec3()
  wrap_geodeticToSpherical(lat, lon, alt, point)
  return point
def calculateBodyUnitX(yaw, pitch):
  vecX = Vec3()
  wrap_calculateBodyUnitX(yaw, pitch, vecX)
  return vecX
def calculateBodyUnitY(yaw, pitch, roll):
  vecY = Vec3()
  wrap_calculateBodyUnitY(yaw, pitch, roll, vecY)
  return vecY
def calculateBodyUnitZ(yaw, pitch, roll):
  vecZ = Vec3()
  wrap_calculateBodyUnitZ(yaw, pitch, roll, vecZ)
  return vecZ
def calculateVelFromGeodeticPos(currPos, prevPos, deltaTime):
  velVec = Vec3()
  wrap_calculateVelFromGeodeticPos(currPos, prevPos, deltaTime, velVec)
  return velVec
def calculateVelOriFromPos(currPos, prevPos, deltaTime, sysIn, refLLA, sysOut=COORD_SYS_XEAST):
  velOut = Vec3()
  oriOut = Vec3()
  success = wrap_calculateVelOriFromPos(currPos, prevPos, deltaTime, sysIn, velOut, oriOut, refLLA, sysOut)
  return success, velOut, oriOut
def calculateGeodeticOriFromRelOri(hostYpr, relYpr):
  ypr = Vec3()
  wrap_calculateGeodeticOriFromRelOri(hostYpr, relYpr, ypr)
  return ypr
def calculateGeodeticOffsetPos(llaBgnPos, bodyOriOffset, bodyPosOffset):
  offsetLla = Vec3()
  wrap_calculateGeodeticOffsetPos(llaBgnPos, bodyOriOffset, bodyPosOffset, offsetLla)
  return offsetLla
def calculateGeodeticEndPoint(llaBgnPos, az, el, rng):
  llaEndPos = Vec3()
  wrap_calculateGeodeticEndPoint(llaBgnPos, az, el, rng, llaEndPos)
  return llaEndPos
def calculateFlightPathAngles(velVec):
  fpa = Vec3()
  wrap_calculateFlightPathAngles(velVec, fpa)
  return fpa
def calculateVelocity(speed, heading, pitch):
  velVec = Vec3()
  wrap_calculateVelocity(speed, heading, pitch, velVec)
  return velVec
def getClosestPoint(startLla, endLla, toLla):
  closestLla = Vec3()
  dist = wrap_getClosestPoint(startLla, endLla, toLla, closestLla)
  return dist, closestLla
%}
