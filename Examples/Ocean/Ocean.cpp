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

// NOTE: Using Triton may require an installation of the DirectX End-User Runtime Web Installer:
//   https://www.microsoft.com/en-US/Download/details.aspx?id=35
// Triton depends on an older DirectX implementation that may not come preinstalled on all machines.
// If you try to use Triton and get an error about d3dx9_43.dll missing, you need this download.

/**
 * OCEAN EXAMPLE - SIMDIS SDK
 *
 * Loads a terrain altitude set and enables the osgEarth Ocean processing,
 * demonstrating the sea surface features.
 *
 * NOTE:  An Internet connection is required for this example.
 */
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simData/MemoryDataStore.h"

#include "simVis/BathymetryGenerator.h"
#include "simVis/osgEarthVersion.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simVis/Constants.h"
#include "simVis/OverheadMode.h"
#include "simUtil/ExampleResources.h"

#include "osg/Depth"

#include "osgEarth/SimpleOceanLayer"
#include "osgEarth/MBTiles"
#include "osgEarth/Sky"
#include "osgEarth/TMS"
#include "osgEarth/Version"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif

#ifdef HAVE_TRITON_NODEKIT
#include "osgEarthTriton/TritonLayer"
#include "simUtil/TritonSettings.h"
#endif

#ifdef HAVE_SILVERLINING_NODEKIT
#include "osgEarthSilverLining/SilverLiningNode"
#include "simUtil/SilverLiningSettings.h"
#endif

// Hawaii near Kauai:
#define LAT             21.937611
#define LON           -159.793521
#define ALT              0.0
#define PLATFORM_SHIP "Ship"

using namespace osgEarth::Util;

#if OSGEARTH_VERSION_LESS_THAN(3,0,0)
using namespace osgEarth::Drivers;
namespace osgEarth { namespace Triton { class TritonLayer; } }
#endif

static simCore::Coordinate s_shipPosOri(simCore::COORD_SYS_LLA,
                                        simCore::Vec3(simCore::DEG2RAD*(LAT), simCore::DEG2RAD*(LON), ALT),
                                        simCore::Vec3(0.0, 0.0, 0.0));

static simData::ObjectId     s_shipId;


#ifdef HAVE_TRITON_NODEKIT
// cull callback that adds buoyancy to a platform
// using its offset transform - this is not really
// appropriate in the long run since the offset xform
// is used for other things, but it works for a quick demo
class PlatformBuoyancyCallback : public osg::NodeCallback
{
public:
  explicit PlatformBuoyancyCallback(osgEarth::Triton::TritonLayer* triton) :
    triton_(triton),
    enabled_(false),
    reset_(false)
  {
    srs_ = osgEarth::SpatialReference::get("wgs84");
    isect_ = new osgEarth::Triton::TritonIntersections();
    isect_->addLocalPoint(osg::Vec3d(0, 0, 0));
    triton_->addIntersections(isect_.get());
  }

  void setEnabled(bool enable)
  {
    if (enabled_ && !enable)
    {
      reset_ = true;
    }
    enabled_ = enable;
  }

  bool enabled() const
  {
    return enabled_;
  }

  virtual ~PlatformBuoyancyCallback()
  {
    // removeIntersections is not currently exposed in the Triton Layer API
    //osg::ref_ptr<osgEarth::Triton::TritonLayer> triton;
    //if (triton_.lock(triton))
    //  triton_->removeIntersections(isect_.get());
  }

  void operator()(osg::Node* node, osg::NodeVisitor* nv)
  {
    if (enabled_ || reset_)
    {
      simVis::PlatformNode* platform = dynamic_cast<simVis::PlatformNode*>(node);
      if (platform)
      {
        osg::MatrixTransform* xform = dynamic_cast<osg::MatrixTransform*>(platform->getModel()->offsetNode());
        if (reset_)
        {
          xform->setMatrix(osg::Matrix::identity());
          reset_ = false;
        }
        else
        {
          simCore::Vec3 pos;
          platform->getPosition(&pos, simCore::COORD_SYS_LLA);
          osgEarth::GeoPoint anchor(srs_.get(), simCore::RAD2DEG*pos.lon(), simCore::RAD2DEG*pos.lat(), 0);
          isect_->setAnchor(anchor);

          xform->setMatrix(
            osg::Matrix::translate(0, 0, isect_->getHeights()[0]) *
            osg::Matrix::rotate(osg::Vec3d(0,0,1), isect_->getNormals()[0]) );
        }
      }
    }

    traverse(node, nv);
  }

  osg::ref_ptr<const osgEarth::SpatialReference> srs_;
  osg::ref_ptr<osgEarth::Triton::TritonIntersections> isect_;
  osg::observer_ptr<osgEarth::Triton::TritonLayer> triton_;
  bool enabled_;
  bool reset_;
};
#else
// Provide a typedef for the callback since Triton isn't available
typedef osg::NodeCallback PlatformBuoyancyCallback;
#endif

// An event handler to assist in testing Ocean
struct MenuHandler : public osgGA::GUIEventHandler
{
  osg::observer_ptr<simVis::Viewer> viewer_;
  osg::observer_ptr<simVis::SceneManager> scene_;
#ifndef HAVE_IMGUI
  osgEarth::Util::Controls::Control* menuControl_;

  MenuHandler(simVis::Viewer* viewer, simVis::SceneManager* scene, osgEarth::Util::Controls::Control* menuControl)
    : viewer_(viewer),
      scene_(scene),
      menuControl_(menuControl)
  { }
#else
  MenuHandler(simVis::Viewer* viewer, simVis::SceneManager* scene)
    : viewer_(viewer),
    scene_(scene)
  { }
#endif

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    bool handled = false;

    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
      case '0':
      {
        osg::observer_ptr<simVis::PlatformNode> ship = scene_->getScenario()->find<simVis::PlatformNode>(s_shipId);
        if (ship != nullptr)
        {
          viewer_->getMainView()->tetherCamera(ship.get());
          viewer_->getMainView()->setFocalOffsets(0, -10.0, 20000.0, 2.5);
        }
        else
        {
          SIM_WARN << "Cannot find the Ship's platform" << std::endl;
        }
        break;
      }
      case '1':
        viewer_->getMainView()->tetherCamera(nullptr);
        break;
#ifndef HAVE_IMGUI
      case 'h':
        if (menuControl_)
          menuControl_->setVisible(!menuControl_->visible());
        break;
#endif
      case 'a':
          viewer_->setLogarithmicDepthBufferEnabled(
              !viewer_->isLogarithmicDepthBufferEnabled());
          break;
      }
    }
    return handled;
  }
};

