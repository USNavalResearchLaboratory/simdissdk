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
#ifndef SIMVIS_GOG_GOGNODE_H
#define SIMVIS_GOG_GOGNODE_H

#include <map>
#include <memory>
#include <string>
#include "osgEarth/GeoData"
#include "osgEarthSymbology/Style"
#include "osg/Group"
#include "simCore/Common/Common.h"
#include "simCore/Calc/Vec3.h"

namespace simCore { class UnitsRegistry; }

namespace simVis { namespace GOG
{
  /**
  * Used to track which fields are default values to avoid serializing them
  */
  enum GogSerializableField
  {
    GOG_ALL_DEFAULTS = 0,
    GOG_LINE_WIDTH_SET,
    GOG_LINE_COLOR_SET,
    GOG_LINE_STYLE_SET,
    GOG_FILL_COLOR_SET,
    GOG_DEPTH_BUFFER_SET,
    GOG_FONT_NAME_SET,
    GOG_FONT_SIZE_SET,
    GOG_TESSELLATE_SET,
    GOG_OUTLINE_SET,
    GOG_THREE_D_OFFSET_ALT_SET,
    GOG_EXTRUDE_SET,
    GOG_POINT_SIZE_SET,
    GOG_LINE_PROJECTION_SET
  };

  // stipple defines
  static const unsigned short GogDotStipple = 0xf0f0;
  static const unsigned short GogDashStipple = 0xfff0;
  static const unsigned short GogSolidStipple = 0xffff;

  // surface tessellation size (meters) for ellipsoids, spheres, etc.
  static float GogSurfaceResolution = 5000.0f;

  // Keyword in meta data to indicate the shape is relative, i.e. has xyz positions
  static const std::string RelativeShapeKeyword = "RELATIVE_SHAPE";
  // Keyword in meta data to indicate the shape has a referencepoint which may be obtained from the node's geometry
  static const std::string ReferencePointKeyword = "REFERENCE_POINT";
  // Keyword in config to indicate shape has absolute points
  static const std::string AbsoluteKeyword = "ABSOLUTE_SHAPE";

  // Forward declare an error handler for the context (see simVis/GOG/ErrorHandler.h"
  class ErrorHandler;

  /// internal: context object used by the GOG parser
  struct GOGContext
  {
    /** reference coordinate for relative objects */
    osgEarth::optional<osgEarth::GeoPoint> refPoint_;
    /** Error reporting */
    std::shared_ptr<ErrorHandler> errorHandler_;
    /** Possibly NULL pointer to the shared Units Registry */
    const simCore::UnitsRegistry* unitsRegistry_;

    GOGContext()
      : unitsRegistry_(NULL)
    {
    }
  };


  /**
   * GOG node types
   */
  enum GOGNodeType
  {
    /** Independent GOG with a specified position on the map. */
    GOGNODE_GEOGRAPHIC,

    /** GOG with relative positioning only, for attachment to an entity. */
    GOGNODE_HOSTED
  };


  /**
  * Describes the GOG's shape type
  */
  enum GogShape
  {
    GOG_UNKNOWN = 0,
    GOG_ANNOTATION,
    GOG_POINTS,
    GOG_LINE,
    GOG_LINESEGS,
    GOG_POLYGON,
    GOG_ARC,
    GOG_CIRCLE,
    GOG_ELLIPSE,
    GOG_ELLIPSOID,
    GOG_CYLINDER,
    GOG_SPHERE,
    GOG_HEMISPHERE,
    GOG_LATLONALTBOX
  };

  /** Describes the original load format of the shape */
  enum LoadFormat
  {
    FORMAT_GOG = 0,
    FORMAT_KML
  };

  /**
  * Struct that defines meta data for a GOG. The string is metadata that captures attributes of the GOG
  * that may be lost when translated to an osg::Node, specifically things like the shape type
  * (circle, polygon, etc.), as well as other characteristics that are not easily accessible
  * through the osg::Node. The metadata is captured directly from the GOG file, so it will be
  * in GOG format, with multiple lines in the single string '\n' delimited.
  * The GogShape is an enum identifying the exact shape type of the GOG for quick identification.
  */
  struct SDKVIS_EXPORT GogMetaData
  {
  public:
    std::string             metadata; ///< attributes of the GOG
    GogShape                shape; ///< identifying the exact shape type of the GOG
    LoadFormat              loadFormat; ///< indicate the original load format of the GOG
  public:
    GogMetaData();
    bool                    isSetExplicitly(GogSerializableField field) const; ///< determines if a field is explicitly set
    void                    setExplicitly(GogSerializableField field); ///< marks a field as explicitly set
    void                    clearSetFields(); ///< sets setFields to ALL_DEFAULTS
    void                    allowSetExplicitly(bool allow); ///< enable or disable setExplicitly

  private:
    uint32_t                setFields_; ///< bitmap tracking which fields have been set explicitly by a loaded file or user input
    bool                    allowingSetExplicitly_; ///< bool to prevent setExplicitly from working when setting defaults
  };

  /**
  * Struct that defines the follow orientation data for an attached gog, its locator flags will be updated with
  * the orientation components to follow, simVis::Locator::COMP_HEADING, simVis::Locator::COMP_PITCH,
  * simVis::Locator::COMP_ROLL. The offset values are in the orientationOffsets vector. Constructor initializes
  * locatorFlags to simVis::Locator::COMP_NONE.
  */
  struct SDKVIS_EXPORT GogFollowData
  {
    unsigned int            locatorFlags; ///< The orientation components to follow
    simCore::Vec3           orientationOffsets; ///< Offset values
    /** Constructor */
    GogFollowData();
  };

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_GOGNODE_H
