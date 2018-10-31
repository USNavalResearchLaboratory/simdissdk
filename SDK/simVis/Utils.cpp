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
#include "osg/Billboard"
#include "osg/BlendFunc"
#include "osg/Depth"
#include "osg/Geode"
#include "osg/Geometry"
#include "osg/Math"
#include "osg/MatrixTransform"
#include "osg/NodeVisitor"
#include "osg/PositionAttitudeTransform"
#include "osg/Version"
#include "osgDB/FileNameUtils"
#include "osgDB/Registry"
#include "osgSim/DOFTransform"
#include "osgUtil/RenderBin"
#include "osgUtil/TriStripVisitor"
#include "osgViewer/ViewerEventHandlers"

#include "osgEarth/Capabilities"
#include "osgEarth/CullingUtils"
#include "osgEarth/MapNode"
#include "osgEarth/Terrain"
#include "osgEarth/Utils"
#include "osgEarth/VirtualProgram"
#include "simVis/osgEarthVersion.h"

#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
#include "osgEarth/Lighting"
#endif

//#define USE_SIMCORE_CALC_MATH

#ifdef USE_SIMCORE_CALC_MATH
#include "simCore/Calc/Math.h"
#endif

#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/String/Format.h"
#include "simNotify/Notify.h"
#include "simVis/AlphaTest.h"
#include "simVis/Constants.h"
#include "simVis/DisableDepthOnAlpha.h"
#include "simVis/LineDrawable.h"
#include "simVis/PlatformModel.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"

namespace
{
  // NED/ENU swapping matrix:
  // http://www.ecsutton.ece.ufl.edu/ens/handouts/quaternions.pdf
  static const double NED_ENU[3][3] =
  {
    { 0.0, 1.0,  0.0 },
    { 1.0, 0.0,  0.0 },
    { 0.0, 0.0, -1.0 }
  };

  static const osg::Matrixd NWU_ENU(
    0.0, 1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0);

  /**
   * Utility visitor that will swap a node's coordinate system.
   */
  struct SwapCoordSys : public osg::NodeVisitor
  {
    osg::Matrixd swapper_;

    explicit SwapCoordSys(const osg::Matrixd& swapper)
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        swapper_(swapper)
    { }

    void apply(osg::MatrixTransform& xform)
    {
      xform.setMatrix(xform.getMatrix() * swapper_);
      traverse(xform);
    }

    void apply(osg::PositionAttitudeTransform& xform)
    {
      xform.setPosition(xform.getPosition() * swapper_);
      xform.setPivotPoint(xform.getPivotPoint() * swapper_);
      traverse(xform);
    }

    void apply(osg::Geode& geode)
    {
      for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
      {
        osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
        apply(geom);
      }
      traverse(geode);
    }

    void apply(osg::Geometry* geom)
    {
      osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
      if (verts)
        for (unsigned int i = 0; i < verts->size(); ++i)
          (*verts)[i] = (*verts)[i] * swapper_;

      osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geom->getNormalArray());
      if (normals)
        for (unsigned int i = 0; i < normals->size(); ++i)
          (*normals)[i] = (*normals)[i] * swapper_;
    }

