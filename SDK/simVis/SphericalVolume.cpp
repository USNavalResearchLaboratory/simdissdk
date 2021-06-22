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
#include "osg/BlendFunc"
#include "osg/CullFace"
#include "osg/Depth"
#include "osg/Geode"
#include "osg/MatrixTransform"
#include "osg/PolygonMode"
#include "osg/ref_ptr"
#include "osg/UserDataContainer"
#include "osgUtil/Simplifier"
#include "osgEarth/GLUtils"
#include "osgEarth/LineDrawable"

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simVis/Constants.h"
#include "simVis/PolygonStipple.h"
#include "simVis/SphericalVolume.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"

//---------------------------------------------------------------------------
// note that these are distinct values, not components of a bitmask.
#define USAGE_CONE     0x00
#define USAGE_CONENEAR 0x01
#define USAGE_CONEFAR  0x02
#define USAGE_NEAR     0x04
#define USAGE_FAR      0x08
#define USAGE_TOP      0x10
#define USAGE_BOTTOM   0x20
#define USAGE_LEFT     0x40
#define USAGE_RIGHT    0x80

#define Q(T) #T

namespace
{
  struct SVMeta
  {
    unsigned char usage_; // where in the sv is this vertex: near face, far face, etc. Note that this is not a bitmask.
    float     anglex_;    // vertex is at this angle in X; different meaning for cone than for pyramid
    float     anglez_;    // vertex is at this angle in Z, different meaning for cone than for pyramid
    float     ratio_;     // ratio of vertex magnitude to beam range/magnitude; 0 at near, 1 at far
    osg::Vec3 unit_;      // vertex unit vector

    SVMeta() : usage_(static_cast<unsigned char>(0)), anglex_(0.0f), anglez_(0.0f), ratio_(0.0f)
    {
    }

    explicit SVMeta(unsigned char usage) : usage_(usage), anglex_(0.0f), anglez_(0.0f), ratio_(0.0f)
    {
    }

    SVMeta(unsigned char usage, float anglex, float anglez, const osg::Vec3& unit, float ratio)
    {
      usage_ = usage;
      set(anglex, anglez, unit, ratio);
    }

    inline void set(unsigned char usage, float anglex, float anglez, const osg::Vec3& unit, float ratio)
    {
      usage_ = usage;
      set(anglex, anglez, unit, ratio);
    }

    inline void set(float anglex, float anglez, const osg::Vec3& unit, float ratio)
    {
      anglex_ = anglex;
      anglez_ = anglez;
      unit_ = unit;
      ratio_ = ratio;
    }
  };

  struct SVMetaContainer : public osg::Referenced
  {
    /// vector of vertex metadata
    std::vector<SVMeta> vertMeta_;
    /// quaternion that will "point" the volume along our direction vector
    osg::Quat           dirQ_;
    /// range of near face of sv
    float               nearRange_;
    /// range of far face of sv
    float               farRange_;
  };

  // class that adds an outline to an svPyramid
  // the x-axis roughly parallels the gate horizontals; if you look from gate origin down the y-axis, x increases from left to right
  // the y-axis connects the gate origin to the gate centroid
  // the z-axis roughly parallels the gate verticals;  if you look from gate origin down the y-axis, z increases from bottom to top
  class svPyramidOutline : public osgEarth::LineGroup
  {
  public:
    svPyramidOutline(osg::MatrixTransform& xform, const osg::Vec3Array* vertexArray, unsigned int numPointsX, unsigned int numPointsZ, unsigned short farFaceOffset, unsigned short nearFaceOffset, bool drawWalls);
    void regenerate();
    void setColor(const osg::Vec4f& color);
  protected:
    /// osg::Referenced-derived
    virtual ~svPyramidOutline();

  private:
    osg::ref_ptr<const osg::Vec3Array> vertexArray_; // the vertex array that contains the vertices that are sifted through to produce the outline
    osg::ref_ptr<osgEarth::LineDrawable> bottomOutline_;
    osg::ref_ptr<osgEarth::LineDrawable> topOutline_;
    osg::ref_ptr<osgEarth::LineDrawable> farLeftOutline_;
    osg::ref_ptr<osgEarth::LineDrawable> farRightOutline_;
    osg::ref_ptr<osgEarth::LineDrawable> nearLeftOutline_;
    osg::ref_ptr<osgEarth::LineDrawable> nearRightOutline_;
    osg::Vec4f outlineColor_;
    unsigned int numPointsX_;
    unsigned int numPointsZ_;
    unsigned short farFaceOffset_;
    unsigned short nearFaceOffset_;
    bool drawWalls_;
  };

  svPyramidOutline::svPyramidOutline(osg::MatrixTransform& xform, const osg::Vec3Array* vertexArray, unsigned int numPointsX, unsigned int numPointsZ, unsigned short farFaceOffset, unsigned short nearFaceOffset, bool drawWalls)
    : vertexArray_(vertexArray),
    numPointsX_(numPointsX),
    numPointsZ_(numPointsZ),
    farFaceOffset_(farFaceOffset),
    nearFaceOffset_(nearFaceOffset),
    drawWalls_(drawWalls)
  {
    // svPyramid must provide a non-nullptr vertex array
    assert(vertexArray);
    setName("simVis::SphericalVolume::svPyramidOutline");
    xform.addChild(this);
    // Line smoothing is not enabled by default, as it might cause problems with multisampling, or maybe with draw order.

    const bool hasNearFace = (nearFaceOffset_ > 0);
    // if we are drawing near and far faces, bottom and top outlines are each line loops, if not, (far face) outlines are each simple line strips
    if (hasNearFace)
    {
      // bottom outline is a quadrilateral loop connecting near face bottom horizontal (numPointsX_) and far face bottom horizontal (numPointsX_)
      bottomOutline_ = new osgEarth::LineDrawable(GL_LINE_LOOP);
      bottomOutline_->allocate(2 * numPointsX_);
      // top outline is a quadrilateral loop connecting near face top horizontal (numPointsX_) and far face top horizontal (numPointsX_)
      topOutline_ = new osgEarth::LineDrawable(GL_LINE_LOOP);
      topOutline_->allocate(2 * numPointsX_);
    }
    else if (drawWalls_)
    {
      // bottom outline is a triangular loop connecting gate origin (1) and far face bottom horizontal (numPointsX_)
      bottomOutline_ = new osgEarth::LineDrawable(GL_LINE_LOOP);
      bottomOutline_->allocate(1 + numPointsX_);
      // top outline is a triangular loop connecting gate origin (1) and far face top horizontal (numPointsX_)
      topOutline_ = new osgEarth::LineDrawable(GL_LINE_LOOP);
      topOutline_->allocate(1 + numPointsX_);
    }
    else
    {
      // bottom outline is the line outline of the far face bottom horizontal (numPointsX_)
      bottomOutline_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
      bottomOutline_->allocate(numPointsX_);
      // top outline is the line outline of the far face top horizontal (numPointsX_)
      topOutline_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
      topOutline_->allocate(numPointsX_);
    }
    bottomOutline_->setName("simVis::SphericalVolumeBottomOutline");
    bottomOutline_->setColor(outlineColor_);
    bottomOutline_->setDataVariance(osg::Object::DYNAMIC);
    addChild(bottomOutline_.get());
    topOutline_->setName("simVis::SphericalVolumeTopOutline");
    topOutline_->setColor(outlineColor_);
    topOutline_->setDataVariance(osg::Object::DYNAMIC);
    addChild(topOutline_.get());

    // the gate's far face left side vertical (numPointsZ_)
    farLeftOutline_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
    farLeftOutline_->allocate(numPointsZ_);
    farLeftOutline_->setName("simVis::SphericalVolume-FarOutline");
    farLeftOutline_->setColor(outlineColor_);
    farLeftOutline_->setDataVariance(osg::Object::DYNAMIC);
    addChild(farLeftOutline_.get());
    // the gate's far face right side vertical (numPointsZ_)
    farRightOutline_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
    farRightOutline_->allocate(numPointsZ_);
    farRightOutline_->setName("simVis::SphericalVolume-FarOutline");
    farRightOutline_->setColor(outlineColor_);
    farRightOutline_->setDataVariance(osg::Object::DYNAMIC);
    addChild(farRightOutline_.get());

    if (hasNearFace)
    {
      // the gate's near face left side vertical (numPointsZ_)
      nearLeftOutline_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
      nearLeftOutline_->allocate(numPointsZ_);
      nearLeftOutline_->setName("simVis::SphericalVolume-NearOutline");
      nearLeftOutline_->setColor(outlineColor_);
      nearLeftOutline_->setDataVariance(osg::Object::DYNAMIC);
      addChild(nearLeftOutline_.get());
      // the gate's near face right side vertical (numPointsZ_)
      nearRightOutline_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
      nearRightOutline_->allocate(numPointsZ_);
      nearRightOutline_->setName("simVis::SphericalVolume-NearOutline");
      nearRightOutline_->setColor(outlineColor_);
      nearRightOutline_->setDataVariance(osg::Object::DYNAMIC);
      addChild(nearRightOutline_.get());
    }
  }

