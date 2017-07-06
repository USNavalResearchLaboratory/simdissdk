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
#include "osg/BlendFunc"
#include "osg/CullFace"
#include "osg/Depth"
#include "osg/Geode"
#include "osg/LineWidth"
#include "osg/Point"
#include "osg/PolygonMode"
#include "osg/PolygonStipple"
#include "osg/UserDataContainer"
#include "osgUtil/Simplifier"

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simVis/Constants.h"
#include "simVis/SphericalVolume.h"
#include "simVis/Utils.h"

using namespace simVis;

//---------------------------------------------------------------------------

#define TESS_SPACING_DEG 1.0f

#define USAGE_NONE     0x00
#define USAGE_NEAR     0x01
#define USAGE_FAR      0x02
#define USAGE_CENTROID 0x04

#define FACE_NONE     0
#define FACE_NEAR     1
#define FACE_FAR      2
#define FACE_CONE     3
#define FACE_CENTROID 4

#define Q(T) #T

namespace
{
  struct SVMeta
  {
    char      usage_;     // near, far, or centroid
    float     anglex_;    // angle in X
    float     anglez_;    // angle in Z
    float     ratio_;     // 0 at near, 1 at far
    osg::Vec3 unit_;      // unit vector

    SVMeta(): usage_(static_cast<char>(0)), anglex_(0.0f), anglez_(0.0f), ratio_(0.0f)
    {
    }

    explicit SVMeta(char usage): usage_(usage), anglex_(0.0f), anglez_(0.0f), ratio_(0.0f)
    {
    }

    SVMeta(char usage, float anglex, float anglez, const osg::Vec3& unit, float ratio)
    {
      usage_ = usage;
      set(anglex, anglez, unit, ratio);
    }

    inline void set(char usage, float anglex, float anglez, const osg::Vec3& unit, float ratio)
    {
      usage_ = usage;
      set(anglex, anglez, unit, ratio);
    }

    inline void set(float anglex, float anglez, const osg::Vec3& unit, float ratio)
    {
      anglex_ = anglex;
      anglez_ = anglez;
      unit_   = unit;
      ratio_  = ratio;
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

  // custom draw elements class that lets us toggle individual primitives at draw time.
  template<typename BASE>
  class ToggleDE : public BASE
  {
  public:
    ToggleDE(): BASE(), toggle_(NULL) {}
    ToggleDE(GLenum mode, bool* togglePtr =NULL) : BASE(mode), toggle_(togglePtr) {}
    ToggleDE(const ToggleDE<BASE>& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) : BASE(rhs, copyop), toggle_(NULL) {}
    virtual osg::Object* cloneType() const { return new ToggleDE<BASE>(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new ToggleDE<BASE>(*this, copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const ToggleDE<BASE>*>(obj) != NULL; }
    virtual const char* libraryName() const { return "simVis"; }
    virtual const char* className() const { return "ToggleDE<BASE>"; }

  public:
    virtual void draw(osg::State& state, bool useVertexBufferObjects) const
    {
      if ((!toggle_) || (*toggle_))
        BASE::draw(state, useVertexBufferObjects);
    }

  private:
    bool* toggle_;
  };

  typedef ToggleDE<osg::DrawElementsUByte>  ToggleDrawElementsUByte;
  typedef ToggleDE<osg::DrawElementsUShort> ToggleDrawElementsUShort;
  typedef ToggleDE<osg::DrawElementsUInt>   ToggleDrawElementsUInt;
}


void SVFactory::createPyramid_(osg::Geode& geode, const SVData& d, const osg::Vec3& direction)
{
  if (d.drawMode_ == SVData::DRAW_MODE_NONE || d.capRes_ == 0)
  {
    // early out; at present, geode must have a non-NULL geometry, even if empty
    geode.addDrawable(new osg::Geometry());
    return;
  }

  const float nearRange = d.nearRange_ * d.scale_;
  const float farRange = d.farRange_  * d.scale_;

  // quaternion that will "point" the volume along our direction vector
  osg::Quat dirQ;
  dirQ.makeRotate(osg::Vec3(0.0f, 1.0f, 0.0f), direction);

  osg::Vec3Array* vertexArray = new osg::Vec3Array();
  osg::IntArray* faceArray = new osg::IntArray();
  osg::Vec3Array* normalArray = new osg::Vec3Array();

  SVMetaContainer* metaContainer = new SVMetaContainer();
  metaContainer->dirQ_ = dirQ;
  metaContainer->nearRange_ = nearRange;
  metaContainer->farRange_ = farRange;
  std::vector<SVMeta>* vertexMetaData = &metaContainer->vertMeta_;


  const bool drawFaces = (d.drawMode_ != SVData::DRAW_MODE_OUTLINE);
  // we always have a solid/face geometry, even if we add no primitives b/c we are drawing outline only
  osg::Geometry* faceGeom = new osg::Geometry();
  geode.addDrawable(faceGeom);
  faceGeom->setUseVertexBufferObjects(true);
  faceGeom->setUseDisplayList(false);
  faceGeom->setDataVariance(osg::Object::DYNAMIC); // prevent draw/update overlap

  osg::Vec4Array* colorArray = new osg::Vec4Array(1);
  faceGeom->setColorArray(colorArray);
  faceGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
  (*colorArray)[0] = d.color_;

  faceGeom->setVertexArray(vertexArray);

  faceGeom->setUserData(metaContainer);

  faceGeom->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, faceArray);
  faceGeom->setVertexAttribBinding(osg::Drawable::ATTRIBUTE_6, osg::Geometry::BIND_PER_VERTEX);
  faceGeom->setVertexAttribNormalize(osg::Drawable::ATTRIBUTE_6, false);

  faceGeom->setNormalArray(normalArray);
  faceGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);


  osg::Geometry* outlineGeom = NULL;
  const bool drawOutlines = (SVData::DRAW_MODE_OUTLINE & d.drawMode_) == SVData::DRAW_MODE_OUTLINE;
  if (drawOutlines)
  {
    outlineGeom = new osg::Geometry();
    geode.addDrawable(outlineGeom);
    outlineGeom->setUseVertexBufferObjects(true);
    outlineGeom->setUseDisplayList(false);
    outlineGeom->setDataVariance(osg::Object::DYNAMIC); // prevent draw/update overlap
    outlineGeom->setCullingActive(false);

    osg::Vec4Array* outlineColor = new osg::Vec4Array(1);
    (*outlineColor)[0] = d.color_;
    (*outlineColor)[0][3] = 1.0f; // no transparency in the outline
    outlineGeom->setColorArray(outlineColor);
    outlineGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    outlineGeom->setVertexArray(vertexArray);

    outlineGeom->setUserData(metaContainer);

    outlineGeom->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, faceArray);
    outlineGeom->setVertexAttribBinding(osg::Drawable::ATTRIBUTE_6, osg::Geometry::BIND_PER_VERTEX);
    outlineGeom->setVertexAttribNormalize(osg::Drawable::ATTRIBUTE_6, false);

    outlineGeom->setNormalArray(normalArray);
    outlineGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    // configure a state set
    outlineGeom->getOrCreateStateSet()->setAttributeAndModes(
      new osg::LineWidth(d.outlineWidth_),
      osg::StateAttribute::ON);
  }

