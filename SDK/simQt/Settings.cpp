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
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <QColor>
#include <QDataStream>
#include "simQt/Settings.h"

namespace simQt
{

bool Settings::MetaData::operator==(const MetaData& rhs) const
{
  return type_ == rhs.type_ && defaultValue_ == rhs.defaultValue_ && toolTip_ == rhs.toolTip_
    && level_ == rhs.level_ && minValue_ == rhs.minValue_ && maxValue_ == rhs.maxValue_
    && numDecimals_ == rhs.numDecimals_ && filenameFilter_ == rhs.filenameFilter_ && enumValues_ == rhs.enumValues_;
}

Settings::DataTypes Settings::MetaData::type() const
{
  return type_;
}

QVariant Settings::MetaData::defaultValue() const
{
  return defaultValue_;
}

QString Settings::MetaData::toolTip() const
{
  return toolTip_;
}

Settings::DataLevel Settings::MetaData::level() const
{
  return level_;
}

QVariant Settings::MetaData::minValue() const
{
  return minValue_;
}

QVariant Settings::MetaData::maxValue() const
{
  return maxValue_;
}

int Settings::MetaData::numDecimals() const
{
  return numDecimals_;
}

QVariant Settings::MetaData::filenameFilter() const
{
  return filenameFilter_;
}

QMap<int, QString> Settings::MetaData::enumValues() const
{
  return enumValues_;
}

Settings::MetaData& Settings::MetaData::addEnumeration(int key, const QString& value)
{
  enumValues_[key] = value;
  return *this;
}

/** Factory method for meta data representing an integer value */
Settings::MetaData Settings::MetaData::makeInteger(const QVariant& defaultValue, const QString& tooltip,
                                                   Settings::DataLevel inLevel, const QVariant& minValue,
                                                   const QVariant& maxValue)
{
  return Settings::MetaData(INTEGER, defaultValue, tooltip, inLevel, minValue, maxValue);
}

/** Factory method for meta data representing a double value */
Settings::MetaData Settings::MetaData::makeDouble(const QVariant& defaultValue, const QString& tooltip,
                                                  Settings::DataLevel inLevel, const QVariant& minValue,
                                                  const QVariant& maxValue, int numDecimals)
{
  return Settings::MetaData(DOUBLE, defaultValue, tooltip, inLevel, minValue, maxValue, numDecimals);
}

/** Factory method for meta data representing a boolean value */
Settings::MetaData Settings::MetaData::makeBoolean(const QVariant& defaultValue, const QString& tooltip,
                                                   Settings::DataLevel inLevel)
{
  return Settings::MetaData(BOOLEAN, defaultValue, tooltip, inLevel);
}

/** Factory method for meta data representing a string value */
Settings::MetaData Settings::MetaData::makeString(const QVariant& defaultValue,
                                                  const QString& tooltip, Settings::DataLevel inLevel)
{
  return Settings::MetaData(STRING, defaultValue, tooltip, inLevel);
}

/**
* Factory method for meta data representing a filename value.  Filter follows rules of
* formatting described by simQt::FileDialog::loadFile() and simQt::FileSelectorWidget.
*/
Settings::MetaData Settings::MetaData::makeFilename(const QVariant& defaultValue,
                                                    const QString& tooltip, Settings::DataLevel inLevel,
                                                    const QVariant filenameFilter)
{
  return Settings::MetaData(FILENAME, defaultValue, tooltip, inLevel, QVariant(), QVariant(), 2, filenameFilter);
}

/**
* Factory method for meta data representing a directory value.  Filter follows rules of
* formatting described by simQt::DirectorySelectorWidget.
*/
Settings::MetaData Settings::MetaData::makeDirectory(const QVariant& defaultValue,
                                                    const QString& tooltip, Settings::DataLevel inLevel)
{
  return Settings::MetaData(DIRECTORY, defaultValue, tooltip, inLevel, QVariant(), QVariant());
}

/** Factory method for meta data representing a color value.  Use QColor or QRgb. */
Settings::MetaData Settings::MetaData::makeColor(const QVariant& defaultValue,
                                                 const QString& tooltip, Settings::DataLevel inLevel)
{
  if (static_cast<int>(defaultValue.type()) == QMetaType::QColor)
    return Settings::MetaData(COLOR, defaultValue.value<QColor>().rgba(), tooltip, inLevel);
  return Settings::MetaData(COLOR, defaultValue, tooltip, inLevel);
}

/** Factory method for meta data representing a point value */
Settings::MetaData Settings::MetaData::makePoint(const QVariant& defaultValue,
                                                 const QString& tooltip, Settings::DataLevel inLevel)
{
  return Settings::MetaData(POINT, defaultValue, tooltip, inLevel);
}

/** Factory method for meta data representing a size value */
Settings::MetaData Settings::MetaData::makeSize(const QVariant& defaultValue,
                                                const QString& tooltip, Settings::DataLevel inLevel)
{
  return Settings::MetaData(SIZE, defaultValue, tooltip, inLevel);
}

/** Factory method for meta data representing an enumeration value */
Settings::MetaData Settings::MetaData::makeEnumeration(const QVariant& defaultValue, const QMap<int, QString>& enumValues,
                                                       const QString& tooltip, Settings::DataLevel inLevel)
{
  Settings::MetaData rv(ENUMERATION, defaultValue, tooltip, inLevel, QVariant(), QVariant(),
    2, QVariant());
  rv.enumValues_ = enumValues;
  return rv;
}

/** Factory method for meta data representing an enumeration value, with no initial values (use addEnumeration()) */
Settings::MetaData Settings::MetaData::makeEnumeration(const QVariant& defaultValue, const QString& tooltip, Settings::DataLevel inLevel)
{
  return Settings::MetaData(ENUMERATION, defaultValue, tooltip, inLevel, QVariant(), QVariant(), 2, QVariant());
}

/**
* Factory method for meta data representing a font value.
*/
Settings::MetaData Settings::MetaData::makeFont(const QVariant& defaultValue,
                                                const QString& tooltip, Settings::DataLevel inLevel)
{
  return Settings::MetaData(FONT, defaultValue, tooltip, inLevel, QVariant(), QVariant());
}

Settings::MetaData Settings::MetaData::makeQFont(const QVariant& defaultValue,
                                       const QString& tooltip, Settings::DataLevel inLevel)
{
  return Settings::MetaData(QFONT, defaultValue, tooltip, inLevel, QVariant(), QVariant());
}

Settings::MetaData Settings::MetaData::makeHex(const QVariant& defaultValue,
                                               const QString& tooltip,
                                               Settings::DataLevel inLevel,
                                               const QVariant& minValue,
                                               const QVariant& maxValue)
{
  return Settings::MetaData(HEX, defaultValue, tooltip, inLevel, minValue, maxValue);
}

/** Public multifaceted default constructor; NOTE: Used by QVariant */
Settings::MetaData::MetaData(Settings::DataTypes inType, const QVariant& inDefaultValue,
                             const QString& inToolTip, Settings::DataLevel inLevel,
                             const QVariant& inMinValue, const QVariant& inMaxValue,
                             int inNumDecimals, const QVariant filenameFilter)
 : type_(inType),
   defaultValue_(inDefaultValue),
   toolTip_(inToolTip),
   level_(inLevel),
   minValue_(inMinValue),
   maxValue_(inMaxValue),
   numDecimals_(inNumDecimals),
   filenameFilter_(filenameFilter)
{
}

void Settings::MetaData::serialize(QDataStream& out) const
{
  out << static_cast<int>(type_) << defaultValue_ << toolTip_ << static_cast<int>(level_)
    << minValue_ << maxValue_ << numDecimals_ << filenameFilter_ << enumValues_;
}

void Settings::MetaData::deserialize(QDataStream& in)
{
  int tmpInt;
  in >> tmpInt;
  type_ = static_cast<Settings::DataTypes>(tmpInt);
  in >> defaultValue_ >> toolTip_;
  in >> tmpInt;
  level_ = static_cast<Settings::DataLevel>(tmpInt);
  in >> minValue_ >> maxValue_ >> numDecimals_ >> filenameFilter_ >> enumValues_;
}

}

/// Serialize for Settings availability
QDataStream& operator<<(QDataStream& out, const simQt::Settings::MetaData& metaData)
{
  metaData.serialize(out);
  return out;
}

/// Deserialize for Settings availability
QDataStream& operator>>(QDataStream& in, simQt::Settings::MetaData& metaData)
{
  metaData.deserialize(in);
  return in;
}
