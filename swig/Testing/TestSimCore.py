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
# TODO: Boolean references are failing
#rv, northPole, utmEasting, utmNorthing, err = mgrs.convertMgrsToUtm(33, "CWM", 19744.0, 18352.0)
#assert(rv is not None and northPole is not None and utmEasting is not None
#	and utmNorthing is not None and err is not None)
rv, lat, lon, err = mgrs.convertUtmToGeodetic(33, False, 19744.0, 18352.0)
assert(rv is not None and lat is not None and lon is not None and err is not None)
# TODO: Boolean references are failing
#rv, northPole, upsEasting, upsNorthing, err = mgrs.convertMgrsToUps("33CWM1974418352", 19744.0, 18352.0)
#assert(rv is not None and northPole is not None and upsEasting is not None
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

# TODO: More testing here

print("Success!")