  const float hfov_deg = osg::clampBetween(d.hfov_deg_, 0.01f, 360.0f);
  const unsigned int numPointsX = d.capRes_ + 1;
  float x_start = -0.5f * hfov_deg;
  const float spacingX = hfov_deg / (numPointsX - 1);
  // in sphere-seg mode, bake the azim offsets into the model
  if (d.drawAsSphereSegment_)
  {
    x_start += d.azimOffset_deg_;
  }

  float vfov_deg = osg::clampBetween(d.vfov_deg_, 0.01f, 180.0f);
  float z_start = -0.5f * vfov_deg;
  float z_end = 0.5f * vfov_deg;
  // in sphere-seg mode, bake the elev offsets into the model, and clamp to [-90,90]
  if (d.drawAsSphereSegment_)
  {
    z_start = simCore::angFix90(z_start + d.elevOffset_deg_);
    z_end = simCore::angFix90(z_end + d.elevOffset_deg_);
    vfov_deg = z_end - z_start;
  }
  const unsigned int numPointsZ = d.capRes_ + 1;
  const float spacingZ = vfov_deg / (numPointsZ - 1);

  // only draw the near face if:
  const bool drawCone = (d.drawCone_ && d.wallRes_ != 0);
  const bool hasNear = d.nearRange_ > 0.0f && d.drawCone_;
  const unsigned int loop = hasNear ? 2 : 1;

  // Calculate the number of vertices for performance hotspot fix in push_back()
  // vertices will be added in this order: far face, near face (if drawn), cone bottom, then cone right (if drawn), cone top (if drawn), cone left (if drawn)
  const unsigned int reserveSizeFace = loop * numPointsX * numPointsZ;
  unsigned int reserveSizeCone = 0;
  if (drawCone)
  {
    // bottom & top faces are only drawn if vfov_deg < 180
    if (vfov_deg < 180.0f)
      reserveSizeCone += (numPointsX - 1) * (1 + d.wallRes_) * 2 * 2;
    // right & left faces only drawn if hfov_deg < 360
    if (hfov_deg < 360.0f)
      reserveSizeCone += (numPointsZ - 1) * (1 + d.wallRes_) * 2 * 2;
  }
  vertexArray->reserve(reserveSizeFace + reserveSizeCone);
  normalArray->reserve(reserveSizeFace + reserveSizeCone);
  faceArray->reserve(reserveSizeFace + reserveSizeCone);
  vertexMetaData->reserve(reserveSizeFace + reserveSizeCone);

  // first calculate vertices for the far face, then if hasNear, the near face
  for (unsigned int i = 0; i < loop; i++)
  {
    const float r = (i == 0) ? farRange : nearRange;
    const float normalDir = (i == 0) ? 1.0f : -1.0f;

    // populate vertex array and other arrays for face geometry
    // if you are looking from the gate origin, 1st gate vertex is at bottom left corner, then vertices go up to top left corner
    // then, starting at bottom again for next x, and going up to top.
    // iterate from x min (left) to xmax (right)
    for (unsigned int x = 0; x < numPointsX; ++x)
    {
      const float angleX_rad = osg::DegreesToRadians(x_start + spacingX * x);
      const float sinAngleX = sin(angleX_rad);
      const float cosAngleX = cos(angleX_rad);

      for (unsigned int z = 0; z < numPointsZ; ++z)
      {
        const float angleZ_rad = osg::DegreesToRadians(z_start + spacingZ * z);
        const float sinAngleZ = sin(angleZ_rad);
        const float cosAngleZ = cos(angleZ_rad);

        const osg::Vec3 unitUnrot(sinAngleX*cosAngleZ, cosAngleX*cosAngleZ, sinAngleZ);
        const osg::Vec3 unit = dirQ * unitUnrot;
        const osg::Vec3 p = unit * r;
        vertexArray->push_back(p);
        normalArray->push_back(unit * normalDir);
        faceArray->push_back(i == 0 ? FACE_FAR : FACE_NEAR);
        vertexMetaData->push_back(SVMeta(i == 0 ? USAGE_FAR : USAGE_NEAR, angleX_rad, angleZ_rad, unitUnrot, i == 0 ? 1.0f : 0.0f));
      }
    }
  }
  // if either assert fails, vertex counts in face no longer match expected/reserved count; vector reserve calls must be updated to match changes to face vertex generation
  assert(vertexArray->size() == reserveSizeFace);
  assert(vertexMetaData->size() == reserveSizeFace);
  const unsigned short farFaceOffset = 0;
  const unsigned short nearFaceOffset = hasNear ? numPointsX * numPointsZ : 0;
  if (hasNear)
  {
    // if assert fails, algorithm for face vertex generation has changed, and nearFaceOffset must be updated to match
    assert(nearFaceOffset == vertexArray->size() / 2);
  }
  // render geometry for the face outlines
  if (drawOutlines)
  {
    osg::ref_ptr<osg::DrawElementsUShort> outline = new osg::DrawElementsUShort(GL_LINES);
    unsigned int numElements = (4 * (numPointsX - 1)) + (4 * (numPointsZ - 1));
    if (hasNear)
      numElements *= 2;
    outline->reserveElements(numElements);

    // horizontals of the gate face outline
    for (unsigned int z = 0; z < numPointsZ; z += (numPointsZ - 1)) // iterate twice, first for the bottom, 2nd for the top
    {
      // iterate across the gate horizontals (x) from left to right (if you look from gate origin)
      for (unsigned int x = 0; x < numPointsX - 1; ++x)
      {
        outline->push_back(farFaceOffset + x*numPointsZ + z);
        outline->push_back(farFaceOffset + (x + 1)*numPointsZ + z);
        if (hasNear)
        {
          outline->push_back(nearFaceOffset + x*numPointsZ + z);
          outline->push_back(nearFaceOffset + (x + 1)*numPointsZ + z);
        }
      }
    }

    // verticals of the gate face outline
    for (unsigned int x = 0; x < numPointsX; x += (numPointsX - 1)) // iterate twice, first for the left, 2nd for the right (if you look from gate origin)
    {
      // this is the index offset for the bottom of either face at the current x
      const unsigned int xOffset = x * numPointsZ;
      for (unsigned int z = 0; z < numPointsZ - 1; ++z)
      {
        outline->push_back(farFaceOffset + xOffset + z);
        outline->push_back(farFaceOffset + xOffset + (z + 1));
        if (hasNear)
        {
          outline->push_back(nearFaceOffset + xOffset + z);
          outline->push_back(nearFaceOffset + xOffset + (z + 1));
        }
      }
    }
    // if assert fails, check that numElements calculation matches the iterations used in this block
    assert(outline->size() == numElements);
    outlineGeom->addPrimitiveSet(outline);
  }