    void apply(osg::Billboard& billboard)
    {
      osg::Billboard::PositionList& list = billboard.getPositionList();
      for (unsigned int i = 0; i < list.size(); ++i)
      {
        billboard.setPosition(i, list[i] * swapper_);
        billboard.setAxis(billboard.getAxis() * swapper_);
      }
      traverse(billboard);
    }
  };

  // Unscaled line length in meters for Platform line Vectors
  const int BASE_LINE_LENGTH = 50;

  /**
   * Custom render bin that implements a two-pass technique for rendering multiple
   * semi-transparent objects. It draws the entire bin twice: the first time with
   * depth-buffer writes turned off to enable full translucent blending; the second
   * time to populate the depth buffer.
   *
   * Since the bin needs to manage its own state, we have to manually draw the
   * render leaves and skip OSG's default state-tracking RenderBin code.
   *
   * Testing in OSG reveals that the StateSet associated with the render bin is inserted
   * into the render graph very "early," before even the camera's state set.  That means
   * any PROTECTED value later in the scene will override the behavior of the TPA.  This
   * matters for a TPA item because although a leaf node may have TPA set as the render
   * bin, a PROTECTED depth setting between the camera and the leaf node could override
   * the TPA behavior, disrupting the graphics.
   *
   * If you are reading this comment because you're debugging TPA not working, set a
   * breakpoint in the first call to State::apply() after the call here to
   * osgUtil::RenderBin::drawImplementation(), and inspect the _stateStateStack carefully.
   * You should see TPA setting Depth early in the stack; ensure nothing else overrides
   * that depth later with a PROTECTED attribute.
   */
  class TwoPassAlphaRenderBin : public osgUtil::RenderBin
  {
  public:
    TwoPassAlphaRenderBin()
      : osgUtil::RenderBin(SORT_BACK_TO_FRONT),
        haveInit_(false)
    {
      setName(simVis::BIN_TWO_PASS_ALPHA);
      setStateSet(NULL);

      // Note! We do not protect the depth settings here, because this then allows us to
      // disable the depth buffer at a higher level (e.g. when enabling Overhead mode).
      const osg::StateAttribute::GLModeValue overrideOn = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE;
      const osg::StateAttribute::GLModeValue overrideProtectedOn = overrideOn | osg::StateAttribute::PROTECTED;

      pass1_ = new osg::StateSet();
      pass1_->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, false), overrideOn);
      pass1_->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), overrideProtectedOn);

      pass2_ = new osg::StateSet();
      pass2_->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, true), overrideOn);
      pass2_->setAttributeAndModes(new osg::ColorMask(false, false, false, false), overrideProtectedOn);
    }

    TwoPassAlphaRenderBin(const TwoPassAlphaRenderBin& rhs, const osg::CopyOp& copy)
      : osgUtil::RenderBin(rhs, copy),
        pass1_(rhs.pass1_),
        pass2_(rhs.pass2_),
        haveInit_(rhs.haveInit_)
    {
      //nop
    }

    virtual osg::Object* clone(const osg::CopyOp& copyop) const
    {
      return new TwoPassAlphaRenderBin(*this, copyop);
    }

    // Draw the same geometry twice, once for each pass.
    // We ignore the incoming "previous" leaf since we are handling state changes
    // manually in this bin.
    void drawImplementation(osg::RenderInfo& ri, osgUtil::RenderLeaf*& previous)
    {
      // Initialize the alpha test, which cannot be done in the constructor due to static
      // initialization conflicts with its use of osgEarth::Registry::capabilities()
      if (!haveInit_)
      {
        haveInit_ = true;
        simVis::AlphaTest::setValues(pass2_.get(), 0.05f, osg::StateAttribute::ON |
          osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
      }

      // Create a copy of the state set stack so we can fix the internal stack after first drawImplementation()
      osgUtil::RenderLeaf* oldPrevious = previous;

      // Render once with the first state set.  Note that the state set is inserted into the
      // state set stack relatively early -- probably earlier than you expect -- and therefore
      // later PROTECTED elements can override the TPA state.
      const osg::State::StateSetStack previousStateStack = ri.getState()->getStateSetStack();
      setStateSet(pass1_.get());
      osgUtil::RenderBin::drawImplementation(ri, previous);

      // Get back to where we were at the start of this method, backing out state changes
      migrateState_(*ri.getState(), previousStateStack);
      previous = oldPrevious;

      // Now do the second pass with the original values but with second set of state values
      setStateSet(pass2_.get());
      osgUtil::RenderBin::drawImplementation(ri, previous);
    }

  private:
    /**
     * Given a current state, migrates its state stack backwards and forwards to get to the state
     * provided.  This algorithm does the following:
     *   - Pop the current state until it's the same size or smaller
     *   - Find the first item in state that doesn't match the to-state-stack
     *   - Pop off items from current state until it's down to the common ancestor
     *   - Push on all remaining items from the to-state-stack
     */
    void migrateState_(osg::State& state, const osg::State::StateSetStack& toStateStack) const
    {
      // Pop off states from the current, until it matches incoming size
      state.popStateSetStackToSize(toStateStack.size());
      // State's size is now less or equal to the size requested.  If less or equal, we're OK
      assert(state.getStateSetStackSize() <= toStateStack.size());

      // Figure out the first mismatching state
      unsigned int mismatchIndex = 0;
      for (mismatchIndex = 0; mismatchIndex < state.getStateSetStackSize(); ++mismatchIndex)
      {
        if (state.getStateSetStack()[mismatchIndex] != toStateStack[mismatchIndex])
          break;
      }
      // Pop off anything at or past the mismatch
      state.popStateSetStackToSize(mismatchIndex);
      // Assert failure means that the popStateSetStackToSize() isn't doing what is advertised
      assert(state.getStateSetStackSize() == mismatchIndex);

      // Push on the states from the original until we're matching again
      for (; mismatchIndex < toStateStack.size(); ++mismatchIndex)
        state.pushStateSet(toStateStack[mismatchIndex]);
      // Assert failure means that the pushStateSet() isn't doing what is advertised
      assert(state.getStateSetStackSize() == toStateStack.size());
    }

    osg::ref_ptr<osg::StateSet> pass1_;
    osg::ref_ptr<osg::StateSet> pass2_;
    bool haveInit_;
  };

  /** the actual registration. */
  extern "C" void osgEarth_BIN_TWO_PASS_ALPHA(void) {}
  static osgEarth::osgEarthRegisterRenderBinProxy<TwoPassAlphaRenderBin> s_regbin(simVis::BIN_TWO_PASS_ALPHA);
}

