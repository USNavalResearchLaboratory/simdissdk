import os, sys, math, timeit

# Try to use SIMDIS_DIR to set up import paths
if 'SIMDIS_DIR' in os.environ:
	# For _module shared object:
	if os.name == "nt":
		sys.path.append(os.environ['SIMDIS_DIR'] + '/lib/amd64-nt/python3.8')
		try:
			# Python 3.8 does not want to respect PATH for loading dependent DLLs.  It introduces
			# a new method to attempt to fix the problem.  Try/except ignores errors in older Python.
			# Without this, the _simCore.pyd needs to go in the same place as simNotify/simCore.
			os.add_dll_directory(os.environ['SIMDIS_DIR'] + '/bin/amd64-nt')
		except:
			pass
		pass
	else:
		sys.path.append(os.environ['SIMDIS_DIR'] + '/lib/amd64-linux/python3.8/lib-dynload')
	# For module wrapper:
	sys.path.append(os.environ['SIMDIS_DIR'] + '/bin/pythonScripts')

import simCore

# simCore/Common

#############################
# Exception.h
e = simCore.Exception("ExceptionFromPython", "Test exception successfully generated. Ignore Error.", 37)
assert(e is not None)
assert(e.rawWhat() == "Test exception successfully generated. Ignore Error.")
assert(e.line() == 37)
e = None

#############################
# Time.h
# Wrapper function that sleeps for a second and takes no args, so that it can be passed through python's timeit() function easily.
# Sleeping for more than a second, as function occasionally executes in slightly less than a tenth of a second if given 100 ms as a parameter.
def sleepWrapper():
	simCore.Sleep(100)
print("Sleeping for about a tenth of a second.")
assert(timeit.timeit(sleepWrapper, number=1) >= .05)

# simCore/Calc

#############################
# Vec3.h
assert(simCore.Vec3() is not None)
v = simCore.Vec3(1, 2, 3)
assert(v is not None)
assert(v.x() == 1)
assert(v.y() == 2)
assert(v.z() == 3)
v.set(3, 1, 2)
assert(v.x() == 3)
assert(v.y() == 1)
assert(v.z() == 2)
v.scale(2)
assert(v.x() == 6)
assert(v.y() == 2)
assert(v.z() == 4)

#############################
# Angle.h
assert(simCore.angFix360(720) == 0)
assert(simCore.areAnglesEqual(0, math.pi * 2) == True)
angleExtents = simCore.ANGLEEXTENTS_TWOPI
assert(angleExtents is not None)
assert(simCore.angFixDegrees(360, angleExtents) == 0)

#############################
# CoordinateSystem.h
assert(simCore.WGS_84 is not None)
assert(simCore.GEOMETRIC_HORIZON is not None)
assert(simCore.WGS_E == 0.0818191908426)
assert(simCore.WGS_ESQ == simCore.WGS_E * simCore.WGS_E)
assert(simCore.EARTH_ROTATION_RATE == 7292115.1467e-11)
coordSystem = simCore.COORD_SYS_NED
assert(coordSystem is not None)
assert(simCore.coordinateSystemToString(coordSystem) == "Topo_NED")
rv, coordSystem = simCore.coordinateSystemFromString("Topo_NED")
assert(rv == 0)
assert(coordSystem == simCore.COORD_SYS_NED)

#############################
# Coordinate.h
coordinateOne = simCore.Coordinate()
coordinateTwo = simCore.Coordinate(simCore.COORD_SYS_LLA, v)
assert(coordinateOne is not None)
assert(coordinateTwo is not None)
assert(coordinateOne.hasAcceleration() == False)
assert(coordinateTwo.x() == 6)
assert(coordinateTwo.y() == 2)
assert(coordinateTwo.z() == 4)