  // if we are drawing the face (not just the outline) add primitives that index into the vertex array
  if (drawFaces)
  {
    // draw vertical triangle strip(s) for each (x, x+1) pair
    for (unsigned int x = 0; x < numPointsX - 1; ++x)
    {
      osg::ref_ptr<osg::DrawElementsUShort> farFaceStrip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
      osg::ref_ptr<osg::DrawElementsUShort> nearFaceStrip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
      const unsigned int numElements = 2 * numPointsZ;
      farFaceStrip->reserveElements(numElements);
      if (hasNear)
        nearFaceStrip->reserveElements(numElements);

      // these are index offsets for the bottom of either face at the current x
      const unsigned int leftX = x * numPointsZ;
      const unsigned int rightX = (x + 1) * numPointsZ;

      for (unsigned int z = 0; z < numPointsZ; ++z)
      {
        farFaceStrip->push_back(farFaceOffset + leftX + z);
        farFaceStrip->push_back(farFaceOffset + rightX + z);
        if (hasNear)
        {
          nearFaceStrip->push_back(nearFaceOffset + leftX + z);
          nearFaceStrip->push_back(nearFaceOffset + rightX + z);
        }
      }

      // if assert fails, check that numElements calculation matches the iterations used in this block
      assert(farFaceStrip->size() == numElements);
      faceGeom->addPrimitiveSet(farFaceStrip);
      if (hasNear)
      {
        assert(nearFaceStrip->size() == numElements);
        faceGeom->addPrimitiveSet(nearFaceStrip);
      }
    }
  }

  // if the near face range is <= 0, then we draw the walls but not that face
  if (drawCone)
  {
    // build vertex sets for the walls. we have to duplicate verts in order to get
    // unique normals, unfortunately.
    unsigned short
      farIndexLL=0, farIndexUL=0, farIndexLR=0, farIndexUR=0,
      nearIndexLL=0, nearIndexUL=0, nearIndexLR=0, nearIndexUR=0;

    const float tessStep = 1.0f / d.wallRes_;
    const float coneLen = farRange - nearRange;
    const unsigned int numElements = (1 + d.wallRes_) * 2;

    // bottom:
    if (vfov_deg < 180.0f)
    {
      // store LR corners for later use in outline (these are indices to points that are added immediately below)
      farIndexLR = vertexArray->size();
      nearIndexLR = farIndexLR + 2 * d.wallRes_;
      // draw the bottom wall outline and face, drawn as triangle strips from the near face to the far face;
      // iterate x across the face from right to left, (looking from near face to far face)
      for (unsigned int x = numPointsX - 1; x > 0; --x)
      {
        // starting index for near and far face vertices for right edge of strip starting at x
        const unsigned int offsetStart = x * numPointsZ;

        osg::ref_ptr<osg::DrawElementsUShort> strip = drawFaces ? new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numElements) : NULL;
        // iterate out from the near face to the far face, in tesselated steps
        for (unsigned int q = 0; q < d.wallRes_ + 1; ++q)
        {
          const float w = tessStep * q;
          for (unsigned int i = 0; i < 2; ++i)
          {
            // i=0 is right edge of strip, i=1 is left edge of strip
            const unsigned int off = offsetStart - (i * numPointsZ);
            const unsigned int foff = farFaceOffset + off;
            const osg::Vec3 nf = hasNear ? (*vertexArray)[nearFaceOffset + off] : osg::Vec3();
            const SVMeta& metafoff = (*vertexMetaData)[foff];
            const osg::Vec3& unit = metafoff.unit_;
            const osg::Vec3 vert = nf + unit * coneLen * w;
            vertexArray->push_back(vert);
            // normal should be the unit vector rotated 90deg around x axis
            normalArray->push_back(osg::Vec3(unit.x(), unit.z(), -unit.y()));
            faceArray->push_back(FACE_CONE);
            vertexMetaData->push_back(SVMeta(USAGE_NONE, metafoff.anglex_, metafoff.anglez_, unit, w));

            if (drawFaces)
              strip->setElement(2 * q + i, vertexArray->size() - 1);
          }
        }
        if (drawFaces)
        {
          faceGeom->addPrimitiveSet(strip);
        }
      }
    }

