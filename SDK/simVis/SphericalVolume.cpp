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
#include "osg/PolygonMode"
#include "osg/UserDataContainer"
#include "osgUtil/Simplifier"

#include "simVis/LineDrawable.h"

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simVis/Constants.h"
#include "simVis/PointSize.h"
#include "simVis/PolygonStipple.h"
#include "simVis/SphericalVolume.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"

using namespace simVis;

//---------------------------------------------------------------------------

#define USAGE_CONE     0x00
#define USAGE_NEAR     0x01
#define USAGE_FAR      0x02
#define USAGE_TOP      0x04
#define USAGE_BOTTOM   0x08
#define USAGE_LEFT     0x10
#define USAGE_RIGHT    0x20

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
  // we always add a solid/face geometry, even if we add no primitives b/c we are drawing outline only; and faceGeom must be the first geometry in the geode
  osg::Geometry* faceGeom = new osg::Geometry();
  faceGeom->setName("simVis::SphericalVolume::pyramid");
  geode.addDrawable(faceGeom);

  if (d.drawMode_ == SVData::DRAW_MODE_NONE || d.capRes_ == 0)
  {
    // early out; at present, geode must have a non-NULL geometry, even if empty
    return;
  }

  const float nearRange = d.nearRange_ * d.scale_;
  const float farRange = d.farRange_  * d.scale_;

  // quaternion that will "point" the volume along our direction vector
  osg::Quat dirQ;
  dirQ.makeRotate(osg::Vec3(0.0f, 1.0f, 0.0f), direction);

  osg::Vec3Array* vertexArray = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
  osg::Vec3Array* normalArray = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);

  SVMetaContainer* metaContainer = new SVMetaContainer();
  metaContainer->dirQ_ = dirQ;
  metaContainer->nearRange_ = nearRange;
  metaContainer->farRange_ = farRange;
  std::vector<SVMeta>* vertexMetaData = &metaContainer->vertMeta_;

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
  // vertices will be added in this order: gate origin, far face, near face (if drawn), cone bottom, then cone right (if drawn), cone top (if drawn), cone left (if drawn)
  const unsigned int reserveSizeFace = 1 + (loop * numPointsX * numPointsZ);
  unsigned int reserveSizeCone = 0;
  const bool drawFaces = (d.drawMode_ != SVData::DRAW_MODE_OUTLINE);
  if (drawFaces && drawCone)
  {
    // bottom & top faces are only drawn if vfov_deg < 180
    if (vfov_deg < 180.0f) // 2 faces * (2 * (numPointsX - 1) * (1 + d.wallRes_)) vertices/face
      reserveSizeCone += (numPointsX - 1) * (1 + d.wallRes_) * 2 * 2;
    // right & left faces only drawn if hfov_deg < 360
    if (hfov_deg < 360.0f) // 2 faces * (2 * (numPointsZ - 1) * (1 + d.wallRes_)) vertices/face
      reserveSizeCone += (numPointsZ - 1) * (1 + d.wallRes_) * 2 * 2;
  }
  vertexArray->reserve(reserveSizeFace + reserveSizeCone);
  normalArray->reserve(reserveSizeFace + reserveSizeCone);
  vertexMetaData->reserve(reserveSizeFace + reserveSizeCone);

  // add a vertex at gate origin, to support outline drawing to origin when minrange is 0
  // only need this point if drawing outline (with or without fillpattern) and there is no near face b/c minrange is zero.
  // but adding it in every case to make code simpler
  {
    vertexArray->push_back(osg::Vec3());
    normalArray->push_back(osg::Vec3());
    vertexMetaData->push_back(SVMeta(USAGE_NEAR, 0.f, 0.f, osg::Vec3(), 0.0f));
  }

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
        vertexMetaData->push_back(SVMeta(i == 0 ? USAGE_FAR : USAGE_NEAR, angleX_rad, angleZ_rad, unitUnrot, i == 0 ? 1.0f : 0.0f));
      }
    }
  }
  // if either assert fails, vertex counts in face no longer match expected/reserved count; vector reserve calls must be updated to match changes to face vertex generation
  assert(vertexArray->size() == reserveSizeFace);
  assert(vertexMetaData->size() == reserveSizeFace);

  const unsigned short farFaceOffset = 1;
  const unsigned short nearFaceOffset = hasNear ? farFaceOffset + (numPointsX * numPointsZ) : 0;

  // render geometry for the face outlines
  const bool drawOutlines = (SVData::DRAW_MODE_OUTLINE & d.drawMode_) == SVData::DRAW_MODE_OUTLINE;
  if (drawOutlines)
  {
    osg::Geometry* outlineGeom = new osg::Geometry();
    outlineGeom->setName("simVis::SphericalVolume");
    geode.addDrawable(outlineGeom);
    outlineGeom->setUseVertexBufferObjects(true);
    outlineGeom->setUseDisplayList(false);
    outlineGeom->setDataVariance(osg::Object::DYNAMIC); // prevent draw/update overlap

    osg::Vec4Array* outlineColor = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    (*outlineColor)[0] = d.color_;
    (*outlineColor)[0][3] = 1.0f; // no transparency in the outline
    outlineGeom->setColorArray(outlineColor);
    outlineGeom->setVertexArray(vertexArray);
    outlineGeom->setUserData(metaContainer);
    outlineGeom->setNormalArray(normalArray);

    // horizontals of the gate face outline
    for (unsigned int z = 0; z < numPointsZ; z += (numPointsZ - 1)) // iterate twice, first for the bottom, 2nd for the top
    {
      // if we are drawing near and far faces, bottom and top outlines are each line loops, if not, far face outlines are each line strips
      osg::ref_ptr<osg::DrawElementsUShort> outline;
      if (hasNear)
      {
        // outline is a loop connecting near and far face
        outline = new osg::DrawElementsUShort(GL_LINE_LOOP, 2 * numPointsX);
      }
      else if (drawCone)
      {
        // outline is loop connecting gate origin and far face
        outline = new osg::DrawElementsUShort(GL_LINE_LOOP, numPointsX + 1);
      }
      else
      {
        // outline is the far face
        outline = new osg::DrawElementsUShort(GL_LINE_STRIP, numPointsX);
      }

      // iterate across the gate horizontals (x) from left to right (if you look from gate origin)
      for (unsigned int x = 0; x < numPointsX; ++x)
      {
        outline->setElement(x, farFaceOffset + x*numPointsZ + z);
        if (hasNear)
          outline->setElement((2 * numPointsX) - x - 1, nearFaceOffset + x*numPointsZ + z);
      }
      if (drawCone && !hasNear)
      {
        // there is no near face, add index to origin/zero point
        outline->setElement(numPointsX, 0);
      }
      outlineGeom->addPrimitiveSet(outline.get());
    }

    // verticals of the gate face outline
    for (unsigned int x = 0; x < numPointsX; x += (numPointsX - 1)) // iterate twice, first for the left, 2nd for the right (if you look from gate origin)
    {
      osg::ref_ptr<osg::DrawElementsUShort> farOutline = new osg::DrawElementsUShort(GL_LINE_STRIP, numPointsZ);
      osg::ref_ptr<osg::DrawElementsUShort> nearOutline = (hasNear) ? new osg::DrawElementsUShort(GL_LINE_STRIP, numPointsZ) : NULL;

      // this is the index offset for the bottom of either face at the current x
      const unsigned int xOffset = x * numPointsZ;
      for (unsigned int z = 0; z < numPointsZ; ++z)
      {
        farOutline->setElement(z, farFaceOffset + xOffset + z);
        if (hasNear)
          nearOutline->setElement(z, nearFaceOffset + xOffset + z);
      }
      // assertion fail indicates that algorithm for outline generation has changed, check that reserve matches actual usage
      assert(farOutline->size() == numPointsZ);
      outlineGeom->addPrimitiveSet(farOutline.get());
      if (hasNear)
      {
        assert(nearOutline->size() == numPointsZ);
        outlineGeom->addPrimitiveSet(nearOutline.get());
      }
    }
  }


  if (!drawFaces) // if drawing outline-only, we're done
    return;

  // set up the face geometry
  faceGeom->setUseVertexBufferObjects(true);
  faceGeom->setUseDisplayList(false);
  faceGeom->setDataVariance(osg::Object::DYNAMIC); // prevent draw/update overlap

  osg::Vec4Array* colorArray = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
  (*colorArray)[0] = d.color_;
  faceGeom->setColorArray(colorArray);
  faceGeom->setVertexArray(vertexArray);
  faceGeom->setUserData(metaContainer);
  faceGeom->setNormalArray(normalArray);

  // if we are drawing the face (not just the outline) add primitives that index into the vertex array
  {
    const unsigned int numFaceElements = 2 * numPointsZ;

    // draw far face with vertical triangle strip(s) for each (x, x+1) pair
    for (unsigned int x = 0; x < numPointsX - 1; ++x)
    {
      osg::ref_ptr<osg::DrawElementsUShort> farFaceStrip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numFaceElements);

      // these are index offsets for the bottom of the face at the current x
      const unsigned int leftX = x * numPointsZ;
      const unsigned int rightX = (x + 1) * numPointsZ;
      for (unsigned int z = 0; z < numPointsZ; ++z)
      {
        const unsigned int elementIndex = 2 * z;
        farFaceStrip->setElement(elementIndex, farFaceOffset + rightX + z);
        farFaceStrip->setElement(elementIndex + 1, farFaceOffset + leftX + z);
      }
      faceGeom->addPrimitiveSet(farFaceStrip.get());
    }

    // the near face is drawn separately to mitigate near/far face artifacts
    if (hasNear)
    {
      // draw vertical triangle strip(s) for each (x, x+1) pair
      for (unsigned int x = 0; x < numPointsX - 1; ++x)
      {
        osg::ref_ptr<osg::DrawElementsUShort> nearFaceStrip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numFaceElements);

        // these are index offsets for the bottom of the face at the current x
        const unsigned int leftX = x * numPointsZ;
        const unsigned int rightX = (x + 1) * numPointsZ;

        for (unsigned int z = 0; z < numPointsZ; ++z)
        {
          const unsigned int elementIndex = 2 * z;
          nearFaceStrip->setElement(elementIndex, nearFaceOffset + leftX + z);
          nearFaceStrip->setElement(elementIndex + 1, nearFaceOffset + rightX + z);
        }
        faceGeom->addPrimitiveSet(nearFaceStrip.get());
      }
    }
  }


  if (!drawCone) // if not drawing the walls of the pyramid shape, we're done
    return;

  // if the near face range is <= 0 (hasNear = false), then there is no near face, walls go to gate origin
  // build vertex sets for the walls. we have to duplicate verts in order to get unique normals, unfortunately.

  const float tessStep = 1.0f / d.wallRes_;
  const float coneLen = farRange - nearRange;
  const unsigned int numWallElements = (1 + d.wallRes_) * 2;

  // bottom:
  if (vfov_deg < 180.0f)
  {
    // draw the bottom wall outline and face, drawn as triangle strips from the near face to the far face;
    // iterate x across the face from right to left, (looking from near face to far face)
    for (unsigned int x = numPointsX - 1; x > 0; --x)
    {
      // starting index for near and far face vertices for right edge of strip starting at x
      const unsigned int offsetStart = x * numPointsZ;

      osg::ref_ptr<osg::DrawElementsUShort> strip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numWallElements);
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
          vertexMetaData->push_back(SVMeta(USAGE_BOTTOM, metafoff.anglex_, metafoff.anglez_, unit, w));

          strip->setElement(2 * q + i, vertexArray->size() - 1);
        }
      }
      faceGeom->addPrimitiveSet(strip.get());
    }
  }

  // right:
  if (hfov_deg < 360.0f)
  {
    // draw the right wall outline and face, drawn as triangle strips from the near face to the far face;
    // iterate z across the face from top to bottom, (looking from near face to far face)
    for (unsigned int z = numPointsZ - 1; z > 0; --z)
    {
      // starting index for near and far face vertices for the top edge of the strip starting at z
      const unsigned int offsetStart = numPointsZ * (numPointsX - 1) + z;

      osg::ref_ptr<osg::DrawElementsUShort> strip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numWallElements);

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
          vertexMetaData->push_back(SVMeta(USAGE_RIGHT, metafoff.anglex_, metafoff.anglez_, unit, w));

          strip->setElement(2 * q + i, vertexArray->size() - 1);
        }
      }
      faceGeom->addPrimitiveSet(strip.get());
    }
  }

  // top:
  if (vfov_deg < 180.0f)
  {
    // draw the top wall outline and face, drawn as triangle strips from the near face to the far face;
    // iterate x across the face from left to right, (looking from near face to far face)
    for (unsigned int x = 0; x < numPointsX - 1; ++x)
    {
      // starting index for near and far face vertices for left edge of the strip starting at x
      const unsigned int offsetStart = (x * numPointsZ) + (numPointsZ - 1);

      osg::ref_ptr<osg::DrawElementsUShort> strip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numWallElements);

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
          vertexMetaData->push_back(SVMeta(USAGE_TOP, metafoff.anglex_, metafoff.anglez_, unit, w));

          strip->setElement(2 * q + i, vertexArray->size() - 1);
        }
      }
      faceGeom->addPrimitiveSet(strip.get());
    }
  }

  // left:
  if (hfov_deg < 360.0f)
  {
    // draw the left wall outline and face, drawn as triangle strips from the near face to the far face;
    // iterate z across the face from bottom to top, (looking from near face to far face)
    for (unsigned int z = 0; z < numPointsZ - 1; ++z)
    {
      osg::ref_ptr<osg::DrawElementsUShort> strip = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP, numWallElements);

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
          vertexMetaData->push_back(SVMeta(USAGE_LEFT, metafoff.anglex_, metafoff.anglez_, unit, w));

          strip->setElement(2 * q + i, vertexArray->size() - 1);
        }
      }
      faceGeom->addPrimitiveSet(strip.get());
    }

    // if either assert fails, vertex counts in cone no longer match expected/reserved count; vector reserve calls must be updated to match changes to cone vertex generation
    assert(vertexArray->size() == reserveSizeFace + reserveSizeCone);
    assert(vertexMetaData->size() == reserveSizeFace + reserveSizeCone);
  }
}

