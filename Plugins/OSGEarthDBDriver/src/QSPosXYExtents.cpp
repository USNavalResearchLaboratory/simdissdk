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

#ifndef USE_SIMDIS_SDK
#include <assert.h>
#include <stdlib.h>

#include "simCore/Calc/Math.h"
#include "stlstdlib.h"
#include "iostreamc"
#include "iomanipc"
#include "swapbytes.h"
#else // used in SIMDIS SDK
#include <iostream>
#include "simCore/Calc/Math.h"
#include "swapbytes.h"

using namespace simVis_db;

#endif

#include "QSCommonGeo.h"

//=====================================================================================
PosXPosYExtents::PosXPosYExtents(QsPosType minXIn, QsPosType maxXIn, QsPosType minYIn, QsPosType maxYIn)
  : minX(minXIn),
    maxX(maxXIn),
    minY(minYIn),
    maxY(maxYIn)
{
}

void PosXPosYExtents::Initialize()
{
  minX = gQsMaxLength;
  maxX = 0;
  minY = gQsMaxLength;
  maxY = 0;
}

bool PosXPosYExtents::Valid() const
{
  return ((minX >= maxX) || (minY >= maxY)) ? false : true;
}

void PosXPosYExtents::SetAll(const PosXPosYExtents& given)
{
  minX = given.minX;
  maxX = given.maxX;
  minY = given.minY;
  maxY = given.maxY;
}

void PosXPosYExtents::SetAll(const QsPosType& minXIn, const QsPosType& maxXIn, const QsPosType& minYIn, const QsPosType& maxYIn)
{
  minX = minXIn;
  maxX = maxXIn;
  minY = minYIn;
  maxY = maxYIn;
}

void PosXPosYExtents::Pack(uint8_t* buffer) const
{
  if (buffer == NULL)
    return;
  bewrite(buffer, &minX);
  bewrite(buffer + sizeof(minX), &maxX);
  bewrite(buffer + sizeof(minX) + sizeof(maxX), &minY);
  bewrite(buffer + sizeof(minX) + sizeof(maxX) + sizeof(minY), &maxY);
}

void PosXPosYExtents::UnPack(const uint8_t* buffer)
{
  if (buffer == NULL)
    return;
  beread(buffer, &minX);
  beread(buffer + sizeof(minX), &maxX);
  beread(buffer + sizeof(minX) + sizeof(maxX), &minY);
  beread(buffer + sizeof(minX) + sizeof(maxX) + sizeof(minY), &maxY);
}

void PosXPosYExtents::UnPackHexChars(const char* hexChars)
{
  if (hexChars == NULL)
    return;
  uint8_t buffer[sizeof(PosXPosYExtents)];

  uint32_t tmpInt;
  char tmpHex[3];
  tmpHex[2] = 0;

  for (size_t i = 0; i < sizeof(PosXPosYExtents); ++i)
  {
    tmpHex[0] = hexChars[i*2];
    tmpHex[1] = hexChars[i*2 + 1];
    sscanf(tmpHex, "%x", &tmpInt);
    buffer[i] = (uint8_t)tmpInt;
  }
  UnPack(buffer);
}

void PosXPosYExtents::Print()
{
  std::cerr << "minX = " << minX << "\n";
  std::cerr << "maxX = " << maxX << "\n";
  std::cerr << "minY = " << minY << "\n";
  std::cerr << "maxY = " << maxY << "\n";
}

//=====================================================================================
#ifdef USE_SIMDIS_SDK
bool simVis_db::equalTo(const PosXPosYExtents& a, const PosXPosYExtents& b)
#else
bool equalTo(const PosXPosYExtents& a, const PosXPosYExtents& b)
#endif
{
  if (a.minX != b.minX) return false;
  if (a.maxX != b.maxX) return false;
  if (a.minY != b.minY) return false;
  if (a.maxY != b.maxY) return false;
  return true;
}

#ifdef USE_SIMDIS_SDK
bool simVis_db::operator==(const PosXPosYExtents& a, const PosXPosYExtents& b)
#else
bool operator==(const PosXPosYExtents& a, const PosXPosYExtents& b)
#endif
{
  return equalTo(a, b);
}

bool operator!=(const PosXPosYExtents& a, const PosXPosYExtents& b)
{
#ifdef USE_SIMDIS_SDK
  return !simVis_db::equalTo(a, b);
#else
  return !equalTo(a, b);
#endif
}

//=====================================================================================
#ifdef USE_SIMDIS_SDK
void simVis_db::UpdateExtents(const QsPosType& posX, const QsPosType& posY, PosXPosYExtents* extents)
#else
void UpdateExtents(const QsPosType& posX, const QsPosType& posY, PosXPosYExtents* extents)
#endif
{
  if (extents == NULL)
    return;

  extents->minX = simCore::sdkMin(extents->minX, posX);
  extents->minY = simCore::sdkMin(extents->minY, posY);
  extents->maxX = simCore::sdkMax(extents->maxX, posX);
  extents->maxY = simCore::sdkMax(extents->maxY, posY);
}

#ifdef USE_SIMDIS_SDK
bool simVis_db::Copy6Extents(const PosXPosYExtents* copyFrom, PosXPosYExtents* copyTo)
#else
bool Copy6Extents(const PosXPosYExtents* copyFrom, PosXPosYExtents* copyTo)
#endif
{
  if ((copyFrom == NULL) || (copyTo == NULL))
    return false;

  FaceIndexType faceIndex;
  for (faceIndex = 0; faceIndex < 6; ++faceIndex)
    copyTo[faceIndex].SetAll(copyFrom[faceIndex]);

  return true;
}

#ifdef USE_SIMDIS_SDK
bool simVis_db::AnyOverlap(const PosXPosYExtents& extA, const PosXPosYExtents& extB)
#else
bool AnyOverlap(const PosXPosYExtents& extA, const PosXPosYExtents& extB)
#endif
{
  if ((extA.Valid() == false) || (extB.Valid() == false))
    return false;

  // checks for no x overlap
  if ((extA.minX > extB.maxX) ||
     (extA.maxX < extB.minX))
    return false;

  // checks for no y overlap
  if ((extA.minY > extB.maxY) ||
     (extA.maxY < extB.minY))
    return false;

  return true;
}

#ifdef USE_SIMDIS_SDK
bool simVis_db::AnyOverlap(const QsPosType& posX, const QsPosType& posY, const PosXPosYExtents& extents)
#else
bool AnyOverlap(const QsPosType& posX, const QsPosType& posY, const PosXPosYExtents& extents)
#endif
{
  if (extents.Valid() == false)
    return false;

  return ((posX < extents.minX) ||
	  (posX > extents.maxX) ||
	  (posY < extents.minY) ||
	  (posY > extents.maxY)) ? false : true;
}