    // right:
    if (hfov_deg < 360.0f)
    {
      // store UR corners for use in outline
      farIndexUR = vertexArray->size();
      nearIndexUR = farIndexUR + 2 * d.wallRes_;

      // draw the right wall outline and face, drawn as triangle strips from the near face to the far face;
      // iterate z across the face from top to bottom, (looking from near face to far face)
      for (unsigned int z = numPointsZ - 1; z > 0; --z)
      {
        // starting index for near and far face vertices for the top edge of the strip starting at z
        const unsigned int offsetStart = numPointsZ * (numPointsX - 1) + z;

        osg::ref_ptr<osg::DrawElementsUShort> strip = drawFaces ? new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numElements) : NULL;

        // iterate out from the near face to the far face, in tesselated steps
        for (unsigned int q = 0; q < d.wallRes_ + 1; ++q)
        {
          const float w = tessStep * q;
          for (unsigned int i = 0; i < 2; ++i)
          {
            // i=0 is top edge of strip, i=1 is bottom edge of strip
            const unsigned int off = offsetStart - i;
            const unsigned int foff = farFaceOffset + off;
            const osg::Vec3 nf = hasNear ? (*vertexArray)[nearFaceOffset + off] : osg::Vec3();
            const SVMeta& metafoff = (*vertexMetaData)[foff];
            const osg::Vec3& unit = metafoff.unit_;
            const osg::Vec3 vert = nf + unit * coneLen * w;
            vertexArray->push_back(vert);
            // normal should be the unit vector rotated 90deg around z axis
            normalArray->push_back(osg::Vec3(unit.y(), -unit.x(), unit.z()));
            faceArray->push_back(FACE_CONE);
            vertexMetaData->push_back(SVMeta(USAGE_NONE, metafoff.anglex_, metafoff.anglez_, unit, w));

            if (drawFaces)
              strip->setElement(2 * q + i, vertexArray->size() - 1);
          }
        }
        if (drawFaces)
        {
          faceGeom->addPrimitiveSet(strip);
        }
      }
    }

    // top:
    if (vfov_deg < 180.0f)
    {
      // store UL corners for use in outline
      farIndexUL = vertexArray->size();
      nearIndexUL = farIndexUL + 2 * d.wallRes_;
      // draw the top wall outline and face, drawn as triangle strips from the near face to the far face;
      // iterate x across the face from left to right, (looking from near face to far face)
      for (unsigned int x = 0; x < numPointsX - 1; ++x)
      {
        // starting index for near and far face vertices for left edge of the strip starting at x
        const unsigned int offsetStart = (x * numPointsZ) + (numPointsZ - 1);

        osg::ref_ptr<osg::DrawElementsUShort> strip = drawFaces ? new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numElements) : NULL;

        // iterate out from the near face to the far face, in tesselated steps
        for (unsigned int q = 0; q < d.wallRes_ + 1; ++q)
        {
          const float w = tessStep * q;
          for (unsigned int i = 0; i < 2; ++i)
          {
            // i=0 is left edge of strip, i=1 is right edge of strip
            const unsigned int off = offsetStart + (i * numPointsZ);

            const unsigned int foff = farFaceOffset + off;
            const osg::Vec3 nf = hasNear ? (*vertexArray)[nearFaceOffset + off] : osg::Vec3();
            const SVMeta& metafoff = (*vertexMetaData)[foff];
            const osg::Vec3& unit = metafoff.unit_;
            const osg::Vec3 vert = nf + unit * coneLen * w;
            vertexArray->push_back(vert);
            // normal should be the unit vector rotated -90deg around x axis
            normalArray->push_back(osg::Vec3(unit.x(), -unit.z(), unit.y()));
            faceArray->push_back(FACE_CONE);
            vertexMetaData->push_back(SVMeta(USAGE_NONE, metafoff.anglex_, metafoff.anglez_, unit, w));

            if (drawFaces)
              strip->setElement(2 * q + i, vertexArray->size() - 1);
          }
        }
        if (drawFaces)
        {
          faceGeom->addPrimitiveSet(strip);
        }
      }
    }

    // left:
    if (hfov_deg < 360.0f)
    {
      // store LL corners for use in outline
      farIndexLL = vertexArray->size();
      nearIndexLL = farIndexLL + 2 * d.wallRes_;
      // draw the left wall outline and face, drawn as triangle strips from the near face to the far face;
      // iterate z across the face from bottom to top, (looking from near face to far face)
      for (unsigned int z = 0; z < numPointsZ - 1; ++z)
      {
        osg::ref_ptr<osg::DrawElementsUShort> strip = drawFaces ? new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numElements) : NULL;

        // iterate out from the near face to the far face, in tesselated steps
        for (unsigned int q = 0; q < d.wallRes_ + 1; ++q)
        {
          const float w = tessStep * q;
          for (unsigned int i = 0; i < 2; ++i)
          {
            // i=0 is bottom edge of strip, i=1 is top edge of strip
            const unsigned int off = z + i;
            const unsigned int foff = farFaceOffset + off;
            const osg::Vec3 nf = hasNear ? (*vertexArray)[nearFaceOffset + off] : osg::Vec3();
            const SVMeta& metafoff = (*vertexMetaData)[foff];
            const osg::Vec3& unit = metafoff.unit_;
            const osg::Vec3 vert = nf + unit * coneLen * w;
            vertexArray->push_back(vert);
            // normal should be the unit vector rotated -90deg around z axis
            normalArray->push_back(osg::Vec3(-unit.y(), unit.x(), unit.z()));
            faceArray->push_back(FACE_CONE);
            vertexMetaData->push_back(SVMeta(USAGE_NONE, metafoff.anglex_, metafoff.anglez_, unit, w));

            if (drawFaces)
              strip->setElement(2 * q + i, vertexArray->size() - 1);
          }
        }
        if (drawFaces)
        {
          faceGeom->addPrimitiveSet(strip);
        }
      }
    }

    // if either assert fails, vertex counts in cone no longer match expected/reserved count; vector reserve calls must be updated to match changes to cone vertex generation
    assert(vertexArray->size() == reserveSizeFace + reserveSizeCone);
    assert(vertexMetaData->size() == reserveSizeFace + reserveSizeCone);

    // next, render geometry for the wall outlines
    if (drawOutlines)
    {
      osg::ref_ptr<osg::DrawElementsUShort> outline = new osg::DrawElementsUShort(GL_LINES, 8);
      outline->setElement(0, nearIndexLL);
      outline->setElement(1, farIndexLL);
      outline->setElement(2, nearIndexUL);
      outline->setElement(3, farIndexUL);
      outline->setElement(4, nearIndexLR);
      outline->setElement(5, farIndexLR);
      outline->setElement(6, nearIndexUR);
      outline->setElement(7, farIndexUR);
      outlineGeom->addPrimitiveSet(outline);
    }
  }
}