static simData::ObjectId createShip(simData::DataStore& dataStore)
{
  simData::ObjectId result;

  // create the platform in the database:
  simData::DataStore::Transaction transaction;
  {
    simData::PlatformProperties* props = dataStore.addPlatform(&transaction);
    result = props->id();
    transaction.complete(&props);
  }

  // initialize the prefs:
  {
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(result, &transaction);
    prefs->mutable_commonprefs()->set_name(PLATFORM_SHIP);
    prefs->set_icon(EXAMPLE_SHIP_ICON);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    transaction.complete(&prefs);
  }

  // give it a starting position:
  {
    // convert to ECEF:
    simCore::Coordinate ecef;
    simCore::CoordinateConverter::convertGeodeticToEcef(s_shipPosOri, ecef);

    simData::PlatformUpdate* newUpdate = dataStore.addPlatformUpdate(result, &transaction);
    newUpdate->set_x(ecef.x());
    newUpdate->set_y(ecef.y());
    newUpdate->set_z(ecef.z());
    newUpdate->set_psi(ecef.psi());
    newUpdate->set_theta(ecef.theta());
    newUpdate->set_phi(ecef.phi());
    newUpdate->set_time(0.0);
    transaction.complete(&newUpdate);
  }

  dataStore.update(0.0);

  return result;
}

static char s_menu[] =
"0 : reset view (to ship)\n"
"1 : untether camera\n"
"h : toggle this menu\n";

#ifdef HAVE_TRITON_NODEKIT
static osg::ref_ptr<simUtil::TritonSettingsAdapter> s_TritonSettings(new simUtil::TritonSettingsAdapter);
#endif /* HAVE_TRITON_NODEKIT */

namespace
{
#ifndef HAVE_IMGUI
  using namespace osgEarth::Util::Controls;

  /** Applies an opacity value from a slider */
  class ApplyOpacity : public ControlEventHandler
  {
  public:
    ApplyOpacity(VisibleLayer* layer, LabelControl* pctLabel)
      : layer_(layer),
        pctLabel_(pctLabel)
    {
    }

    virtual void onValueChanged(Control* c, float value)
    {
      // Clamp between 0.f and 1.f
      value = simCore::sdkMax(0.f, simCore::sdkMin(1.f, value));
      if (layer_.valid())
        layer_->setOpacity(value);
      if (pctLabel_.valid())
        pctLabel_->setText(Stringify() << static_cast<int>(value * 100.f) << "%");
    }

  private:
    osg::observer_ptr<VisibleLayer> layer_;
    osg::observer_ptr<LabelControl> pctLabel_;
  };

  /** Applies a sea level value from a slider */
  class ApplySeaLevel : public ControlEventHandler
  {
  public:
    ApplySeaLevel(SimpleOceanLayer* ocean, LabelControl* valueLabel)
      : ocean_(ocean),
        valueLabel_(valueLabel)
    {
    }

    virtual void onValueChanged(Control* c, float value)
    {
      if (ocean_.valid())
        ocean_->setSeaLevel(value);
      if (valueLabel_.valid())
        valueLabel_->setText(Stringify() << value << " m");
    }

  private:
    osg::observer_ptr<SimpleOceanLayer> ocean_;
    osg::observer_ptr<LabelControl> valueLabel_;
  };

  /** Check or uncheck to toggle sky lighting */
  class ToggleLighting : public ControlEventHandler
  {
  public:
    explicit ToggleLighting(SkyNode* sky)
      : sky_(sky)
    {
    }
    virtual void onValueChanged(Control* control, bool value)
    {
      if (sky_.valid())
      {
        if (value)
          sky_->setLighting(osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
        else
          sky_->setLighting(osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);
      }
    }

  private:
    osg::observer_ptr<SkyNode> sky_;
  };

  /** Check or uncheck to toggle overhead mode */
  class ToggleOverheadMode : public ControlEventHandler
  {
  public:
    explicit ToggleOverheadMode(simVis::View* view)
      : view_(view)
    {
    }
    virtual void onValueChanged(Control* control, bool value)
    {
      if (view_.valid())
        view_->enableOverheadMode(!view_->isOverheadEnabled());
    }

  private:
    osg::observer_ptr<simVis::View> view_;
  };

#ifdef HAVE_TRITON_NODEKIT
  /** Toggler for the Triton buoyancy simulation */
  class ToggleBuoyancySimulation : public ControlEventHandler
  {
  public:
    explicit ToggleBuoyancySimulation(PlatformBuoyancyCallback* cb)
      : _cb(cb)
    {
    }
    virtual void onValueChanged(Control* control, bool value)
    {
      _cb->setEnabled(value);
    }
  private:
    osg::ref_ptr<PlatformBuoyancyCallback> _cb;
  };
#endif /* HAVE_TRITON_NODEKIT */
#endif /* HAVE_IMGUI */

#ifdef HAVE_SILVERLINING_NODEKIT
  osg::ref_ptr<simUtil::SilverLiningSettingsAdapter> s_SlSettings = new simUtil::SilverLiningSettingsAdapter;

  /** Adds and removes clouds in the SL Callback code */
  class CloudManager : public simUtil::SilverLiningValue
  {
  public:
    /** Remove all clouds on next round */
    void clearClouds()
    {
      clearClouds_ = true;
      setShouldApply_();
    }
    /** Add a new cloud type on next round */
    void addCloudType(SilverLining::CloudTypes cloudType)
    {
      addCloud_ = cloudType;
      setShouldApply_();
    }

  protected:
    virtual ~CloudManager() {}

    /** Clears out then applies clouds as needed */
    virtual void apply_(osgEarth::SilverLining::Atmosphere& atmosphere)
    {
      if (clearClouds_.isSet())
      {
        atmosphere.GetConditions().RemoveAllCloudLayers();
        clearClouds_.unset();
      }
      if (addCloud_.isSet())
      {
        addCloudLayer_(atmosphere, addCloud_.get());
        addCloud_.unset();
      }
    }