  svPyramidOutline::~svPyramidOutline() {}

  void svPyramidOutline::setColor(const osg::Vec4f& color)
  {
    outlineColor_ = color;
    // no alpha in the outline
    outlineColor_[3] = 1.0f;
    for (unsigned int i = 0; i < getNumChildren(); ++i)
    {
      osgEarth::LineDrawable* line = getLineDrawable(i);
      // line drawable can set to same-as-current color w/o penalty
      if (line)
        line->setColor(outlineColor_);
    }
  }

  void svPyramidOutline::regenerate()
  {
    const bool hasNearFace = (nearFaceOffset_ > 0);
#ifndef NDEBUG
    // only needed to support assertions
    const size_t vertexArraySize = vertexArray_->size();
    const size_t bottomOutlineSize = bottomOutline_->size();
    const size_t topOutlineSize = topOutline_->size();
    const size_t farLeftOutlineSize = farLeftOutline_->size();
    const size_t nearLeftOutlineSize = hasNearFace ? nearLeftOutline_->size() : 0;
    const size_t farRightOutlineSize = farRightOutline_->size();
    const size_t nearRightOutlineSize = hasNearFace ? nearRightOutline_->size() : 0;
#endif
    // bottom outline
    {
      // gate's bottom edge is z = 0, for near or far face
      const unsigned int z = 0;
      // iterate across the gate horizontals (x) from left to right (if you look from gate origin)
      for (unsigned int x = 0; x < numPointsX_; ++x)
      {
        assert(bottomOutlineSize > x);
        assert(vertexArraySize > (farFaceOffset_ + x * numPointsZ_ + z));
        bottomOutline_->setVertex(x, (*vertexArray_)[farFaceOffset_ + x * numPointsZ_ + z]);
        if (hasNearFace)
        {
          assert(bottomOutlineSize > ((2 * numPointsX_) - x - 1));
          assert(vertexArraySize > (nearFaceOffset_ + x * numPointsZ_ + z));
          bottomOutline_->setVertex((2 * numPointsX_) - x - 1, (*vertexArray_)[nearFaceOffset_ + x * numPointsZ_ + z]);
        }
      }
      if (drawWalls_ && !hasNearFace)
      {
        // there is no near face, add index to origin/zero point
        assert(bottomOutlineSize > numPointsX_);
        assert(vertexArraySize > 0);
        bottomOutline_->setVertex(numPointsX_, (*vertexArray_)[0]);
      }
    }
    // top outline
    {
      // gate's top edge is z = numPointsZ_ - 1, for near or far face
      const unsigned int z = (numPointsZ_ - 1);
      // iterate across the gate horizontals (x) from left to right (if you look from gate origin)
      for (unsigned int x = 0; x < numPointsX_; ++x)
      {
        assert(topOutlineSize > x);
        assert(vertexArraySize > (farFaceOffset_ + x * numPointsZ_ + z));
        topOutline_->setVertex(x, (*vertexArray_)[farFaceOffset_ + x * numPointsZ_ + z]);
        if (hasNearFace)
        {
          assert(topOutlineSize > ((2 * numPointsX_) - x - 1));
          assert(vertexArraySize > (nearFaceOffset_ + x * numPointsZ_ + z));
          topOutline_->setVertex((2 * numPointsX_) - x - 1, (*vertexArray_)[nearFaceOffset_ + x * numPointsZ_ + z]);
        }
      }
      if (drawWalls_ && !hasNearFace)
      {
        // there is no near face, add index to origin/zero point
        assert(topOutlineSize > numPointsX_);
        assert(vertexArraySize > 0);
        topOutline_->setVertex(numPointsX_, (*vertexArray_)[0]);
      }
    }

    // left outlines
    {
      // gate's left edge is x = 0, for near or far face
      const unsigned int x = 0;
      // this is the index offset for the bottom of either face at the current x
      const unsigned int xOffset = x * numPointsZ_;
      for (unsigned int z = 0; z < numPointsZ_; ++z)
      {
        assert(farLeftOutlineSize > z);
        assert(vertexArraySize > (farFaceOffset_ + xOffset + z));
        farLeftOutline_->setVertex(z, (*vertexArray_)[farFaceOffset_ + xOffset + z]);
        if (hasNearFace)
        {
          assert(nearLeftOutlineSize > z);
          assert(vertexArraySize > (nearFaceOffset_ + xOffset + z));
          nearLeftOutline_->setVertex(z, (*vertexArray_)[nearFaceOffset_ + xOffset + z]);
        }
      }
    }
    // right outlines
    {
      // gate's right edge is x = numPointsX_ - 1, for near or far face
      const unsigned int x = (numPointsX_ - 1);
      // this is the index offset for the bottom of either face at the current x
      const unsigned int xOffset = x * numPointsZ_;
      for (unsigned int z = 0; z < numPointsZ_; ++z)
      {
        assert(farRightOutlineSize > z);
        assert(vertexArraySize >(farFaceOffset_ + xOffset + z));
        farRightOutline_->setVertex(z, (*vertexArray_)[farFaceOffset_ + xOffset + z]);
        if (hasNearFace)
        {
          assert(nearRightOutlineSize > z);
          assert(vertexArraySize > (nearFaceOffset_ + xOffset + z));
          nearRightOutline_->setVertex(z, (*vertexArray_)[nearFaceOffset_ + xOffset + z]);
        }
      }
    }
  }

  class svPyramidFactory
  {
  private:
    enum Face
    {
      FARFACE,
      NEARFACE
    };

  public:
    svPyramidFactory(osg::MatrixTransform& xform, const simVis::SVData& data, const osg::Vec3& direction);

  private:  // helper methods
    void initializeData_(const simVis::SVData& d, const osg::Vec3& direction);
    void initializePyramid(osg::MatrixTransform& xform);
    void populateFaceVertices_(Face face);
    void generateFaces_(osg::Geometry* geometry);
    void generateWalls_(osg::Geometry* volumeGeometry);

  private:  // data
    osg::Vec4f color_;
    unsigned int wallRes_;
    osg::ref_ptr<osg::Geometry> solidGeometry_;
    osg::ref_ptr<osg::Vec3Array> vertexArray_;
    osg::ref_ptr<osg::Vec3Array> normalArray_;
    SVMetaContainer* metaContainer_;
    float hfov_deg_;
    float vfov_deg_;
    unsigned int numPointsX_;
    float x_start_;
    float spacingX_;
    unsigned int numPointsZ_;
    float z_start_;
    float spacingZ_;
    unsigned int reserveSizeFace_;
    unsigned int reserveSizeCone_;
    unsigned short farFaceOffset_;
    unsigned short nearFaceOffset_;
    bool drawWalls_;
    bool drawFaces_;
    bool hasNear_;
  };

  svPyramidFactory::svPyramidFactory(osg::MatrixTransform& xform, const simVis::SVData& data, const osg::Vec3& direction)
  {
    if (data.drawMode_ == simVis::SVData::DRAW_MODE_NONE || data.capRes_ == 0)
      return;

    initializeData_(data, direction);

    initializePyramid(xform);

    populateFaceVertices_(FARFACE);
    if (hasNear_)
      populateFaceVertices_(NEARFACE);

    if (drawFaces_) // drawing more than outline (far face, possibly walls, possibly near face)
    {
      generateFaces_(solidGeometry_.get());
      if (drawWalls_)
        generateWalls_(solidGeometry_.get());

      // release our ref_ptr, we don't need it anymore
      solidGeometry_ = nullptr;
    }

    const bool drawOutlines = (simVis::SVData::DRAW_MODE_OUTLINE & data.drawMode_) == simVis::SVData::DRAW_MODE_OUTLINE;
    if (drawOutlines)
    {
      // must provide a non-nullptr vertexArray to svPyramidOutline
      assert(vertexArray_);
      svPyramidOutline* outline = new svPyramidOutline(xform, vertexArray_.get(), numPointsX_, numPointsZ_, farFaceOffset_, nearFaceOffset_, drawWalls_);
      outline->setColor(color_);
      outline->regenerate();
    }
  }

