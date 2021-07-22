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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cassert>
#include "osg/Depth"
#include "osgEarth/NodeUtils"
#include "osgEarth/VirtualProgram"
#include "simNotify/Notify.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/String/Format.h"
#include "simVis/LocatorNode.h"
#include "simVis/OverheadMode.h"
#include "simVis/Scenario.h"
#include "simVis/Shaders.h"
#include "simVis/View.h"
#include "simVis/Constants.h"

namespace simVis {

/** User Data Container key value for whether overhead mode is enabled */
static const std::string OVERHEAD_MODE_TOKEN = "simSDK.OverheadModeEnabled";

/** String name of the uniform controlling flattening; uses \#define for shader use */
#define FLATTEN_UNIFORM "simVis_useFlattenShader"

namespace
{
  // Just for debugging. It will turn any flattened geometry Yellow.
  static const char* s_overheadModeDebugFS =
        "#version 330\n"
        "uniform bool " FLATTEN_UNIFORM ";\n"
        "void simVis_flatten_FS_debug(inout vec4 color) { \n"
        "    if (" FLATTEN_UNIFORM ") { \n"
        "        color.rgb = vec3(1,1,0); \n"
        "    } \n"
        "}\n";

  // Returns an indicitor callback found on the node, and if it's not
  // found, creates and installs it before returning it.
  OverheadMode::IndicatorCallback* getOrCreateIndicatorCallback(osg::Node* node)
  {
    osg::Callback* cb = node->getCullCallback();
    while (cb)
    {
      OverheadMode::IndicatorCallback* icb = dynamic_cast<OverheadMode::IndicatorCallback*>(cb);
      if (icb)
        return icb;
      cb = cb->getNestedCallback();
    }

    OverheadMode::IndicatorCallback* icb = new OverheadMode::IndicatorCallback();
    node->addCullCallback(icb);
    return icb;
  }

  // Callback that will clamp a GeoTransform's matrix to a certain radius. (earth radius + altitude).
  // GeoTransform nodes are used to position osgEarth LabelNode and PlaceNode, which in turn are
  // used in the SDK by the EntityLabelNode and the TextAnnotation GOG, for example.
  struct ClampMatrixCallback : public osgEarth::GeoTransform::ComputeMatrixCallback
  {
    virtual bool computeLocalToWorldMatrix(const osgEarth::GeoTransform* xform, osg::Matrix& m, osg::NodeVisitor* nv) const
    {
      osg::Matrix matrix = xform->getMatrix();

      if (simVis::OverheadMode::isActive(nv))
      {
        osg::Vec3d trans = matrix.getTrans();
        trans.normalize();
        trans *= simVis::OverheadMode::getClampingRadius(trans.z());
        matrix.setTrans(trans);
      }

      m.preMult(matrix);

      return true;
    }
  };

  // Visitor that will dirty the bounds of all LocatorNodes and GeoTransforms
  // so that they will cull properly during a switchover.
  // NOTE: this doesn't seem to work. Could be an order or operations issue,
  // whereby the dirty is happening before the matrix selection changes. Not sure.
  struct DirtyBoundVisitor : public osg::NodeVisitor
  {
    void apply(osg::Node& node)
    {
      simVis::LocatorNode* locator = dynamic_cast<simVis::LocatorNode*>(&node);
      if (locator)
      {
        locator->dirtyBound();
        ++_count;
      }

      osgEarth::GeoTransform* geoxform = dynamic_cast<osgEarth::GeoTransform*>(&node);
      if (geoxform)
      {
        geoxform->dirtyBound();
        ++_count;
      }

      traverse(node);
    }

    DirtyBoundVisitor()
      : _count(0)
    {
      setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
      setNodeMaskOverride(~0);
    }
    int _count;
  };

  /**
   * Cull callback for ocean layers that will change the stateset
   * when in overhead mode
   */
  class OceanOverheadModeCallback : public osgEarth::Layer::TraversalCallback
  {
  public:
    osg::ref_ptr<osg::StateSet> _stateset;

    OceanOverheadModeCallback()
      : _stateset(new osg::StateSet())
    {
      // draw the ocean in the same render bin as the terrain
      _stateset->setRenderBinDetails(simVis::BIN_TERRAIN, simVis::BIN_GLOBAL_SIMSDK, osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
      // disable depth buffer writes
      _stateset->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false));
    }

    void operator()(osg::Node* node, osg::NodeVisitor* nv) const
    {
      if (simVis::OverheadMode::isActive(nv))
      {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        cv->pushStateSet(_stateset.get());
        traverse(node, nv);
        cv->popStateSet();
      }
      else
      {
        traverse(node, nv);
      }
    }
  };
}

void OverheadMode::install(osg::Node* root)
{
  if (root)
  {
    // default setting is to NOT flatten geometry.
    // call enableGeometryFlattening on a node to flatten it in overhead mode.
    root->getOrCreateStateSet()->addUniform(new osg::Uniform(FLATTEN_UNIFORM, false));
  }
}

void OverheadMode::uninstall(osg::Node* root)
{
  if (root)
  {
    osg::StateSet* ss = root->getStateSet();
    if (ss)
      ss->removeUniform(FLATTEN_UNIFORM);
  }
}