  private:
    /** Adds a reasonably initialized cloud layer */
    void addCloudLayer_(SilverLining::Atmosphere& atmosphere, SilverLining::CloudTypes cloudType)
    {
#if OSGEARTH_VERSION_GREATER_OR_EQUAL(3,1,0)
      SilverLining::CloudLayer cloudLayer = SilverLining::CloudLayerFactory::Create(cloudType, atmosphere);
#else
      SilverLining::CloudLayer cloudLayer = SilverLining::CloudLayerFactory::Create(cloudType);
#endif
      cloudLayer.SetIsInfinite(true);
      cloudLayer.SetThickness(50);
      switch (cloudType)
      {
      case SilverLining::CUMULONIMBUS_CAPPILATUS:
        cloudLayer.SetBaseWidth(5000);
        cloudLayer.SetBaseLength(5000);
        break;
      default:
        cloudLayer.SetBaseWidth(100000);
        cloudLayer.SetBaseLength(100000);
        break;
      }
      switch (cloudType)
      {
      case SilverLining::CUMULUS_CONGESTUS:
      case SilverLining::CUMULUS_CONGESTUS_HI_RES:
      case SilverLining::CUMULUS_MEDIOCRIS:
        cloudLayer.SetBaseAltitude(2500);
        break;
      case SilverLining::CUMULONIMBUS_CAPPILATUS:
        cloudLayer.SetBaseAltitude(400);
        break;
      case SilverLining::STRATUS:
        cloudLayer.SetBaseAltitude(1250);
        cloudLayer.SetThickness(1500);
        break;
      case SilverLining::STRATOCUMULUS:
        cloudLayer.SetBaseAltitude(1250);
        cloudLayer.SetThickness(3000);
        break;
      case SilverLining::CIRRUS_FIBRATUS:
        cloudLayer.SetBaseAltitude(8000);
        break;
      case SilverLining::SANDSTORM:
        cloudLayer.SetBaseAltitude(0.0);
        cloudLayer.SetThickness(0);
        break;
      default:
        cloudLayer.SetBaseAltitude(3000);
        break;
      }
      cloudLayer.SetDensity(1.0);
      cloudLayer.SetLayerPosition(0, 0);
      cloudLayer.SetFadeTowardEdges(true);
      cloudLayer.SetAlpha(0.8);
      cloudLayer.SetCloudAnimationEffects(0.1, false, 0, 0);
      cloudLayer.SeedClouds(atmosphere);
      atmosphere.GetConditions().AddCloudLayer(cloudLayer);
    }

    optional<bool> clearClouds_;
    optional<SilverLining::CloudTypes> addCloud_;
  };
  osg::ref_ptr<CloudManager> s_CloudManager = new CloudManager;

  /** Control handler that on click will add a specific cloud type */
  struct AddCloudType : public osgEarth::Util::Controls::ControlEventHandler
  {
    SilverLining::CloudTypes cloudType_;
    explicit AddCloudType(SilverLining::CloudTypes cloudType) : cloudType_(cloudType) {}
    void onClick(osgEarth::Util::Controls::Control*) { s_CloudManager->addCloudType(cloudType_); }
  };
  /** Control handler that on click will remove all clouds */
  struct ClearClouds : public osgEarth::Util::Controls::ControlEventHandler
  {
    void onClick(osgEarth::Util::Controls::Control*) { s_CloudManager->clearClouds(); }
  };
#endif

#ifdef HAVE_IMGUI
  // ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
  // the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
  // while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(250); func("##" label, __VA_ARGS__)

  class ControlPanel : public simExamples::SimExamplesGui
  {
  public:
    ControlPanel(osgEarth::SimpleOceanLayer* simpleOceanLayer, osgEarth::VisibleLayer* tritonLayer,
      PlatformBuoyancyCallback* buoyancyCallback, SkyNode* skyNode, simVis::View* view,
      bool useTriton, bool useSilverLining)
      : simExamples::SimExamplesGui("Ocean Demo"),
      simpleOceanLayer_(simpleOceanLayer),
      tritonLayer_(tritonLayer),
      buoyancyCallback_(buoyancyCallback),
      skyNode_(skyNode),
      view_(view),
      useTriton_(useTriton),
      useSilverLining_(useSilverLining)
    {
    }