namespace simVis
{

bool useRexEngine()
{
  // The MP engine is no longer supported.  Always use rex.
  return true;
}

bool getLighting(osg::StateSet* stateset, osg::StateAttribute::OverrideValue& out_value)
{
  if (!stateset)
    return false;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  auto* definePair = stateset->getDefinePair(OE_LIGHTING_DEFINE);
  if (!definePair)
    return false;
  out_value = definePair->second;
  return out_value != osg::StateAttribute::INHERIT;
#else
  osg::StateAttribute::GLModeValue value = stateset->getMode(GL_LIGHTING);
  out_value = stateset->getMode(value);
  return out_value != osg::StateAttribute::INHERIT;
#endif
}


void setLighting(osg::StateSet* stateset, osg::StateAttribute::GLModeValue value)
{
  if (stateset)
  {
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    stateset->setDefine(OE_LIGHTING_DEFINE, value);
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    // GL_LIGHTING is deprecated in GL CORE builds
    stateset->setMode(GL_LIGHTING, value);
#endif
#else
    osg::Uniform* u = osgEarth::Registry::shaderFactory()->createUniformForGLMode(GL_LIGHTING, value);
    u->set((value & osg::StateAttribute::ON) != 0);
    stateset->addUniform(u, value);
#endif
  }
}

void setLightingToInherit(osg::StateSet* stateset)
{
  // (There's no method yet to query the name, so we just need to use the
  // internal name directly. At some point I will add a method to osgEarth
  // to properly query the name instead. -gw)
  if (stateset)
  {
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    stateset->removeDefine(OE_LIGHTING_DEFINE);
    stateset->removeMode(GL_LIGHTING);
#else
    osg::ref_ptr<osg::Uniform> temp = osgEarth::Registry::shaderFactory()
      ->createUniformForGLMode(GL_LIGHTING, 0);
    stateset->removeUniform(temp->getName());
#endif
  }
}

void fixTextureForGlCoreProfile(osg::Texture* texture)
{
  if (!texture)
    return;

  // No change is required if we're not supporting core profile
#ifndef OSG_GL_FIXED_FUNCTION_AVAILABLE
  for (unsigned int k = 0; k < texture->getNumImages(); ++k)
  {
    // Get a pointer to the image, continuing if none
    osg::Image* image = texture->getImage(k);
    if (!image)
      continue;

    // Detect the image's pixel format, changing it out for a GL3-compatible one, fixing swizzle
    if (image->getPixelFormat() == GL_LUMINANCE || image->getPixelFormat() == GL_RED)
    {
      image->setPixelFormat(GL_RED);
      texture->setSwizzle(osg::Vec4i(GL_RED, GL_RED, GL_RED, GL_ONE));
    }
    else if (image->getPixelFormat() == GL_LUMINANCE_ALPHA || image->getPixelFormat() == GL_RG)
    {
      image->setPixelFormat(GL_RG);
      texture->setSwizzle(osg::Vec4i(GL_RED, GL_RED, GL_RED, GL_GREEN));
    }
  }
#endif
}

void convertNWUtoENU(osg::Node* node)
{
  if (node)
  {
    SwapCoordSys swap(NWU_ENU);
    node->accept(swap);
  }
}


bool isImageFile(const std::string& location)
{
  std::string ext = osgDB::getLowerCaseFileExtension(location);
  if (!ext.empty())
  {
    // first check some known extensions (based on SIMDIS_MODEL_FILE_PATTERNS in simCore/String/FilePatterns.h)
    if (ext == "3db" || ext == "opt" || ext == "ive" || ext == "flt" || ext == "3ds" || ext == "obj" || ext == "lwo" || ext == "dxf" || ext == "osg" || ext == "osga" || ext == "osgb")
      return false;

    if (ext == "jpg" || ext == "png" || ext == "gif" || ext == "bmp" || ext == "tmd" || ext == "lst")
      return true;

    // something else; so check for rw support.
    osg::ref_ptr<osgDB::ReaderWriter> rw = osgDB::Registry::instance()->getReaderWriterForExtension(ext);
    if (rw.valid())
    {
      unsigned int features = static_cast<unsigned>(rw->supportedFeatures());
      if ((features & osgDB::ReaderWriter::FEATURE_READ_IMAGE) != 0)
      {
        return true;
      }
    }
  }
  return false;
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
std::string findFontFile(const std::string& fontFile)
{
  // Note that findFontFile() is deprecated and only provided for
  // compatibility reasons.  It may be removed in a future release.
  return Registry::instance()->findFontFile(fontFile);
}
#endif

osgEarth::Units convertUnitsToOsgEarth(const simData::DistanceUnits& input)
{
    return
        input == simData::UNITS_CENTIMETERS    ? osgEarth::Units::CENTIMETERS :
        input == simData::UNITS_DATAMILES      ? osgEarth::Units::DATA_MILES  :
        input == simData::UNITS_FATHOMS        ? osgEarth::Units::FATHOMS :
        input == simData::UNITS_FEET           ? osgEarth::Units::FEET :
        input == simData::UNITS_INCHES         ? osgEarth::Units::INCHES :
        input == simData::UNITS_KILOFEET       ? osgEarth::Units::KILOFEET :
        input == simData::UNITS_KILOMETERS     ? osgEarth::Units::KILOMETERS :
        input == simData::UNITS_KILOYARDS      ? osgEarth::Units::KILOYARDS :
        input == simData::UNITS_METERS         ? osgEarth::Units::METERS :
        input == simData::UNITS_MILES          ? osgEarth::Units::MILES :
        input == simData::UNITS_MILLIMETERS    ? osgEarth::Units::MILLIMETERS :
        input == simData::UNITS_NAUTICAL_MILES ? osgEarth::Units::NAUTICAL_MILES :
        input == simData::UNITS_YARDS          ? osgEarth::Units::YARDS :
        osgEarth::Units(); // invalid
}

osgEarth::Units convertUnitsToOsgEarth(const simData::SpeedUnits& input)
{
    return
        input == simData::UNITS_METERS_PER_SECOND     ? osgEarth::Units::METERS_PER_SECOND :
        input == simData::UNITS_KILOMETERS_PER_HOUR   ? osgEarth::Units::KILOMETERS_PER_HOUR :
        input == simData::UNITS_KNOTS                 ? osgEarth::Units::KNOTS :
        input == simData::UNITS_MILES_PER_HOUR        ? osgEarth::Units::MILES_PER_HOUR :
        input == simData::UNITS_FEET_PER_SECOND       ? osgEarth::Units::FEET_PER_SECOND :
        input == simData::UNITS_KILOMETERS_PER_SECOND ? osgEarth::Units::KILOMETERS_PER_SECOND :
        input == simData::UNITS_DATAMILES_PER_HOUR    ? osgEarth::Units::DATA_MILES_PER_HOUR :
        input == simData::UNITS_YARDS_PER_SECOND      ? osgEarth::Units::YARDS_PER_SECOND :
        osgEarth::Units(); // invalid
}

float outlineThickness(simData::TextOutline outline)
{
  switch (outline)
  {
  case simData::TO_NONE:
    return 0;
  case simData::TO_THIN:
    return .04f;
  case simData::TO_THICK:
    return 0.14f;
  }
  return 0;
}

float osgFontSize(float simFontSize)
{
  // When comparing SIMDIS 9 text, considered the standard for text size for SIMDIS applications,
  // the OSG font size was typically about 3/4 the size of a SIMDIS string for the same font and
  // same size.  To to convert the SIMDIS font size to OSG, we multiply by the inversion, 1.333f.
  return simFontSize * 1.333f;
}

float simdisFontSize(float osgFontSize)
{
  // See the discussion above, explaining that OSG fonts are about 3/4 the size of a historic
  // SIMDIS font size.
  return osgFontSize * 0.75f;
}

osgText::Text::BackdropType backdropType(simData::BackdropType type)
{
  switch (type)
  {
  case simData::BDT_SHADOW_BOTTOM_RIGHT:
    return osgText::Text::DROP_SHADOW_BOTTOM_RIGHT;
  case simData::BDT_SHADOW_CENTER_RIGHT:
    return osgText::Text::DROP_SHADOW_CENTER_RIGHT;
  case simData::BDT_SHADOW_TOP_RIGHT:
    return osgText::Text::DROP_SHADOW_TOP_RIGHT;
  case simData::BDT_SHADOW_BOTTOM_CENTER:
    return osgText::Text::DROP_SHADOW_BOTTOM_CENTER;
  case simData::BDT_SHADOW_TOP_CENTER:
    return osgText::Text::DROP_SHADOW_TOP_CENTER;
  case simData::BDT_SHADOW_BOTTOM_LEFT:
    return osgText::Text::DROP_SHADOW_BOTTOM_LEFT;
  case simData::BDT_SHADOW_CENTER_LEFT:
    return osgText::Text::DROP_SHADOW_CENTER_LEFT;
  case simData::BDT_SHADOW_TOP_LEFT:
    return osgText::Text::DROP_SHADOW_TOP_LEFT;
  case simData::BDT_OUTLINE:
    return osgText::Text::OUTLINE;
  case simData::BDT_NONE:
    return osgText::Text::NONE;
  }

  // Return default, which is NONE
  return osgText::Text::NONE;
}

osgText::Text::BackdropImplementation backdropImplementation(simData::BackdropImplementation implementation)
{
  switch (implementation)
  {
  case simData::BDI_POLYGON_OFFSET:
    return osgText::Text::POLYGON_OFFSET;
  case simData::BDI_NO_DEPTH_BUFFER:
    return osgText::Text::NO_DEPTH_BUFFER;
  case simData::BDI_DEPTH_RANGE:
    return osgText::Text::DEPTH_RANGE;
  case simData::BDI_STENCIL_BUFFER:
    return osgText::Text::STENCIL_BUFFER;
  case simData::BDI_DELAYED_DEPTH_WRITES:
    return osgText::Text::DELAYED_DEPTH_WRITES;
  }

  // Return default, which is POLYGON_OFFSET
  return osgText::Text::POLYGON_OFFSET;
}

void fixStatsHandlerGl2BlockyText(osgViewer::StatsHandler* statsHandler)
{
#if OSG_VERSION_GREATER_OR_EQUAL(3, 4, 1) && defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
  if (statsHandler && statsHandler->getCamera())
    statsHandler->getCamera()->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::PROGRAM);
#endif
}

//--------------------------------------------------------------------------

osg::Quat Math::eulerDegToQuat(double h, double p, double r)
{
  return eulerRadToQuat(simCore::DEG2RAD * h, simCore::DEG2RAD * p, simCore::DEG2RAD * r);
}

osg::Quat Math::eulerRadToQuat(double h, double p, double r)
{
#ifdef USE_SIMCORE_CALC_MATH

  double q[4];
  simCore::d3EulertoQ(simCore::Vec3(h, p, r), q);
  //simCore::d3EulertoQ( simCore::Vec3(h, -r, p), q );
  return osg::Quat(q[3], q[0], q[1], q[2]); //, q[3] );

#else // USE_SIMCORE_CALC_MATH

  // NOTE: OSG coordinate systems are all RIGHT-HANDED.
  // Here we create a series of quaternions based on the simCore
  // semantics for HPR, as detailed below.

  // +H is a "right turn", a right-handed rotation about the -Z axis:
  const osg::Quat azim_q = (!osg::equivalent(h, 0.0)) ? osg::Quat(h, osg::Vec3d(0, 0, -1)) : osg::Quat();

  // +P is "nose up"; a right-handed rotation about the +X axis:
  const osg::Quat pitch_q = (!osg::equivalent(p, 0.0)) ? osg::Quat(p, osg::Vec3d(1, 0, 0)) : osg::Quat();

  // +R is "right wing down", a right-handed rotation about the +Y axis:
  const osg::Quat roll_q = (!osg::equivalent(r, 0.0)) ? osg::Quat(r, osg::Vec3d(0, 1, 0)) : osg::Quat();

  // combine them in the reverse of the desired rotation order:
  // azim-pitch-roll
  return roll_q * pitch_q * azim_q;

#endif // USE_SIMCORE_CALC_MATH
}

osg::Vec3d Math::quatToEulerDeg(const osg::Quat& quat)
{
  const osg::Vec3d& rad = quatToEulerRad(quat);
  return rad * simCore::RAD2DEG;
}

osg::Vec3d Math::quatToEulerRad(const osg::Quat& quat)
{
#ifdef USE_SIMCORE_CALC_MATH

  double q[4];
  q[0] = quat.x(), q[1] = quat.y(), q[2] = quat.z(), q[3] = quat.w();
  simCore::Vec3 hpr_rad;
  simCore::d3QtoEuler(q, &hpr_rad);
  return osg::Vec3d(hpr_rad[0], hpr_rad[1], hpr_rad[2]);

#else // USE_SIMCORE_CALC_MATH

  double h, p, r;

  const osg::Quat& q = quat;
  p = atan2(2*(q.y()*q.z() + q.w()*q.x()), (q.w()*q.w() - q.x()*q.x() - q.y()*q.y() + q.z() * q.z()));
  h = asin(2*q.x()*q.y() + 2*q.z()*q.w());
  r = atan2(2*q.x()*q.w()-2*q.y()*q.z(), 1 - 2*q.x()*q.x() - 2*q.z()*q.z());

  if (osg::equivalent(q.x()*q.y() + q.z() *q.w(), 0.5))
  {
    p = (float)(2 * atan2(q.x(), q.w()));
    r = 0;
  }
  else if (osg::equivalent(q.x()*q.y() + q.z()*q.w(), -0.5))
  {
    p = (float)(-2 * atan2(q.x(), q.w()));
    r = 0;
  }

  return osg::Vec3d(h, p, r);

#endif // USE_SIMCORE_CALC_MATH
}



/**
* Converts a SIMDIS ECEF orientation (psi/theta/phi) into an OSG
* ENU rotation matrix. The SIMDIS d3EulertoQ() method results in a
* NED orientation frame. We want ENU so we have to fix the conversion.
*/
void Math::ecefEulerToEnuRotMatrix(const simCore::Vec3& in, osg::Matrix& out)
{
  // first convert the ECEF orientation to a 3x3 matrix:
  double ned_dcm[3][3];
  simCore::d3EulertoDCM(in, ned_dcm);
  double enu_dcm[3][3];
  simCore::d3MMmult(NED_ENU, ned_dcm, enu_dcm);

  // poke the values into the OSG matrix:
  out.set(
    enu_dcm[0][0], enu_dcm[0][1], enu_dcm[0][2], 0.0,
    enu_dcm[1][0], enu_dcm[1][1], enu_dcm[1][2], 0.0,
    enu_dcm[2][0], enu_dcm[2][1], enu_dcm[2][2], 0.0,
    0.0, 0.0, 0.0, 1.0);
}

/**
* Converts an ENU (OSG style) rotation matrix into SIMDIS
* (NED frame) global Euler angles -- this is the inverse of
* the method ecefEulerToEnuRotMatrix().
*/
void Math::enuRotMatrixToEcefEuler(const osg::Matrix& in, simCore::Vec3& out)
{
  // direction cosine matrix in ENU frame
  double enu_dcm[3][3] = {
    { in(0,0), in(0,1), in(0,2) },
    { in(1,0), in(1,1), in(1,2) },
    { in(2,0), in(2,1), in(2,2) }
  };

  // convert DCM to NED frame:
  double ned_dcm[3][3];
  simCore::d3MMmult(NED_ENU, enu_dcm, ned_dcm);

  // and into Euler angles.
  simCore::d3DCMtoEuler(ned_dcm, out);
}

void Math::clampMatrixOrientation(osg::Matrixd& mat, osg::Vec3d& min_hpr_deg, osg::Vec3d& max_hpr_deg)
{
  const osg::Quat& q = mat.getRotate();
  const osg::Vec3d& hpr_deg = quatToEulerDeg(q);
  double delta[3];
  for (int i = 0; i < 3; i++)
  {
    delta[i] =
      hpr_deg[i] < min_hpr_deg[i] ? hpr_deg[i] - min_hpr_deg[i] :
      hpr_deg[i] > max_hpr_deg[i] ? max_hpr_deg[i] - hpr_deg[i] :
      0.0;
  }
  if (delta[0] != 0.0 || delta[1] != 0.0 || delta[2] != 0.0)
  {
    const osg::Quat& q = eulerDegToQuat(osg::Vec3d(delta[0], delta[1], delta[2]));
    mat.postMultRotate(q);
  }
}

osg::Vec4f ColorUtils::RgbaToVec4(unsigned int color)
{
  // Convert 0xRRGGBBAA to (R, G, B, A)
  return osg::Vec4f(static_cast<double>((color >> 24) & 0xFF) / 255.0,
      static_cast<double>((color >> 16) & 0xFF) / 255.0,
      static_cast<double>((color >> 8) & 0xFF) / 255.0,
      static_cast<double>(color & 0xFF) / 255.0);
}

ColorUtils::ColorUtils(float gainAlpha)
{
  gainThresholdColorMap_[120]  = osg::Vec4(1.0f, 0.0f, 0.0f, gainAlpha); //0x800000FF;
  gainThresholdColorMap_[100]  = osg::Vec4(1.0f, 1.0f, 0.0f, gainAlpha); //0x8000FFFF;
  gainThresholdColorMap_[80]   = osg::Vec4(1.0f, 0.0f, 1.0f, gainAlpha); //0x80FF00FF;
  gainThresholdColorMap_[60]   = osg::Vec4(0.0f, 0.0f, 1.0f, gainAlpha); //0x80FF0000;
  gainThresholdColorMap_[40]   = osg::Vec4(0.0f, 1.0f, 0.0f, gainAlpha); //0x8000FF00;
  gainThresholdColorMap_[20]   = osg::Vec4(1.0f, 0.5f, 0.0f, gainAlpha); //0x800080FF;
  gainThresholdColorMap_[0]    = osg::Vec4(0.0f, 0.5f, 0.5f, gainAlpha); //0x80808000;
  gainThresholdColorMap_[-20]  = osg::Vec4(0.0f, 0.5f, 0.0f, gainAlpha); //0x80008000;
  gainThresholdColorMap_[-40]  = osg::Vec4(0.0f, 0.0f, 0.5f, gainAlpha); //0x80800000;
  gainThresholdColorMap_[-60]  = osg::Vec4(0.75f, 0.75f, 0.75f, gainAlpha); //0x80C0C0C0;
  gainThresholdColorMap_[-80]  = osg::Vec4(0.0f, 1.0f, 1.0f, gainAlpha); //0x80FFFF00;
  gainThresholdColorMap_[-100] = osg::Vec4(0.5f, 0.0f, 0.5f, gainAlpha); //0x80800080;
}

void ColorUtils::GainThresholdColor(int gain, osg::Vec4f &color, float alpha)
{
  if (gain > 100)
    color.set(1.0f, 0.0f, 0.0f, alpha); //0x800000FF;
  else if (gain > 80)
    color.set(1.0f, 1.0f, 0.0f, alpha); //0x8000FFFF;
  else if (gain > 60)
    color.set(1.0f, 0.0f, 1.0f, alpha); //0x80FF00FF;
  else if (gain > 40)
    color.set(0.0f, 0.0f, 1.0f, alpha); //0x80FF0000;
  else if (gain > 20)
    color.set(0.0f, 1.0f, 0.0f, alpha); //0x8000FF00;
  else if (gain > 0)
    color.set(1.0f, 0.5f, 0.0f, alpha); //0x800080FF;
  else if (gain > -20)
    color.set(0.0f, 0.5f, 0.5f, alpha); //0x80808000;
  else if (gain > -40)
    color.set(0.0f, 0.5f, 0.0f, alpha); //0x80008000;
  else if (gain > -60)
    color.set(0.0f, 0.0f, 0.5f, alpha); //0x80800000;
  else if (gain > -80)
    color.set(0.75f, 0.75f, 0.75f, alpha); //0x80C0C0C0;
  else if (gain > -100)
    color.set(0.0f, 1.0f, 1.0f, alpha); //0x80FFFF00;
  else
    color.set(0.5f, 0.0f, 0.5f, alpha); //0x80800080;
}

const osg::Vec4f& ColorUtils::GainThresholdColor(int gain)
{
  if (gain > 100) return gainThresholdColorMap_[120];

  ColorMap::const_iterator iter = gainThresholdColorMap_.lower_bound(gain);
  if (iter != gainThresholdColorMap_.end())
    return iter->second;

  return gainThresholdColorMap_[-100];
}

bool convertCoordToGeoPoint(const simCore::Coordinate& input, osgEarth::GeoPoint& output, const osgEarth::SpatialReference* srs)
{
  if (srs && input.coordinateSystem() == simCore::COORD_SYS_ECEF)
  {
    simCore::Vec3 llaPos;
    simCore::CoordinateConverter::convertEcefToGeodeticPos(input.position(), llaPos);
    output.set(
      srs->getGeographicSRS(),
      osg::RadiansToDegrees(llaPos.lon()),
      osg::RadiansToDegrees(llaPos.lat()),
      llaPos.alt(),
      osgEarth::ALTMODE_ABSOLUTE);

    return true;
  }

  else if (srs && input.coordinateSystem() == simCore::COORD_SYS_LLA)
  {
    output.set(
      srs->getGeographicSRS(),
      osg::RadiansToDegrees(input.lon()),
      osg::RadiansToDegrees(input.lat()),
      input.alt(),
      osgEarth::ALTMODE_ABSOLUTE);

    return true;
  }

  return false;
}

bool convertGeoPointToCoord(const osgEarth::GeoPoint& input, simCore::Coordinate& out_coord, osgEarth::MapNode* mapNode)
{
  // can't convert a relative-Z point without the mapNode.
  if (input.altitudeMode() == osgEarth::ALTMODE_RELATIVE && !mapNode)
    return false;

  // convert to absolute Z
  osgEarth::GeoPoint absInput = input;
  absInput.makeAbsolute(mapNode->getTerrain());

  // convert to lat/long if necessary:
  if (!absInput.getSRS()->isGeographic())
  {
    if (!absInput.transform(absInput.getSRS()->getGeographicSRS(), absInput))
      return false;
  }

  out_coord = simCore::Coordinate(
    simCore::COORD_SYS_LLA,
    simCore::Vec3(absInput.y() * simCore::DEG2RAD, absInput.x() * simCore::DEG2RAD, absInput.alt()));

  return true;
}


osg::Image* makeBrokenImage(int size)
{
  osg::Image* image = new osg::Image();

  int edge = size - 1; // used to center the x
  image->allocateImage(size, size, 1, GL_RGBA, GL_UNSIGNED_BYTE);
  for (int s=0; s < image->s(); ++s)
  {
    for (int t=0; t < image->t(); ++t)
    {
      *((unsigned*)image->data(s, t)) = (s == t || s == static_cast<int>(edge-t)) ? 0xff0000ff : 0x4fffffff;
    }
  }

  return image;
}

osg::Matrix computeLocalToWorld(const osg::Node* node)
{
  osg::Matrix m;
  if (node)
  {
    const osg::NodePathList nodePaths = node->getParentalNodePaths();
    if (nodePaths.size() > 0)
    {
      m = osg::computeLocalToWorld(nodePaths[0]);
    }
    else
    {
      const osg::Transform* t = dynamic_cast<const osg::Transform*>(node);
      if (t)
      {
        t->computeLocalToWorldMatrix(m, 0L);
      }
    }
  }
  return m;
}

simCore::Vec3 computeNodeGeodeticPosition(const osg::Node* node)
{
  if (node == NULL)
    return simCore::Vec3();
  const osg::Vec3d& ecefPos = computeLocalToWorld(node).getTrans();
  simCore::Vec3 llaPos;
  simCore::CoordinateConverter::convertEcefToGeodeticPos(simCore::Vec3(ecefPos.x(), ecefPos.y(), ecefPos.z()), llaPos);
  return llaPos;
}


bool VectorScaling::fieldsChanged(const simData::PlatformPrefs& lastPrefs, const simData::PlatformPrefs& newPrefs)
{
  return PB_FIELD_CHANGED(&lastPrefs, &newPrefs, dynamicscale) ||
    PB_FIELD_CHANGED(&lastPrefs, &newPrefs, axisscale);
}

osg::Vec3f VectorScaling::boundingBoxSize(const osg::BoundingBoxf& bbox)
{
  return osg::Vec3f(fabs(bbox.xMax() - bbox.xMin()),
    fabs(bbox.yMax() - bbox.yMin()),
    fabs(bbox.zMax() - bbox.zMin()));
}

float VectorScaling::boundingBoxMaxDimension(const osg::BoundingBoxf& bbox)
{
  const osg::Vec3f& dims = VectorScaling::boundingBoxSize(bbox);
  return osg::maximum(dims.x(), osg::maximum(dims.y(), dims.z()));
}

float VectorScaling::lineLength(const PlatformModelNode* node, float axisScale)
{
  float adjustedLength = BASE_LINE_LENGTH;
  if (node)
    adjustedLength = VectorScaling::boundingBoxMaxDimension(node->getUnscaledIconBounds());
  return adjustedLength * axisScale;
}

void VectorScaling::generatePoints(osg::Vec3Array& vertices, const osg::Vec3& start, const osg::Vec3& end)
{
  const unsigned int numPointsPerLine = vertices.getNumElements();
  // Avoid divide-by-zero problems
  if (numPointsPerLine < 2)
    return;

  const osg::Vec3 delta = (end - start);
  for (unsigned int k = 0; k < numPointsPerLine - 1; ++k)
  {
    // Translate [0,numPointsPerLine-1) into [0,1)
    const float pct = static_cast<float>(k) / (numPointsPerLine - 1);
    vertices[k] = (start + delta * pct);
  }
  vertices[numPointsPerLine - 1] = end;
}

void VectorScaling::generatePoints(osgEarth::LineDrawable& line, const osg::Vec3& start, const osg::Vec3& end)
{
  const unsigned int numPointsPerLine = line.getNumVerts();
  // Avoid divide-by-zero problems
  if (numPointsPerLine < 2)
    return;

  const osg::Vec3 delta = (end - start);
  for (unsigned int k = 0; k < numPointsPerLine - 1; ++k)
  {
    // Translate [0,numPointsPerLine-1) into [0,1)
    const float pct = static_cast<float>(k) / (numPointsPerLine - 1);
    line.setVertex(k, start + delta * pct);
  }
  line.setVertex(numPointsPerLine - 1, end);
}


//--------------------------------------------------------------------------

SequenceTimeUpdater::SequenceTimeUpdater(osg::FrameStamp* replacementStamp)
  : sceneStamp_(replacementStamp),
    modifiedStamp_(new osg::FrameStamp)
{
  if (sceneStamp_.valid())
    updateModifiedStamp_();
}

SequenceTimeUpdater::~SequenceTimeUpdater()
{
}

void SequenceTimeUpdater::setFrameStamp(osg::FrameStamp* frameStamp)
{
  sceneStamp_ = frameStamp;
  if (sceneStamp_.valid())
    updateModifiedStamp_();
}

void SequenceTimeUpdater::updateModifiedStamp_()
{
  if (!sceneStamp_.valid())
    return;
  *modifiedStamp_ = *sceneStamp_;
  modifiedStamp_->setSimulationTime(modifiedStamp_->getReferenceTime());
}

void SequenceTimeUpdater::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
  // Only apply to Sequence nodes and their children
  osg::ref_ptr<osg::FrameStamp> sceneStamp;
  if (!sceneStamp_.lock(sceneStamp))
  {
    traverse(node, nv);
    return;
  }