  void svPyramidFactory::initializeData_(const simVis::SVData& data, const osg::Vec3& direction)
  {
    color_ = data.color_;
    wallRes_ = data.wallRes_;

    hfov_deg_ = osg::clampBetween(data.hfov_deg_, 0.01f, 360.0f);
    numPointsX_ = data.capRes_ + 1;
    x_start_ = -0.5f * hfov_deg_;
    spacingX_ = hfov_deg_ / (numPointsX_ - 1);
    // in sphere-seg mode, bake the azim offsets into the model
    if (data.drawAsSphereSegment_)
    {
      x_start_ += data.azimOffset_deg_;
    }

    vfov_deg_ = osg::clampBetween(data.vfov_deg_, 0.01f, 180.0f);
    z_start_ = -0.5f * vfov_deg_;
    float z_end = 0.5f * vfov_deg_;
    // in sphere-seg mode, bake the elev offsets into the model, and clamp to [-90,90]
    if (data.drawAsSphereSegment_)
    {
      z_start_ = simCore::angFix90(z_start_ + data.elevOffset_deg_);
      z_end = simCore::angFix90(z_end + data.elevOffset_deg_);
      vfov_deg_ = z_end - z_start_;
    }
    numPointsZ_ = data.capRes_ + 1;
    spacingZ_ = vfov_deg_ / (numPointsZ_ - 1);

    // only draw the near face if:
    drawWalls_ = (data.drawCone_ && data.wallRes_ != 0);
    hasNear_ = data.nearRange_ > 0.0f && data.drawCone_;

    // Calculate the number of vertices for performance hotspot fix in push_back()
    // vertices will be added in this order: gate origin, far face, near face (if drawn), cone bottom, then cone right (if drawn), cone top (if drawn), cone left (if drawn)
    const unsigned int loop = hasNear_ ? 2u : 1u;
    reserveSizeFace_ = 1 + (loop * numPointsX_ * numPointsZ_);
    reserveSizeCone_ = 0;
    drawFaces_ = (data.drawMode_ != simVis::SVData::DRAW_MODE_OUTLINE);
    if (drawFaces_ && drawWalls_)
    {
      // bottom & top faces are only drawn if vfov_deg < 180
      if (vfov_deg_ < 180.0f) // 2 faces * (2 * (numPointsX - 1) * (1 + d.wallRes_)) vertices/face
        reserveSizeCone_ += (numPointsX_ - 1) * (1 + data.wallRes_) * 2 * 2;
      // right & left faces only drawn if hfov_deg < 360
      if (hfov_deg_ < 360.0f) // 2 faces * (2 * (numPointsZ - 1) * (1 + d.wallRes_)) vertices/face
        reserveSizeCone_ += (numPointsZ_ - 1) * (1 + data.wallRes_) * 2 * 2;
    }

    farFaceOffset_ = 1;
    nearFaceOffset_ = hasNear_ ? farFaceOffset_ + (numPointsX_ * numPointsZ_) : 0;

    vertexArray_ = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    vertexArray_->reserve(reserveSizeFace_ + reserveSizeCone_);

    normalArray_ = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    normalArray_->reserve(reserveSizeFace_ + reserveSizeCone_);

    metaContainer_ = new SVMetaContainer();
    // quaternion that "points" the volume along our direction vector
    metaContainer_->dirQ_.makeRotate(osg::Y_AXIS, direction);
    metaContainer_->nearRange_ = data.nearRange_;
    metaContainer_->farRange_ = data.farRange_;
    std::vector<SVMeta>& vertexMetaData = metaContainer_->vertMeta_;
    vertexMetaData.reserve(reserveSizeFace_ + reserveSizeCone_);

    // add a vertex at gate origin, to support outline drawing to origin when minrange is 0
    // only need this point if drawing outline (with or without fillpattern) and there is no near face b/c minrange is zero.
    // but adding it in every case to make code simpler
    {
      vertexArray_->push_back(osg::Vec3());
      normalArray_->push_back(osg::Vec3());
      vertexMetaData.push_back(SVMeta(USAGE_NEAR, 0.f, 0.f, osg::Vec3(), 0.0f));
    }
  }

  void svPyramidFactory::initializePyramid(osg::MatrixTransform& xform)
  {
    // by convention, the sv xform always contains a primary geode for the volume
    osg::ref_ptr<osg::Geode> geodeSolid = new osg::Geode();
    geodeSolid->setName("simVis::SphericalVolume::PrimaryGeode");
    xform.addChild(geodeSolid.get());

    // if we are drawing outline only, we still need a solid geometry (with no primitives) to hold the metadata that support in-place-update of the vertices that lineDrawable uses
    solidGeometry_ = new osg::Geometry();
    // set up the face geometry
    solidGeometry_->setName("simVis::SphericalVolume::PyramidFaceGeometry");
    solidGeometry_->setUseVertexBufferObjects(true);
    solidGeometry_->setUseDisplayList(false);
    solidGeometry_->setDataVariance(osg::Object::DYNAMIC); // prevent draw/update overlap

    osg::Vec4Array* colorArray = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    (*colorArray)[0] = color_;
    solidGeometry_->setColorArray(colorArray);
    solidGeometry_->setVertexArray(vertexArray_.get());
    solidGeometry_->setUserData(metaContainer_);
    solidGeometry_->setNormalArray(normalArray_.get());
    geodeSolid->addDrawable(solidGeometry_.get());
  }

  void svPyramidFactory::populateFaceVertices_(Face face)
  {
    const float r = (face == FARFACE) ? metaContainer_->farRange_ : metaContainer_->nearRange_;
    const float normalDir = (face == FARFACE) ? 1.0f : -1.0f;
    const char usage = (face == FARFACE ? USAGE_FAR : USAGE_NEAR);
    const float ratio = (face == FARFACE ? 1.0f : 0.0f);
    std::vector<SVMeta>& vertexMetaData = metaContainer_->vertMeta_;
    const osg::Quat& dirQ = metaContainer_->dirQ_;

    // populate vertex array and other arrays for face geometry
    // if you are looking from the gate origin, 1st gate vertex is at bottom left corner, then vertices go up to top left corner
    // then, starting at bottom again for next x, and going up to top.
    // iterate from x min (left) to xmax (right)
    for (unsigned int x = 0; x < numPointsX_; ++x)
    {
      const float angleX_rad = osg::DegreesToRadians(x_start_ + spacingX_ * x);
      const float sinAngleX = sin(angleX_rad);
      const float cosAngleX = cos(angleX_rad);

      for (unsigned int z = 0; z < numPointsZ_; ++z)
      {
        const float angleZ_rad = osg::DegreesToRadians(z_start_ + spacingZ_ * z);
        const float sinAngleZ = sin(angleZ_rad);
        const float cosAngleZ = cos(angleZ_rad);

        const osg::Vec3 unitUnrot(sinAngleX*cosAngleZ, cosAngleX*cosAngleZ, sinAngleZ);
        const osg::Vec3 unit = dirQ * unitUnrot;
        const osg::Vec3 p = unit * r;
        vertexArray_->push_back(p);
        normalArray_->push_back(unit * normalDir);
        vertexMetaData.push_back(SVMeta(usage, angleX_rad, angleZ_rad, unitUnrot, ratio));
      }
    }

    // if either assert fails, vertex counts in face no longer match expected/reserved count; vector reserve calls must be updated to match changes to face vertex generation
    const size_t arraySize = (face == FARFACE) ? 1 + (numPointsX_ * numPointsZ_) : 1 + (2 * numPointsX_ * numPointsZ_);
    assert(vertexArray_->size() == arraySize);
    assert(metaContainer_->vertMeta_.size() == arraySize);
  }

  void svPyramidFactory::generateFaces_(osg::Geometry* faceGeom)
  {
    // if we are drawing the face (not just the outline) add primitives that index into the vertex array
    const unsigned int numFaceElements = 2 * numPointsZ_;

    // draw far face with vertical triangle strip(s) for each (x, x+1) pair
    for (unsigned int x = 0; x < numPointsX_ - 1; ++x)
    {
      osg::ref_ptr<osg::DrawElementsUShort> farFaceStrip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numFaceElements);

      // these are index offsets for the bottom of the face at the current x
      const unsigned int leftX = x * numPointsZ_;
      const unsigned int rightX = (x + 1) * numPointsZ_;
      for (unsigned int z = 0; z < numPointsZ_; ++z)
      {
        const unsigned int elementIndex = 2 * z;
        farFaceStrip->setElement(elementIndex, farFaceOffset_ + rightX + z);
        farFaceStrip->setElement(elementIndex + 1, farFaceOffset_ + leftX + z);
      }
      faceGeom->addPrimitiveSet(farFaceStrip.get());
    }