#############################
# Math.h
assert(simCore.intSdkMax(-1, 2) == 2)
assert(simCore.intSdkMin(-1, 2) == -1)
assert(simCore.doubleSdkMax(-1, 2) == 2)
assert(simCore.doubleSdkMin(-1.0, 2) == -1)
assert(simCore.intSquare(2) == 4)
assert(simCore.intSign(2) == 1)
assert(simCore.intSign(0) == 0)
assert(simCore.doubleSign(-1.3) == -1)
assert(simCore.doubleSdkMax(-1.3, 2.0) == 2.0)
assert(simCore.doubleSdkMin(-1.3, 2.0) == -1.3)
assert(simCore.doubleSquare(2.0) == 4.0)
assert(simCore.doubleSign(-1.2) == -1)
assert(simCore.doubleSign(0.0) == 0)
assert(simCore.rint(1.1) == 1)
assert(simCore.rint(1.5) == 2)
assert(simCore.rint(2.5) == 2)
assert(simCore.rint(3.5) == 4)
assert(simCore.rint(2) == 2)
assert(simCore.round(1) == 1)
assert(simCore.round(1.1) == 1)
assert(simCore.round(1.5) == 2)
assert(not simCore.odd(2))
assert(simCore.odd(3))
assert(simCore.areEqual(1, 1.0000001))
assert(simCore.guessStepSize(10, 0) == 1)
assert(simCore.isFinite(v))
v2 = simCore.Vec3(1, 2, 3)
assert(v2 is not None)
assert(simCore.v3Distance(v, v2) == math.sqrt(26))
v2 = simCore.v3Scale(2, v)
assert(v2.x() == 12)
assert(v2.y() == 4)
assert(v2.z() == 8)
vecLen = simCore.v3Unit(v2)
assert(vecLen is not None)
assert(vecLen == math.sqrt(224))
assert(v2.x() == 12 / vecLen)
assert(v2.y() == 4 / vecLen)
assert(v2.z() == 8 / vecLen)
mantissa, numZeroes = simCore.toScientific(12345)
assert(mantissa is not None)
assert(numZeroes is not None)
mantissa, numZeroes = simCore.toScientific(12345)
assert(mantissa is not None)
assert(mantissa == 1.2345)
assert(numZeroes is not None)
assert(numZeroes == 4)

#############################
# CoordinateConverter.h
coordConverter = simCore.CoordinateConverter()
assert(coordConverter is not None)
assert(not coordConverter.hasReferenceOrigin())
refOrigin = simCore.Vec3(44.544, -72.782, 1699)
assert(refOrigin.x() == 44.544)
assert(refOrigin.y() == -72.782)
assert(refOrigin.z() == 1699)
coordConverter.setReferenceOriginDegrees(refOrigin)
assert(coordConverter.hasReferenceOrigin())
assert(simCore.Coordinate() is not None)
llaCoordinate = simCore.Coordinate(simCore.COORD_SYS_LLA, simCore.Vec3(0, 45, 1000))
assert(llaCoordinate is not None)
assert(llaCoordinate.coordinateSystem() == simCore.COORD_SYS_LLA)
assert(llaCoordinate.position() is not None)
assert(coordConverter.lonRadius() is not None)
assert(coordConverter.latRadius() is not None)
assert(coordConverter.tangentPlaneOffsetX() is not None)
assert(coordConverter.tangentPlaneOffsetY() is not None)
assert(coordConverter.tangentPlaneRotation() is not None)
assert(coordConverter.setTangentPlaneOffsetX(1.0) is None)
assert(coordConverter.setTangentPlaneOffsetY(1.0) is None)
assert(coordConverter.setTangentPlaneRotation(1.0) is None)
assert(coordConverter.setReferenceOriginDegrees() is None)
assert(coordConverter.setReferenceOriginDegrees(1.0) is None)
assert(coordConverter.setReferenceOriginDegrees(1.0, 2.0) is None)
assert(coordConverter.setReferenceOriginDegrees(1.0, 2.0, 3.0) is None)
assert(coordConverter.setReferenceOriginDegrees(simCore.Vec3(1.0, 2.0, 3.0)) is None)
assert(coordConverter.setReferenceOrigin() is None)
assert(coordConverter.setReferenceOrigin(1.0) is None)
assert(coordConverter.setReferenceOrigin(1.0, 2.0) is None)
assert(coordConverter.setReferenceOrigin(1.0, 2.0, 3.0) is None)
assert(coordConverter.setReferenceOrigin(simCore.Vec3(1.0, 2.0, 3.0)) is None)
assert(coordConverter.setTangentPlaneOffsets(1.0, 2.0, 3.0) is None)
assert(coordConverter.setTangentPlaneOffsets(1.0, 2.0) is None)
rv, outCoord = coordConverter.convert(llaCoordinate, simCore.COORD_SYS_ECEF)
assert(rv is not None and outCoord is not None)
assert(outCoord.coordinateSystem() == simCore.COORD_SYS_ECEF)
# Note, static conversion methods are called by CoordinateConverter.convert() internally.  They
# are not explicitly tested in this script.