  // If the last update time does not matches the scene stamp's time, fix modifiedStamp_
  if (sceneStamp->getFrameNumber() != modifiedStamp_->getFrameNumber())
    updateModifiedStamp_();

  // Copy the frame stamp and update it to a strictly increasing time
  osg::ref_ptr<const osg::FrameStamp> oldFs = nv->getFrameStamp();
  nv->setFrameStamp(modifiedStamp_.get());

  // Visit the Sequence itself
  traverse(node, nv);

  // Revert back to the old time
  nv->setFrameStamp(const_cast<osg::FrameStamp*>(oldFs.get()));
}

//--------------------------------------------------------------------------

StatsTimer::StatsTimer(osgViewer::View* mainView, const std::string& key, RecordFrequency recordFrequency)
  : mainView_(mainView),
    beginKey_(StatsTimer::beginName(key)),
    endKey_(StatsTimer::endName(key)),
    timeTakenKey_(StatsTimer::timeTakenName(key)),
    recordFrequency_(recordFrequency),
    cumulativeMs_(0),
    firstStartTickMs_(0),
    lastStopTickMs_(0),
    startTickMs_(0),
    currentFrameNumber_(0),
    currentFrameStartTickMs_(0)
{
}

StatsTimer::~StatsTimer()
{
  // Stop if we are started
  if (isStarted_())
    stop();
}