osg::Geometry* SVFactory::createCone_(const SVData& d, const osg::Vec3& direction)
{
  osg::Geometry* geom = new osg::Geometry();
  geom->setName("simVis::SphericalVolume::cone");
  geom->setUseVertexBufferObjects(true);
  geom->setUseDisplayList(false);
  geom->setDataVariance(osg::Object::DYNAMIC); // prevent draw/update overlap

  // the number of angular slices into which to tessellate the ellipsoid.
  const unsigned int numSlices = osg::clampBetween(d.coneRes_, 4u, 40u);
  const double sliceAngle_rad = M_TWOPI / numSlices;

  // the number of concentric rings forming the facade
  const unsigned int numRings = osg::clampBetween(d.capRes_, 1u, 10u);
  const float hfov_deg = osg::clampBetween(d.hfov_deg_, 0.01f, 360.0f);
  const float vfov_deg = osg::clampBetween(d.vfov_deg_, 0.01f, 180.0f);
  const double ringSpanX = 0.5 * osg::DegreesToRadians(hfov_deg) / numRings;
  const double ringSpanZ = 0.5 * osg::DegreesToRadians(vfov_deg) / numRings;

  const bool hasNear = d.nearRange_ > 0.0 && d.drawCone_;

  const double nearRange = d.nearRange_ * d.scale_;
  const double farRange = d.farRange_  * d.scale_;

  const unsigned int vertsPerSlice = numRings; // not including the center point
  const unsigned int vertsPerFace = (vertsPerSlice * numSlices) + 1; // +1 for the center point
  const unsigned int vertsOnWall = numSlices * (d.wallRes_ + 1) * 2;
  const unsigned int numVerts = hasNear ?
    (2 * vertsPerFace) + vertsOnWall :
    vertsPerFace + vertsOnWall;

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

  // metadata (for fast updates)
  SVMetaContainer* metaContainer = new SVMetaContainer();
  geom->setUserData(metaContainer);
  std::vector<SVMeta>* m = &metaContainer->vertMeta_;
  m->resize(numVerts);
  metaContainer->nearRange_ = d.nearRange_ * d.scale_;
  metaContainer->farRange_  = d.farRange_ * d.scale_;

  // quaternion that will "point" the volume along our direction vector
  osg::Quat dirQ;
  dirQ.makeRotate(osg::Vec3(0.0f, 1.0f, 0.0f), direction);
  metaContainer->dirQ_ = dirQ;

  // vertices for far face start at beginning of vertex array
  const unsigned short farOffset = 0;
  // vertices for near face start immediately after the far face vertices
  const unsigned short nearOffset = farOffset + vertsPerFace;

  // near and far faces are built with triangle strip radial slices using two vertices per concentric ring
  unsigned int vptr = 0;
  // first point in each strip  is the center point.
  (*v)[vptr] = dirQ * osg::Vec3(0.0f, farRange, 0.0f);
  (*n)[vptr] = dirQ * osg::Vec3(0.0f, 1.0f, 0.0f);
  (*m)[vptr] = SVMeta(USAGE_FAR, 0.0f, 0.0f, osg::Vec3(0.0f, 1.0f, 0.0f), 1.0f);
  if (hasNear)
  {
    // first point in strip is the center point.
    (*v)[vptr + vertsPerFace] = dirQ * osg::Vec3(0.0f, nearRange, 0.0f);
    (*n)[vptr + vertsPerFace] = dirQ * osg::Vec3(0.0f, -1.0f, 0.0f);
    (*m)[vptr + vertsPerFace] = SVMeta(USAGE_NEAR, 0.0f, 0.0f, osg::Vec3(0.0f, 1.0f, 0.0f), 0.0f);
  }
  vptr++;

  const unsigned int elsPerSlice = 1 + (2 * numRings);

  // loop over the slices and build the vert array (far first, near second if required)
  for (unsigned int slice = 0; slice < numSlices; ++slice)
  {
    // starting and ending angles of the slice.
    // (the PI_2 offset ensures a vertex on the top.)
    const double phi = simCore::angFixPI(M_PI_2 + sliceAngle_rad * slice);
    const double xRingScale = ringSpanX * cos(phi);
    const double zRingScale = ringSpanZ * sin(phi);

    // a triangle strip for the slice. each always starts as the center point.
    osg::ref_ptr<osg::DrawElementsUShort> farWedge = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
    farWedge->reserveElements(elsPerSlice);
    farWedge->push_back(farOffset); // start with the center point

    for (unsigned int ring = 0; ring < numRings; ++ring)
    {
      const double rx = (ring + 1) * xRingScale;
      const double rz = (ring + 1) * zRingScale;
      osg::Vec3 rawUnitVec(sin(rx)*cos(rz), cos(rx)*cos(rz), sin(rz));
      rawUnitVec.normalize();
      const osg::Vec3 unitVec = dirQ * rawUnitVec;
      const osg::Vec3 farVec = unitVec * farRange;

      (*v)[vptr] = farVec;
      (*n)[vptr] = unitVec;
      (*m)[vptr].set(USAGE_FAR, rx, rz, rawUnitVec, 1.0f);

      // add the new point to the slice's far face geometry:
      // vptr + numRings is the corresponding vertex in the next slice; can't use that when we get to last slice.
      const unsigned int correspondingVertexInNextSlice = (slice + 1 < numSlices) ? (vptr + numRings) : (farOffset + 1 + ring);
      farWedge->push_back(correspondingVertexInNextSlice);
      farWedge->push_back(vptr);

      if (hasNear)
      {
        const osg::Vec3 nearVec = unitVec * nearRange;
        (*v)[vptr + vertsPerFace] = nearVec;
        (*n)[vptr + vertsPerFace] = -unitVec;
        (*m)[vptr + vertsPerFace].set(USAGE_NEAR, rx, rz, rawUnitVec, 0.0f);
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

      for (unsigned int ring = 0; ring < numRings; ++ring)
      {
        nearWedge->push_back(vptr);
        // vptr + numRings is the corresponding vertex in the next slice; can't use that when we get to last slice.
        const unsigned int correspondingVertexInNextSlice = (slice + 1 < numSlices) ? (vptr + numRings) : (nearOffset + 1 + ring);
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
    // next, build the walls. we need two additional outer rings with out-facing normals.
    // yes this can be computed while we are building the faces but that is an optimization for later.
    const int wallOffset = vptr;

    // ensure that cone is aligned to cap, since cap is drawn normally, but cone is drawn in alternating strips from bottom.
    bool evenSlice = ((numSlices % 2) == 0);

    // iterate for triangle strip slices that start at tip of cone and extend to far end(base) of cone
    for (unsigned int slice = 0; slice < numSlices; ++slice)
    {
      osg::Vec3 rawUnitVec[2], unitVec[2], nearVec[2], lengthVec[2];
      double rx[2], rz[2];

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

      // precalculate
      for (unsigned int i = 0; i < 2; ++i)
      {
        // starting and ending angles of the slice, in order to set winding correctly
        const double phi = (i == 0) ? simCore::angFixPI(sliceAngle + sliceAngle_rad) : simCore::angFixPI(sliceAngle);

        // these are the offset factors for the actual face size:
        rx[i] = ringSpanX * numRings * cos(phi);
        rz[i] = ringSpanZ * numRings * sin(phi);
        rawUnitVec[i].set(sin(rx[i])*cos(rz[i]), cos(rx[i])*cos(rz[i]), sin(rz[i]));
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
        // this appears to be duplicating vertices that are shared between slices, could be optimized to reuse vertices from prev or next slice.
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
          (*m)[vptr].set(USAGE_CONE, rx[i], rz[i], rawUnitVec[i], w);
          side->addElement(vptr);
          vptr++;
        }
      }
      geom->addPrimitiveSet(side.get());
    }

    // asserting that we used all the vertices we expected to
    // if assert fails, check numVerts calculation
    assert(numVerts == vptr);

    // highlights the face points for a visual effect:
    if (SVData::DRAW_MODE_POINTS & d.drawMode_)
    {
      geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, wallOffset));
      PointSize::setValues(geom->getOrCreateStateSet(), 3.f, osg::StateAttribute::ON);
    }
  }

  return geom;
}


