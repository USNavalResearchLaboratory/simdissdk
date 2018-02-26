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
#include "osg/Math"
#include "osg/CullFace"
#include "osg/BlendFunc"
#include "osg/NodeVisitor"
#include "osg/MatrixTransform"
#include "osg/PositionAttitudeTransform"
#include "osg/Billboard"
#include "osg/Geode"
#include "osg/Geometry"
#include "osg/Depth"
#include "osg/Multisample"
#include "osg/AlphaFunc"
#include "osgDB/FileUtils"
#include "osgDB/FileNameUtils"
#include "osgDB/Registry"
#include "osgUtil/RenderBin"
#include "osgViewer/ViewerEventHandlers"

#include "osgEarth/Capabilities"
#include "osgEarth/MapNode"
#include "osgEarth/Terrain"
#include "osgEarth/Utils"
#include "simVis/osgEarthVersion.h"

#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
#include "osgEarth/Lighting"
#endif

//#define USE_SIMCORE_CALC_MATH

#ifdef USE_SIMCORE_CALC_MATH
#include "simCore/Calc/Math.h"
#endif

#include "simCore/String/Format.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simNotify/Notify.h"
#include "simVis/Registry.h"
#include "simVis/PlatformModel.h"
#include "simVis/Utils.h"
#include "simVis/Constants.h"
#include "osgEarth/VirtualProgram"

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
   * Custom render bin that implements a two-pass technique for 
   * rendering multiple transparent objects.
   */
  class TwoPassAlphaRenderBin : public osgUtil::RenderBin
  {
  public:
      TwoPassAlphaRenderBin() : osgUtil::RenderBin(SORT_BACK_TO_FRONT)
      {
          this->setName(simVis::BIN_TWO_PASS_ALPHA);

          const osg::StateAttribute::GLModeValue forceOn = 
              osg::StateAttribute::ON | 
              osg::StateAttribute::PROTECTED | 
              osg::StateAttribute::OVERRIDE;

          pass1_ = new osg::StateSet();
          pass1_->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, false), forceOn);
          pass1_->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), forceOn);

          pass2_ = new osg::StateSet();
          pass2_->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, true), forceOn);
          pass2_->setAttributeAndModes(new osg::ColorMask(false, false, false, false), forceOn);
          pass2_->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.05f), forceOn);
          pass2_->setMode(GL_ALPHA_TEST, forceOn);
      }

      TwoPassAlphaRenderBin(const TwoPassAlphaRenderBin& rhs, const osg::CopyOp& copy)
          : osgUtil::RenderBin(rhs, copy),
          pass1_(rhs.pass1_),
          pass2_(rhs.pass2_)
      {
          //nop
      }

      virtual osg::Object* clone(const osg::CopyOp& copyop) const
      {
          return new TwoPassAlphaRenderBin(*this, copyop);
      }

      void drawPass(osg::StateSet* pass, osg::RenderInfo& ri, osgUtil::RenderLeaf*& previous)
      {
          ri.getState()->apply(pass);
          //ri.getState()->pushStateSet(pass);
          //ri.getState()->apply();

          for (RenderLeafList::iterator rlitr = _renderLeafList.begin();
              rlitr != _renderLeafList.end();
              ++rlitr)
          {
              osgUtil::RenderLeaf* rl = *rlitr;
              rl->render(ri, previous);
              previous = rl;
          }

          //ri.getState()->popStateSet();
      }

      void drawImplementation(osg::RenderInfo& ri, osgUtil::RenderLeaf*& previous)
      {
          drawPass(pass1_.get(), ri, previous);          
          drawPass(pass2_.get(), ri, previous);
      }

      osg::ref_ptr<osg::StateSet> pass1_, pass2_;
  };

  /** the actual registration. */
  extern "C" void osgEarth_BIN_TWO_PASS_ALPHA(void) {}
  static osgEarth::osgEarthRegisterRenderBinProxy<TwoPassAlphaRenderBin> s_regbin(simVis::BIN_TWO_PASS_ALPHA);
}