int StatsTimer::start()
{
  // Avoid executing if start() called while active; note that due to nested calls
  // it is inadvisable to assert on this condition.
  if (isStarted_())
    return 1;

  // ref_ptr from observer_ptr idiom
  osg::ref_ptr<osgViewer::View> view;
  if (!mainView_.lock(view) || !view.valid() || !view->getFrameStamp())
    return 1;

  // Cache some important values for timing calcs
  const osg::Timer* timer = osg::Timer::instance();
  const osg::Timer_t nowTick = timer->tick();
  const unsigned int thisFrame = view->getFrameStamp()->getFrameNumber();

  // If this is a new frame, we need to reset some stale values
  if (thisFrame != currentFrameNumber_)
  {
    // Record frame if needed
    if (recordFrequency_ == RECORD_PER_FRAME_ON_START || recordFrequency_ == RECORD_PER_FRAME_RESTAMPED_ON_START)
      record_();

    // Reset all values
    reset_();
    currentFrameNumber_ = thisFrame;
    currentFrameStartTickMs_ = view->getStartTick();
    firstStartTickMs_ = nowTick;
  }

  // Save the start tick so we can know the delta when stop() gets called
  startTickMs_ = nowTick;
  return 0;
}

int StatsTimer::stop()
{
  // Avoid stopping if start() has not been called; note that due to nested calls
  // it is inadvisable to assert on this condition.
  if (!isStarted_())
    return 1;

  // Save the current tick, and calculate new cumulative delta
  lastStopTickMs_ = osg::Timer::instance()->tick();
  cumulativeMs_ += (lastStopTickMs_ - startTickMs_);

  // Record the frame if needed
  if (recordFrequency_ == RECORD_PER_STOP)
    record_();
  // Reset startTickMs_ so we're ready for another start() this frame
  startTickMs_ = 0;
  return 0;
}

