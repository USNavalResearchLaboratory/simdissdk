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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_UTILS_H
#define SIMVIS_UTILS_H

#include <functional>

#include "simCore/Common/Common.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/Vec3.h"
#include "simCore/Time/Clock.h"
#include "simData/DataStore.h"
#include "simData/DataStoreHelpers.h"
#include "simData/DataTypes.h"

#include "osg/BoundingBox"
#include "osg/Geometry"
#include "osg/Matrix"
#include "osg/NodeCallback"
#include "osg/Notify"
#include "osg/Quat"
#include "osg/Transform"
#include "osgGA/GUIEventHandler"
#include "osgText/Text"

#include "osgEarth/Color"
#include "osgEarth/GeoData"
#include "osgEarth/MapNode"
#include "osgEarth/Registry"
#include "osgEarth/ShaderFactory"
#include "osgEarth/SpatialReference"
#include "osgEarth/Units"
#include "osgEarth/Version"

#if OSGEARTH_SOVERSION >= 151
namespace osgEarth { class UnitsType; }
#else
namespace osgEarth { using UnitsType = Units; }
#endif

// MACROS to test for changes in protobuf properties.

#define PB_HAS_FIELD(a, field) ( \
  ((a)!=nullptr) && ((a)->has_##field()) )

#define PB_DOESNT_HAVE_FIELD(a, field) ( \
  ((a)==nullptr) || (!(a)->has_##field()) )

#define PB_FIELD_APPEARED(a, b, field) ( \
  PB_DOESNT_HAVE_FIELD((a), field) && \
  PB_HAS_FIELD((b), field) )

#define PB_FIELD_DISAPPEARED(a, b, field) ( \
  PB_HAS_FIELD((a), field) && \
  PB_DOESNT_HAVE_FIELD((b), field) )

#define PB_FIELD_STATUS_CHANGED(a, b, field) ( \
  PB_HAS_FIELD((a), field) != PB_HAS_FIELD((b), field) )

#define PB_BOTH_HAVE_FIELD(a, b, field) ( \
  PB_HAS_FIELD((a), field) && PB_HAS_FIELD((b), field) )

#define PB_FIELD_CHANGED(a, b, field) ( \
  PB_FIELD_STATUS_CHANGED((a), (b), field) || ( \
    PB_BOTH_HAVE_FIELD((a), (b), field) && ((a)->field() != (b)->field()) ) )

#define PB_REPEATED_FIELD_CHANGED(a, b, field) ( \
  ((a)->field ## _size() != (b)->field ## _size()) || \
  (simData::DataStoreHelpers::vecFromRepeated((a)->field()) != simData::DataStoreHelpers::vecFromRepeated((b)->field())) )


#define PB_HAS_SUBFIELD(a, first, second) ( \
  PB_HAS_FIELD((a), first) && (a)->first().has_##second() )

#define PB_DOESNT_HAVE_SUBFIELD(a, first, second) ( \
  PB_DOESNT_HAVE_FIELD((a), first) || (!(a)->first().has_##second()) )

#define PB_BOTH_HAVE_SUBFIELD(a, b, first, second) ( \
  PB_HAS_SUBFIELD((a), first, second) && PB_HAS_SUBFIELD((b), first, second) )

#define PB_SUBFIELD_APPEARED(a, b, first, second) ( \
  PB_DOESNT_HAVE_SUBFIELD((a), first, second) && PB_HAS_SUBFIELD((b), first, second) )

#define PB_SUBFIELD_STATUS_CHANGED(a, b, first, second) ( \
  PB_HAS_SUBFIELD((a), first, second) != PB_HAS_SUBFIELD((b), first, second) )

#define PB_SUBFIELD_CHANGED(a, b, first, second) ( \
  PB_SUBFIELD_STATUS_CHANGED((a), (b), first, second) || ( \
    PB_BOTH_HAVE_SUBFIELD((a), (b), first, second) && \
    (a)->first().second() != (b)->first().second() ) )

namespace osgEarth { class LineDrawable; }

namespace osgViewer {
  class StatsHandler;
  class View;
}

namespace simVis
{
  class PlatformModelNode;

  /**
   * Enable or disable lighting on a state set. We must set both the
   * FFP mode and the uniform to support shader-based and non-shader-based
   * nodes.
   */
  SDKVIS_EXPORT void setLighting(osg::StateSet* stateset, osg::StateAttribute::GLModeValue value);

  /**
   * remove the lighting directive from a state set so that it inherits
   * its lighting setting.
   */
  SDKVIS_EXPORT void setLightingToInherit(osg::StateSet* stateset);