  void draw(osg::RenderInfo& ri) override
  {
    if (!isVisible())
      return;

    if (firstDraw_)
    {
      ImGui::SetNextWindowPos(ImVec2(5, 25));
      firstDraw_ = false;
    }
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("0: reset view (to ship)");
    ImGui::Text("1: untether camera");

    if (ImGui::BeginTable("Table", 2))
    {
      // Opacity
      float opacity = opacity_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Opacity", &opacity_, 0.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
      if (opacity != opacity_)
      {
        if (simpleOceanLayer_.get())
          simpleOceanLayer_->setOpacity(opacity * 0.01f);
        else if (tritonLayer_.get())
          tritonLayer_->setOpacity(opacity * 0.01f);
      }

      // Lighting
      bool lighting = lighting_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Lighting", &lighting_);
      if (lighting != lighting_)
      {
        if (skyNode_.get())
          skyNode_->setLighting(osg::StateAttribute::OVERRIDE | (lighting_ ? osg::StateAttribute::ON : osg::StateAttribute::OFF));
      }

      // Overhead mode
      bool overhead = view_->isOverheadEnabled();
      IMGUI_ADD_ROW(ImGui::Checkbox, "Overhead Mode", &overhead);
      if (view_->isOverheadEnabled() != overhead)
        view_->enableOverheadMode(overhead);

      if (useTriton_)
      {
        ImGui::TableNextColumn();
        pushLargeFont_();
        ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Triton"); // Yellow
        popLargeFont_();
        ImGui::TableNextColumn();

        // Choppiness
        float choppiness = static_cast<float>(s_TritonSettings->choppiness()->value());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Choppiness", &choppiness, 0.f, 3.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (choppiness != static_cast<float>(s_TritonSettings->choppiness()->value()))
          s_TritonSettings->choppiness()->set(choppiness);

        // Note: simUtil::TritonSettingsAdapter's quality setter is a no-op, so don't provide the user a control for it

        // Wind direction
        float direction = static_cast<float>(s_TritonSettings->seaState()->windDirection());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Wind Direction", &direction, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (direction != static_cast<float>(s_TritonSettings->seaState()->windDirection()))
          s_TritonSettings->seaState()->setWindDirection(direction);

        // Sea state
        float seaState = static_cast<float>(s_TritonSettings->seaState()->seaState());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Sea State", &seaState, 0.f, 12.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (seaState != static_cast<float>(s_TritonSettings->seaState()->seaState()))
          s_TritonSettings->seaState()->setSeaState(seaState);

        // Sun Intensity
        float sunIntensity = static_cast<float>(s_TritonSettings->sunIntensity()->value());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Sun Intensity", &sunIntensity, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (sunIntensity != static_cast<float>(s_TritonSettings->sunIntensity()->value()))
          s_TritonSettings->sunIntensity()->set(sunIntensity);

        // Spray
        bool spray = s_TritonSettings->enableSpray()->value();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Spray", &spray);
        if (s_TritonSettings->enableSpray()->value() != spray)
          s_TritonSettings->enableSpray()->set(spray);

        // Wireframe
        bool wireframe = s_TritonSettings->enableWireframe()->value();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Wireframe", &wireframe);
        if (s_TritonSettings->enableWireframe()->value() != wireframe)
          s_TritonSettings->enableWireframe()->set(wireframe);

        // God rays
        bool godRays = s_TritonSettings->enableGodRays()->value();
        IMGUI_ADD_ROW(ImGui::Checkbox, "God Rays", &godRays);
        if (s_TritonSettings->enableGodRays()->value() != godRays)
          s_TritonSettings->enableGodRays()->set(godRays);

        // God rays fade
        float godRaysFade = static_cast<float>(s_TritonSettings->godRaysFade()->value());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "God Rays Fade", &godRaysFade, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (godRaysFade != static_cast<float>(s_TritonSettings->godRaysFade()->value()))
          s_TritonSettings->godRaysFade()->set(godRaysFade);

        // Platform Buoyancy
        bool platformBouyancy = buoyancyCallback_->enabled();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Platform Buoyancy", &platformBouyancy);
        if (buoyancyCallback_->enabled() != platformBouyancy)
          buoyancyCallback_->setEnabled(platformBouyancy);
      }

      if (useSilverLining_)
      {
        ImGui::TableNextColumn();
        pushLargeFont_();
        ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "SilverLining"); // Yellow
        popLargeFont_();
        ImGui::TableNextColumn();

        // Visibility
        float visibility = static_cast<float>(s_SlSettings->visibility()->value());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Visibility", &visibility, 100.f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (visibility != static_cast<float>(s_SlSettings->visibility()->value()))
          s_SlSettings->visibility()->set(visibility);

        // Turbidity
        float turbidity = static_cast<float>(s_SlSettings->turbidity()->value());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Turbidity", &turbidity, 1.8f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (turbidity != static_cast<float>(s_SlSettings->turbidity()->value()))
          s_SlSettings->turbidity()->set(turbidity);

        // Light Pollution
        float lightPollution = static_cast<float>(s_SlSettings->lightPollution()->value());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Light Pollution", &lightPollution, 0.f, 0.01f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (lightPollution != static_cast<float>(s_SlSettings->lightPollution()->value()))
          s_SlSettings->lightPollution()->set(lightPollution);

        // Rain
        float rainRate = static_cast<float>(s_SlSettings->rainRate()->value());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Rain", &rainRate, 0.f, 30.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (rainRate != static_cast<float>(s_SlSettings->rainRate()->value()))
          s_SlSettings->rainRate()->set(rainRate);

        // Snow
        float snowRate = static_cast<float>(s_SlSettings->snowRate()->rate());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Snow", &snowRate, 0.f, 30.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (snowRate != static_cast<float>(s_SlSettings->snowRate()->rate()))
          s_SlSettings->snowRate()->setRate(snowRate);

        // Wet Snow
        bool wetSnow = s_SlSettings->snowRate()->isWet();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Wet Snow", &wetSnow);
        if (s_SlSettings->snowRate()->isWet() != wetSnow)
          s_SlSettings->snowRate()->setWet(wetSnow);

        // Sleet
        float sleetRate = static_cast<float>(s_SlSettings->sleetRate()->value());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Sleet", &sleetRate, 0.f, 30.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (sleetRate != static_cast<float>(s_SlSettings->sleetRate()->value()))
          s_SlSettings->sleetRate()->set(sleetRate);

        // Lens Flare
        bool lensFlare = s_SlSettings->lensFlare()->value();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Lens Flare", &lensFlare);
        if (s_SlSettings->lensFlare()->value() != lensFlare)
          s_SlSettings->lensFlare()->set(lensFlare);

        // Gamma
        float gamma = static_cast<float>(s_SlSettings->gamma()->value());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Gamma", &gamma, 0.f, 6.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (gamma != static_cast<float>(s_SlSettings->gamma()->value()))
          s_SlSettings->gamma()->set(gamma);

        // Wind Speed
        float windSpeed = static_cast<float>(s_SlSettings->wind()->speed());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Wind Speed", &windSpeed, 0.f, 75.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (windSpeed != static_cast<float>(s_SlSettings->wind()->speed()))
          s_SlSettings->wind()->setSpeed(windSpeed);

        // Wind Direction
        float windDirection = static_cast<float>(s_SlSettings->wind()->direction());
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Wind Direction", &windDirection, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (windDirection != static_cast<float>(s_SlSettings->wind()->direction()))
          s_SlSettings->wind()->setDirection(windDirection);

        // Infrared
        bool infrared = s_SlSettings->infrared()->value();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Infrared", &infrared);
        if (s_SlSettings->infrared()->value() != infrared)
          s_SlSettings->infrared()->set(infrared);

        // Hosek-Wilkie
        osgEarth::SilverLining::Atmosphere::SkyModel skyModel = static_cast<osgEarth::SilverLining::Atmosphere::SkyModel>(s_SlSettings->skyModel()->value());
        bool hosekWilkie = (skyModel == osgEarth::SilverLining::Atmosphere::HOSEK_WILKIE);
        bool newValue = hosekWilkie;
        IMGUI_ADD_ROW(ImGui::Checkbox, "Hosek-Wilkie", &newValue);
        if (hosekWilkie != newValue)
          s_SlSettings->skyModel()->set(newValue ? osgEarth::SilverLining::Atmosphere::HOSEK_WILKIE : osgEarth::SilverLining::Atmosphere::PREETHAM);

        ImGui::TableNextColumn();
        ImGui::Text("Add Clouds");
        ImGui::TableNextColumn();

        // Cloud type combo box
        static const char* CLOUDTYPES[] = { "Cirrocumulus", "Cirrus Fibratus", "Stratus", "Cumulus Mediocris", "Cumulus Congestus",
        "Cumulus Congestus HiRes", "Cumulonimbus Cappilatus", "Stratocumulus", "Towering Cumulus", "Sandstorm" };
        static int currentCloudTypeIdx = 0;
        if (ImGui::BeginCombo("##cloudtype", CLOUDTYPES[currentCloudTypeIdx], 0))
        {
          for (int i = 0; i < IM_ARRAYSIZE(CLOUDTYPES); i++)
          {
            const bool isSelected = (currentCloudTypeIdx == i);
            if (ImGui::Selectable(CLOUDTYPES[i], isSelected))
              currentCloudTypeIdx = i;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }

        ImGui::SameLine();
        // Add clouds button
        if (ImGui::Button("Add"))
          s_CloudManager->addCloudType(static_cast<SilverLining::CloudTypes>(currentCloudTypeIdx));
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
          s_CloudManager->clearClouds();

        ImGui::TableNextColumn();
        ImGui::Text("Presets");
        ImGui::TableNextColumn();
        // Presets combo box
        static const char* PRESETS[] = { "Fair", "Partly Cloudy", "Mostly Cloudy", "Overcast" };
        static int currentPresetIdx = 0;
        if (ImGui::BeginCombo("##presets", PRESETS[currentPresetIdx], 0))
        {
          for (int i = 0; i < IM_ARRAYSIZE(PRESETS); i++)
          {
            const bool isSelected = (currentPresetIdx == i);
            if (ImGui::Selectable(PRESETS[i], isSelected))
              currentPresetIdx = i;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }

        ImGui::SameLine();
        // Apply preset button
        if (ImGui::Button("Apply"))
          s_SlSettings->conditionPreset()->set(static_cast<osgEarth::SilverLining::AtmosphericConditions::ConditionPresets>(currentPresetIdx));
      }

      ImGui::EndTable();
    }

    ImGui::End();
  }

  private:
    osg::observer_ptr<osgEarth::SimpleOceanLayer> simpleOceanLayer_;
    osg::observer_ptr<osgEarth::VisibleLayer> tritonLayer_;
    PlatformBuoyancyCallback* buoyancyCallback_;
    osg::observer_ptr<SkyNode> skyNode_;
    osg::observer_ptr<simVis::View> view_;
    bool useTriton_ = false;
    bool useSilverLining_ = false;
    float opacity_ = 80.f;
    bool lighting_ = true;
};

#else
  Control* createMenu(osgEarth::SimpleOceanLayer* simpleOceanLayer, osgEarth::VisibleLayer* tritonLayer,
                      PlatformBuoyancyCallback* buoyancyCallback, SkyNode* skyNode, simVis::View* view,
                      bool isTriton, bool isSilverLining)
  {
    static const float TITLE_SIZE = 16.f;
    static const float TEXT_SIZE = 12.f;
    static const osg::Vec4f YELLOW(1.f, 1.f, 0.f, 1.f);
    static const osg::Vec4f WHITE(1.f, 1.f, 1.f, 1.f);

    VBox* b = new VBox();
    b->setBackColor(0, 0, 0, 0.5);
    b->addControl(new LabelControl("OCEAN DEMO", TITLE_SIZE, YELLOW));
    b->addControl(new LabelControl(s_menu, TEXT_SIZE));

    Grid* grid = b->addControl(new Grid);
    grid->setChildSpacing(1.f); // Decrease spacing because of so many controls

    // Opacity
    int row = 0;
    grid->setControl(0, row, new LabelControl("Opacity", TEXT_SIZE, WHITE));
    LabelControl* opacityPctLabel = grid->setControl(2, row, new LabelControl("80%", TEXT_SIZE, WHITE));
    // Provide a little buffer on either side so we can get to 0% and 100%...
    HSliderControl* opacitySlider;
    if (simpleOceanLayer)
      opacitySlider = grid->setControl(1, row, new HSliderControl(-0.1f, 1.1f, 0.8f, new ApplyOpacity(simpleOceanLayer, opacityPctLabel)));
    else
      opacitySlider = grid->setControl(1, row, new HSliderControl(-0.1f, 1.1f, 0.8f, new ApplyOpacity(tritonLayer, opacityPctLabel)));
    opacitySlider->setHorizFill(true, 250.0f);

    // Sea level
    if (simpleOceanLayer)
    {
    ++row;
    grid->setControl(0, row, new LabelControl("Sea Level", TEXT_SIZE, WHITE));
    LabelControl* seaLevelLabel = grid->setControl(2, row, new LabelControl("0 m", TEXT_SIZE, WHITE));
    HSliderControl* seaLevelSlider = grid->setControl(1, row, new HSliderControl(-100.f, 100.f, 0.f, new ApplySeaLevel(simpleOceanLayer, seaLevelLabel)));
    seaLevelSlider->setHorizFill(true, 250.0f);
    }

    // Sky lighting
    ++row;
    grid->setControl(0, row, new LabelControl("Lighting", TEXT_SIZE));
    grid->setControl(1, row, new CheckBoxControl(true, new ToggleLighting(skyNode)));

    ++row;
    grid->setControl(0, row, new LabelControl("Overhead mode", TEXT_SIZE));
    grid->setControl(1, row, new CheckBoxControl(false, new ToggleOverheadMode(view)));

#ifdef HAVE_TRITON_NODEKIT
    if (isTriton)
    {
      // For Triton, we have several more settings...
      ++row;
      grid->setControl(0, row, new LabelControl("Triton", TITLE_SIZE, YELLOW));

      // Choppiness
      ++row;
      grid->setControl(0, row, new LabelControl("Choppiness", TEXT_SIZE, WHITE));
      ControlEventHandler* evtHandler = new simUtil::ChoppinessEventHandler(s_TritonSettings->choppiness());
      HSliderControl* slider = grid->setControl(1, row, new HSliderControl(0.f, 3.f, 3.f, evtHandler));
      slider->setHorizFill(true, 250.0f);
      slider->setValue(s_TritonSettings->choppiness()->value());
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE, WHITE));