    // the near face is drawn separately to mitigate near/far face artifacts
    if (hasNear_)
    {
      // draw vertical triangle strip(s) for each (x, x+1) pair
      for (unsigned int x = 0; x < numPointsX_ - 1; ++x)
      {
        osg::ref_ptr<osg::DrawElementsUShort> nearFaceStrip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numFaceElements);

        // these are index offsets for the bottom of the face at the current x
        const unsigned int leftX = x * numPointsZ_;
        const unsigned int rightX = (x + 1) * numPointsZ_;

        for (unsigned int z = 0; z < numPointsZ_; ++z)
        {
          const unsigned int elementIndex = 2 * z;
          nearFaceStrip->setElement(elementIndex, nearFaceOffset_ + leftX + z);
          nearFaceStrip->setElement(elementIndex + 1, nearFaceOffset_ + rightX + z);
        }
        faceGeom->addPrimitiveSet(nearFaceStrip.get());
      }
    }
  }

  void svPyramidFactory::generateWalls_(osg::Geometry* faceGeom)
  {
    std::vector<SVMeta>& vertexMetaData = metaContainer_->vertMeta_;

    // if the near face range is <= 0 (hasNear = false), then there is no near face, walls go to gate origin
    // build vertex sets for the walls. we have to duplicate verts in order to get unique normals, unfortunately.

    const float tessStep = 1.0f / wallRes_;
    const float coneLen = metaContainer_->farRange_ - metaContainer_->nearRange_;
    const unsigned int numWallElements = (1 + wallRes_) * 2;

    // bottom:
    if (vfov_deg_ < 180.0f)
    {
      // draw the bottom wall outline and face, drawn as triangle strips from the near face to the far face;
      // iterate x across the face from right to left, (looking from near face to far face)
      for (unsigned int x = numPointsX_ - 1; x > 0; --x)
      {
        // starting index for near and far face vertices for right edge of strip starting at x
        const unsigned int offsetStart = x * numPointsZ_;

        osg::ref_ptr<osg::DrawElementsUShort> strip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numWallElements);
        // iterate out from the near face to the far face, in tesselated steps
        for (unsigned int q = 0; q < wallRes_ + 1; ++q)
        {
          const float w = tessStep * q;
          for (unsigned int i = 0; i < 2; ++i)
          {
            // i=0 is right edge of strip, i=1 is left edge of strip
            const unsigned int off = offsetStart - (i * numPointsZ_);
            const unsigned int foff = farFaceOffset_ + off;
            const osg::Vec3 nf = hasNear_ ? (*vertexArray_)[nearFaceOffset_ + off] : osg::Vec3();
            const SVMeta& metafoff = (vertexMetaData)[foff];
            const osg::Vec3& unit = metafoff.unit_;
            const osg::Vec3 vert = nf + unit * coneLen * w;
            vertexArray_->push_back(vert);
            // normal should be the unit vector rotated 90deg around x axis
            normalArray_->push_back(osg::Vec3(unit.x(), unit.z(), -unit.y()));
            vertexMetaData.push_back(SVMeta(USAGE_BOTTOM, metafoff.anglex_, metafoff.anglez_, unit, w));

            strip->setElement(2 * q + i, vertexArray_->size() - 1);
          }
        }
        faceGeom->addPrimitiveSet(strip.get());
      }
    }

    // right:
    if (hfov_deg_ < 360.0f)
    {
      // draw the right wall outline and face, drawn as triangle strips from the near face to the far face;
      // iterate z across the face from top to bottom, (looking from near face to far face)
      for (unsigned int z = numPointsZ_ - 1; z > 0; --z)
      {
        // starting index for near and far face vertices for the top edge of the strip starting at z
        const unsigned int offsetStart = numPointsZ_ * (numPointsX_ - 1) + z;

        osg::ref_ptr<osg::DrawElementsUShort> strip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numWallElements);

        // iterate out from the near face to the far face, in tesselated steps
        for (unsigned int q = 0; q < wallRes_ + 1; ++q)
        {
          const float w = tessStep * q;
          for (unsigned int i = 0; i < 2; ++i)
          {
            // i=0 is top edge of strip, i=1 is bottom edge of strip
            const unsigned int off = offsetStart - i;
            const unsigned int foff = farFaceOffset_ + off;
            const osg::Vec3 nf = hasNear_ ? (*vertexArray_)[nearFaceOffset_ + off] : osg::Vec3();
            const SVMeta& metafoff = (vertexMetaData)[foff];
            const osg::Vec3& unit = metafoff.unit_;
            const osg::Vec3 vert = nf + unit * coneLen * w;
            vertexArray_->push_back(vert);
            // normal should be the unit vector rotated 90deg around z axis
            normalArray_->push_back(osg::Vec3(unit.y(), -unit.x(), unit.z()));
            vertexMetaData.push_back(SVMeta(USAGE_RIGHT, metafoff.anglex_, metafoff.anglez_, unit, w));

            strip->setElement(2 * q + i, vertexArray_->size() - 1);
          }
        }
        faceGeom->addPrimitiveSet(strip.get());
      }
    }

    // top:
    if (vfov_deg_ < 180.0f)
    {
      // draw the top wall outline and face, drawn as triangle strips from the near face to the far face;
      // iterate x across the face from left to right, (looking from near face to far face)
      for (unsigned int x = 0; x < numPointsX_ - 1; ++x)
      {
        // starting index for near and far face vertices for left edge of the strip starting at x
        const unsigned int offsetStart = (x * numPointsZ_) + (numPointsZ_ - 1);

        osg::ref_ptr<osg::DrawElementsUShort> strip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numWallElements);

        // iterate out from the near face to the far face, in tesselated steps
        for (unsigned int q = 0; q < wallRes_ + 1; ++q)
        {
          const float w = tessStep * q;
          for (unsigned int i = 0; i < 2; ++i)
          {
            // i=0 is left edge of strip, i=1 is right edge of strip
            const unsigned int off = offsetStart + (i * numPointsZ_);

            const unsigned int foff = farFaceOffset_ + off;
            const osg::Vec3 nf = hasNear_ ? (*vertexArray_)[nearFaceOffset_ + off] : osg::Vec3();
            const SVMeta& metafoff = (vertexMetaData)[foff];
            const osg::Vec3& unit = metafoff.unit_;
            const osg::Vec3 vert = nf + unit * coneLen * w;
            vertexArray_->push_back(vert);
            // normal should be the unit vector rotated -90deg around x axis
            normalArray_->push_back(osg::Vec3(unit.x(), -unit.z(), unit.y()));
            vertexMetaData.push_back(SVMeta(USAGE_TOP, metafoff.anglex_, metafoff.anglez_, unit, w));

            strip->setElement(2 * q + i, vertexArray_->size() - 1);
          }
        }
        faceGeom->addPrimitiveSet(strip.get());
      }
    }

    // left:
    if (hfov_deg_ < 360.0f)
    {
      // draw the left wall outline and face, drawn as triangle strips from the near face to the far face;
      // iterate z across the face from bottom to top, (looking from near face to far face)
      for (unsigned int z = 0; z < numPointsZ_ - 1; ++z)
      {
        osg::ref_ptr<osg::DrawElementsUShort> strip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numWallElements);

        // iterate out from the near face to the far face, in tesselated steps
        for (unsigned int q = 0; q < wallRes_ + 1; ++q)
        {
          const float w = tessStep * q;
          for (unsigned int i = 0; i < 2; ++i)
          {
            // i=0 is bottom edge of strip, i=1 is top edge of strip
            const unsigned int off = z + i;
            const unsigned int foff = farFaceOffset_ + off;
            const osg::Vec3 nf = hasNear_ ? (*vertexArray_)[nearFaceOffset_ + off] : osg::Vec3();
            const SVMeta& metafoff = (vertexMetaData)[foff];
            const osg::Vec3& unit = metafoff.unit_;
            const osg::Vec3 vert = nf + unit * coneLen * w;
            vertexArray_->push_back(vert);
            // normal should be the unit vector rotated -90deg around z axis
            normalArray_->push_back(osg::Vec3(-unit.y(), unit.x(), unit.z()));
            vertexMetaData.push_back(SVMeta(USAGE_LEFT, metafoff.anglex_, metafoff.anglez_, unit, w));

            strip->setElement(2 * q + i, vertexArray_->size() - 1);
          }
        }
        faceGeom->addPrimitiveSet(strip.get());
      }

      // if either assert fails, vertex counts in cone no longer match expected/reserved count; vector reserve calls must be updated to match changes to cone vertex generation
      assert(vertexArray_->size() == reserveSizeFace_ + reserveSizeCone_);
      assert(vertexMetaData.size() == reserveSizeFace_ + reserveSizeCone_);
    }
    return;
  }
}