osg::Geometry* SVFactory::createCone_(const SVData& d, const osg::Vec3& direction)
{
  osg::Geometry* geom = new osg::Geometry();
  geom->setUseVertexBufferObjects(true);
  geom->setUseDisplayList(false);
  geom->setDataVariance(osg::Object::DYNAMIC); // prevent draw/update overlap

  // the number of angular slices into which to tessellate the ellipsoid.
  const unsigned int numSlices = osg::clampBetween(d.coneRes_, static_cast<unsigned int>(4), static_cast<unsigned int>(40));
  const double sliceAngle_rad = 2.0 * M_PI / numSlices;

  // ellipse parameters:
  const double h = 1.0;
  const double w = 1.0 * (d.hfov_deg_ / d.vfov_deg_);

  // the number of concentric rings forming the facade
  const unsigned int numRings = osg::clampBetween(d.capRes_, static_cast<unsigned int>(1), static_cast<unsigned int>(10));
  const double ringSpanX = 0.5 * osg::DegreesToRadians(d.hfov_deg_) / numRings;
  const double ringSpanZ = 0.5 * osg::DegreesToRadians(d.vfov_deg_) / numRings;

  const bool hasNear = d.nearRange_ > 0.0 && d.drawCone_;
  unsigned short nearOffset = 0;
  unsigned short farOffset = 0;

  const double nearRange = d.nearRange_ * d.scale_;
  const double farRange = d.farRange_  * d.scale_;

  const unsigned int vertsPerSlice = numRings; // not including the center point
  const unsigned int vertsPerFace = (vertsPerSlice * numSlices) + 1; // +1 for the center point
  const unsigned int vertsOnWall = numSlices * (d.wallRes_ + 1) * 2;
  const unsigned int numVerts = hasNear ?
    (2 * vertsPerFace) + vertsOnWall :
    vertsPerFace + vertsOnWall;

  // create the vertices
  osg::Vec3Array* v = new osg::Vec3Array(numVerts);
  geom->setVertexArray(v);

  // and the color array
  osg::Vec4Array* c = new osg::Vec4Array(1);
  geom->setColorArray(c);
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);
  (*c)[0] = d.color_;

  // and the normals
  osg::Vec3Array* n = new osg::Vec3Array(numVerts);
  geom->setNormalArray(n);
  geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

  // metadata (for fast updates)
  SVMetaContainer* metaContainer = new SVMetaContainer();
  geom->setUserData(metaContainer);
  std::vector<SVMeta>* m = &metaContainer->vertMeta_;
  m->resize(numVerts);
  metaContainer->nearRange_ = d.nearRange_ * d.scale_;
  metaContainer->farRange_  = d.farRange_ * d.scale_;

  // face identifiers
  osg::IntArray* f = new osg::IntArray(numVerts);
  geom->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, f);
  geom->setVertexAttribBinding(osg::Drawable::ATTRIBUTE_6, osg::Geometry::BIND_PER_VERTEX);
  geom->setVertexAttribNormalize(osg::Drawable::ATTRIBUTE_6, false);

  // quaternion that will "point" the volume along our direction vector
  osg::Quat dirQ;
  dirQ.makeRotate(osg::Vec3(0.0f, 1.0f, 0.0f), direction);
  metaContainer->dirQ_ = dirQ;

  unsigned int vptr = 0;
  farOffset = 0;
  nearOffset = farOffset + vertsPerFace;

  // first point in each face is the center point.
  (*v)[vptr] = dirQ * osg::Vec3(0.0f, farRange, 0.0f);
  (*n)[vptr] = dirQ * osg::Vec3(0.0f, 1.0f, 0.0f);
  (*f)[vptr] = FACE_FAR;
  (*m)[vptr] = SVMeta(USAGE_FAR, 0.0f, 0.0f, osg::Vec3(0.0f, 1.0f, 0.0f), 1.0f);

  if (hasNear)
  {
    (*v)[vptr + vertsPerFace] = dirQ * osg::Vec3(0.0f, nearRange, 0.0f);
    (*n)[vptr + vertsPerFace] = dirQ * osg::Vec3(0.0f, -1.0f, 0.0f);
    (*f)[vptr + vertsPerFace] = FACE_NEAR;
    (*m)[vptr + vertsPerFace] = SVMeta(USAGE_NEAR, 0.0f, 0.0f, osg::Vec3(0.0f, 1.0f, 0.0f), 0.0f);
  }
  vptr++;

  const unsigned int elsPerSlice = 1 + (2 * numRings);

  // loop over the slices and build the vert array (far first, near second if required)
  for (unsigned int slice = 0; slice < numSlices; ++slice)
  {
    // starting and ending angles of the slice.
    // (the PI_2 offset ensures a vertex on the top.)
    const double t = simCore::angFixPI(M_PI_2 + sliceAngle_rad * slice);
    const double x = w * cos(t);
    const double z = h * sin(t);

    // calculate the local point on the "unit" face ellipse:
    osg::Vec3 unit = osg::Vec3(x, 0.0f, z);
    unit.normalize();

    // a triangle strip for the slice. each always starts as the center point.
    osg::ref_ptr<osg::DrawElementsUShort> farWedge = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
    farWedge->reserveElements(elsPerSlice);
    farWedge->push_back(farOffset); // start with the center point

    osg::ref_ptr<osg::DrawElementsUShort> nearWedge = NULL;
    if (hasNear)
    {
      nearWedge = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
      nearWedge->reserveElements(elsPerSlice);
      nearWedge->push_back(nearOffset); // start with the center point
    }

    for (unsigned int ring = 0; ring < numRings; ++ring)
    {
      const double rx = ringSpanX * (ring + 1) * unit.x();
      const double rz = ringSpanZ * (ring + 1) * unit.z();
      osg::Vec3 rawUnitVec(sin(rx), cos(rx), sin(rz));
      rawUnitVec.normalize();
      const osg::Vec3 unitVec = dirQ * rawUnitVec;

      const osg::Vec3 farVec = unitVec * farRange;

      (*v)[vptr] = farVec;
      (*n)[vptr] = unitVec;
      (*f)[vptr] = FACE_FAR;
      (*m)[vptr].set(USAGE_FAR, rx, rz, rawUnitVec, 1.0f);

      // add the new point to the slice's geometry:
      farWedge->push_back((slice + 1 < numSlices) ? vptr + numRings : farOffset + 1 + ring);
      farWedge->push_back(vptr);

      // do the same for the near face.
      if (hasNear)
      {
        const osg::Vec3 nearVec = unitVec * nearRange;

        (*v)[vptr+vertsPerFace] = nearVec;
        (*n)[vptr+vertsPerFace] = -unitVec;
        (*f)[vptr+vertsPerFace] = FACE_NEAR;
        (*m)[vptr+vertsPerFace].set(USAGE_NEAR, rx, rz, rawUnitVec, 0.0f);

        nearWedge->push_back(vptr + vertsPerFace);
        nearWedge->push_back(slice+1 < numSlices ? (vptr+vertsPerFace)+numRings : nearOffset+1+ring);
      }

      vptr++;
    }

    // add each face to the geometry
    // if assert fails, check that elsPerSlice still represents the number of vertices that are added
    assert(farWedge.get()->size() == elsPerSlice);
    geom->addPrimitiveSet(farWedge);
    if (nearWedge)
      geom->addPrimitiveSet(nearWedge);
  }

  if (d.drawCone_)
  {
    // next, build the walls. we need two additional outer rings with out-facing normals.
    // yes this can be computed while we are building the faces but that is an optimization
    // for later.
    if (hasNear)
      vptr += vertsPerFace;

    int wallOffset = vptr;
    bool evenSlice = true;

    // iterate for triangle strip slices that start at tip of cone and extend to far end(base) of cone
    for (unsigned int slice = 0; slice < numSlices; ++slice)
    {
      osg::Vec3 unit[2], rawUnitVec[2], unitVec[2], nearVec[2], lengthVec[2];
      double rx[2], rz[2];

      // start at bottom of cone and alternately build strips on either side ascending, to manage draw order
      // this approach fixes obvious artifacts when beam is viewed from above (SDK-54) but may display artifacts when cone is viewed from side or from below,
      // or more obviously if roll offset is applied
      double sliceAngle;
      if (evenSlice)
        sliceAngle = -M_PI_2 + sliceAngle_rad * 0.5 * slice;
      else
        sliceAngle = -M_PI_2 - sliceAngle_rad * 0.5 * (slice + 1);
      evenSlice = !evenSlice;

      // build a triangle strip for the slice

      // precalculate
      for (unsigned int i = 0; i < 2; ++i)
      {
        // starting and ending angles of the slice, in order to set winding correctly
        const double t = (i == 0) ? simCore::angFixPI(sliceAngle + sliceAngle_rad) : simCore::angFixPI(sliceAngle);
        const float x = static_cast<float>(w * cos(t));
        const float z = static_cast<float>(h * sin(t));

        // the unit face vector will form the vertex normal:
        unit[i].set(x, 0, z);
        unit[i].normalize();

        // these are the offset factors for the actual face size:
        rx[i] = ringSpanX * numRings * unit[i].x();
        rz[i] = ringSpanZ * numRings * unit[i].z();
        rawUnitVec[i].set(sin(rx[i]), cos(rx[i]), sin(rz[i]));
        rawUnitVec[i].normalize();
        unitVec[i] = dirQ * rawUnitVec[i];

        // the point on the near face (or at the origin if there's no near face)
        nearVec[i].set(0.0f, 0.0f, 0.0f);
        if (hasNear)
          nearVec[i] = unitVec[i] * nearRange;

        lengthVec[i] = (unitVec[i] * farRange) - nearVec[i];
      }

      osg::ref_ptr<osg::DrawElementsUShort> side = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
      side->reserveElements(2 * (d.wallRes_ + 1));

      const float tessStep = 1.0f / d.wallRes_;
      for (unsigned int q = 0; q < d.wallRes_ + 1; ++q)
      {
        const float w = tessStep * q;
        for (unsigned int i = 0; i < 2; ++i)
        {
          (*v)[vptr] = nearVec[i] + (lengthVec[i] * w);
          (*n)[vptr] = unit[i]; // TODO: SDK-55 this is the unit vector from beam axis to radius, which will not be normal to the cone surface (we are not drawing a cylinder)
          (*f)[vptr] = FACE_CONE;
          if (w == 0.0f)
            (*m)[vptr].set(USAGE_NEAR, rx[i], rz[i], rawUnitVec[i], w);
          else if (w == 1.0f)
            (*m)[vptr].set(USAGE_FAR, rx[i], rz[i], rawUnitVec[i], w);
          else
            (*m)[vptr].set(USAGE_NONE, rx[i], rz[i], rawUnitVec[i], w);

          side->addElement(vptr);
          vptr++;
        }
      }
      geom->addPrimitiveSet(side);
    }

    // asserting that we used all the vertices we expected to
    // if assert fails, check numVerts calculation
    assert(numVerts == vptr);

    // highlights the face points for a visual effect:
    if (SVData::DRAW_MODE_POINTS & d.drawMode_)
    {
      geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, wallOffset));
      geom->getOrCreateStateSet()->setAttributeAndModes(new osg::Point(3.0), 1);
    }
  }

  // finally, configure the stateset
  geom->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(d.outlineWidth_), osg::StateAttribute::ON);
  return geom;
}