  /** gets the lighting state if there is one (true if there is, false if not) */
  SDKVIS_EXPORT bool getLighting(osg::StateSet* stateset, osg::StateAttribute::OverrideValue& out_value);

  /**
  * Fixes an osg::Texture to be OpenGL core profile compliant.  A texture cannot have a pixel format that
  * matches GL_LUMINANCE or GL_LUMINANCE_ALPHA in OpenGL core profile.  This method detects that case, fixes
  * the pixel format, and applies a swizzle to correctly map GL_RED or GL_RG components to visible spectrum.
  * By necessity, this modifies texture->getImage().
  */
  SDKVIS_EXPORT void fixTextureForGlCoreProfile(osg::Texture* texture);

  /**
   * Wraps a simple callback method. This callback can be used for any of a variety of OSG
   * methods, such as update callbacks. For example:
   *
   * <code>
   * node->addUpdateCallback(new LambdaOsgCallback([] { SIM_INFO << "Update!\n"; }));
   * </code>
   *
   * This class supports lambdas matching the interface void(void).
   */
  struct LambdaOsgCallback : public osg::Callback
  {
    /** Instantiates the callback with your lambda */
    explicit LambdaOsgCallback(const std::function<void()>& voidFunc)
      : voidFunc_(voidFunc)
    {
    }

    /** Calls update on the node and continues traversal */
    virtual bool run(osg::Object* object, osg::Object* data) override
    {
      voidFunc_();
      return traverse(object, data);
    }

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }
    /** Return the class name */
    virtual const char* className() const { return "LambdaOsgCallback"; }

  protected:
    /// osg::Referenced-derived
    virtual ~LambdaOsgCallback() {}