#############################
# Gars.h
isValid, err, lonBand, latPrimaryIdx, latSecondaryIdx, quad15, key5 = simCore.Gars.isValidGars("718KR45")
assert(isValid is not None and err is not None and lonBand is not None and latPrimaryIdx is not None and latSecondaryIdx is not None
	and quad15 is not None and key5 is not None)
rv, latRad, lonRad, err = simCore.Gars.convertGarsToGeodetic("718KR45")
assert(rv is not None and latRad is not None and lonRad is not None and err is not None)
rv, gars, err = simCore.Gars.convertGeodeticToGars(1.0, 1.1, simCore.Gars.GARS_5)
assert(rv is not None and gars is not None and err is not None)
rv, gars, err = simCore.Gars.convertGeodeticToGars(1.0, 1.1)
assert(rv is not None and gars is not None and err is not None)

#############################
# Geometry.h
v1 = simCore.Vec3(0, 0, 0)
v2 = simCore.Vec3(0, 0, 0)
v3 = simCore.Vec3(0, 0, 0)
p = simCore.Plane(v1, v2, v3)
assert(p is not None)
v4 = simCore.Vec3(0, 0, 0)
assert(p.distance(v4) is not None)
poly = simCore.Polytope()
assert(poly is not None)
assert(poly.contains(v4) is not None)
fence = simCore.GeoFence()
assert(fence is not None)
assert(fence.valid() is not None)
assert(fence.contains(v4) is not None)
assert(fence.contains(coordinateTwo) is not None)

#############################
# Interpolation.h
assert(simCore.getFactor(10, 10, 10) is not None)
assert(simCore.nearestNeighborInterpolate(10, 10, 10) is not None)
outputVec = simCore.Vec3LinearInterpolate(v1, v2, 3, 4, 5)
assert(outputVec is not None and outputVec.x() is not None)
assert(simCore.DoubleLinearInterpolate(2.0, 4.0, 10.0, 25.0, 30.0) == 3.5)
assert(simCore.linearInterpolateAngle(1, 2, 3, 4, 5) is not None)
assert(simCore.intBilinearInterpolate(1, 2, 3, 4, 5, 6) is not None)
assert(simCore.doubleBilinearInterpolate(1, 2, 3, 4, 5, 6) is not None)

#############################
# MagneticVariance.h
assert(simCore.MAGVAR_TRUE == 1)
wmm = simCore.WorldMagneticModel()
assert(wmm is not None)
rv, varianceRad = wmm.calculateMagneticVariance(v, 1, 2)
assert(rv is not None and varianceRad is not None)

#############################
# Mgrs.h
mgrs = simCore.Mgrs()
rv, lat, lon, err = mgrs.convertMgrsToGeodetic("33CWM1974418352")
assert(rv is not None and lat is not None and lon is not None and err is not None)
rv, zone, gzdLetters, easting, northing, err = mgrs.breakMgrsString("33CWM1974418352")
assert(rv is not None and zone is not None and gzdLetters is not None and easting is not None
	and northing is not None and err is not None)