void OverheadMode::setEnabled(bool enable, simVis::View* view)
{
  // OverheadMode requires a newer version of osgEarth that includes a compute matrix callback on GeoTransform.

  osg::Camera* viewCam = view->getCamera();
  if (!viewCam)
  {
    SIM_WARN << "Did not find a camera\n";
    return;
  }

  if (enable)
  {
    // Install a shader that transforms all vertices to the ellipsoid.
    osg::StateSet* ss = viewCam->getOrCreateStateSet();
    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(ss);
    simVis::Shaders package;
    package.load(vp, package.overheadModeVertex());

    ss->setDefine("SIMVIS_WGS_A", simCore::buildString("", simCore::WGS_A, 0, 1));
    ss->setDefine("SIMVIS_WGS_B", simCore::buildString("", simCore::WGS_B, 0, 10));

    // Uncomment the following line to help with debugging
    //vp->setFunction("simVis_flatten_FS_debug", s_overheadModeDebugFS, osgEarth::ShaderComp::LOCATION_FRAGMENT_COLORING);

    // The depth buffer code was removed per SIM-7317.  If the code must be added back in,
    // then turn depth buffer back on for the platform icons so they look correct.

    OverheadMode::IndicatorCallback* icb = getOrCreateIndicatorCallback(viewCam);
    icb->setEnabled(true);
  }
  else
  {
    osg::StateSet* ss = viewCam->getStateSet();
    if (ss)
    {
      osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::get(ss);
      if (vp)
      {
        simVis::Shaders package;
        package.unload(vp, package.overheadModeVertex());

        // Uncomment the following line to help with debugging
        //vp->removeShader("simVis_flatten_FS_debug");
      }
    }

    OverheadMode::IndicatorCallback* icb = getOrCreateIndicatorCallback(viewCam);
    icb->setEnabled(false);
  }
  DirtyBoundVisitor dirtyBound;
  viewCam->accept(dirtyBound);
  SIM_DEBUG << "[simVis::OverheadMode]  Count = " << dirtyBound._count << "\n";
}

double OverheadMode::getClampingRadius(double sinLat)
{
  // Check for domain error on the incoming value
  if (sinLat < -1.0 || sinLat > 1.0)
    return simCore::EARTH_RADIUS;
  return simCore::calculateEarthRadius(asin(sinLat));
}

void OverheadMode::enableGeometryFlattening(bool value, osg::Node* node)
{
  if (node)
  {
    node->getOrCreateStateSet()->addUniform(new osg::Uniform(FLATTEN_UNIFORM, value));
  }
}

void OverheadMode::enableGeoTransformClamping(bool value, osgEarth::GeoTransform* xform)
{
  if (xform)
  {
    xform->setComputeMatrixCallback(value ? new ClampMatrixCallback() : 0L);
  }
}

bool OverheadMode::isActive(osg::NodeVisitor* nv)
{
  if (!nv)
    return false;
  osg::UserDataContainer* udc = nv->getUserDataContainer();
  if (!udc)
    return false;
  bool value = false;
  if (!udc->getUserValue(OVERHEAD_MODE_TOKEN, value))
    return false;
  return value;
}

void OverheadMode::prepareVisitor(const simVis::View* view, osg::NodeVisitor* nv)
{
  if (nv && view)
  {
    osg::UserDataContainer* udc = nv->getOrCreateUserDataContainer();
    udc->setUserValue(OVERHEAD_MODE_TOKEN, view->isOverheadEnabled());
  }
}

///////////////////////////////////////////////////////////

OverheadMode::IndicatorCallback::IndicatorCallback()
  : osg::NodeCallback(),
    enabled_(false)
{
}

OverheadMode::IndicatorCallback::~IndicatorCallback()
{
}

void OverheadMode::IndicatorCallback::setEnabled(bool value)
{
  enabled_ = value;
}

void OverheadMode::IndicatorCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
  osg::UserDataContainer* udc = nv->getOrCreateUserDataContainer();
  udc->setUserValue(OVERHEAD_MODE_TOKEN, enabled_);
  traverse(node, nv);
}

void OverheadMode::configureOceanLayer(osgEarth::Layer* layer)
{
  layer->setCullCallback(new OceanOverheadModeCallback());
}

///////////////////////////////////////////////////////////

ToggleOverheadMode::ToggleOverheadMode(simVis::View* view, int key, int toggleClampKey)
  : view_(view),
    overheadKey_(key),
    toggleClampKey_(toggleClampKey)
{
}

ToggleOverheadMode::~ToggleOverheadMode()
{
}

void ToggleOverheadMode::setOverheadKey(int key)
{
  overheadKey_ = key;
}

void ToggleOverheadMode::setToggleClampingKey(int key)
{
  toggleClampKey_ = key;
}

bool ToggleOverheadMode::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (ea.getEventType() == ea.KEYDOWN && view_.valid())
  {
    if (overheadKey_ != 0 && ea.getKey() == overheadKey_)
    {
      view_->enableOverheadMode(!view_->isOverheadEnabled());
      return true;
    }
    if (toggleClampKey_ != 0 && ea.getKey() == toggleClampKey_)
    {
      view_->setUseOverheadClamping(!view_->useOverheadClamping());
      return true;
    }
  }
  return false;
}

}