std::string StatsTimer::beginName(const std::string& key)
{
  return key + " begin";
}

std::string StatsTimer::endName(const std::string& key)
{
  return key + " end";
}

std::string StatsTimer::timeTakenName(const std::string& key)
{
  return key + " time taken";
}

void StatsTimer::addLine(osgViewer::StatsHandler* stats, const std::string& title, const std::string& key, const osg::Vec4& color)
{
  if (!stats)
    return;
  static const float SEC_TO_MSEC_MULTIPLIER = 1000.f;
  static const float MAX_TIME = 0.016f; // 60 fps (1 / 60) == 0.016
  stats->addUserStatsLine(title, color, color,
    StatsTimer::timeTakenName(key),
    SEC_TO_MSEC_MULTIPLIER, true, false,
    StatsTimer::beginName(key),
    StatsTimer::endName(key),
    MAX_TIME);
}

void StatsTimer::removeLine(osgViewer::StatsHandler* stats, const std::string& title)
{
  if (stats)
    stats->removeUserStatsLine(title);
}

bool StatsTimer::isStarted_() const
{
  return startTickMs_ != 0;
}

int StatsTimer::record_()
{
  // Break out if we do not have a frame number; implies invalid data
  if (currentFrameNumber_ == 0)
    return 1;

  // ref_ptr from observer_ptr idiom
  osg::ref_ptr<osgViewer::View> view;
  if (!mainView_.lock(view) || !view.valid() || !view->getViewerBase())
    return 1;
  // Make sure stats are valid
  osg::Stats* stats = view->getViewerBase()->getViewerStats();
  if (!stats)
    return 1;

  // If restamping, update the frame number
  unsigned int frameNumber = currentFrameNumber_;
  if (recordFrequency_ == RECORD_PER_FRAME_RESTAMPED_ON_START)
    frameNumber = simCore::sdkMax(stats->getEarliestFrameNumber(), frameNumber);

  // Calculate the begin and end time for this frame's ticks
  const osg::Timer* timer = osg::Timer::instance();
  const double cumulativeTime = cumulativeMs_ * timer->getSecondsPerTick();
  const double beginTime = timer->delta_s(currentFrameStartTickMs_, firstStartTickMs_);
  const double endTime = timer->delta_s(currentFrameStartTickMs_, lastStopTickMs_);

  // Set the attributes on the stats object for our key on the given frame
  stats->setAttribute(frameNumber, timeTakenKey_, cumulativeTime);
  stats->setAttribute(frameNumber, beginKey_, beginTime);
  stats->setAttribute(frameNumber, endKey_, endTime);
  return 0;
}