# rv, northPole, utmEasting, utmNorthing, err = mgrs.convertMgrsToUtm(33, "CWM", 19744.0, 18352.0)
# assert(rv is not None and northPole is not None and utmEasting is not None
#	and utmNorthing is not None and err is not None)
rv, lat, lon, err = mgrs.convertUtmToGeodetic(33, False, 19744.0, 18352.0)
assert(rv is not None and lat is not None and lon is not None and err is not None)
# rv, northPole, upsEasting, upsNorthing, err = mgrs.convertMgrsToUps("33CWM1974418352", 19744.0, 18352.0)
# assert(rv is not None and northPole is not None and upsEasting is not None
#	and upsNorthing is not None and err is not None)
rv, lat, lon, err = mgrs.convertUpsToGeodetic(False, 19744.0, 18352.0)
assert(rv is not None and lat is not None and lon is not None and err is not None)

#############################
# MultiFrameCoordinate.h
mfc = simCore.MultiFrameCoordinate()
assert(mfc is not None)
coord = simCore.Coordinate()
rv = mfc.setCoordinate(coord)
assert(rv is not None)
rv = mfc.setCoordinate(coord, coordConverter)
assert(rv is not None)
retCoord = mfc.llaCoordinate()
assert(retCoord is not None)
assert(retCoord.coordinateSystem() is not None)

#############################
# NumericalAnalysis.h
assert(simCore.SEARCH_MAX_ITER is not None)
biSearch = simCore.BisectionSearch()
assert(biSearch is not None)
assert(biSearch.count() is not None)
indicator, x, xlo, xhi = biSearch.searchX(1, 2, 3, 4, simCore.SEARCH_INIT_X)
assert(indicator is not None and x is not None and xlo is not None and xhi is not None)
liSearch = simCore.LinearSearch()
assert(liSearch is not None)
indicator, x = liSearch.searchX(1, 2, 3, 4, 5, simCore.SEARCH_INIT_X)
assert(indicator is not None and x is not None)
times = simCore.DoubleArray(3)
values = simCore.DoubleArray(3)
for k in range(0, 3):
	times[k] = k
	values[k] = k + 3
rv, value = simCore.newtonInterp(1.0, times, values)
assert(rv is not None)
assert(value == 4.0)
rv, value = simCore.invLinearInterp(4.5, times, values, 0.1)
assert(rv is not None)
assert(value == 1.5)