      // Quality
      ++row;
      grid->setControl(0, row, new LabelControl("Quality", TEXT_SIZE, WHITE));
      evtHandler = new simUtil::QualityEventHandler(s_TritonSettings->quality());
      slider = grid->setControl(1, row, new HSliderControl(0.f, 3.f, 2.5f, evtHandler));
      slider->setHorizFill(true, 250.0f);
      LabelControl* qualityLabel = grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE, WHITE));
      slider->addEventHandler(new simUtil::QualityTextUpdater(qualityLabel));
      slider->setValue(s_TritonSettings->quality()->value());

      // Wind Direction
      ++row;
      grid->setControl(0, row, new LabelControl("Wind Direction", TEXT_SIZE, WHITE));
      evtHandler = new simUtil::WindDirectionDegEventHandler(s_TritonSettings->seaState());
      slider = grid->setControl(1, row, new HSliderControl(-180.f, 180, 1.f, evtHandler));
      slider->setHorizFill(true, 250.0f);
      slider->setValue(s_TritonSettings->seaState()->windDirection());
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE, WHITE));

      // Sea state
      ++row;
      grid->setControl(0, row, new LabelControl("Sea State", TEXT_SIZE, WHITE));
      evtHandler = new simUtil::SeaStateEventHandler(s_TritonSettings->seaState());
      slider = grid->setControl(1, row, new HSliderControl(0.f, 12.f, 12.f, evtHandler));
      slider->setHorizFill(true, 250.0f);
      slider->setValue(s_TritonSettings->seaState()->seaState());
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE, WHITE));

      // Sun Intensity
      ++row;
      grid->setControl(0, row, new LabelControl("Sun Intensity", TEXT_SIZE, WHITE));
      evtHandler = new simUtil::SunIntensityEventHandler(s_TritonSettings->sunIntensity());
      slider = grid->setControl(1, row, new HSliderControl(0.f, 1.f, 1.f, evtHandler));
      slider->setHorizFill(true, 250.0f);
      slider->setValue(s_TritonSettings->sunIntensity()->value());
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE, WHITE));

      // Spray
      ++row;
      grid->setControl(0, row, new LabelControl("Spray", TEXT_SIZE, WHITE));
      evtHandler = new simUtil::EnableSprayEventHandler(s_TritonSettings->enableSpray());
      grid->setControl(1, row, new CheckBoxControl(s_TritonSettings->enableSpray()->value(), evtHandler));

      // Wireframe
      ++row;
      grid->setControl(0, row, new LabelControl("Wireframe", TEXT_SIZE, WHITE));
      evtHandler = new simUtil::EnableWireframeEventHandler(s_TritonSettings->enableWireframe());
      grid->setControl(1, row, new CheckBoxControl(s_TritonSettings->enableWireframe()->value(), evtHandler));

      // God Rays
      ++row;
      grid->setControl(0, row, new LabelControl("God Rays", TEXT_SIZE, WHITE));
      evtHandler = new simUtil::EnableGodRaysEventHandler(s_TritonSettings->enableGodRays());
      grid->setControl(1, row, new CheckBoxControl(s_TritonSettings->enableGodRays()->value(), evtHandler));

      // God Ray fade
      ++row;
      grid->setControl(0, row, new LabelControl("God Ray Fade", TEXT_SIZE, WHITE));
      evtHandler = new simUtil::GodRaysFadeEventHandler(s_TritonSettings->godRaysFade());
      slider = grid->setControl(1, row, new HSliderControl(0.f, 1.f, 0.5f, evtHandler));
      slider->setHorizFill(true, 250.0f);
      slider->setValue(s_TritonSettings->godRaysFade()->value());
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE, WHITE));

      // Platform Buoyancy
      ++row;
      grid->setControl(0, row, new LabelControl("Platform Buoyancy", TEXT_SIZE, WHITE));
      evtHandler = new ToggleBuoyancySimulation(buoyancyCallback);
      grid->setControl(1, row, new CheckBoxControl(false, evtHandler));
    }