namespace simVis
{
float SVFactory::calcYValue_(double x, double z)
{
  // calc what y needs to be to have a valid unit vector using x and z
  const double xzsumsqr = (x * x) + (z * z);
  // avoid sqrt(negative number)
  return (xzsumsqr >= 1.0 ? 0.f : static_cast<float>(sqrt(1. - xzsumsqr)));
}

/// Class that creates a cone geometry as a child of the geode arg.
/// The x-axis roughly parallels the cone horizontals; if you look from cone origin down the y-axis, x increases from left to right.
/// The y-axis connects the cone origin to the cone center at the range extent.
/// The z-axis roughly parallels the cone verticals;  if you look from cone origin down the y-axis, z increases from bottom to top.
void SVFactory::createCone_(osg::Geode* geode, const SVData& d, const osg::Vec3& direction)
{
  osg::Geometry* geom = new osg::Geometry();
  geode->addDrawable(geom);
  geom->setName("simVis::SphericalVolume::cone");
  geom->setUseVertexBufferObjects(true);
  geom->setUseDisplayList(false);
  geom->setDataVariance(osg::Object::DYNAMIC); // prevent draw/update overlap

  // the number of angular slices into which to tessellate the cone and its face(s). works best when wallres is a multiple of 4.
  const unsigned int numSlices = osg::clampBetween(d.coneRes_, 4u, 40u);
  const double sliceAngle_rad = M_TWOPI / numSlices;

  // the number of concentric rings forming a face
  const unsigned int numRings = osg::clampBetween(d.capRes_, 1u, 10u);
  // cone cannot support anything > 180
  const double hfov_deg = osg::clampBetween(static_cast<double>(d.hfov_deg_), 0.01, 180.0);
  const double vfov_deg = osg::clampBetween(static_cast<double>(d.vfov_deg_), 0.01, 180.0);
  // each ring has this angular span
  const double ringSpanX = 0.5 * osg::DegreesToRadians(hfov_deg) / numRings;
  const double ringSpanZ = 0.5 * osg::DegreesToRadians(vfov_deg) / numRings;

  // determine if the near face will be drawn
  const bool hasNear = d.nearRange_ > 0.0 && d.drawCone_;

  // determine number of vertices on a cone face
  const unsigned int vertsPerSlice = numRings; // not including the center point
  const unsigned int vertsPerFace = (vertsPerSlice * numSlices) + 1; // +1 for the center point

  // determine number of vertices on the cone wall
  const unsigned int vertsOnWall = numSlices * (d.wallRes_ + 1) * 2;
  // total number of verts is face(s) + wall
  const unsigned int numVerts = hasNear ? (2 * vertsPerFace) + vertsOnWall : vertsPerFace + vertsOnWall;

  // create the vertices
  osg::Vec3Array* v = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX, numVerts);
  geom->setVertexArray(v);

  // and the color array
  osg::Vec4Array* c = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
  geom->setColorArray(c);
  (*c)[0] = d.color_;

  // and the normals
  osg::Vec3Array* n = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX, numVerts);
  geom->setNormalArray(n);

  // metadata (for fast in-place updates)
  SVMetaContainer* metaContainer = new SVMetaContainer();
  geom->setUserData(metaContainer);
  std::vector<SVMeta>* m = &metaContainer->vertMeta_;
  m->resize(numVerts);
  metaContainer->nearRange_ = d.nearRange_;
  metaContainer->farRange_  = d.farRange_;

  // quaternion that will "point" the volume along our direction vector
  metaContainer->dirQ_.makeRotate(osg::Y_AXIS, direction);
  const osg::Quat& dirQ = metaContainer->dirQ_;

  // vertices for far face start at beginning of vertex array
  const unsigned short farOffset = 0;
  // vertices for near face start immediately after the far face vertices
  const unsigned short nearOffset = farOffset + vertsPerFace;

  // near and far faces are built with triangle strip radial slices using two vertices on each concentric ring
  unsigned int vptr = 0;
  // first point in each strip  is the center point.
  (*v)[vptr] = dirQ * osg::Vec3(0.0f, d.farRange_, 0.0f);
  (*n)[vptr] = dirQ * osg::Y_AXIS;
  (*m)[vptr] = SVMeta(USAGE_CONEFAR, 0.0f, 0.0f, osg::Y_AXIS, 1.0f);
  if (hasNear)
  {
    // first point in strip is the center point.
    (*v)[vptr + vertsPerFace] = dirQ * osg::Vec3(0.0f, d.nearRange_, 0.0f);
    (*n)[vptr + vertsPerFace] = -dirQ * osg::Y_AXIS;
    (*m)[vptr + vertsPerFace] = SVMeta(USAGE_CONENEAR, 0.0f, 0.0f, osg::Y_AXIS, 0.0f);
  }
  vptr++;

  const unsigned int elsPerSlice = 1 + (2 * numRings);
  for (unsigned int slice = 0; slice < numSlices; ++slice)
  {
    // iterate around the 360 degrees of the cone, slice by slice; phi is the angle
    // the PI_2 offset ensures a vertex at the top, to ensure that slices in face matches slices in walls
    const double phi = simCore::angFixPI(M_PI_2 + sliceAngle_rad * slice);
    const double cosphi = cos(phi);
    const double sinphi = sin(phi);

    // a triangle strip for this pie slice of the face. each always starts at the center point, extends radially to the outer edge of the cone face.
    osg::ref_ptr<osg::DrawElementsUShort> farWedge = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
    farWedge->reserveElements(elsPerSlice);
    farWedge->push_back(farOffset); // start with the center point

    for (unsigned int ring = 1; ring <= numRings; ++ring)
    {
      // x-axis angular extent/radius in radians for this ring
      const double angleX = ring * ringSpanX;
      // z-axis angular extent/radius in radians for this ring
      const double angleZ = ring * ringSpanZ;
      // clamping of numRings, hfov_deg, and vfov_deg guarantee these asserts
      assert(angleX > 0. && angleX <= M_PI_2);
      assert(angleZ > 0. && angleZ <= M_PI_2);
      const double ringRadiusX = sin(angleX);
      const double ringRadiusZ = sin(angleZ);
      // previous asserts guarantee this assert
      assert(ringRadiusX > 0. && ringRadiusZ > 0.);

      // create a unit vector from cone origin to the point on the face at angle phi
      const double rx = ringRadiusX * cosphi;
      const double rz = ringRadiusZ * sinphi;
      // Calculate the y value that will make a unit vector from rx and rz
      const float ry = calcYValue_(rx, rz);
      const osg::Vec3 rawUnitVec(static_cast<float>(rx), ry, static_cast<float>(rz));
      // this is a unit vector, no need to normalize
      assert(simCore::areEqual(rawUnitVec.length(), 1.0));

      const osg::Vec3 unitVec = dirQ * rawUnitVec;
      const osg::Vec3 farVec = unitVec * d.farRange_;

      (*v)[vptr] = farVec;
      (*n)[vptr] = unitVec;
      (*m)[vptr].set(USAGE_CONEFAR, angleX, angleZ, rawUnitVec, 1.0f);

      // add the new point to the slice's far face geometry:
      // vptr + numRings is the corresponding vertex in the next slice; can't use that when we get to last slice.
      const unsigned int correspondingVertexInNextSlice = (slice + 1 < numSlices) ? (vptr + numRings) : (farOffset + ring);
      farWedge->push_back(correspondingVertexInNextSlice);
      farWedge->push_back(vptr);

      // if drawing the near face, the vertices are calculated and stored in the vertex array here,
      // but added to primitive separately below, to mitigate near/far face artifacts
      if (hasNear)
      {
        const osg::Vec3 nearVec = unitVec * d.nearRange_;
        (*v)[vptr + vertsPerFace] = nearVec;
        (*n)[vptr + vertsPerFace] = -unitVec;
        (*m)[vptr + vertsPerFace].set(USAGE_CONENEAR, rx, angleZ, rawUnitVec, 0.0f);
      }
      vptr++;
    }
    // add face to the geometry
    // if assert fails, check that elsPerSlice still represents the number of vertices that are added
    assert(farWedge->size() == elsPerSlice);
    geom->addPrimitiveSet(farWedge.get());
  }

  // the near face geometry is created separately to mitigate near/far face artifacts
  if (hasNear)
  {
    // vptr has until now only counted far face vertices; we need it to count near face vertices too
    assert(vptr == nearOffset);
    vptr++; // increment one for near face center vertex

    // loop over the slices and build the near geometry using vertex array indices
    for (unsigned int slice = 0; slice < numSlices; ++slice)
    {
      osg::ref_ptr<osg::DrawElementsUShort> nearWedge = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
      nearWedge->reserveElements(elsPerSlice);
      nearWedge->push_back(nearOffset); // start with the center point

      for (unsigned int ring = 1; ring <= numRings; ++ring)
      {
        nearWedge->push_back(vptr);
        // vptr + numRings is the corresponding vertex in the next slice; can't use that when we get to last slice.
        const unsigned int correspondingVertexInNextSlice = (slice + 1 < numSlices) ? (vptr + numRings) : (nearOffset + ring);
        nearWedge->push_back(correspondingVertexInNextSlice);
        vptr++;
      }
      // add each face to the geometry
      // if assert fails, check that elsPerSlice still represents the number of vertices that are added
      assert(nearWedge->size() == elsPerSlice);
      geom->addPrimitiveSet(nearWedge.get());
    }
  }

  if (d.drawCone_)
  {
    // next, build the cone wall. we need out-facing normals.
    // yes this can be computed while we are building the faces but that is an optimization for later.

    // ensure that cone is aligned to cap, since cap is drawn normally, but cone is drawn in alternating strips from bottom.
    bool evenSlice = ((numSlices % 2) == 0);

    const double angleX = ringSpanX * numRings;
    const double angleZ = numRings * ringSpanZ;
    // clamping of numRings, hfov_deg, and vfov_deg guarantee this assert
    assert(angleX > 0. && angleX <= M_PI_2);
    assert(angleZ > 0. && angleZ <= M_PI_2);
    const double coneRadiusX = sin(angleX);
    const double coneRadiusZ = sin(angleZ);
    // previous asserts guarantee this assert
    assert(coneRadiusX > 0. && coneRadiusZ > 0.);


    // iterate for triangle strip slices that start at tip of cone and extend to far end(base) of cone
    for (unsigned int slice = 0; slice < numSlices; ++slice)
    {
      // start at bottom of cone and alternately build strips on either side ascending, to manage draw order
      // this approach fixes obvious artifacts when beam is viewed from above, but may display artifacts when cone is viewed from side or from below,
      // or more obviously if roll offset is applied
      double sliceAngle;
      if (evenSlice)
        sliceAngle = -M_PI_2 + sliceAngle_rad * 0.5 * slice;
      else
        sliceAngle = -M_PI_2 - sliceAngle_rad * 0.5 * (slice + 1);
      evenSlice = !evenSlice;

      // build a triangle strip for the slice

      // precalculate the two vertices on the far end of cone
      osg::Vec3 rawUnitVec[2], unitVec[2], nearVec[2], lengthVec[2];
      float rx[2], rz[2];
      for (unsigned int i = 0; i < 2; ++i)
      {
        // starting and ending angles of the slice, in order to set winding correctly
        const double phi = (i == 0) ? simCore::angFixPI(sliceAngle + sliceAngle_rad) : simCore::angFixPI(sliceAngle);

        // create a unit vector to the point on the face at angle phi
        const double r_x = coneRadiusX * cos(phi);
        const double r_z = coneRadiusZ * sin(phi);
        rx[i] = static_cast<float>(r_x);
        rz[i] = static_cast<float>(r_z);

        // Calculate the y value that will make a unit vector from r_x and r_z
        const float ry = calcYValue_(r_x, r_z);
        rawUnitVec[i].set(rx[i], ry, rz[i]);
        // this is a unit vector, no need to normalize
        assert(simCore::areEqual(rawUnitVec[i].length(), 1.0));

        unitVec[i] = dirQ * rawUnitVec[i];

        // the point on the near face (or at the origin if there's no near face)
        nearVec[i] = (hasNear ? (unitVec[i] * d.nearRange_) : osg::Vec3());
        lengthVec[i] = (unitVec[i] * d.farRange_) - nearVec[i];
      }

      osg::ref_ptr<osg::DrawElementsUShort> side = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
      side->reserveElements(2 * (d.wallRes_ + 1));

      // calculate vertices, tesselating from cone origin out to pre-calc'd points on cone far edge
      const float tessStep = 1.0f / d.wallRes_;
      for (unsigned int q = 0; q < d.wallRes_ + 1; ++q)
      {
        // double precision calcs might be required below.
        const float w = tessStep * q;
        // this duplicates vertices that are shared between slices;
        // could be optimized instead to reuse vertices from prev or next slice (but calc'ing normals might be complicated)
        for (unsigned int i = 0; i < 2; ++i)
        {
          (*v)[vptr] = nearVec[i] + (lengthVec[i] * w);
          // normal vector is the vector difference between the vertex position vector and the position vector defined by the vertex position vector length along the y axis
          // this should approximate a right triangle with vertices at beam origin, vertex position, and on the y-axis, with hypotenuse down the y axis.
          const double y = (*v)[vptr].length();
          osg::Vec3 normal;
          if (y != 0.)
            normal.set((*v)[vptr].x(), (*v)[vptr].y() - y, (*v)[vptr].z());
          else
          {
            // at the origin, set something usable
            normal.set(rx[i], 0.f, rz[i]);
          }

          normal.normalize();
          (*n)[vptr] = normal;
          (*m)[vptr].set(USAGE_CONE, angleX, angleZ, rawUnitVec[i], w);
          side->addElement(vptr);
          vptr++;
        }
      }
      geom->addPrimitiveSet(side.get());
    }

    // asserting that we used all the vertices we expected to
    // if assert fails, check numVerts calculation
    assert(numVerts == vptr);
  }
}