#############################
# Calculations.h
# TODO: Following fail, fix output
sphereVec, sphereTpOrigin = simCore.tangentPlane2Sphere(v1, v2)
assert(sphereVec is not None and sphereTpOrigin is not None)
assert(simCore.geodeticToSpherical(0, 0, 0) is not None)
# success, fromPos, toPos = simCore.convertLocations(simCore.Coordinate(), simCore.Coordinate(), simCore.WGS_84, coordConverter)
# assert(success is not None and fromPos is not None and toPos is not None)
assert(simCore.calculateBodyUnitX(1, 1) is not None)
assert(simCore.calculateBodyUnitY(1, 1, 1) is not None)
assert(simCore.calculateBodyUnitZ(1, 1, 1) is not None)
assert(simCore.calculateVelFromGeodeticPos(v1, v2, 1) is not None)
success, velOut, oriOut = simCore.calculateVelOriFromPos(v1, v2, 1, simCore.WGS_84, v3)
assert(success is not None and velOut is not None and oriOut is not None)
success, velOut, oriOut = simCore.calculateVelOriFromPos(v1, v2, 1, simCore.WGS_84, v3, simCore.COORD_SYS_XEAST)
assert(success is not None and velOut is not None and oriOut is not None)
assert(simCore.calculateGeodeticOriFromRelOri(v1, v2) is not None)
assert(simCore.calculateGeodeticOffsetPos(v1, v2, v3) is not None)
assert(simCore.calculateGeodeticEndPoint(v1, 1, 1, 1) is not None)
# midPoint, wrapsDateline = simCore.calculateGeodeticMidPoint(v1, v2, False)
# assert(midPoint is not None and wrapsDateline is not None)
assert(simCore.calculateFlightPathAngles(v1) is not None)
assert(simCore.calculateVelocity(1, 1, 1) is not None)
aoa, ss, totalAoa = simCore.calculateAoaSideslipTotalAoa(v1, v2, True)
assert(aoa is not None and ss is not None and totalAoa is not None)
distance, closestLla = simCore.getClosestPoint(v1, v2, v3)
assert(distance is not None and closestLla is not None)
azim, elev, cmp = simCore.calculateRelAzEl(v1, v2, v3, simCore.WGS_84, coordConverter)
assert(azim is not None and elev is not None and cmp is not None)
azim, elev, cmp = simCore.calculateAbsAzEl(v1, v2, simCore.WGS_84, coordConverter)
assert(azim is not None and elev is not None and cmp is not None)
assert(simCore.calculateSlant(v1, v2, simCore.WGS_84, coordConverter) is not None)
nst, dr, xr = simCore.calculateGeodesicDRCR(v1, 1, v2)
assert(nst is not None and dr is not None and xr is not None)
v5 = simCore.Vec3(1, 1, 1)
assert(simCore.calculateRangeRate(v, v1, v2, v3, simCore.WGS_84, coordConverter, v4, v5) is not None)
azim, elev, cmp = simCore.calculateRelAng(v1, v2)
assert(azim is not None and elev is not None and cmp is not None)
azim, elev, cmp = simCore.calculateRelAngToTrueAzEl(1, 2, v1)
assert(azim is not None and elev is not None and cmp is not None)
assert(simCore.positionInGate(v1, v2, 1, 2, 3, 4, 5, 6, simCore.WGS_84, coordConverter) is not None)
assert(simCore.laserInGate(v1, v2, 1, 2, 3, 4, 5, 6, 7, 8, 9, simCore.WGS_84, coordConverter) is not None)
yaw, pitch = simCore.calculateYawPitchFromBodyUnitX(v1)
assert(yaw is not None and pitch is not None)
lat, lon, azbck = simCore.sodanoDirect(1, 2, 3, 100.0, 3.14159)
assert(lat is not None and lon is not None and azbck is not None)
distance, azfwd, azbck, = simCore.sodanoInverse(1, 2, 3, 2, 3)
assert(distance is not None and azfwd is not None and azbck is not None)
assert(simCore.calculateEarthRadius(1.0) is not None)
# assert(simCore.clampEcefPointToGeodeticSurface(v1) is not None)
assert(simCore.calculateHorizonDist(v1) is not None)
assert(simCore.calculateHorizonDist(v1, simCore.GEOMETRIC_HORIZON) is not None)
assert(simCore.calculateHorizonDist(v1, simCore.GEOMETRIC_HORIZON, 1.06) is not None)
assert(simCore.calculateHorizonDist(v1, simCore.GEOMETRIC_HORIZON, 1.06, 1.333) is not None)
assert(simCore.positionInGate(v1, v2, 1.0, 0.0, 1.0, 1.0, 1000.0, 3000.0, simCore.WGS_84, coordConverter) is not None)
assert(simCore.laserInGate(v1, v2, 1.0, 0.0, 1.0, 1.0, 1000.0, 3000.0, 1.5, 1.5, 2000.0, simCore.WGS_84, coordConverter) is not None)
assert(simCore.laserInGate(v1, v2, 1.0, 0.0, 1.0, 1.0, 1000.0, 3000.0, 1.5, 1.5, 2000.0, simCore.WGS_84, coordConverter, 100) is not None)

