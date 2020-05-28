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
#include <iostream>
#include "simCore/Calc/Math.h"
#include "swapbytes.h"
#include "QSCommon.h"
#include "QSPosXYExtents.h"

namespace simVis_db {

//=====================================================================================
PosXPosYExtents::PosXPosYExtents(QsPosType minXIn, QsPosType maxXIn, QsPosType minYIn, QsPosType maxYIn)
  : minX(minXIn),
    maxX(maxXIn),
    minY(minYIn),
    maxY(maxYIn)
{
}

void PosXPosYExtents::initialize()
{
  minX = QS_MAX_LENGTH_UINT64;
  maxX = 0;
  minY = QS_MAX_LENGTH_UINT64;
  maxY = 0;
}

bool PosXPosYExtents::isValid() const
{
  return ((minX >= maxX) || (minY >= maxY)) ? false : true;
}

void PosXPosYExtents::setAll(const PosXPosYExtents& given)
{
  minX = given.minX;
  maxX = given.maxX;
  minY = given.minY;
  maxY = given.maxY;
}

void PosXPosYExtents::setAll(const QsPosType& minXIn, const QsPosType& maxXIn, const QsPosType& minYIn, const QsPosType& maxYIn)
{
  minX = minXIn;
  maxX = maxXIn;
  minY = minYIn;
  maxY = maxYIn;
}

void PosXPosYExtents::pack(uint8_t* buffer) const
{
  if (buffer == NULL)
    return;
  beWrite(buffer, &minX);
  beWrite(buffer + sizeof(minX), &maxX);
  beWrite(buffer + sizeof(minX) + sizeof(maxX), &minY);
  beWrite(buffer + sizeof(minX) + sizeof(maxX) + sizeof(minY), &maxY);
}

void PosXPosYExtents::unpack(const uint8_t* buffer)
{
  if (buffer == NULL)
    return;
  beRead(buffer, &minX);
  beRead(buffer + sizeof(minX), &maxX);
  beRead(buffer + sizeof(minX) + sizeof(maxX), &minY);
  beRead(buffer + sizeof(minX) + sizeof(maxX) + sizeof(minY), &maxY);
}

//=====================================================================================
bool equalTo(const PosXPosYExtents& a, const PosXPosYExtents& b)
{
  if (a.minX != b.minX) return false;
  if (a.maxX != b.maxX) return false;
  if (a.minY != b.minY) return false;
  if (a.maxY != b.maxY) return false;
  return true;
}

bool operator==(const PosXPosYExtents& a, const PosXPosYExtents& b)
{
  return simVis_db::equalTo(a, b);
}

bool operator!=(const PosXPosYExtents& a, const PosXPosYExtents& b)
{
  return !simVis_db::equalTo(a, b);
}

}