// A SphericalVolume is a MatrixTransform that parents up to two geode/groups.
// The first contains the primary geometry; that geometry will always exist, but in some cases will have no primitives.
// That second in the MatrixTransform (if it exists) contains the opaque elements of the sv:
// For the pyramid sv, it contains the outline.
// For the cone sv, it contains a wireframe (polygon) geometry.

osg::MatrixTransform* SVFactory::createNode(const SVData& d, const osg::Vec3& dir)
{
  osg::MatrixTransform* xform = new osg::MatrixTransform();
  xform->setName("SVFactory Node Transform");

  if (d.shape_ == SVData::SHAPE_PYRAMID)
  {
    svPyramidFactory(*xform, d, dir);
  }
  else
  {
    osg::ref_ptr<osg::Geode> geodeSolid = new osg::Geode();
    geodeSolid->setName("Solid Geode");
    xform->addChild(geodeSolid.get());
    createCone_(geodeSolid.get(), d, dir);
  }

  // draw-as-wireframe or add wireframe to stipple/solid geom
  if (SVData::DRAW_MODE_WIRE & d.drawMode_)
    processWireframe_(xform, d.drawMode_);

  // Turn off backface culling
  xform->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

  updateLighting(xform, d.lightingEnabled_);
  updateBlending(xform, d.blendingEnabled_);
  updateStippling(xform, ((SVData::DRAW_MODE_STIPPLE & d.drawMode_) == SVData::DRAW_MODE_STIPPLE));

  return xform;
}

void SVFactory::processWireframe_(osg::MatrixTransform* xform, int drawMode)
{
  if (SVData::DRAW_MODE_WIRE & drawMode)
  {
    osg::Geometry* solidGeom = SVFactory::solidGeometry(xform);
    if (solidGeom == nullptr || solidGeom->empty())
    {
      assert(0);
      return;
    }
    if ((SVData::DRAW_MODE_SOLID & drawMode) || (SVData::DRAW_MODE_STIPPLE & drawMode))
    {
      // create a new wireframe geometry as a shallow copy of the solid geometry
      osg::Geometry* wireframeGeom = new osg::Geometry(*solidGeom);
      wireframeGeom->setName("simVis::SphericalVolume::cone-wireframe");

      // but with its own color array
      osg::Vec4Array* wireframeColor = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
      // default to white
      (*wireframeColor)[0] = simVis::Color::White;
      // but use the solid geometry color if it can be found
      osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>(solidGeom->getColorArray());
      if (colors)
      {
        if (colors->size() == 1)
        {
          (*wireframeColor)[0] = (*colors)[0];
          (*wireframeColor)[0][3] = 1.0f; // no transparency in the wireframe
        }
        else
        {
          // sv color arrays are fixed at size 1
          assert(0);
        }
      }
      wireframeGeom->setColorArray(wireframeColor);

      // add this to a 2nd group in the xform: the 2nd group in the xform is for opaque features
      osg::Group* groupWire = new osg::Group();
      groupWire->addChild(wireframeGeom);
      xform->addChild(groupWire);

      osg::StateSet* stateset = wireframeGeom->getOrCreateStateSet();
      osg::PolygonMode* pm = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
      stateset->setAttributeAndModes(pm, osg::StateAttribute::ON);

      // wireframe is neither lit nor blended when it is paired with another draw type
      simVis::setLighting(stateset, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
      stateset->setMode(GL_BLEND,
        osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    }
    else
    {
      // wireframe is the primary/'solid' geometry - it can be lit, blended
      osg::PolygonMode* pm = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
      solidGeom->getOrCreateStateSet()->setAttributeAndModes(pm, osg::StateAttribute::ON);
    }
  }
}

void SVFactory::updateStippling(osg::MatrixTransform* xform, bool stippling)
{
  // only the solid geometry can be stippled
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == nullptr || geom->empty())
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  simVis::PolygonStipple::setValues(geom->getOrCreateStateSet(), stippling, 0u);
}

void SVFactory::updateLighting(osg::MatrixTransform* xform, bool lighting)
{
  // lighting is only applied to the solid geometry
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == nullptr || geom->empty())
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  osg::StateSet* stateSet = geom->getOrCreateStateSet();
  simVis::setLighting(stateSet, lighting ?
    osg::StateAttribute::ON  | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE :
    osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
}

void SVFactory::updateBlending(osg::MatrixTransform* xform, bool blending)
{
  // blending is only applied to the solid geometry
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == nullptr || geom->empty())
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  };
  geom->getOrCreateStateSet()->setMode(GL_BLEND, blending ?
    osg::StateAttribute::ON :
    osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
}