#############################
# Random.h
nv = simCore.NormalVariable()
assert(nv is not None)
assert(nv() is not None)
assert(nv.setMean(1) is None and nv.setStdDev(2) is None)
assert(nv.mean() == 1 and nv.stdDev() == 2)
gVar = simCore.GaussianVariable()
assert(gVar is not None)
assert(gVar.stdDev() == 1)
pv = simCore.PoissonVariable()
assert(pv is not None)
assert(pv.seeds() is not None)
assert(pv.setSeeds(1) is None)

#############################
# SquareMatrix.h
sm = simCore.SquareMatrix()
# TODO: Implement.
# assert(sm.row(0) is not None)
assert(sm is not None)
assert(sm.makeIdentity() is not None)
assert(sm.transpose() is not None)
assert(sm.set(0, 0, 0) is not None)
assert(sm.get(0, 0) == 0)
assert(sm.data() is not None)
assert(sm.add(simCore.SquareMatrix()) is not None)

#############################
# Units.h
assert(simCore.Units.UNITLESS is not None)
assert(simCore.Units.MIL is not None)
assert(simCore.Units.DATA_MILES is not None)
assert(simCore.Units.DATA_MILES_PER_HOUR is not None)
assert(simCore.Units.NAUTICAL_MILES_PER_SECOND_SQUARED is not None)
assert(simCore.Units.RANKINE is not None)
assert(simCore.Units.REVOLUTIONS_PER_MINUTE is not None)
assert(simCore.Units.TEASPOON is not None)
assert(simCore.Units.PASCALS is not None)
assert(simCore.Units.VOLUME_FAMILY is not None)
assert(simCore.Units.FEET.name() is not None)
assert(simCore.Units.FEET.abbreviation() is not None)
assert(simCore.Units.FEET.family() is not None)
assert(simCore.Units.FEET.canConvert(simCore.Units.METERS))
assert(simCore.Units.FEET.convertTo(simCore.Units.METERS, 1) is not None)
assert(simCore.Units.FEET == simCore.Units.FEET)
assert(simCore.Units.FEET != simCore.Units.METERS)
assert(simCore.Units.FEET.isValid())
assert(simCore.Units.FEET.toBaseScalar() is not None)
assert(simCore.Units.offsetThenScaleUnit("Yard", "yd", 0, 1.0 / 3, simCore.Units.LENGTH_FAMILY).name() == "Yard")
ur = simCore.UnitsRegistry()
assert(ur is not None)
assert(ur.registerDefaultUnits() is None)
assert(ur.registerUnits(simCore.Units.FEET) is not None)
assert(ur.units(simCore.Units.LENGTH_FAMILY) is not None)
# TODO: Fix
# assert(ur.families() is not None)
assert(ur.unitsByName("feet") == simCore.Units.FEET)
assert(ur.unitsByAbbreviation("ft") == simCore.Units.FEET)

#############################
# VerticalDatum.h
assert(simCore.VERTDATUM_WGS84 is not None)
assert(simCore.VERTDATUM_MSL is not None)
assert(simCore.VERTDATUM_USER is not None)

# simCore/LUT

#############################
# LUT1.h
dLut = simCore.doubleLUT1()
assert(dLut is not None)
assert(dLut.initialize(1, 2, 1, 1) is None)
iLut = simCore.intLUT1()
assert(iLut is not None)
assert(iLut.initialize(1, 2, 1, 1) is None)
assert(iLut.minX() is not None)
assert(iLut.numX() == 1)
assert(iLut(0) is not None)
assert(simCore.index(1, 1, 1) is not None)
assert(simCore.intIndex(iLut, 1) is not None)
assert(simCore.doubleIndex(dLut, 1) is not None)
assert(simCore.intLowValue(iLut, 1) is not None)
assert(simCore.doubleLowValue(dLut, 0) is not None)