osg::MatrixTransform* SVFactory::createNode(const SVData& d, const osg::Vec3& dir)
{
  osg::ref_ptr<osg::Geode> geodeSolid = new osg::Geode();

  if (d.shape_ == SVData::SHAPE_PYRAMID || d.hfov_deg_ > 90.0 || d.vfov_deg_ > 90.0)
  {
    // pyramid always adds a solid geometry, can also add an outline geometry
    createPyramid_(*geodeSolid, d, dir);
    if (geodeSolid->getNumDrawables() < 1)
    {
      // assertion failure means that createPyramid_ changed and no longer guarantees to return a geode with geometry
      assert(0);
      return NULL;
    }
  }
  else
  {
    osg::Geometry* geom = createCone_(d, dir);
    if (geom == NULL)
    {
      // Assertion failure means create*_() did not return a valid geometry
      assert(0);
      return NULL;
    }
    geodeSolid->addDrawable(geom);
  }

  osg::MatrixTransform* xform = new osg::MatrixTransform();
  xform->addChild(geodeSolid);

  // Turn off backface culling
  xform->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

  // apply wireframe mode if necessary
  if (SVData::DRAW_MODE_WIRE & d.drawMode_)
  {
    osg::Geometry* geom = geodeSolid->getDrawable(0)->asGeometry();
    if ((SVData::DRAW_MODE_SOLID & d.drawMode_) || (SVData::DRAW_MODE_STIPPLE & d.drawMode_))
    {
      // include a second geode that will re-draw the item in wireframe mode.
      osg::Geode* geodeWire = new osg::Geode();
      geodeWire->addDrawable(geom);
      xform->addChild(geodeWire);
      osg::PolygonMode* pm = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
      geodeWire->getOrCreateStateSet()->setAttributeAndModes(pm, osg::StateAttribute::ON);
    }
    else
    {
      osg::PolygonMode* pm = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
      geom->getOrCreateStateSet()->setAttributeAndModes(pm, osg::StateAttribute::ON);
    }
  }

  updateLighting(xform, d.lightingEnabled_);
  updateBlending(xform, d.blendingEnabled_);
  updateStippling(xform, ((SVData::DRAW_MODE_STIPPLE & d.drawMode_) == SVData::DRAW_MODE_STIPPLE));

  return xform;
}

