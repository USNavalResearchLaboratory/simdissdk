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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#ifndef QS_COMMON_GEO_H
#define QS_COMMON_GEO_H

#include "QSCommon.h"
#include "QSPosXYExtents.h"

#ifdef USE_SIMDIS_SDK
namespace simVis_db
{
#endif
  //=====================================================================================
  /** Updates extents such that the given lat/lon is within the extents */
  void UpdateExtents(const double& lon, const double& lat,
		     double& minLon, double& maxLon,
		     double& minLat, double& maxLat);

  //=====================================================================================
  /** Gets the extents of a child node */
  void GetChildExtents(const PosXPosYExtents& parentExtents,
		       const ChildIndexInt& childIndex,
		       const QsPosType& half,
		       PosXPosYExtents* childExtents);

  //=====================================================================================
  /**
   * Determines the angular spacing between pixels (assuming rasters that are SD_TILE_SIZE
   * pixels long) for the given level
   */
  double getSpacingFromLevel(const LevelInt& givenLevel);

  //=====================================================================================
  /**
   * Determines the deepest level needed in order to have the given amount of angular spacing
   * between pixels (assuming rasters that are SD_TILE_SIZE pixels long)
   */
  LevelInt getDeepestLevelOfFaceQuadTree(double srcImageLongitudeBetweenPixels);

  //=====================================================================================
  /** Gets a sphere xyz from a lat/lon/alt and sphere radius */
  void qsd3MFromLLA(const double& lat, const double& lon, const double& alt, double* point, const double& sphereRadius);

  //=====================================================================================
  /** Gets a lat/lon/alt from a sphere xyz and sphere radius */
  void qsd3MToLLA(double& lat, double& lon, double& alt, const double* point, const double& sphereRadius);

  //=====================================================================================
  /**
   * Attempt to find the point within the specified geographic rectangle that is closest
   * to pointLon/pointLat
   */
  void getLatLongClosestToLLARectangleThatDoesNotSpanTheDateline(const double& wLon,
							         const double& eLon,
							         const double& sLat,
							         const double& nLat,
							         const double& pointLon,
							         const double& pointLat,
							         double* closestLon,
							         double* closestLat);

  //=====================================================================================
  /** Returns a cube face index 0-5 for the given lat/lon */
  bool GetFaceIndex(const double& longitude, const double& latitude, FaceIndexType& faceIndex);

  //=====================================================================================
  // The x and y values for these are referenced to the lower left corner
  void GetLatLonFromNorthXY(const double& minLat, const double& squareLength,
			    const double& x, const double& y,
			    double* longitude, double* latitude);
  void GetLatLonFromSouthXY(const double& maxLat, const double& squareLength,
			    const double& x, const double& y,
			    double* longitude, double* latitude);
  void GetLatLonGivenRange(const double& x, const double& y, double* longitude, double* latitude, const double& xMinLon, const double& yMinLat);
  void GetLatLonFromFaceXY(const FaceIndexType& faceIndex, const double& x, const double& y, double* longitude, double* latitude);
  void GetXYFromEquitorialLL(const double& longitude, const double& latitude,
			     const double& minLon, const double& maxLon,
			     const double& minLat, const double& maxLat,
			     const double& squareLength,
			     double* x, double* y);
  bool GetXYFromLonLat(const double& longitude, const double& latitude, const double& squareLength, double* posX, double* posY, FaceIndexType& faceIndex);

  //=====================================================================================
  // The x and y values for these are referenced to the pole (not the lower left corner)
  void GetXYFromNorthWWLL(const double& halfSquareLength,
			  const double& latDistance,
			  const double& longitude,
			  double* x,
			  double* y);
  void GetXYFromNorthWLL(const double& halfSquareLength,
		         const double& latDistance,
		         const double& longitude,
		         double* x,
		         double* y);
  void GetXYFromNorthELL(const double& halfSquareLength,
		         const double& latDistance,
		         const double& longitude,
		         double* x,
		         double* y);
  void GetXYFromNorthEELL(const double& halfSquareLength,
			  const double& latDistance,
			  const double& longitude,
			  double* x,
			  double* y);
  bool GetXYFromNorthLL(const double& minLat,
		        const double& squareLength,
		        const double& longitude,
		        const double& latitude,
		        double* x,
		        double* y);
  void GetXYFromSouthWWLL(const double& halfSquareLength,
		         const double& latDistance,
		         const double& longitude,
		         double* x,
		         double* y);
  void GetXYFromSouthWLL(const double& halfSquareLength,
		         const double& latDistance,
		         const double& longitude,
		         double* x,
		         double* y);
  void GetXYFromSouthELL(const double& halfSquareLength,
		         const double& latDistance,
		         const double& longitude,
		         double* x,
		         double* y);
  void GetXYFromSouthEELL(const double& halfSquareLength,
			  const double& latDistance,
			  const double& longitude,
			  double* x,
			  double* y);
  bool GetXYFromSouthLL(const double& maxLat,
		        const double& squareLength,
		        const double& longitude,
		        const double& latitude,
		        double* x,
		        double* y);