#endif /* HAVE_TRITON_NODEKIT */

#ifdef HAVE_SILVERLINING_NODEKIT
    if (isSilverLining)
    {
      // For SilverLining, we have several more settings...
      ++row;
      grid->setControl(0, row, new LabelControl("SilverLining", TITLE_SIZE, YELLOW));

      ++row;
      grid->setControl(0, row, new LabelControl("Visibility", TEXT_SIZE));
      HSliderControl* slider = grid->setControl(1, row, new HSliderControl(100.0f, 100000.0f, 30000.0f, new simUtil::VisibilityEventHandler(s_SlSettings->visibility())));
      slider->setHorizFill(true, 175);
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE));

      ++row;
      grid->setControl(0, row, new LabelControl("Turbidity", TEXT_SIZE));
      slider = grid->setControl(1, row, new HSliderControl(1.8f, 8.0f, 2.2f, new simUtil::TurbidityEventHandler(s_SlSettings->turbidity())));
      slider->setHorizFill(true, 175);
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE));

      ++row;
      grid->setControl(0, row, new LabelControl("Light Pollution", TEXT_SIZE));
      slider = grid->setControl(1, row, new HSliderControl(0.0f, 0.01f, 0.0f, new simUtil::LightPollutionEventHandler(s_SlSettings->lightPollution())));
      slider->setHorizFill(true, 175);
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE));

      ++row;
      grid->setControl(0, row, new LabelControl("Rain", TEXT_SIZE));
      slider = grid->setControl(1, row, new HSliderControl(0, 30.0, 0, new simUtil::RainRateEventHandler(s_SlSettings->rainRate())));
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE));
      ++row;
      grid->setControl(0, row, new LabelControl("Snow", TEXT_SIZE));
      slider = grid->setControl(1, row, new HSliderControl(0, 30.0, 0, new simUtil::SnowRateEventHandler(s_SlSettings->snowRate())));
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE));
      ++row;
      grid->setControl(0, row, new LabelControl("Wet Snow", TEXT_SIZE));
      grid->setControl(1, row, new CheckBoxControl(false, new simUtil::SnowIsWetEventHandler(s_SlSettings->snowRate())));
      ++row;
      grid->setControl(0, row, new LabelControl("Sleet", TEXT_SIZE));
      slider = grid->setControl(1, row, new HSliderControl(0, 30.0, 0, new simUtil::SleetRateEventHandler(s_SlSettings->sleetRate())));
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE));

      ++row;
      grid->setControl(0, row, new LabelControl("Lens Flare", TEXT_SIZE));
      grid->setControl(1, row, new CheckBoxControl(true, new simUtil::LensFlareEventHandler(s_SlSettings->lensFlare())));

      ++row;
      grid->setControl(0, row, new LabelControl("Gamma", TEXT_SIZE));
      slider = grid->setControl(1, row, new HSliderControl(0, 6.0, 1.8, new simUtil::GammaEventHandler(s_SlSettings->gamma())));
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE));
      ++row;
      grid->setControl(0, row, new LabelControl("Wind Speed", TEXT_SIZE));
      slider = grid->setControl(1, row, new HSliderControl(0, 75.0, 0.0, new simUtil::SlWindSpeedEventHandler(s_SlSettings->wind())));
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE));
      ++row;
      grid->setControl(0, row, new LabelControl("Wind Direction", TEXT_SIZE));
      slider = grid->setControl(1, row, new HSliderControl(-180, 180.0, 0, new simUtil::SlWindDirectionDegEventHandler(s_SlSettings->wind())));
      grid->setControl(2, row, new LabelControl(slider, TEXT_SIZE));
      ++row;
      grid->setControl(0, row, new LabelControl("Infra-Red", TEXT_SIZE));
      grid->setControl(1, row, new CheckBoxControl(false, new simUtil::InfraredEventHandler(s_SlSettings->infrared())));
      ++row;
      grid->setControl(0, row, new LabelControl("Hosek-Wilkie", TEXT_SIZE));
      grid->setControl(1, row, new CheckBoxControl(false, new simUtil::HosekWilkieToggleEventHandler(s_SlSettings->skyModel())));
      ++row;
      grid->setControl(0, row, new LabelControl("Add Clouds", TEXT_SIZE));
      HBox* cloudBoxes1 = grid->setControl(1, row, new HBox());
      ButtonControl* button = cloudBoxes1->addControl(new ButtonControl("Cirrocumulus", new AddCloudType(SilverLining::CIRROCUMULUS)));
      button->setFontSize(TEXT_SIZE);
      button = cloudBoxes1->addControl(new ButtonControl("Cirrus Fibratus", new AddCloudType(SilverLining::CIRRUS_FIBRATUS)));
      button->setFontSize(TEXT_SIZE);
      button = cloudBoxes1->addControl(new ButtonControl("Stratus", new AddCloudType(SilverLining::STRATUS)));
      button->setFontSize(TEXT_SIZE);
      button = cloudBoxes1->addControl(new ButtonControl("Cumulus Mediocris", new AddCloudType(SilverLining::CUMULUS_MEDIOCRIS)));
      button->setFontSize(TEXT_SIZE);
      ++row;
      HBox* cloudBoxes2 = grid->setControl(1, row, new HBox());
      button = cloudBoxes2->addControl(new ButtonControl("Cumulus Congestus", new AddCloudType(SilverLining::CUMULUS_CONGESTUS)));
      button->setFontSize(TEXT_SIZE);
      button = cloudBoxes2->addControl(new ButtonControl("Cumulus Congestus HiRes", new AddCloudType(SilverLining::CUMULUS_CONGESTUS_HI_RES)));
      button->setFontSize(TEXT_SIZE);
      ++row;
      HBox* cloudBoxes3 = grid->setControl(1, row, new HBox());
      button = cloudBoxes3->addControl(new ButtonControl("Cumolonimbus Cappilatus", new AddCloudType(SilverLining::CUMULONIMBUS_CAPPILATUS)));
      button->setFontSize(TEXT_SIZE);
      button = cloudBoxes3->addControl(new ButtonControl("Stratocumulus", new AddCloudType(SilverLining::STRATOCUMULUS)));
      button->setFontSize(TEXT_SIZE);
      ++row;
      HBox* cloudBoxes4 = grid->setControl(1, row, new HBox());
      button = cloudBoxes4->addControl(new ButtonControl("Towering Cumulus", new AddCloudType(SilverLining::TOWERING_CUMULUS)));
      button->setFontSize(TEXT_SIZE);
      button = cloudBoxes4->addControl(new ButtonControl("Sandstorm", new AddCloudType(SilverLining::SANDSTORM)));
      button->setFontSize(TEXT_SIZE);
      button = cloudBoxes4->addControl(new ButtonControl("Clear", new ClearClouds));
      button->setFontSize(TEXT_SIZE);
      ++row;
      grid->setControl(0, row, new LabelControl("Presets", TEXT_SIZE));
      HBox* condBoxes = grid->setControl(1, row, new HBox());
      button = condBoxes->addControl(new ButtonControl("Fair", new simUtil::SetConditionPresetEventHandler(s_SlSettings->conditionPreset(), osgEarth::SilverLining::AtmosphericConditions::FAIR)));
      button->setFontSize(TEXT_SIZE);
      button = condBoxes->addControl(new ButtonControl("Partly Cloudy", new simUtil::SetConditionPresetEventHandler(s_SlSettings->conditionPreset(), osgEarth::SilverLining::AtmosphericConditions::PARTLY_CLOUDY)));
      button->setFontSize(TEXT_SIZE);
      button = condBoxes->addControl(new ButtonControl("Mostly Cloudy", new simUtil::SetConditionPresetEventHandler(s_SlSettings->conditionPreset(), osgEarth::SilverLining::AtmosphericConditions::MOSTLY_CLOUDY)));
      button->setFontSize(TEXT_SIZE);
      button = condBoxes->addControl(new ButtonControl("Overcast", new simUtil::SetConditionPresetEventHandler(s_SlSettings->conditionPreset(), osgEarth::SilverLining::AtmosphericConditions::OVERCAST)));
      button->setFontSize(TEXT_SIZE);
    }