// A SphericalVolume is a MatrixTransform that parents up to two geodes.
// The first geode always contains the primary geometry, and possibly a second outline geometry (for pyramid sv only).
// That outline geometry is converted to a LineGroup geode, and is then removed.
// The converted LineGroup becomes a 2nd geode in the MatrixTransform.
// For the cone sv, the second geode (if it exists) contains a wireframe (polygon) geometry.

osg::MatrixTransform* SVFactory::createNode(const SVData& d, const osg::Vec3& dir)
{
  osg::ref_ptr<osg::Geode> geodeSolid = new osg::Geode();

  if (d.shape_ == SVData::SHAPE_PYRAMID)
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
      // create a new wireframe geometry as a shallow copy of the solid geometry
      osg::Geometry* wireframeGeom = new osg::Geometry(*geom);
      wireframeGeom->setName("simVis::SphericalVolume::cone-wireframe");

      // but with its own color array
      osg::Vec4Array* wireframeColor = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
      // default to white
      (*wireframeColor)[0] = simVis::Color::White;
      // but use the solid geometry color if it can be found
      osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
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

      // add this to a 2nd geode in the xform: the 2nd geode in the xform is for opaque features
      osg::Geode* geodeWire = new osg::Geode();
      geodeWire->addDrawable(wireframeGeom);
      xform->addChild(geodeWire);

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
      geom->getOrCreateStateSet()->setAttributeAndModes(pm, osg::StateAttribute::ON);
    }
  }

  updateLighting(xform, d.lightingEnabled_);
  updateBlending(xform, d.blendingEnabled_);
  updateStippling(xform, ((SVData::DRAW_MODE_STIPPLE & d.drawMode_) == SVData::DRAW_MODE_STIPPLE));

  // convert line geometries to LineDrawables, and remove old drawables.
  osgEarth::LineGroup* lineGroup = new osgEarth::LineGroup();
  lineGroup->import(geodeSolid.get(), true);
  // Apply the line width to all the items in the line group
  for (unsigned int i = 0; i < lineGroup->getNumChildren(); ++i)
  {
    osgEarth::LineDrawable* line = lineGroup->getLineDrawable(i);
    if (line)
      line->setLineWidth(d.outlineWidth_);
  }
  xform->addChild(lineGroup);

  return xform;
}