void SVFactory::updateColor(osg::MatrixTransform* xform, const osg::Vec4f& color)
{
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == nullptr || geom->empty())
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
  if (colors)
  {
    const size_t colorsSize = colors->size();
    // check that all geometries use BIND_OVERALL, and color arrays are fixed at size 1
    assert(colorsSize == 1);
#ifndef NDEBUG
    OE_INFO << "update color, size = " << colorsSize << std::endl;
#endif
    if ((*colors)[0] != color)
    {
      colors->assign(colorsSize, color);
      colors->dirty();
    }
  }

  // if we have an 2nd (optional) group, it is opaque; update its color, but remove transparency
  osg::Group* opaqueGroup = SVFactory::opaqueGroup(xform);
  if (opaqueGroup == nullptr)
    return;

  // the opaque group may be an svPyramidOutline; svPyramidOutline sets the opacity itself
  svPyramidOutline* pyramidOutline = dynamic_cast<svPyramidOutline*>(opaqueGroup);
  if (pyramidOutline)
  {
    pyramidOutline->setColor(color);
    return;
  }

  // if the opaque group is not a svPyramidOutline, it may contain a wireframe geometry
  if (opaqueGroup->getNumChildren() == 1)
  {
    geom = opaqueGroup->getChild(0)->asGeometry();
    if (geom == nullptr)
    {
      // Assertion failure means internal consistency error, or caller has inconsistent input
      assert(0);
      return;
    }
    if (geom->empty())
      return;
    colors = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
    if (colors)
    {
      if (colors->size() != 1)
      {
        // check that all geometries use BIND_OVERALL, and color arrays are fixed at size 1
        assert(0);
        return;
      }

      osg::Vec4f opaqueColor = color;
      opaqueColor.a() = 1.0f;

      // do not dirty the geometry if there is no change
      if ((*colors)[0][0] != color[0] ||
        (*colors)[0][1] != color[1] ||
        (*colors)[0][2] != color[2])
      {
        colors->assign(1, opaqueColor);
        colors->dirty();
      }
    }
  }
}

void SVFactory::updateNearRange(osg::MatrixTransform* xform, double nearRange)
{
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == nullptr || geom->empty())
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(verts);
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(meta);
  if (verts == nullptr || meta == nullptr)
    return;

  const std::vector<SVMeta>& m = meta->vertMeta_;
  nearRange = simCore::sdkMax(1.0, nearRange);
  meta->nearRange_ = nearRange;
  const double range = meta->farRange_ - nearRange;
  for (unsigned int i = 0; i < verts->size(); ++i)
  {
    const double farRatio = m[i].ratio_;
    (*verts)[i] = m[i].unit_ * (nearRange + range*farRatio);
  }
  verts->dirty();
  dirtyBound_(xform);
}

void SVFactory::updateFarRange(osg::MatrixTransform* xform, double farRange)
{
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == nullptr || geom->empty())
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(verts);
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(meta);
  if (verts == nullptr || meta == nullptr)
    return;

  const std::vector<SVMeta>& m = meta->vertMeta_;
  farRange = simCore::sdkMax(1.0, farRange);
  meta->farRange_ = farRange;
  const double range = farRange - meta->nearRange_;
  for (unsigned int i = 0; i < verts->size(); ++i)
  {
    const double farRatio = m[i].ratio_;
    (*verts)[i] = m[i].unit_ * (meta->nearRange_ + range*farRatio);
  }
  verts->dirty();
  dirtyBound_(xform);
}

void SVFactory::updateHorizAngle(osg::MatrixTransform* xform, double oldAngle, double newAngle)
{
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == nullptr || geom->empty())
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  osg::Vec3Array* normals = static_cast<osg::Vec3Array*>(geom->getNormalArray());
  if (verts == nullptr || meta == nullptr || normals == nullptr)
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  std::vector<SVMeta>& vertMeta = meta->vertMeta_;
  const bool isCone = (vertMeta[0].usage_ == USAGE_CONEFAR);
  // for horiz bw: cone clamped to PI, pyramid clamped to TWOPI
  const double maxClamp = (isCone ? M_PI : M_TWOPI);
  oldAngle = osg::clampBetween(oldAngle, (0.01 * simCore::DEG2RAD), maxClamp);
  newAngle = osg::clampBetween(newAngle, (0.01 * simCore::DEG2RAD), maxClamp);
  const double oldMinAngle = oldAngle * 0.5;
  const double newMinAngle = newAngle * 0.5;
  for (unsigned int i = 0; i < verts->size(); ++i)
  {
    SVMeta& m = vertMeta[i];
    // exclude cone origin
    if (m.unit_.x() == 0.0f && m.unit_.z() == 0.0f)
      continue;

    // recalc metadata
    switch (m.usage_)
    {
    // cone metadata has different meaning than pyramid metadata wrt to h and v angles, and the calcs are distinct.
    case USAGE_CONE:
    case USAGE_CONENEAR:
    case USAGE_CONEFAR:
    {
      // osg::clampBetween above guarantees this assert
      assert(oldMinAngle > 0.);
      // anglez_ in metadata is in a fixed ratio to max angle;
      // it may be the angle to a subring (in face), or max angle (in the cone)
      // max angle will change, but that ratio does not change, as we're not adding rings.

      // createCone_ and updateVertAngle guarantee this
      assert(m.anglex_ > 0. && m.anglex_ <= (M_PI_2 + std::numeric_limits<float>::epsilon()));

      // developer error indicated; oldMinAngle is the max possible value for m.anglex_
      assert(m.anglex_ <= (oldMinAngle + std::numeric_limits<float>::epsilon()));

      // if ratio is less than 1, this is a vertex in a subring in the face
      // simple ratio logic works since cone is symmetric around 0; if not symmetric around 0, need to use pyramid logic
      const double ratio = (simCore::areEqual(m.anglex_, oldMinAngle) ? 1.0 : m.anglex_ / oldMinAngle);
      // previous asserts and clamps guarantee this
      assert(ratio > 0. && ratio <= 1.);
      const double ax_new = newMinAngle * ratio;

      // clamping and previous asserts guaranteee this
      assert(ax_new > 0. && ax_new <= M_PI_2);

      // this assert guaranteed by assert above
      assert(sin(m.anglex_) != 0.);
      // x value of vert is proportional to sin(h beamwidth)
      const double multiplier = sin(ax_new) / sin(m.anglex_);

      m.anglex_ = ax_new;
      // change in x is proportional to change in sin(vert beamwidth) (phi is not changing)
      const double rx = m.unit_.x() * multiplier;

      // Calculate the y value that will make a unit vector from rx and m.unit_.z
      const float ry = calcYValue_(rx, static_cast<double>(m.unit_.z()));
      m.unit_.set(static_cast<float>(rx), ry, m.unit_.z());

      // this is a unit vector, no need to normalize
      assert(simCore::areEqual(m.unit_.length(), 1.0));
      break;
    }
    // cone metadata has different meaning than pyramid metadata wrt to h and v angles, and the calcs are distinct.
    case USAGE_NEAR:
    case USAGE_FAR:
    case USAGE_TOP:
    case USAGE_BOTTOM:
    case USAGE_LEFT:
    case USAGE_RIGHT:
    {
      // osg::clampBetween above guarantees this assert
      assert(oldAngle > 0.);
      const double t = (m.anglex_ + oldMinAngle) / oldAngle;
      const double ax = -newMinAngle + t * newAngle;
      const double sinx = sin(ax);
      const double cosx = cos(ax);
      const float sinz = sin(m.anglez_);
      const float cosz = cos(m.anglez_);
      m.anglex_ = ax;
      m.unit_.set(sinx*cosz, cosx*cosz, sinz);
      // this is a unit vector, no need to normalize
      assert(simCore::areEqual(m.unit_.length(), 1.0));
      break;
    }
    }
    // rotate the unit vector in the direction dirQ_
    const osg::Vec3 unitRot = meta->dirQ_ * m.unit_;

    // recalc vertex and its normal
    switch (m.usage_)
    {
    case USAGE_CONENEAR:
    case USAGE_NEAR:
      (*verts)[i] = unitRot * meta->nearRange_;
      (*normals)[i] = (unitRot * -1);
      break;
    case USAGE_CONEFAR:
    case USAGE_FAR:
      (*verts)[i] = unitRot * meta->farRange_;
      (*normals)[i] = unitRot;
      break;
    case USAGE_BOTTOM:
      (*verts)[i] = unitRot * (*verts)[i].length();
      (*normals)[i] = (osg::Vec3(unitRot.x(), unitRot.z(), -unitRot.y()));
      break;
    case USAGE_TOP:
      (*verts)[i] = unitRot * (*verts)[i].length();
      (*normals)[i] = (osg::Vec3(unitRot.x(), -unitRot.z(), unitRot.y()));
      break;
    case USAGE_RIGHT:
      (*verts)[i] = unitRot * (*verts)[i].length();
      (*normals)[i] = (osg::Vec3(unitRot.y(), -unitRot.x(), unitRot.z()));
      break;
    case USAGE_LEFT:
      (*verts)[i] = unitRot * (*verts)[i].length();
      (*normals)[i] = (osg::Vec3(-unitRot.y(), unitRot.x(), unitRot.z()));
      break;
    case USAGE_CONE:
    {
      const float range = (*verts)[i].length();
      (*verts)[i] = unitRot * range;
      osg::Vec3 normal((*verts)[i].x(), (*verts)[i].y() - range, (*verts)[i].z());
      normal.normalize();
      (*normals)[i] = normal;
      break;
    }
    }
  }

  verts->dirty();
  normals->dirty();
  dirtyBound_(xform);
}