#endif /* HAVE_SILVERLINING_NODEKIT */

    return b;
  }
#endif /* HAVE_IMGUI */

  /** Factory for a sky node */
  SkyNode* makeSky(simVis::SceneManager* scene, bool useSilverLining, const std::string& slUser = "", const std::string& slLicense = "", const std::string& resourcePath = "")
  {
#ifdef HAVE_SILVERLINING_NODEKIT
    if (useSilverLining)
    {
      osgEarth::SilverLining::SilverLiningOptions skyOptions;
      if (!slUser.empty())
        skyOptions.user() = slUser;
      if (!slLicense.empty())
        skyOptions.licenseCode() = slLicense;
      if (!resourcePath.empty())
        skyOptions.resourcePath() = resourcePath;
      skyOptions.drawClouds() = true;
      skyOptions.cloudsMaxAltitude() = 100000.0;

      s_SlSettings->lensFlare()->set(true);

      // Configure clouds with the SilverLining callback settings
      s_SlSettings->addValue(s_CloudManager.get());
      return new SilverLining::SilverLiningNode(skyOptions, s_SlSettings.get());
    }
    else
    {
      return SkyNode::create();
    }
#else
    return SkyNode::create();
#endif /* HAVE_SILVERLINING_NODEKIT */
  }

#ifdef HAVE_TRITON_NODEKIT
  osgEarth::Triton::TritonLayer* makeTriton(const std::string& tritonUser = "", const std::string& tritonLicense = "", const std::string& resourcePath = "")
  {
    osgEarth::Triton::TritonLayer* rv = new osgEarth::Triton::TritonLayer();
    rv->setUserName(tritonUser);
    rv->setLicenseCode(tritonLicense);
    rv->setResourcePath(resourcePath);
    rv->setUseHeightMap(false);
    rv->setMaxAltitude(30000.0f);
    rv->setRenderBinNumber(simVis::BIN_OCEAN);
    rv->setOpacity(0.8f);
    simVis::OverheadMode::configureOceanLayer(rv);
    return rv;
  }