void StatsTimer::reset_()
{
  cumulativeMs_ = firstStartTickMs_ = lastStopTickMs_ = 0;
  startTickMs_ = 0;
  currentFrameNumber_ = 0;
  currentFrameStartTickMs_ = 0;
}

//--------------------------------------------------------------------------

ScopedStatsTimerToken::ScopedStatsTimerToken(StatsTimer& tick)
  : tick_(tick)
{
  tick_.start();
}

ScopedStatsTimerToken::~ScopedStatsTimerToken()
{
  tick_.stop();
}

//--------------------------------------------------------------------------

ScopedStatsTimer::ScopedStatsTimer(osgViewer::View* mainView, const std::string& key)
  : statsTimer_(mainView, key, StatsTimer::RECORD_PER_STOP)
{
  statsTimer_.start();
}

//--------------------------------------------------------------------------

RemoveModeVisitor::RemoveModeVisitor(GLenum mode)
  : NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    mode_(mode)
{
}

void RemoveModeVisitor::apply(osg::Node& node)
{
  osg::StateSet* stateSet = node.getStateSet();
  if (stateSet)
    stateSet->removeMode(mode_);
  osg::NodeVisitor::apply(node);
}

//--------------------------------------------------------------------------

FixDeprecatedDrawModes::FixDeprecatedDrawModes()
  : NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
}