#############################
# LUT2.h
dLut2 = simCore.doubleLUT2()
assert(dLut2 is not None)
assert(dLut2.initialize(1, 2, 1, 1, 2, 1, 1) is None)
iLut2 = simCore.intLUT2()
assert(iLut2 is not None)
assert(iLut2.initialize(1, 2, 1, 1, 2, 1, 1) is None)
assert(iLut2.minX() is not None)
assert(iLut2.numX() is not None)
assert(iLut2(0, 0) is not None)

#############################
# InterpTable.h
e = simCore.InterpTableException("InterpTableException test error message")
assert(e is not None)
assert(e.what() is not None)
e = simCore.intInterpTableLimitException("InterpTableLimitException test error message", 1, 2, 3)
assert(e is not None)
assert(e.what() is not None)
e = simCore.doubleInterpTableLimitException("InterpTableLimitException test error message", 1, 2, 3)
assert(e is not None)
iit = simCore.intInterpTable()
assert(iit is not None)
assert(iit.initialize(1, 2, 1, 1, 2, 1) is None)
assert(iit(0, 0) is not None)
dit = simCore.doubleInterpTable()
assert(dit is not None)
assert(dit.initialize(1, 2, 1, 1, 2, 1) is None)
assert(dit.lut() is not None)

# simCore/String

#############################
# Angle.h
assert(simCore.DEG_SYM_NONE is not None)
assert(simCore.getDegreeSymbol(simCore.DEG_SYM_NONE) is not None)
success, angle = simCore.getAngleFromDegreeString("20", True)
assert(success is not None and angle is not None)
assert(simCore.FMT_BAM is not None)
assert(simCore.getAngleString(1, simCore.FMT_BAM, True, 1, simCore.DEG_SYM_NONE, 0, 0) is not None)
assert(simCore.printLatitude(1, simCore.FMT_BAM, True, 1, simCore.DEG_SYM_NONE) is not None)
assert(simCore.printLongitude(1, simCore.FMT_BAM, True, 1, simCore.DEG_SYM_NONE) is not None)

#############################
# Constants.h
assert(simCore.STR_WHITE_SPACE_CHARS is not None)
assert(simCore.STR_DEGREE_SYMBOL_ASCII is not None)
assert(simCore.STR_DEGREE_SYMBOL_UNICODE is not None)
assert(simCore.STR_DEGREE_SYMBOL_UTF8 is not None)

#############################
# FilePatterns.h
assert(simCore.ALL_SIMDIS_FILE_PATTERNS is not None)
assert(simCore.SIMDIS_FILE_PATTERNS is not None)
assert(simCore.ALL_DATA_FILE_PATTERNS is not None)

#############################
# Format.h
assert(simCore.caseCompare("a", "b") is not None)
assert(simCore.lowerCase("APPLE") == "apple")
assert(simCore.upperCase("apple") == "APPLE")
assert(simCore.stringCaseFind("Apple", "A") == 0)
assert(simCore.getExtension("hi.cpp") == ".cpp")
assert(simCore.hasExtension("hi.cpp", ".cpp"))
assert(simCore.buildString("hi", 1) is not None)

#############################
# Tokenizer.h
word, endWordPos = simCore.extractWord("Lots of words", 5)
assert(word == "of" and endWordPos == 7)
word, endWordPos = simCore.extractWordWithQuotes('Lots "of" words', 5)
assert(word == '"of"' and endWordPos == 9)
assert(simCore.getTerminateForStringPos("", 1) is not None)
assert(simCore.getFirstCharPosAfterString("abc", 0, "b") == 2)
assert(simCore.removeQuotes("'Too many quotes'") == "Too many quotes")
err, tokenName, tokenValue = simCore.getNameAndValueFromToken("key=value")
assert(err is not None and tokenName == "key" and tokenValue == "value")