  private:
    std::function<void()> voidFunc_;
  };

  /**
   * Utility template method to find the first Update Callback of the given type.
   * @param node Node to search the update callback chain of
   * @return First update callback that is of type T, or nullptr if none is found.
   */
  template <typename T>
  T* findUpdateCallbackOfType(osg::Node* node)
  {
    osg::Callback* callback = node->getUpdateCallback();
    while (callback)
    {
      T* asType = dynamic_cast<T*>(callback);
      if (asType)
        return asType;
      callback = callback->getNestedCallback();
    }
    return nullptr;
  }

  /// convert a simCore::Coordinate to a GeoPoint, if possible
  SDKVIS_EXPORT bool convertCoordToGeoPoint(
    const simCore::Coordinate&        input,
    osgEarth::GeoPoint&               output,
    const osgEarth::SpatialReference* mapSRS);

  /// convert a GeoPoint to a simCore::Coordinate, if possible
 SDKVIS_EXPORT bool convertGeoPointToCoord(
    const osgEarth::GeoPoint&         input,
    simCore::Coordinate&              output,
    osgEarth::MapNode*                mapNode);

  /// whether a simCore::CoordinateSystem represents world-space coords.
  inline bool isWorldCoordSys(const simCore::CoordinateSystem& cs)
  {
    return
      cs == simCore::COORD_SYS_ECEF ||
      cs == simCore::COORD_SYS_LLA  ||
      cs == simCore::COORD_SYS_ECI;
  }

  /// whether a coordinate system represents localized coords.
  inline bool isLocalCoordSys(const simCore::CoordinateSystem& cs)
  {
    return
      cs == simCore::COORD_SYS_ENU ||
      cs == simCore::COORD_SYS_GTP ||
      cs == simCore::COORD_SYS_NED ||
      cs == simCore::COORD_SYS_NWU ||
      cs == simCore::COORD_SYS_XEAST;
  }

  /// convert an OSG vec3 to a SIM vec3
  inline simCore::Vec3 convertToSim(const osg::Vec3d& in)
  {
    return simCore::Vec3(in.x(), in.y(), in.z());
  }

  /// convert a OSG vec3 to a simCore::Coordinate
  inline simCore::Coordinate convertOSGtoSimCoord(const osg::Vec3d& in, const simCore::CoordinateSystem& cs)
  {
    return simCore::Coordinate(cs, simCore::Vec3(in.x(), in.y(), in.z()));
  }

  /// convert a SIM vec3 to an OSG vec3
  inline osg::Vec3d convertToOSG(const simCore::Vec3& in)
  {
    return osg::Vec3d(in.x(), in.y(), in.z());
  }

  /**
   * Converts a model from NWU to ENU (in place).
   */
  SDKVIS_EXPORT void convertNWUtoENU(osg::Node* node);

  /**
   * True if the filename is that of a known image type; false otherwise.
   * @param[in ] location Filename or URI.
   * @return     True or false
   */
  SDKVIS_EXPORT bool isImageFile(const std::string& location);

  /**
   * Convert simData DistanceUnits to osgEarth::Units.
   */
  SDKVIS_EXPORT osgEarth::UnitsType convertUnitsToOsgEarth(const simData::DistanceUnits& input);

  /**
   * Convert simData SpeedUnits to osgEarth::Units.
   */
  SDKVIS_EXPORT osgEarth::UnitsType convertUnitsToOsgEarth(const simData::SpeedUnits& input);

  /** Given an icon alignment and image size, gives offsets from center. */
  SDKVIS_EXPORT void iconAlignmentToOffsets(simData::TextAlignment align, const osg::Vec2f& iconDims, osg::Vec2f& outOffsets);

  /// Returns the thickness associated with the TextOutline setting in pixels.
  SDKVIS_EXPORT float outlineThickness(simData::TextOutline outline);

  /// Returns font size that scales relative to SIMDIS 9
  SDKVIS_EXPORT float osgFontSize(float simFontSize);

  /// Returns font size that scales from OSG to SIMDIS 9
  SDKVIS_EXPORT float simdisFontSize(float osgFontSize);

  /// Converts from protobuf label backdrop type to OSG backdrop type
  SDKVIS_EXPORT osgText::Text::BackdropType backdropType(simData::BackdropType type);

  /// Converts from protobuf label backdrop implementation to OSG backdrop implementation
  SDKVIS_EXPORT osgText::Text::BackdropImplementation backdropImplementation(simData::BackdropImplementation implementation);

  /**
   * The removal of the text shader in GL2 compatible contexts in OSG 3.4.1 has a side effect, at least on
   * Linux, of causing the StatsViewer text to appear blocky.  This is because the StatsViewer installs a
   * shader program which does not cover textures for text.  Fastest solution to this problem is to
   * simply remove the program, since it's not necessary in GL2 compatible contexts and only causes problems.
   * This is safe to run on non-GL2 systems and is ifdef'd appropriately.
   * @param statsHandler osgViewer or simUtil StatsHandler for which to remove the program.
   */
  SDKVIS_EXPORT void fixStatsHandlerGl2BlockyText(osgViewer::StatsHandler* statsHandler);

  /**
   * Calculate the intersection point of line defined by start/end points and earth surface
   * @param lat  latitude in radians at which to calculate
   * @param ecefStart  starting point
   * @param ecefEnd  ending point
   * @param earthIntersection  calculated intersection point
   * @return true if an intersection point was calculated
   */
  bool calculateEarthIntersection(double lat, const osg::Vec3d& ecefStart, const osg::Vec3d& ecefEnd, osg::Vec3d& earthIntersection);

  /// Math helper functions
  struct SDKVIS_EXPORT Math
  {
    /**
    * @name simVis::eulerDegToQuat() and simVis::eulerRadToQuat()
    * Convert Euler angles to a quaternion.
    *
    * The input Euler angles must follow the simCore semantics. That is:
    *
    *   +azimuth   => right turn
    *   +elevation => nose up
    *   +roll      => right wing down
    *
    * Rotation order is Azim-Elev-Roll.
    *
    * @param hpr_deg Orientation angles
    * @return        Quaternion
    */
    static osg::Quat eulerDegToQuat(double h, double p, double r);
    static osg::Quat eulerDegToQuat(const osg::Vec3d& hpr) { return eulerDegToQuat(hpr[0], hpr[1], hpr[2]); }
    static osg::Quat eulerRadToQuat(double h, double p, double r);
    static osg::Quat eulerRadToQuat(const osg::Vec3d& hpr) { return eulerRadToQuat(hpr[0], hpr[1], hpr[2]); }
    ///@}

    /**
    * @name simVis::quatToEulerRad() and simVis::quatToEulerDeg()
    * Convert a quaternion to Euler angles (HPR in degrees)
    * @{
    * @param quat Quaternion
    * @return     Euler angles (HPR, degrees)
    */
    static osg::Vec3d quatToEulerRad(const osg::Quat& quat);
    static osg::Vec3d quatToEulerDeg(const osg::Quat& quat);
    ///@}

    /**
    * Converts a SIMDIS ECEF orientation (psi/theta/phi) into an OSG
    * ENU rotation matrix. The SIMDIS d3EulertoQ() method results in a
    * NED orientation frame. We want ENU so we have to fix the conversion.
    * @param in SIMDIS ECEF orientation (psi/theta/phi)
    * @param out OSG ENU rotation matrix
    */
    static void ecefEulerToEnuRotMatrix(const simCore::Vec3& in, osg::Matrix& out);

    /**
    * Converts an ENU (OSG style) rotation matrix into SIMDIS
    * (NED frame) global Euler angles -- this is the inverse of
    * the method ecefEulerToEnuRotMatrix().
    * @param in OSG ENU rotation matrix
    * @param out SIMDIS (NED frame) global Euler angles
    */
    static void enuRotMatrixToEcefEuler(const osg::Matrix& in, simCore::Vec3& out);

    /**
    * Clamp the orientation of a matrix to the specified Euler angles.
    * The matrix cannot be scaled, else this will return incorrect values.
    * @param mat         Matrix within which to clamp values (input/output)
    * @param min_hpr_deg Minimum allowable Euler angles (HPR, degrees)
    * @param max_hpr_deg Maximum allowable Euler angles (HRP, degrees)
    */
    static void clampMatrixOrientation(osg::Matrixd& mat, osg::Vec3d& min_hpr_deg, osg::Vec3d& max_hpr_deg);

    /**
    * Takes an ECEF position and return the projected position at altitude 0
    * Similar to simCore::clampEcefPointToGeodeticSurface() but more efficient when matrix is already available
    * @param ecefPos specified position
    * @param local2world ENU matrix at specified position
    * @return ECEF point at altitude 0
    */
    static osg::Vec3d ecefEarthPoint(const simCore::Vec3& ecefPos, const osg::Matrixd& world2local);

    /**
    * Quaternion spherical linear interpolator - for sweeping one quat onto another
    * when creating 3D arcs (pie slices)
    */
    struct QuatSlerp
    {
      /** Current interpolated state */
      osg::Quat quat_;
      /** Initial state from which to base interpolation */
      osg::Quat from_;
      /** Target state to which to interpolate */
      osg::Quat to_;
      /** Flags whether to take the long way */
      bool longWay_;
      /** Cos of the angle of rotation */
      double cosAngle_;
      /** Caches the sin of angle of rotation */
      double sinAngle_;
      /** Angle of rotation */
      double angle_;

      /** Constructs a new QuatSlerp between two angles */
      QuatSlerp(const osg::Quat& from, const osg::Quat& to, bool longWay)
        : from_(from), to_(to), longWay_(longWay), sinAngle_(0.0), angle_(0)
      {
        cosAngle_ = from.asVec4() * to.asVec4();
        if ((cosAngle_ < 0.0 && !longWay_) || (cosAngle_ > 0.0 && longWay_))
        {
          cosAngle_ = -cosAngle_;
          to_ = -to_;
        }

        if ((1.0-cosAngle_) > 0.00001)
        {
          angle_ = acos(cosAngle_);
          sinAngle_ = sin(angle_);
        }
      }

      /// functor operation
      osg::Quat& operator()(double t)
      {
        double scaleFrom, scaleTo;

        if ((1.0-cosAngle_) > 0.00001)
        {
          assert(sinAngle_ != 0.0);
          scaleFrom = sin((1.0-t)*angle_)/sinAngle_;
          scaleTo   = sin(t*angle_)/sinAngle_;
        }
        else
        {
          scaleFrom = 1.0-t;
          scaleTo   = t;
        }

        quat_ = (from_*scaleFrom) + (to_*scaleTo);
        return quat_;
      }
    };
  };

  /** Map of color index to OSG (RGBA) color */
  typedef std::map<int, osg::Vec4> ColorMap;

  /** Color helper functions */
  struct SDKVIS_EXPORT ColorUtils
  {
    /** Maps colors for gain threshold */
    ColorMap gainThresholdColorMap_;

    ///Default constructor for use when accessing non-static members
    ColorUtils(float gainAlpha=1.0);

    /**
    * Convert the 0xRRGGBBAA color value used by SIMDIS to an OSG Vec4f object. With this representation,
    * red is stored in bits (24-31), blue is stored in bits (16-23), green is stored in bits (8-15), and
    * alpha is stored in bits (0-7) of an unsigned integer.
    * @param color Integer value containing a color value encoded with the SIMDIS 0xRRGGBBAA representation
    * @return      osg::Vec4f representation of the specified color value
    */
    static osg::Vec4f RgbaToVec4(unsigned int color);

    /// set the color for the gain algorithmically
    static void GainThresholdColor(int gain, osg::Vec4f &color, float alpha=0.3);
    /// set the color for the gain using a table
    const osg::Vec4f& GainThresholdColor(int gain);
  };


  /** Helper class to unify platform-vector scaling factors */
  class SDKVIS_EXPORT VectorScaling
  {
  public:
    /** Generates scene points between start and end (inclusive) to fill a VertexArray's vertex allocation, setting all vertices to new values */
    static void generatePoints(osg::Vec3Array& vertices, const osg::Vec3& start, const osg::Vec3& end);

    /** Generates scene points between start and end (inclusive) to fill a LineDrawable's vertex allocation, setting all vertices in the LineDrawable to new values */
    static void generatePoints(osgEarth::LineDrawable& line, const osg::Vec3& start, const osg::Vec3& end);

    /** Returns true if one of the prefs has changed that impacts vector scaling (requiring line rebuild) */
    static bool fieldsChanged(const simData::PlatformPrefs& lastPrefs, const simData::PlatformPrefs& newPrefs);
    /** Returns the line length of the platform node's vector, based on axis scale and model size */
    static float lineLength(const simVis::PlatformModelNode* node, float axisScale=1.f);

    /** Returns the size of the bounding box passed in */
    static osg::Vec3f boundingBoxSize(const osg::BoundingBox& bbox);
    /** Returns the maximum dimension of the bounding box */
    static float boundingBoxMaxDimension(const osg::BoundingBox& bbox);
  };


  /// makes a big red "X" square image for the given size in pixels
  SDKVIS_EXPORT osg::Image* makeBrokenImage(int size=32);

  /**
   * Builds a sphere mesh geometry, configured potentially with a two-pass alpha render bin for
   * colors that are transparent.
   * @param r Radius of the sphere, in scene graph units (meters)
   * @param color The geometry is given a BIND_OVERALL mapping to this color.
   * @param maxAngle Degrees, used to divide the number of vertical segments. For example, if the
   *   latitude spans 90 degrees vertically and maxAngle is 10.0, there are 10 vertical strips.
   *   Maximum angle between vertices (controls tessellation).
   * @return Node representing the sphere
   */
  SDKVIS_EXPORT osg::ref_ptr<osg::Node> createSphere(float r, const osg::Vec4& color, float maxAngle = 15.0f);

  /**
   * Builds a hemisphere mesh geometry, configured potentially with a two-pass alpha render bin for
   * colors that are transparent.
   * @param r Radius of the hemisphere, in scene graph units (meters)
   * @param color The geometry is given a BIND_OVERALL mapping to this color.
   * @param maxAngle Degrees, used to divide the number of vertical segments. For example, if the
   *   latitude spans 90 degrees vertically and maxAngle is 10.0, there are 10 vertical strips.
   *   Maximum angle between vertices (controls tessellation).
   * @return Node representing the hemisphere
   */
  SDKVIS_EXPORT osg::ref_ptr<osg::Node> createHemisphere(float r, const osg::Vec4& color, float maxAngle = 15.0f);

  /**
   * Builds an ellipsoidal mesh geometry, configured potentially with a two-pass alpha render bin for
   * colors that are transparent.
   * @param xRadius Radius in X dimension of the ellipsoid, in scene graph units (meters)
   * @param yRadius Radius in Y dimension of the ellipsoid, in scene graph units (meters)
   * @param zRadius Radius in Z dimension of the ellipsoid, in scene graph units (meters)
   * @param color The geometry is given a BIND_OVERALL mapping to this color.
   * @param maxAngle Degrees, used to divide the number of vertical segments. For example, if the
   *   latitude spans 90 degrees vertically and maxAngle is 10.0, there are 10 vertical strips.
   *   Maximum angle between vertices (controls tessellation).
   * @param minLat Used for sectors, this is the minimum vertical angle in degrees.
   * @param maxLat Used for sectors, this is the maximum vertical angle in degrees.
   * @param minLon Used for sectors, this is the minimum horizontal angle in degrees.
   * @param maxLon Used for sectors, this is the maximum horizontal angle in degrees.
   * @return Ellipsoidal mesh adhering to input parameters
   */
  SDKVIS_EXPORT osg::ref_ptr<osg::Node> createEllipsoid(float xRadius, float yRadius, float zRadius,
    const osg::Vec4& color, float maxAngle = 10.0f, float minLat = -90.0, float maxLat = 90.0,
    float minLon = -180.0, float maxLon = 180.0);

  /**
   * Creates an ellipsoidal geometry mesh. This mesh is optionally textured. This code is adapted
   * from the osgEarth AnnotationUtils::createEllipsoidGeometry() call.
   * @param xRadius Radius in X dimension of the ellipsoid, in scene graph units (meters)
   * @param yRadius Radius in Y dimension of the ellipsoid, in scene graph units (meters)
   * @param zRadius Radius in Z dimension of the ellipsoid, in scene graph units (meters)
   * @param color The geometry is given a BIND_OVERALL mapping to this color.
   * @param maxAngle Degrees, used to divide the number of vertical segments. For example, if the
   *   latitude spans 90 degrees vertically and maxAngle is 10.0, there are 10 vertical strips.
   *   Maximum angle between vertices (controls tessellation).
   * @param minLat Used for sectors, this is the minimum vertical angle in degrees.
   * @param maxLat Used for sectors, this is the maximum vertical angle in degrees.
   * @param minLon Used for sectors, this is the minimum horizontal angle in degrees.
   * @param maxLon Used for sectors, this is the maximum horizontal angle in degrees.
   * @param genTexCoords If true, texture coordinates are generated too for each vetex.
   * @return Ellipsoidal mesh adhering to input parameters
   */
  SDKVIS_EXPORT osg::ref_ptr<osg::Geometry> createEllipsoidGeometry(float xRadius, float yRadius, float zRadius,
    const osg::Vec4f& color, float maxAngle = 10.f, float minLat = -90.f, float maxLat = 90.f,
    float minLon = -180.f, float maxLon = 180.f, bool genTexCoords = false);

  /**
   * Computes the world matrix for a node, using its local matrix.
   * @param node Node for which to get the world coordinates
   * @return 4x4 matrix from OSG representing the world coordinates for the node
   */
  SDKVIS_EXPORT osg::Matrix computeLocalToWorld(const osg::Node* node);

  /**
   * Calculates the geodetic position of a node in the scene (radians and meters)
   * @param node Scene graph node for which to calculate the geodetic position
   * @return Geodetic position (latitude, longitude, altitude) in radians and meters
   */
  SDKVIS_EXPORT simCore::Vec3 computeNodeGeodeticPosition(const osg::Node* node);


  /**
   * Node callback that fakes an always-increasing frame stamp.  Intended to be used to update Sequence nodes.
   * Sequence nodes requiring a strictly increasing simulation time stamp.  Time stamps that decrease will
   * break the Sequence update logic (as of 3.4.0 at least).  However, other software like osgEarth's Triton
   * extension require the ability to tie simulation time to the real simulation time, which in SIMDIS is
   * not strictly increasing.
   *
   * This class attaches to the update operation and replaces the time stamp with one that is strictly
   * increasing based on the system time.  This impacts Sequence nodes and all children of Sequence nodes.
   */
  class SDKVIS_EXPORT SequenceTimeUpdater : public osg::NodeCallback
  {
  public:
    /** Initializes the Sequence Time Updater */
    SequenceTimeUpdater(osg::FrameStamp* replacementStamp);

    /** Changes the frame stamp pointer being used for the scene */
    void setFrameStamp(osg::FrameStamp* frameStamp);

    /** Applies a strictly increasing time stamp to the sequence */
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

  protected:
    /** Protected osg::Referenced-derived destructor */
    virtual ~SequenceTimeUpdater();

  private:
    /** Copies the scene stamp data to modified stamp, then modifies the sim time to match ref time */
    void updateModifiedStamp_();

    osg::observer_ptr<osg::FrameStamp> sceneStamp_;
    osg::ref_ptr<osg::FrameStamp> modifiedStamp_;
  };

  /**
   * Helper class that captures timing information for operations that may appear in an
   * osg::Stats implementation.  Generic enough to handle both the case of once-per-frame
   * operations and multiple-per-frame operations.  Timing statistics are recorded into
   * the provided View's osg::Stats.  To visualize, add a custom line to the Stats object.
   *
   * This class is intended to be used either internally by the ScopedStatsTimer for
   * once-per-frame operations, or externally and persistently using a ScopedStatsTimerToken
   * to help with starting and stopping metrics collection.
   */
  class SDKVIS_EXPORT StatsTimer
  {
  public:
    /** Strategy for recording values to osg::Stats */
    enum RecordFrequency
    {
      /**
       * Only record once per frame, and record in the start().  This strategy reduces mutex
       * locks in osg::Stats.  Infrequent operations may never show in performance metrics
       * because the frame change detection logic is in start() and the applicable frame number
       * may be too early.
       */
      RECORD_PER_FRAME_ON_START = 0,

      /**
       * Records on every call to stop().  Frequent calls will result in more mutex locks on
       * osg::Stats, potentially impacting performance, but performance metrics will be
       * most accurate.  If an operation only occurs once per frame, this strategy is best.
       */
      RECORD_PER_STOP,

      /**
       * Similar to RECORD_PER_FRAME_ON_START, but restamps the frame number to force collection.
       * The restamp will update the frame number to the latter of earliest frame number in the
       * Stats container, or actual frame.  This is a good hybrid approach that helps keep the
       * cumulative fairly accurate while keeping mutex locks in osg::Stats to a minimum.
       */
      RECORD_PER_FRAME_RESTAMPED_ON_START
    };

    /** Constructs a new Per-Frame tick */
    StatsTimer(osgViewer::View* mainView, const std::string& key, RecordFrequency recordFrequency = RECORD_PER_FRAME_RESTAMPED_ON_START);
    /** Clears out memory and stops timer if needed */
    virtual ~StatsTimer();

    /** Starts the timer.  Returns non-zero error if timer is already started. */
    int start();
    /** Stops the timer. Returns non-zero error if timer is not started. */
    int stop();

    /** Retrieves the actual string to use when saving begin tick data in osg::Stats for this key. */
    static std::string beginName(const std::string& key);
    /** Retrieves the actual string to use when saving end tick data in osg::Stats for this key. */
    static std::string endName(const std::string& key);
    /** Retrieves the actual string to use when saving elapsed time data in osg::Stats for this key. */
    static std::string timeTakenName(const std::string& key);

    /** For given key, add a line to the StatsHandler with a title and color provided. */
    static void addLine(osgViewer::StatsHandler* stats, const std::string& title, const std::string& key, const osg::Vec4& color);
    /** Removes a line from the stats handler corresponding to the title in addLine(). */
    static void removeLine(osgViewer::StatsHandler* stats, const std::string& title);

  private:
    /** Returns true if start() has been called but not yet stop() */
    bool isStarted_() const;
    /** Saves the values to the Stats on the main view; returns non-zero on error */
    int record_();
    /** Call this once a frame is definitely done and we need to reset for a new frame */
    void reset_();

    /// View associated with the statistic
    osg::observer_ptr<osgViewer::View> mainView_;
    const std::string beginKey_;
    const std::string endKey_;
    const std::string timeTakenKey_;

    /// Strategy for when to record to osg::Stats
    RecordFrequency recordFrequency_;

    /// Overall time taken in current frame
    osg::Timer_t cumulativeMs_;
    /// First tick contributing to cumulativeMs_ (set in start())
    osg::Timer_t firstStartTickMs_;
    /// Last tick contributing to cumulativeMs_ (set in stop())
    osg::Timer_t lastStopTickMs_;

    /// Time of last start() call; if 0, then timer is not started
    osg::Timer_t startTickMs_;

    /// Frame number where start() was called
    unsigned int currentFrameNumber_;
    /// OSG-reported time of the currentFrameNumber_'s frame start time
    osg::Timer_t currentFrameStartTickMs_;
  };

  /**
   * Convenience class to start and stop the timer on an existing StatsTimer.  To
   * use, first allocate a persistent StatsTimer.  Then create a ScopedStatsTimerToken
   * on the stack that represents the area you want to do timing.  This is suitable
   * for operations that occur once per frame or more.  If an operation is guaranteed
   * to occur once per frame or less, consider the simpler ScopedStatsTimer which
   * does not require a persistent StatsTimer.  For example:
   *
   * <code>
   * StatsTimer statsTimer_(mainView, "Repeated Operation"); // persistent object
   * // ...
   * if (...)
   * {
   *   ScopedStatsTimerToken token(statsTimer_);
   *   doOperation();
   * }
   * </code>
   */
  class SDKVIS_EXPORT ScopedStatsTimerToken
  {
  public:
    /** Starts a timer on the given token */
    ScopedStatsTimerToken(StatsTimer& tick);
    /** Stops a timer on the given token */
    virtual ~ScopedStatsTimerToken();

  private:
    StatsTimer& tick_;
  };

  /**
   * Convenience class to record a single per-frame timer metric into the osg::Stats.
   * Implemented using the StatsTimer class.  Use this inside a scope before a (potentially)
   * long operation that occurs no more than once per frame.  If an operation may occur
   * more than once per frame, look at the ScopedStatsTimerToken class.  For example:
   *
   * <code>
   * if (...)
   * {
   *   ScopedStatsTimer tick(mainView, "Long Operation");
   *   doOperation();
   * }
   * </code>
   */
  class SDKVIS_EXPORT ScopedStatsTimer
  {
  public:
    /** Creates a stats timer and starts it.  Will stop on instance destruction. */
    ScopedStatsTimer(osgViewer::View* mainView, const std::string& key);

  private:
    StatsTimer statsTimer_;
  };

  /** Simple visitor that removes the provided Mode from all statesets. */
  class SDKVIS_EXPORT RemoveModeVisitor : public osg::NodeVisitor
  {
  public:
    /** Remove the mode provided from nodes visited */
    explicit RemoveModeVisitor(GLenum mode);

    /** Override apply(osg::Node&) to remove from all statesets */
    virtual void apply(osg::Node& node);

  private:
    GLenum mode_;
  };

  /**
   * In OpenGL 3.2, various geometry draw modes that were previously deprecated were finally
   * removed.  A core profile implementation does not have the capability to render these
   * deprecated draw modes.  This visitor is responsible for detecting any geometry with
   * deprecated draw modes and convert them into triangle strips, which are not deprecated.
   * This is more efficient than using osgUtil::TriStripVisitor directly because it is only
   * executed on geometry that actually contains deprecated modes, rather than all geometry.
   *
   * The deprecated modes that this class handles are GL_POLYGON, GL_QUADS, and GL_QUAD_STRIPS.
   *
   * Usage:
   *   FixDeprecatedDrawModes visitor;
   *   node->accept(visitor);
   */
  class SDKVIS_EXPORT FixDeprecatedDrawModes : public osg::NodeVisitor
  {
  public:
    /** Default constructor */
    FixDeprecatedDrawModes();

    /** Override apply() to detect GL3-incompatible draw modes on primitive sets */
    virtual void apply(osg::Geometry& geom);
  };

  /** Turns a DOF transform's animation on or off. */
  class SDKVIS_EXPORT EnableDOFTransform : public osg::NodeVisitor
  {
  public:
    explicit EnableDOFTransform(bool enabled);
    virtual void apply(osg::Node& node);

  private:
    bool enabled_;
  };

  /**
   * Utility class that is intended to do a transform to screen coordinates, backing out MVPW.
   * This is similar to osg::AutoTransform but does not attempt to maintain an aspect ratio,
   * instead preferring to back out to pixel scale in both the X and the Y axes.
   */
  class SDKVIS_EXPORT PixelScaleHudTransform : public osg::Transform
  {
  public:
    PixelScaleHudTransform();
    PixelScaleHudTransform(const PixelScaleHudTransform& rhs, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
    META_Node(simVis, PixelScaleHudTransform);

    /** Override osg::Transform method. */
    virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const;
    /** Override osg::Transform method. */
    virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const;

  private:
    /** Computes the inverse of the MVPW and saves it */
    osg::Matrixd computeMatrix_(osg::NodeVisitor* nv) const;

  private:
    /** Model-View Projection Window matrix, inverted for performance.  Mutable for caching. */
    mutable osg::Matrixd invertedMvpw_;
  };

  /**
   * Generic GUIEventHandler callback that calls a function (lambda) that you define.  When the screen dimensions
   * change, as detected by the FRAME event on which the callback is attached, your function is called if the
   * dimensions are different from what is currently saved.  This is intended to be used as an easy way to get
   * screen dimensions without having to define multiple helper classes.  To use this, you can write code like:
   *
   * <code>
   * node->addEventCallback(new ViewportSizeCallback([](const osg::Vec2f& dims) {
   *    std::cout << "New dimensions: " << dims.x() << "x" << dims.y() << "\n";
   *   }));
   * </code>
   */
  class SDKVIS_EXPORT ViewportSizeCallback : public osgGA::GUIEventHandler
  {
  public:
    explicit ViewportSizeCallback(std::function<void(const osg::Vec2f&)> func);

    /** Checks for updated viewport size. */
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*) override;

    /** Retrieves the last window size seen */
    osg::Vec2f windowSize() const;

  private:
    osg::Vec2f windowSize_;
    std::function<void(const osg::Vec2f&)> func_;
  };

  /** osg::NotifyHandler instance that removes messages that match filters. */
  class SDKVIS_EXPORT FilteringOsgNotifyDecorator : public osg::NotifyHandler
  {
  public:
    explicit FilteringOsgNotifyDecorator(osg::NotifyHandler* child);

    /** Adds a filter. Messages matching the filter exactly will be removed and not sent to output. */
    void addFilter(const std::string& filter);

    // From NotifyHandler:
    virtual void notify(osg::NotifySeverity severity, const char* message) override;

  protected:
    // Protected, from osg::Referenced
    virtual ~FilteringOsgNotifyDecorator();

  private:
    osg::ref_ptr<osg::NotifyHandler> child_;
    std::vector<std::string> filters_;
  };

  /**
   * Creates a FilteringOsgNotifyDecorator, installs it wrapping the current notify handler, returning
   * itself. This decorator is pre-loaded with filters that may be needed to reduce noise in OSG notify.
   */
  SDKVIS_EXPORT FilteringOsgNotifyDecorator* installFilteringOsgNotifyDecorator();

} // namespace simVis

#endif // SIMVIS_UTILS_H