void FixDeprecatedDrawModes::apply(osg::Geometry& geom)
{
  // Loop through all of the primitive sets on the geometry
  const unsigned int numPrimSets = geom.getNumPrimitiveSets();
  for (unsigned int k = 0; k < numPrimSets; ++k)
  {
    // Only care about non-NULL primitive sets
    const osg::PrimitiveSet* primSet = geom.getPrimitiveSet(k);
    if (primSet == NULL)
      continue;

    // Search for modes that are deprecated in GL3
    if (primSet->getMode() == GL_POLYGON || primSet->getMode() == GL_QUADS || primSet->getMode() == GL_QUAD_STRIP)
    {
      // Turn deprecated geometry into tri-strips; affects whole geometry
      osgUtil::TriStripVisitor triStrip;
      triStrip.stripify(geom);
      break;
    }
  }

  // Call into base class method
  osg::NodeVisitor::apply(geom);
}

//--------------------------------------------------------------------------

// Flags pulled from DOFTransform.cpp and map to osgSim::DOFTransform::getLimitationFlags()
static const unsigned int ROTATION_PITCH_LIMIT_BIT = 0x80000000u >> 3;
static const unsigned int ROTATION_ROLL_LIMIT_BIT = 0x80000000u >> 4;
static const unsigned int ROTATION_YAW_LIMIT_BIT = 0x80000000u >> 5;
static const unsigned int ROTATION_LIMIT_MASK = ROTATION_PITCH_LIMIT_BIT | ROTATION_ROLL_LIMIT_BIT | ROTATION_YAW_LIMIT_BIT;

/**
 * osgSim::DOFTransform blindly adds values to deal with DOF Transform animation, scaled on
 * the delta time.  That's fine for most cases, but when limits are disabled and we're still
 * incrementing, you can get large values in the current HPR.  That means "infinite" rotation
 * breaks.  This callback ensures that all rotations are within [0, 2PI] in those cases.
 *
 * This scaling only applies to angle values for HPR, and does not cover infinitely scaling
 * translate or scale values.
 */
class ConstrainHprValues : public osg::Callback
{
public:
  virtual bool run(osg::Object* object, osg::Object* data)
  {
    // Only work on animated DOFs
    osgSim::DOFTransform* dofXform = dynamic_cast<osgSim::DOFTransform*>(object);
    if (dofXform && dofXform->getAnimationOn())
    {
      const osg::Vec3& increment = dofXform->getIncrementHPR();
      const unsigned long flags = dofXform->getLimitationFlags();
      if ((flags & ROTATION_LIMIT_MASK) != ROTATION_LIMIT_MASK)
      {
        // Constrain from [0,2PI] only in cases where limiting is disabled and we're incrementing the value.
        osg::Vec3f hpr = dofXform->getCurrentHPR();
        if ((flags & ROTATION_YAW_LIMIT_BIT) == 0 && increment.x() != 0)
          hpr.x() = simCore::angFix(hpr.x(), simCore::ANGLEEXTENTS_TWOPI);
        if ((flags & ROTATION_PITCH_LIMIT_BIT) == 0 && increment.y() != 0)
          hpr.y() = simCore::angFix(hpr.y(), simCore::ANGLEEXTENTS_TWOPI);
        if ((flags & ROTATION_ROLL_LIMIT_BIT) == 0 && increment.z() != 0)
          hpr.z() = simCore::angFix(hpr.z(), simCore::ANGLEEXTENTS_TWOPI);
        dofXform->setCurrentHPR(hpr);
      }
    }

    // Continue on
    return traverse(object, data);
  }
};

EnableDOFTransform::EnableDOFTransform(bool enabled)
  : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    enabled_(enabled)
{
}

void EnableDOFTransform::apply(osg::Node& node)
{
  osgSim::DOFTransform* dofXform = dynamic_cast<osgSim::DOFTransform*>(&node);
  if (dofXform)
  {
    dofXform->setAnimationOn(enabled_);

    // We want to add a callback to fix a bug in osgSim::DOFTransform, where infinitely
    // increasing HPR values cause precision problems with high scenario delta time values.
    // Without this, infinite rotations will skip and stutter, and not work in real-time playback.
    const osg::Vec3& incr = dofXform->getIncrementHPR();
    // Add a new callback to constrain HPR values using fmod, if needed
    if (enabled_ && (incr.x() != 0.f || incr.y() != 0.f || incr.z() != 0.f))
    {
      if (!findUpdateCallbackOfType<ConstrainHprValues>(dofXform))
        dofXform->addUpdateCallback(new ConstrainHprValues);
    }
  }
  traverse(node);
}

//--------------------------------------------------------------------------

PixelScaleHudTransform::PixelScaleHudTransform()
{
}

PixelScaleHudTransform::PixelScaleHudTransform(const PixelScaleHudTransform& rhs, const osg::CopyOp& copyop)
  : Transform(rhs, copyop),
    invertedMvpw_(rhs.invertedMvpw_)
{
}

bool PixelScaleHudTransform::computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
  if (_referenceFrame == RELATIVE_RF)
    matrix.preMult(computeMatrix_(nv));
  else // absolute
    matrix = computeMatrix_(nv);
  return true;
}

bool PixelScaleHudTransform::computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
  if (_referenceFrame == RELATIVE_RF)
    matrix.postMult(osg::Matrix::inverse(computeMatrix_(nv)));
  else // absolute
    matrix = osg::Matrix::inverse(computeMatrix_(nv));
  return true;
}

osg::Matrixd PixelScaleHudTransform::computeMatrix_(osg::NodeVisitor* nv) const
{
  osg::CullStack* cs = nv ? nv->asCullStack() : NULL;
  if (cs)
    invertedMvpw_ = osg::Matrix::inverse(*cs->getMVPW());
  return invertedMvpw_;
}

}