namespace simVis
{

bool useRexEngine()
{
  osgEarth::Registry* reg = osgEarth::Registry::instance();

  // first use the default name
  std::string engineName = reg->getDefaultTerrainEngineDriverName();
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  // See if the override is set, which takes precedence. This will be the same as default name
  // if terrain engine was set by environment variable OSGEARTH_TERRAIN_ENGINE
  if (reg->overrideTerrainEngineDriverName().isSet())
    engineName = reg->overrideTerrainEngineDriverName().value();
#endif

  // If we cannot support REX due to GLSL version, then fall back to MP automatically
  if (reg->capabilities().getGLSLVersionInt() < 330)
  {
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    reg->overrideTerrainEngineDriverName() = "mp";
#endif
    return false;
  }

  return simCore::caseCompare(engineName, "rex") == 0;
}

void setLighting(osg::StateSet* stateset, osg::StateAttribute::GLModeValue value)
{
  if (stateset)
  {
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    stateset->setDefine(OE_LIGHTING_DEFINE, value);
    stateset->setMode(GL_LIGHTING, value);
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
  // At lower font sizes (11 or less), we want to make the font a bit
  // crisper and more readable, so we force the return value to be the
  // closest rounded number and add an offset to convert the value from
  // simFontSize to osgFontSize.
  if (simFontSize <= 6.0)
  {
    return simCore::rint(simFontSize) + 1.0;
  }
  else if (simFontSize <= 11.0)
  {
    return simCore::rint(simFontSize) + 2.0;
  }

  // Value of 1.33 was confirmed using fonts of varying sizes in example
  // data files.
  return simFontSize * 1.33;
}

float simdisFontSize(float osgFontSize)
{
  float roundedSize = simCore::rint(osgFontSize);
  if (roundedSize <= 7.0)
  {
    return roundedSize - 1.0;
  }
  else if (roundedSize <= 13.0)
  {
    return roundedSize - 2.0;
  }

  return osgFontSize / 1.33;
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

void VectorScaling::generatePoints(osg::Vec3Array& vertices, const osg::Vec3& start, const osg::Vec3& end, int numPointsPerLine)
{
  // Avoid divide-by-zero problems
  if (numPointsPerLine < 2)
    return;

  const osg::Vec3 delta = (end - start);
  for (int k = 0; k < numPointsPerLine; ++k)
  {
    // Translate [0,numPointsPerLine) into [0,1]
    const float pct = static_cast<float>(k) / (numPointsPerLine - 1);
    vertices.push_back(start + delta * pct);
  }
}


#if 0
osg::Geometry* createEllipsoid(double major, double minor, int segments, const osg::Vec4& color)
{
  osg::EllipsoidModel em(major, minor);

  osg::Geometry* geom = new osg::Geometry();
  geom->setUseVertexBufferObjects(true);

  int latSegments = osg::clampBetween(segments, 10, 100);
  int lonSegments = 2 * latSegments;

  double segmentSize = 180.0/(double)latSegments; // degrees

  osg::Vec3Array* verts = new osg::Vec3Array();
  verts->reserve(latSegments * lonSegments);

  osg::Vec3Array* normals = 0;
  normals = new osg::Vec3Array();
  normals->reserve(latSegments * lonSegments);
  geom->setNormalArray(normals);
  geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

  osg::Vec4Array* colors = new osg::Vec4Array();
  colors->push_back(color);
  geom->setColorArray(colors);
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);

  osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);
  el->reserve(latSegments * lonSegments * 6);

  for (int y = 0; y <= latSegments; ++y)
  {
    double lat = -90.0 + segmentSize * (double)y;
    for (int x = 0; x < lonSegments; ++x)
    {
      double lon = -180.0 + segmentSize * (double)x;
      double gx, gy, gz;
      em.convertLatLongHeightToXYZ(osg::DegreesToRadians(lat), osg::DegreesToRadians(lon), 0.0, gx, gy, gz);
      verts->push_back(osg::Vec3(gx, gy, gz));

      osg::Vec3 normal(gx, gy, gz);
      normal.normalize();
      normals->push_back(normal);

      if (y < latSegments)
      {
        int x_plus_1 = x < lonSegments-1 ? x+1 : 0;
        int y_plus_1 = y+1;
        el->push_back(y*lonSegments + x);
        el->push_back(y*lonSegments + x_plus_1);
        el->push_back(y_plus_1*lonSegments + x);
        el->push_back(y*lonSegments + x_plus_1);
        el->push_back(y_plus_1*lonSegments + x_plus_1);
        el->push_back(y_plus_1*lonSegments + x);
      }
    }
  }

  geom->setVertexArray(verts);
  geom->addPrimitiveSet(el);

  //        OSG_ALWAYS << "s_makeEllipsoidGeometry Bounds: " << geom->computeBound().radius() << " outerRadius: " << outerRadius << std::endl;

  return geom;
}
#endif

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

}