void SVFactory::updateVertAngle(osg::MatrixTransform* xform, double oldAngle, double newAngle)
{
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == nullptr || geom->empty())
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  osg::Vec3Array* normals = static_cast<osg::Vec3Array*>(geom->getNormalArray());
  if (verts == nullptr || meta == nullptr || normals == nullptr)
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  std::vector<SVMeta>& vertMeta = meta->vertMeta_;

  // clamp to M_PI, to match clamping in pyramid and cone
  oldAngle = osg::clampBetween(oldAngle, (0.01 * simCore::DEG2RAD), M_PI);
  newAngle = osg::clampBetween(newAngle, (0.01 * simCore::DEG2RAD), M_PI);
  const double oldMinAngle = oldAngle * 0.5;
  const double newMinAngle = newAngle * 0.5;

  for (unsigned int i = 0; i < verts->size(); ++i)
  {
    SVMeta& m = vertMeta[i];

    // exclude cone origin
    if (m.unit_.x() == 0.0f && m.unit_.z() == 0.0f)
      continue;

    // recalc metadata
    switch (m.usage_)
    {
    // cone metadata has different meaning than pyramid metadata wrt to h and v angles, and the calcs are distinct.
    case USAGE_CONE:
    case USAGE_CONENEAR:
    case USAGE_CONEFAR:
    {
      // osg::clampBetween above guarantees this assert
      assert(oldMinAngle > 0.);
      // anglez_ in metadata is in a fixed ratio to max angle;
      // it may be the angle to a subring (in face), or max angle (in the cone)
      // that ratio does not change, as we're not adding rings

      // createCone_ and updateVertAngle guarantee this
      assert(m.anglez_ > 0. && m.anglez_ <= (M_PI_2 + std::numeric_limits<float>::epsilon()));

      // developer error indicated; oldMinAngle is the max possible value for m.anglez_
      assert(m.anglez_ <= (oldMinAngle + std::numeric_limits<float>::epsilon()));

      // if ratio is less than 1, this is a vertex in a subring in the face
      // simple ratio logic works since cone is symmetric around 0; if not, need to use (pyramid) logic below
      const double ratio = (simCore::areEqual(m.anglez_, oldMinAngle) ? 1.0 : m.anglez_ / oldMinAngle);
      // previous asserts and clamps guarantee this
      assert(ratio > 0. && ratio <= 1.);

      const double az_new = newMinAngle * ratio;
      // clamping and previous asserts guaranteee this
      assert(az_new > 0. && az_new <= M_PI_2);

      // this assert guaranteed by assert above
      assert(sin(m.anglez_) != 0.);
      // z value of vert is proportional to sin(beamwidth)
      const double multiplier = sin(az_new) / sin(m.anglez_);

      m.anglez_ = az_new;
      // change in z is proportional to change in sin(vert beamwidth) (phi is not changing)
      const double rz = m.unit_.z() * multiplier;
      // Calculate the y value that will make a unit vector from m.unit_.x and rz
      const float ry = calcYValue_(static_cast<double>(m.unit_.x()), rz);
      m.unit_.set(m.unit_.x(), ry, static_cast<float>(rz));
      // this is a unit vector, no need to normalize
      assert(simCore::areEqual(m.unit_.length(), 1.0));
      break;
    }
    // cone metadata has different meaning than pyramid metadata wrt to h and v angles, and the calcs are distinct.
    case USAGE_NEAR:
    case USAGE_FAR:
    case USAGE_TOP:
    case USAGE_BOTTOM:
    case USAGE_LEFT:
    case USAGE_RIGHT:
    {
      // osg::clampBetween above guarantees this assert
      assert(oldAngle > 0.);
      const double t = (m.anglez_ + oldMinAngle) / oldAngle;
      const double az = -newMinAngle + t * newAngle;
      const float sinx = sin(m.anglex_);
      const float cosx = cos(m.anglex_);
      const double sinz = sin(az);
      const double cosz = cos(az);
      m.anglez_ = az;
      m.unit_.set(sinx*cosz, cosx*cosz, sinz);
      // this is a unit vector, no need to normalize
      assert(simCore::areEqual(m.unit_.length(), 1.0));
      break;
    }
    }
    // rotate the unit vector in the direction dirQ_
    const osg::Vec3 unitRot = meta->dirQ_ * m.unit_;

    // recalc vertex and normal
    switch (m.usage_)
    {
    case USAGE_CONENEAR:
    case USAGE_NEAR:
      (*verts)[i] = unitRot * meta->nearRange_;
      (*normals)[i] = (unitRot * -1);
      break;
    case USAGE_CONEFAR:
    case USAGE_FAR:
      (*verts)[i] = unitRot * meta->farRange_;
      (*normals)[i] = unitRot;
      break;
    case USAGE_BOTTOM:
      (*verts)[i] = unitRot * (*verts)[i].length();
      (*normals)[i] = (osg::Vec3(unitRot.x(), unitRot.z(), -unitRot.y()));
      break;
    case USAGE_TOP:
      (*verts)[i] = unitRot * (*verts)[i].length();
      (*normals)[i] = (osg::Vec3(unitRot.x(), -unitRot.z(), unitRot.y()));
      break;
    case USAGE_RIGHT:
      (*verts)[i] = unitRot * (*verts)[i].length();
      (*normals)[i] = (osg::Vec3(unitRot.y(), -unitRot.x(), unitRot.z()));
      break;
    case USAGE_LEFT:
      (*verts)[i] = unitRot * (*verts)[i].length();
      (*normals)[i] = (osg::Vec3(-unitRot.y(), unitRot.x(), unitRot.z()));
      break;
    case USAGE_CONE:
    {
      const float range = (*verts)[i].length();
      (*verts)[i] = unitRot * range;
      osg::Vec3 normal((*verts)[i].x(), (*verts)[i].y() - range, (*verts)[i].z());
      normal.normalize();
      (*normals)[i] = normal;
      break;
    }
    }
  }

  verts->dirty();
  normals->dirty();
  dirtyBound_(xform);
}

osg::Geometry* SVFactory::solidGeometry(osg::MatrixTransform* xform)
{
  if (xform == nullptr || xform->getNumChildren() == 0)
    return nullptr;
  osg::Geode* geode = xform->getChild(0)->asGeode();
  if (geode == nullptr || geode->getNumDrawables() == 0)
    return nullptr;
  return geode->getDrawable(0)->asGeometry();
}

// if the sv has a 2nd geode that adds outline or wireframe, it will be the MatrixTransform 2nd child
osg::Group* SVFactory::opaqueGroup(osg::MatrixTransform* xform)
{
  if (xform == nullptr || xform->getNumChildren() < 2)
    return nullptr;
  return xform->getChild(1)->asGroup();
}

// dirty bounds for all geometries in the xform
void SVFactory::dirtyBound_(osg::MatrixTransform* xform)
{
  if (xform == nullptr || xform->getNumChildren() == 0)
    return;

  // handle the geometries in the primary geode
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom && !geom->empty())
    geom->dirtyBound();

  // handle the 2nd/opaque group
  osg::Group* opaqueGroup = SVFactory::opaqueGroup(xform);
  if (opaqueGroup && opaqueGroup->getNumChildren() > 0)
  {
    // the opaque group may be an svPyramidOutline; svPyramidOutline must be regenerated using the updated vertices
    svPyramidOutline* pyramidOutline = dynamic_cast<svPyramidOutline*>(opaqueGroup);
    if (pyramidOutline)
    {
      pyramidOutline->regenerate();
      return;
    }

    geom = opaqueGroup->getChild(0)->asGeometry();
    if (geom && !geom->empty())
      geom->dirtyBound();
  }
}

}