void SVFactory::updateStippling(osg::MatrixTransform* xform, bool stippling)
{
  // only the solid geometry can be stippled
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL || geom->empty())
    return;
  simVis::PolygonStipple::setValues(geom->getOrCreateStateSet(), stippling, 0u);
}

void SVFactory::updateLighting(osg::MatrixTransform* xform, bool lighting)
{
  // lighting is only applied to the solid geometry
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL || geom->empty())
    return;

  osg::StateSet* stateSet = geom->getOrCreateStateSet();
  simVis::setLighting(stateSet, lighting ?
    osg::StateAttribute::ON  | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE :
    osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
}

void SVFactory::updateBlending(osg::MatrixTransform* xform, bool blending)
{
  // blending is only applied to the solid geometry
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(geom);
  if (geom == NULL || geom->empty())
    return;
  geom->getOrCreateStateSet()->setMode(GL_BLEND, blending ?
    osg::StateAttribute::ON :
    osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
}

void SVFactory::updateColor(osg::MatrixTransform* xform, const osg::Vec4f& color)
{
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
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

  // if we have an 2nd (optional) geode, it is opaque; update its color, but remove transparency
  osg::Geode* opaqueGeode = SVFactory::opaqueGeode(xform);
  if (opaqueGeode == NULL)
    return;
  osg::Vec4f opaqueColor = color;
  opaqueColor.a() = 1.0;

  // the opaque geode may be a lineGroup containing lineDrawables
  osgEarth::LineGroup* lines = dynamic_cast<osgEarth::LineGroup*>(opaqueGeode);
  if (lines)
  {
    for (unsigned int i = 0; i < lines->getNumChildren(); ++i)
    {
      osgEarth::LineDrawable* line = lines->getLineDrawable(i);
      // line drawable can set to same-as-current color w/o penalty
      if (line)
        line->setColor(opaqueColor);
    }
    return;
  }

  // if the opaque geode is not a linegroup, it may contain a wireframe geometry
  if (opaqueGeode->getNumDrawables() == 1)
  {
    geom = opaqueGeode->getDrawable(0)->asGeometry();
    if (geom == NULL)
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

#if 0
void SVFactory::updateNearRange(osg::MatrixTransform* xform, float nearRange)
{
  nearRange = simCore::sdkMax(1.0f, nearRange);

  osg::Geometry* geom = SVFactory::validGeometry_(xform);
  if (geom == NULL || geom->empty())
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
  if (verts == NULL || meta == NULL)
    return;

  const std::vector<SVMeta>& m = meta->vertMeta_;
  meta->nearRange_ = nearRange;
  const float range = meta->farRange_ - meta->nearRange_;
  for (unsigned int i = 0; i < verts->size(); ++i)
  {
    const float farRatio = m[i].ratio_;
    (*verts)[i] = m[i].unit_ * (meta->nearRange_ + range*farRatio);
  }

  verts->dirty();
  dirtyBound_(xform);
}
#endif
void SVFactory::updateFarRange(osg::MatrixTransform* xform, float farRange)
{
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == NULL)
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  if (geom->empty())
    return;
  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(verts);
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  // Assertion failure means internal consistency error, or caller has inconsistent input
  assert(meta);
  if (verts == NULL || meta == NULL)
    return;

  const std::vector<SVMeta>& m = meta->vertMeta_;
  farRange = simCore::sdkMax(1.0f, farRange);
  meta->farRange_ = farRange;
  const float range = meta->farRange_ - meta->nearRange_;
  for (unsigned int i = 0; i < verts->size(); ++i)
  {
    const float farRatio = m[i].ratio_;
    (*verts)[i] = m[i].unit_ * (meta->nearRange_ + range*farRatio);
  }
  verts->dirty();
  dirtyBound_(xform);
}

void SVFactory::updateHorizAngle(osg::MatrixTransform* xform, float oldAngle, float newAngle)
{
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == NULL)
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  if (geom->empty())
    return;
  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  osg::Vec3Array* normals = static_cast<osg::Vec3Array*>(geom->getNormalArray());
  if (verts == NULL || meta == NULL || normals == NULL)
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  std::vector<SVMeta>& vertMeta = meta->vertMeta_;

  // clamp to M_TWOPI, to match clamping in pyramid and cone
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
      const osg::Vec3 unitRot = meta->dirQ_ * m.unit_;
      (*verts)[i] = unitRot * range;

      switch (m.usage_)
      {
      case USAGE_NEAR:
        (*normals)[i] = (unitRot * -1);
        break;
      case USAGE_FAR:
        (*normals)[i] = unitRot;
        break;
      case USAGE_BOTTOM:
        (*normals)[i] = (osg::Vec3(unitRot.x(), unitRot.z(), -unitRot.y()));
        break;
      case USAGE_TOP:
        (*normals)[i] = (osg::Vec3(unitRot.x(), -unitRot.z(), unitRot.y()));
        break;
      case USAGE_RIGHT:
        (*normals)[i] = (osg::Vec3(unitRot.y(), -unitRot.x(), unitRot.z()));
        break;
      case USAGE_LEFT:
        (*normals)[i] = (osg::Vec3(-unitRot.y(), unitRot.x(), unitRot.z()));
        break;
      case USAGE_CONE:
      {
        osg::Vec3 normal((*verts)[i].x(), (*verts)[i].y() - range, (*verts)[i].z());
        normal.normalize();
        (*normals)[i] = normal;
        break;
      }
      }
    }
  }

  verts->dirty();
  normals->dirty();
  dirtyBound_(xform);
}

