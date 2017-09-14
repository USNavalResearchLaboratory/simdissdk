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
#ifndef SIMQT_QTCONVERSION_H
#define SIMQT_QTCONVERSION_H

/** @file
* This file defines a number of conversions for common qt objects
*/

#include <osg/Vec4f>
#include <QColor>
#include "simCore/Common/Export.h"

namespace simQt {

/**
 * Get a QColor from an osg Vec4f color vector, assumes color vector is R,G,B,A.
 * QColor values are 0-255, osg values are 0.0-1.0
 * @param colorVec  osg color vector
 * @return QColor  equivalent QColor object
 */
SDKQT_EXPORT QColor getQtColorFromOsg(const osg::Vec4f& colorVec);

/**
 * Get an osg Vec4f from a QColor, color vector returned is constructed as R,G,B,A
 * QColor values are 0-255, osg values are 0.0-1.0
 * @param color  QColor object
 * @return osg::Vec4f  equivalent osg color vector
 */
SDKQT_EXPORT osg::Vec4f getOsgColorFromQt(const QColor& color);

/**
 * Get an QColor from a QString, expected input str is as "R,G,B,A"
 * Int values expected in QString, ex: "255,128,0,255"
 * @param qstr  QString object
 * @return QColor  equivalent of qstr
 */
SDKQT_EXPORT QColor getQColorFromQString(const QString& qstr);

/**
 * Get a QString from a QColor, QString returned is constructed as "R,G,B,A"
 * QColor values are 0-255, ex result: "255,128,0,255"
 * @param color  QColor object
 * @return QString  equivalent of color
 */
SDKQT_EXPORT QString getQStringFromQColor(const QColor& color);

/**
 * Convert any text that may contain a UTF-8 or ANSI encoded degree symbol into a QString
 * @param text  String that may contain a degree symbol
 * @return Valid QString that properly renders degree symbol
 */
SDKQT_EXPORT QString translateDegreeSymbol(const std::string& text);

}

#endif
