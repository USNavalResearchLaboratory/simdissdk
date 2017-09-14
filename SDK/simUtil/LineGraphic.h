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
#ifndef SIMUTIL_LINEGRAPHIC_H
#define SIMUTIL_LINEGRAPHIC_H

#include <string>
#include "osg/ref_ptr"
#include "osg/Referenced"
#include "osgEarthSymbology/Style"
#include "simCore/Calc/Vec3.h"
#include "simCore/Common/Common.h"
#include "simData/DataStore.h"
#include "simVis/Types.h"

namespace osg { class Group; }
namespace osgEarth {
  class MapNode;
  namespace Annotation {
    class LabelNode;
  }
}

namespace simVis { class AnimatedLineNode; }

namespace simUtil {

class Position;

/** Represents a labeled line graphic drawn in 3D from an origin Position to a destination Position. */
class SDKUTIL_EXPORT LineGraphic
{
public:
  LineGraphic(osg::Group* scene, osgEarth::MapNode* mapNode);
  virtual ~LineGraphic();

  /**
  * Sets the origin and destination position and updates the label text.
  * @param origin Position of the line graphic's origin.
  * @param destination Position of the line graphic's destination.
  * @param labelString Text string to set to the label.
  */
  void set(const Position* origin, const Position* destination, const std::string& labelString);

  /**
  * Sets the origin and destination position and updates the label text.
  * @param origin LLA of the line graphic's origin.
  * @param destination LLA of the line graphic's destination.
  * @param labelString Text string to set to the label.
  */
  void set(const simCore::Vec3& originLLA, const simCore::Vec3& destinationLLA, const std::string& labelString);

  /**
   * Hides or reveals both the animatedLine and the label.
   * Calling this on a LineGraphic with equal endpoints is undefined.
   * @param draw If true, the line and label will be displayed
   */
  void setDraw(bool draw);

  /**
  * Sets the width of the line.
  * @param lineWidth Width to which to set the line.
  */
  void setLineWidth(float lineWidth);

  /**
  * Sets the stipple pattern in OpenGL format.
  * @param stipple Stipple pattern for the line.
  */
  void setStipplePattern(unsigned short stipple);

  /**
  * Sets the line color.
  * @param color simVis::Color to set the line color to (RRGGBBAA).
  */
  void setLineColor(const simVis::Color& color);

  /**
  * Sets the label text color.
  * @param color simVis::Color to set the label text color to (RRGGBBAA).
  */
  void setTextColor(const simVis::Color& color);

  /**
  * Sets the label text font.
  * @param fontName Name of the font to set the line text font to (e.g. arialbd.ttf).
  */
  void setFont(const std::string& fontName);

  /**
  * Sets the label text font size.
  * @param fontSize Size of the font, in SIMDIS units; converted into OSG units.
  */
  void setFontSize(float fontSize);

  /**
  * Retrieves the LineGraphic's line component.
  * @return simVis::AnimatedLineNode representing the line component of the LineGraphic.
  */
  simVis::AnimatedLineNode* animatedLine() const;

  /**
  * Retrieves the LineGraphic's label component.
  * @return simVis::AnimatedLineNode representing the label component of the LineGraphic.
  */
  osgEarth::Annotation::LabelNode* label() const;

private:
  osg::Group* scene_;

  osg::ref_ptr<osgEarth::SpatialReference> wgs84Srs_;
  osg::ref_ptr<simVis::AnimatedLineNode> animatedLine_;
  osgEarth::Symbology::Style labelStyle_;
  osg::ref_ptr<osgEarth::Annotation::LabelNode> label_;
};

/** Base class for a position described in an LLA coordinate. */
class SDKUTIL_EXPORT Position : public osg::Referenced
{
public:
  /**
  * Checks if the Position is valid.
  * @return true if the Position is valid, false if invalid.
  */
  virtual bool isValid() const = 0;

  /**
  * If the Position is valid, retrieves the Position's
  * lat/lon/alt coordinate in a simCore::Vec3.
  * @return lla simCore::Vec3 containing a valid lat/lon/alt coordinate.
  */
  virtual const simCore::Vec3& lla() const = 0;

  /** Returns true if equal to other position */
  virtual bool operator==(const Position& other) const = 0;
  /** Returns true if not equal to other position */
  virtual bool operator!=(const Position& other) const = 0;

protected:
  /** Reference-derived */
  virtual ~Position() {}

  /** Helper function that can be used to implement operator==() and operator!=() */
  bool positionEquals_(const Position& other) const;
};

/** Position is defined as static and that doesn't move automatically. */
class SDKUTIL_EXPORT StaticPosition : public Position
{
public:
  /** Initialize without a valid position. */
  StaticPosition();
  /** Initialize with a valid position. */
  explicit StaticPosition(const simCore::Vec3& lla);
  /** Copy constructor from another Position (making it static). */
  explicit StaticPosition(const Position& copy);

  /**
  * Resets the position, setting it to invalid.
  */
  void clear();

  /**
  * Sets a valid position.
  * @param lla simCore::Vec3 containing a valid lat/lon/alt coordinate to set.
  */
  void setLla(const simCore::Vec3& lla);

  virtual bool isValid() const;
  virtual const simCore::Vec3& lla() const;
  virtual bool operator==(const Position& other) const;
  virtual bool operator!=(const Position& other) const;

protected:
  /** Reference-derived */
  virtual ~StaticPosition();

private:
  bool valid_;
  simCore::Vec3 lla_;
};

/** Position based off a platform's LLA coordinate location. */
class SDKUTIL_EXPORT PlatformPosition : public Position
{
public:
  /** Initialize from a data store. */
  PlatformPosition(const simData::DataStore& dataStore, simData::ObjectId platformId);

  virtual bool isValid() const;
  virtual const simCore::Vec3& lla() const;
  virtual bool operator==(const Position& other) const;
  virtual bool operator!=(const Position& other) const;

protected:
  /** Reference-derived */
  virtual ~PlatformPosition();

private:
  /** Returns 0 on success, pulling LLA out of the data store. */
  int pullFromDataStore_(simCore::Vec3& outLla) const;

  const simData::DataStore& dataStore_;
  simData::ObjectId platformId_;
  /** Cache of the LLA from the data store. */
  mutable simCore::Vec3 lla_;
};

}

#endif /* SIMUTIL_LINEGRAPHIC_H */