void SVFactory::updateVertAngle(osg::MatrixTransform* xform, float oldAngle, float newAngle)
{
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom == NULL)
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  if (geom->empty())
    return;
  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  SVMetaContainer* meta = static_cast<SVMetaContainer*>(geom->getUserData());
  osg::Vec3Array* normals = static_cast<osg::Vec3Array*>(geom->getNormalArray());
  if (verts == NULL || meta == NULL || normals == NULL)
  {
    // Assertion failure means internal consistency error, or caller has inconsistent input
    assert(0);
    return;
  }
  std::vector<SVMeta>& vertMeta = meta->vertMeta_;

  // clamp to M_PI, to match clamping in pyramid and cone
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
      const osg::Vec3 unitRot = meta->dirQ_ * m.unit_;
      (*verts)[i] = unitRot * range;

      switch (m.usage_)
      {
      case USAGE_NEAR:
        (*normals)[i] = (unitRot * -1);
        break;
      case USAGE_FAR:
        (*normals)[i] = unitRot;
        break;
      case USAGE_BOTTOM:
        (*normals)[i] = (osg::Vec3(unitRot.x(), unitRot.z(), -unitRot.y()));
        break;
      case USAGE_TOP:
        (*normals)[i] = (osg::Vec3(unitRot.x(), -unitRot.z(), unitRot.y()));
        break;
      case USAGE_RIGHT:
        (*normals)[i] = (osg::Vec3(unitRot.y(), -unitRot.x(), unitRot.z()));
        break;
      case USAGE_LEFT:
        (*normals)[i] = (osg::Vec3(-unitRot.y(), unitRot.x(), unitRot.z()));
        break;
      case USAGE_CONE:
      {
        osg::Vec3 normal((*verts)[i].x(), (*verts)[i].y() - range, (*verts)[i].z());
        normal.normalize();
        (*normals)[i] = normal;
        break;
      }
      }
    }
  }

  verts->dirty();
  normals->dirty();
  dirtyBound_(xform);
}