#############################
# Utils.h
assert(simCore.StringUtils.before("Testing function", " function") == "Testing")
assert(simCore.StringUtils.before("Testing function", " ") == "Testing")
assert(simCore.StringUtils.after("Testing function", "Testing ") == "function")
assert(simCore.StringUtils.after("Testing function", " ") == "function")
assert(simCore.StringUtils.beforeLast("Testing function", "ti") == "Testing func")
assert(simCore.StringUtils.beforeLast("Testing function", "n") == "Testing functio")
assert(simCore.StringUtils.afterLast("Testing function", "ti") == "on")
assert(simCore.StringUtils.afterLast("Testing function", "t") == "ion")
assert(simCore.StringUtils.substitute("Testing function", "t", "m") is not None)
assert(simCore.StringUtils.addEscapeSlashes('"') == '\\\"')
assert(simCore.StringUtils.removeEscapeSlashes('\\"') == '"')
assert(simCore.StringUtils.trim('  test  ') == 'test')
assert(simCore.toNativeSeparators("Users\person\Documents") is not None)
assert(simCore.sanitizeFilename("<Documents>") == "Documents")
assert(simCore.hasEnv("var") is not None)

# TODO: More testing here

"""
Not implemented due to MSVC 2019 errors.
# simCore/EM

#############################
# Constants.h
assert(simCore.LIGHT_SPEED_VACUUM is not None)
assert(simCore.RRE_CONSTANT is not None)
assert(simCore.DEFAULT_ANTENNA_GAIN is not None)
assert(simCore.POLARITY_RIGHTCIRC is not None)
assert(simCore.POLARITY_STRING_UNKNOWN is not None)
assert(simCore.POLARITY_STRING_LEFTCIRC is not None)
assert(simCore.polarityString(simCore.POLARITY_LEFTCIRC) is not None)
assert(simCore.polarityType(simCore.POLARITY_STRING_LEFTCIRC) is not None)
assert(simCore.RCS_XPATCH is not None)
assert(simCore.RCS_SYM_LUT_TYPE is not None)
assert(simCore.RCS_LOG_NORMAL_FUNC is not None)
assert(simCore.ANTENNA_LOBE_BACK is not None)
assert(simCore.ANTENNA_ALGORITHM_SINXX is not None)
assert(simCore.ANTENNA_FORMAT_EZNEC is not None)
assert(simCore.ANTENNA_PATTERN_MONOPULSE is not None)
assert(simCore.ANTENNA_STRING_ALGORITHM_SINXX is not None)
assert(simCore.ANTENNA_STRING_FORMAT_MONOPULSE is not None)
assert(simCore.ANTENNA_STRING_EXTENSION_MONOPULSE is not None)

#############################
# AntennaPattern.h
assert(simCore.antennaPatternTypeString(simCore.ANTENNA_PATTERN_SINXX) is not None)
assert(simCore.antennaPatternType(simCore.ANTENNA_STRING_ALGORITHM_SINXX) is not None)
assert(simCore.loadPatternFile(simCore.ANTENNA_STRING_ALGORITHM_SINXX, 1) is not None)
assert(simCore.AntennaGainParameters() is not None)
apg = simCore.AntennaPatternGauss()
assert(apg is not None)
# TODO: Test
# assert(apg.type() is not None)
assert(apg.gain(simCore.AntennaGainParameters()) is not None)
small, large = apg.minMaxGain(simCore.AntennaGainParameters())
assert(small is not None and large is not None)
# TODO: Implement
# result, lastLobe = simCore.calculateGain({}, {}, 1, 2, 3, 4, 5. True)
# assert(result is not None and lastLobe is not None)
apt = simCore.AntennaPatternTable()
assert(apt is not None)
assert(apt.readPat("") is not None)
assert(apt.setValid(False) is None)
assert(apt.setFilename("") is None)
assert(apt.setAzimData(100, 10) is None)
# TODO: Test after installing LUT/InterpTable.h
# success, symmetricAntennaPattern = simCore.readPattern("", "", 1)
# assert(success is not None and symmetricAntennaPattern is not None)
# success, symmetricGainAntPattern = simCore.readPattern("", "", 1)
# assert(success is not None and symmetricAntennaPattern is not None)
"""

print("Success!")