#endif

  /** Factory an ocean node */
  osgEarth::SimpleOceanLayer* makeSimpleOcean()
  {
    osgEarth::SimpleOceanLayer* ocean = new osgEarth::SimpleOceanLayer();
    ocean->setUseBathymetry(false);
    ocean->setMaxAltitude(30000.0f);
    ocean->setOpacity(0.8f);
    osg::StateSet* stateSet = ocean->getOrCreateStateSet();
    stateSet->setRenderBinDetails(simVis::BIN_OCEAN, simVis::BIN_GLOBAL_SIMSDK);
    stateSet->setDefine("SIMVIS_IGNORE_BATHYMETRY_GEN");
    return ocean;
  }
}

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();

  // check for ocean and sky options.
  osg::ArgumentParser ap(&argc, argv);

  // Read licensing arguments for SilverLining
  std::string sluser;
  std::string sllicense;
  std::string slpath = simExamples::getSilverLiningResourcesPath(); // SilverLining Resource Path
  osg::ArgumentParser::Parameter sluserArg(sluser);
  osg::ArgumentParser::Parameter sllicenseArg(sllicense);
  osg::ArgumentParser::Parameter slpathArg(slpath);
  bool useSilverLining = ap.read("--silverlining");
  ap.read("--sluser", sluserArg);
  ap.read("--sllicense", sllicenseArg);
  ap.read("--slpath", slpathArg);

  // Read licensing arguments for Triton
  std::string tritonuser;
  std::string tritonlicense;
  std::string tritonpath = simExamples::getTritonResourcesPath(); // Triton Resource Path
  osg::ArgumentParser::Parameter tritonuserArg(tritonuser);
  osg::ArgumentParser::Parameter tritonlicenseArg(tritonlicense);
  osg::ArgumentParser::Parameter tritonpathArg(tritonpath);
  bool useTriton = ap.read("--triton");
  ap.read("--tritonuser", tritonuserArg);
  ap.read("--tritonlicense", tritonlicenseArg);
  ap.read("--tritonpath", tritonpathArg);

  // Offset the bathymetry?
  float bathymetryOffset = 0.0f;
  ap.read("--bathymetryoffset", bathymetryOffset);

#ifdef HAVE_TRITON_NODEKIT
  // Set a default level for Triton
  if (ap.read("--good")) // default
    s_TritonSettings->quality()->set(osgEarth::Triton::GOOD);
  else if (ap.read("--better"))
    s_TritonSettings->quality()->set(osgEarth::Triton::BETTER);
  else if (ap.read("--best"))
    s_TritonSettings->quality()->set(osgEarth::Triton::BEST);
#endif /* HAVE_TRITON_NODEKIT */

  // Set up the search paths
  simExamples::configureSearchPaths();

  // start up a SIMDIS viewer, prior to creating the map, to make sure they destruct in proper order
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  osg::ref_ptr<osgEarth::Map> map = new Map();

  // worldwide imagery layer:
  {
    TMSImageLayer* layer = new TMSImageLayer();
    layer->setName("simdis.imagery");
    layer->setURL(EXAMPLE_GLOBAL_IMAGERY_LAYER_TMS);
    map->addLayer(layer);
  }

  // If we are testing the bathymetry offset, only load a Kauai inset.
  if (bathymetryOffset != 0.0f)
  {
    MBTilesElevationLayer* layer = new MBTilesElevationLayer();
    layer->setName("simdis.elevation.no.bathy");
    layer->setURL(simExamples::getSampleDataPath() + "/terrain/" + EXAMPLE_ELEVATION_LAYER_DB);
    map->addLayer(layer);
  }
  else
  {
    TMSElevationLayer* layer = new TMSElevationLayer();
    layer->setName("simdis.elevation");
    layer->setURL(EXAMPLE_ELEVATION_LAYER_TMS);
    map->addLayer(layer);
  }

  viewer->setMap(map.get());
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // the data store houses the entity data model:
  simData::MemoryDataStore dataStore;
  simVis::ScenarioDataStoreAdapter adapter(&dataStore, scene->getScenario());

  // create our ship:
  s_shipId = createShip(dataStore);
  osg::ref_ptr<simVis::PlatformNode> shipNode = scene->getScenario()->find<simVis::PlatformNode>(s_shipId);

  // add a sky to the scene.
  osg::ref_ptr<SkyNode> sky = makeSky(scene.get(), useSilverLining, sluser, sllicense, slpath);
  sky->attach(viewer->getMainView());
  sky->setDateTime(osgEarth::DateTime(2014, 4, 22, 16.5));
  sky->getSunLight()->setAmbient(simVis::Color::Gray);
  scene->setSkyNode(sky.get());

  // add an ocean surface to the scene.
  osg::ref_ptr<osgEarth::SimpleOceanLayer> simpleOceanLayer;
  osg::ref_ptr<osgEarth::VisibleLayer> tritonLayer;
  osg::ref_ptr<PlatformBuoyancyCallback> buoyancyCallback;

#ifdef HAVE_TRITON_NODEKIT
  if (useTriton)
  {
    osgEarth::Triton::TritonLayer* triton = makeTriton(tritonuser, tritonlicense, tritonpath);
    triton->setUserCallback(s_TritonSettings.get());
    viewer->getSceneManager()->getMap()->addLayer(triton);
    tritonLayer = triton;
    buoyancyCallback = new PlatformBuoyancyCallback(triton);
    shipNode->addCullCallback(buoyancyCallback.get());
  }
  else
#endif
  {
    simpleOceanLayer = makeSimpleOcean();
    if (simpleOceanLayer.valid())
        viewer->getSceneManager()->getMap()->addLayer(simpleOceanLayer.get());
  }

  // if we're using Triton, install a module to "sink" the MSL=0 terrain down, creating makeshift bathymetry
  if (bathymetryOffset != 0.0f)
  {
    SIM_NOTICE << "Bathymetry offset = " << -bathymetryOffset << "\n";
    simVis::BathymetryGenerator* bgen = new simVis::BathymetryGenerator();
    bgen->setOffset(-bathymetryOffset);
    scene->getMapNode()->getTerrainEngine()->addEffect(bgen);
  }

  // zoom to the starting AOI:
  viewer->getMainView()->tetherCamera(shipNode.get());
  viewer->getMainView()->setFocalOffsets(80.0, -10.0, 2000.0);

#ifndef HAVE_IMGUI
  // install an on-screen menu
  Control* menu = createMenu(
      simpleOceanLayer.get(),
      tritonLayer.get(),
      buoyancyCallback.get(),
      sky.get(),
      viewer->getMainView(),
      useTriton,
      useSilverLining);
  viewer->getMainView()->addOverlayControl(menu);
  // install the handler for the demo keys in the notify() above
  viewer->addEventHandler(new MenuHandler(viewer.get(), scene.get(), menu));
#else
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new ::GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  ::GUI::OsgImGuiHandler* gui = new ::GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(simpleOceanLayer.get(), tritonLayer.get(), buoyancyCallback.get(),
    sky.get(), viewer->getMainView(), useTriton, useSilverLining));
  viewer->addEventHandler(new MenuHandler(viewer.get(), scene.get()));
#endif

  viewer->installDebugHandlers();
  return viewer->run();
}