osg::Geometry* SVFactory::solidGeometry(osg::MatrixTransform* xform)
{
  if (xform == NULL || xform->getNumChildren() == 0)
    return NULL;
  osg::Geode* geode = xform->getChild(0)->asGeode();
  if (geode == NULL || geode->getNumDrawables() == 0)
    return NULL;
  return geode->getDrawable(0)->asGeometry();
}

// if the sv has a 2nd geode that adds outline or wireframe, it will be the MatrixTransform 2nd child
osg::Geode* SVFactory::opaqueGeode(osg::MatrixTransform* xform)
{
  if (xform == NULL || xform->getNumChildren() < 2)
    return NULL;
  return xform->getChild(1)->asGeode();
}

// dirty bounds for all geometries in the xform
void SVFactory::dirtyBound_(osg::MatrixTransform* xform)
{
  if (xform == NULL || xform->getNumChildren() == 0)
    return;

  // handle the geometries in the primary geode
  osg::Geometry* geom = SVFactory::solidGeometry(xform);
  if (geom && !geom->empty())
    geom->dirtyBound();

  // handle the 2nd geode
  osg::Geode* opaqueGeode = SVFactory::opaqueGeode(xform);
  if (!opaqueGeode || opaqueGeode->getNumDrawables() == 0)
    return;
  geom = opaqueGeode->getDrawable(0)->asGeometry();
  if (geom && !geom->empty())
    geom->dirtyBound();
}