void SVFactory::updateStippling(osg::MatrixTransform* xform, bool stippling)
{
  osg::Drawable* geom = SVFactory::solidGeometry_(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL)
    return;
  osg::StateSet* stateSet = geom->getOrCreateStateSet();

  if (stippling)
  {
    osg::PolygonStipple* ps = new osg::PolygonStipple(gPatternMask1);
    stateSet->setAttributeAndModes(ps, osg::StateAttribute::ON);
  }
  else
  {
    stateSet->removeAttribute(osg::StateAttribute::POLYGONSTIPPLE);
  }
}

void SVFactory::updateLighting(osg::MatrixTransform* xform, bool lighting)
{
  // lighting is only applied to the solid geometry
  osg::Drawable* geom = SVFactory::solidGeometry_(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL)
    return;

  osg::StateSet* stateSet = geom->getOrCreateStateSet();
  simVis::setLighting(stateSet, lighting ?
    osg::StateAttribute::ON  | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE :
    osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
}

void SVFactory::updateBlending(osg::MatrixTransform* xform, bool blending)
{
  // blending is only applied to the solid geometry
  osg::Drawable* geom = SVFactory::solidGeometry_(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL)
    return;

  osg::StateSet* stateSet = geom->getOrCreateStateSet();
  stateSet->setMode(GL_BLEND, blending ?
    osg::StateAttribute::ON :
  osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
}

void SVFactory::updateColor(osg::MatrixTransform* xform, const osg::Vec4f& color)
{
  osg::Geometry* geom = SVFactory::solidGeometry_(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL)
    return;
  osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
  if (colors)
  {
    const size_t colorsSize = colors->size();
    // check that all geometries use BIND_OVERALL, and color arrays are fixed at size 1
    assert(colorsSize == 1);
#ifdef DEBUG
    OE_INFO << "update color, size = " << colorsSize << std::endl;
#endif
    if ((*colors)[0] != color)
    {
      colors->assign(colorsSize, color);
      colors->dirty();
    }
  }

  // if we have an (optional) outline geometry, update its color, remove transparency
  geom = SVFactory::outlineGeometry_(xform);
  if (geom == NULL)
    return;
  colors = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
  if (colors)
  {
    const size_t colorsSize = colors->size();
    // check that all geometries use BIND_OVERALL, and color arrays are fixed at size 1
    assert(colorsSize == 1);
#ifdef DEBUG
    OE_INFO << "update color, size = " << colorsSize << std::endl;
#endif
    if ((*colors)[0][0] != color[0] ||
        (*colors)[0][1] != color[1] ||
        (*colors)[0][2] != color[2])
    {
      colors->assign(colorsSize, color);
      (*colors)[0][3] = 1.0f;
      colors->dirty();
    }
  }
}

void SVFactory::updateNearRange(osg::MatrixTransform* xform, float nearRange)
{
  nearRange = simCore::sdkMax(1.0f, nearRange);

  osg::Geometry*  geom = SVFactory::solidGeometry_(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL)
    return;

  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(verts);
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(meta);
  if (verts == NULL || meta == NULL)
    return;

  std::vector<SVMeta>& m = meta->vertMeta_;
  meta->nearRange_ = nearRange;
  const float range = meta->farRange_ - meta->nearRange_;
  for (unsigned int i = 0; i < verts->size(); ++i)
  {
    const osg::Vec3& unit = m[i].unit_;
    const float      farRatio = m[i].ratio_;
    (*verts)[i] = m[i].unit_*(meta->nearRange_ + range*farRatio);
  }

  verts->dirty();
  geom->dirtyBound();
}

void SVFactory::updateFarRange(osg::MatrixTransform* xform, float farRange)
{
  farRange = simCore::sdkMax(1.0f, farRange);

  osg::Geometry*  geom = SVFactory::solidGeometry_(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL)
    return;

  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(verts);
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(meta);
  if (verts == NULL || meta == NULL)
    return;

  std::vector<SVMeta>& m = meta->vertMeta_;
  meta->farRange_ = farRange;
  const float range = meta->farRange_ - meta->nearRange_;
  for (unsigned int i = 0; i < verts->size(); ++i)
  {
    const osg::Vec3& unit = m[i].unit_;
    const float      farRatio = m[i].ratio_;
    (*verts)[i] = m[i].unit_*(meta->nearRange_ + range*farRatio);
  }

  verts->dirty();
  geom->dirtyBound();
}

void SVFactory::updateHorizAngle(osg::MatrixTransform* xform, float oldAngle, float newAngle)
{
  osg::Geometry* geom = SVFactory::solidGeometry_(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL)
    return;

  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(verts);
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(meta);
  if (verts == NULL || meta == NULL)
    return;
  std::vector<SVMeta>& vertMeta = meta->vertMeta_;

  // clamp to match createPyramid_
  oldAngle = osg::clampBetween(oldAngle, static_cast<float>(0.01f * simCore::DEG2RAD), static_cast<float>(M_TWOPI));
  newAngle = osg::clampBetween(newAngle, static_cast<float>(0.01f * simCore::DEG2RAD), static_cast<float>(M_TWOPI));
  const float oldMinAngle = -oldAngle*0.5f;
  const float newMinAngle = -newAngle*0.5f;
  for (unsigned int i = 0; i < verts->size(); ++i)
  {
    SVMeta& m = vertMeta[i];
    // exclude centroid verts
    if (m.unit_.x() != 0.0f || m.unit_.z() != 0.0f)
    {
      const float t = (m.anglex_ - oldMinAngle) / oldAngle;
      const float ax = newMinAngle + t*newAngle;
      const float sinx = sin(ax);
      const float cosx = cos(ax);
      const float sinz = sin(m.anglez_);
      const float cosz = cos(m.anglez_);
      const float range =
        m.usage_ == USAGE_NEAR ? meta->nearRange_ :
        m.usage_ == USAGE_FAR  ? meta->farRange_  :
        (*verts)[i].length();

      m.anglex_ = ax;
      m.unit_.set(sinx*cosz, cosx*cosz, sinz);
      m.unit_.normalize();
      (*verts)[i] = meta->dirQ_ * m.unit_ * range;

      // wrong for sides.. // (*normals)[i] = m.usage_ == USAGE_NEAR ? -m.unit_*range : m.unit_*range;
    }
  }

  verts->dirty();
  geom->dirtyBound();
}

void SVFactory::updateVertAngle(osg::MatrixTransform* xform, float oldAngle, float newAngle)
{
  osg::Geometry*  geom = SVFactory::solidGeometry_(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL)
    return;

  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(verts);
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(meta);
  if (verts == NULL || meta == NULL)
    return;
  std::vector<SVMeta>& vertMeta = meta->vertMeta_;

  // clamp to match createPyramid_
  oldAngle = osg::clampBetween(oldAngle, static_cast<float>(0.01f * simCore::DEG2RAD), static_cast<float>(M_PI));
  newAngle = osg::clampBetween(newAngle, static_cast<float>(0.01f * simCore::DEG2RAD), static_cast<float>(M_PI));
  const float oldMinAngle = -oldAngle*0.5f;
  const float newMinAngle = -newAngle*0.5f;
  for (unsigned int i = 0; i < verts->size(); ++i)
  {
    SVMeta& m = vertMeta[i];
    // exclude centroid verts
    if (m.unit_.x() != 0.0f || m.unit_.z() != 0.0f)
    {
      const float t = (m.anglez_ - oldMinAngle) / oldAngle;
      const float az = newMinAngle + t*newAngle;
      const float sinx = sin(m.anglex_);
      const float cosx = cos(m.anglex_);
      const float sinz = sin(az);
      const float cosz = cos(az);
      const float range =
        m.usage_ == USAGE_NEAR ? meta->nearRange_ :
        m.usage_ == USAGE_FAR  ? meta->farRange_  :
        (*verts)[i].length();

      m.anglez_ = az;
      m.unit_.set(sinx*cosz, cosx*cosz, sinz);
      m.unit_.normalize();
      (*verts)[i] = meta->dirQ_ * m.unit_ * range;

      // wrong for sides.. // (*normals)[i] = m.usage_ == USAGE_NEAR ? -m.unit_*range : m.unit_*range;
    }
  }

  verts->dirty();
  geom->dirtyBound();
}

osg::Geometry* SVFactory::solidGeometry_(osg::MatrixTransform* xform)
{
  if (xform == NULL || xform->getNumChildren() == 0)
    return NULL;
  osg::Geode* geode = xform->getChild(0)->asGeode();
  if (geode == NULL || geode->getNumDrawables() == 0)
    return NULL;
  return geode->getDrawable(0)->asGeometry();
}

// if the sv pyramid has an outline, it will exist in its own geometry, which should always be the 2nd geometry
osg::Geometry* SVFactory::outlineGeometry_(osg::MatrixTransform* xform)
{
  if (xform == NULL || xform->getNumChildren() == 0)
    return NULL;
  osg::Geode* geode = xform->getChild(0)->asGeode();
  if (geode == NULL || geode->getNumDrawables() < 2)
    return NULL;
  return geode->getDrawable(1)->asGeometry();
}