  //=====================================================================================
  void GetExtentsFromLatLon(const double& minLon,
			    const double& maxLon,
			    const double& minLat,
			    const double& maxLat,
			    PosXPosYExtents* extents);
  void GetExtentsFromLatLonNorth(const double& minLon,
			         const double& maxLon,
			         const double& minLat,
			         const double& maxLat,
			         const double& minLatNorth,
			         const double& squareLength,
			         PosXPosYExtents* extents);
  void GetExtentsFromLatLonSouth(const double& minLon,
			         const double& maxLon,
			         const double& minLat,
			         const double& maxLat,
			         const double& maxLatSouth,
			         const double& squareLength,
			         PosXPosYExtents* extents);
  //=====================================================================================
  bool GetLatLonFromExtentsNorth(double& minLon, double& maxLon, double& minLat, double& maxLat,
			         bool& spansLon180AndDoesNotCoverEntireFace,
			         double& minLon2, double& maxLon2, double& minLat2, double& maxLat2,
			         const PosXPosYExtents& extents);
  bool GetLatLonFromExtentsSouth(double& minLon, double& maxLon, double& minLat, double& maxLat,
			         bool& spansLon180AndDoesNotCoverEntireFace,
			         double& minLon2, double& maxLon2, double& minLat2, double& maxLat2,
			         const PosXPosYExtents& extents);

  //=====================================================================================
  /** Gets a node id for the specified face x/y, also returns the face x/y extents of the node id */
  bool GetNodeIDAndExtents(const QsPosType& posX,
			   const QsPosType& posY,
			   const LevelInt& maxLevel,
			   QSNodeId* nodeID,
			   PosXPosYExtents* areaExtents,
			   LevelInt* level);

  //=====================================================================================
  /** Checks the given values to see if they make sense */
  bool anyLongitudeError(double west, double east);
  bool anyLatitudeError(double south, double north);

  //=====================================================================================
  /** Checks for geographic overlap */
  bool anyLatitudeOverlap(double imageLatSouth, double imageLatNorth,
			  double areaLatMin, double areaLatMax,
			  double* overlapLatSouth, double* overlapLatNorth);
  /** Checks for geographic overlap */
  bool anyLongitudeOverlap(double imageLonWest, double imageLonEast,
			   double areaLonMin, double areaLonMax,
			   double* overlapLonWest, double* overlapLonEast);
  /** Checks for geographic overlap */
  bool anyOverlap(double imageLonWest, double imageLonEast, double imageLatSouth, double imageLatNorth,
		  double areaLonMin, double areaLonMax, double areaLatMin, double areaLatMax,
		  double* overlapLonWest, double* overlapLonEast, double* overlapLatSouth, double* overlapLatNorth);

  //=====================================================================================
  /** Gets the face x/y spacing for a node at the specified level */
  void getWholeHalfQuarter(const LevelInt& level, QsPosType* whole, QsPosType* half, QsPosType* quarter);

  //=====================================================================================
  /** Gets the face x/y spacing for a sub-square at the specified level */
  void getDeltaAndHalfDelta(const LevelInt& level, const int& numSquaresPerNode, QsPosType* delta, QsPosType* halfDelta);

  //=====================================================================================
  /**
   * Modifies the position references such that a consistent reference can always be obtained
   * Even though there are shared vertices among faces
   */
  void modifyPositionReferences(FaceIndexType& faceIndex,
			        QsPosType& posX,
			        QsPosType& posY);

  //=====================================================================================
  void ParseRasterExtentsString(const std::string& tokenValue, double& wLon, double& eLon, double& sLat, double& nLat);
  void WriteRasterExtentsString(std::string& tokenValue, const double& wLon, const double& eLon, const double& sLat, const double& nLat);

  //=====================================================================================
  void GetLatLonExtents(const FaceIndexType& faceIndex,
		        const PosXPosYExtents& extents,
		        double& minLon, double& maxLon, double& minLat, double& maxLat,
		        bool& spansLon180AndDoesNotCoverEntireFace,
		        double& minLon2, double& maxLon2, double& minLat2, double& maxLat2);

#ifdef USE_SIMDIS_SDK
} // namespace simVis_db
#endif

#endif /* QS_COMMON_GEO_H */

