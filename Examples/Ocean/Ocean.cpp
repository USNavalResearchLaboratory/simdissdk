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
#include "simVis/DBOptions.h"
#include "simVis/osgEarthVersion.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simUtil/ExampleResources.h"

#include "osgEarthDrivers/tms/TMSOptions"
#ifdef HAVE_TRITON_NODEKIT
#include "osgEarthTriton/TritonLayer"
#include "osgEarthTriton/TritonOptions"
#include "simUtil/TritonSettings.h"
#endif
#include "osgEarthDrivers/ocean_simple/SimpleOceanOptions"
#ifdef HAVE_SILVERLINING_NODEKIT
#include "osgEarthSilverLining/SilverLiningNode"
#include "simUtil/SilverLiningSettings.h"
#endif
#include "osgEarthUtil/Sky"
#include "osgEarthUtil/Ocean"
#include "osgEarthDrivers/mbtiles/MBTilesOptions"

// Hawaii:
#define LAT             19.698193
#define LON           -156.20224
#define ALT              0.0
#define PLATFORM_SHIP "Ship"

using namespace osgEarth::Util;
using namespace osgEarth::Drivers;
namespace osgEarth { namespace Triton { class TritonLayer; } }

static simCore::Coordinate s_shipPosOri(simCore::COORD_SYS_LLA,
                                        simCore::Vec3(simCore::DEG2RAD*(LAT), simCore::DEG2RAD*(LON), ALT),
                                        simCore::Vec3(0.0, 0.0, 0.0));

static simData::ObjectId     s_shipId;


// An event handler to assist in testing Ocean
struct MenuHandler : public osgGA::GUIEventHandler
{
  osg::ref_ptr<simVis::Viewer> viewer_;
  osg::ref_ptr<simVis::SceneManager> scene_;
  osgEarth::Util::Controls::Control* menuControl_;

  MenuHandler(simVis::Viewer* viewer, simVis::SceneManager* scene, osgEarth::Util::Controls::Control* menuControl)
    : viewer_(viewer),
      scene_(scene),
      menuControl_(menuControl)
  { }

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
        if (ship != NULL)
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
        viewer_->getMainView()->tetherCamera(NULL);
        break;
      case 'h':
        if (menuControl_)
          menuControl_->setVisible(!menuControl_->visible());
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

namespace
{
  using namespace osgEarth::Util::Controls;

  static char s_menu[] =
    "0 : reset view (to ship)\n"
    "1 : untether camera\n"
    "h : toggle this menu\n";

#ifdef HAVE_TRITON_NODEKIT
  static osg::ref_ptr<simUtil::TritonSettingsAdapter> s_TritonSettings(new simUtil::TritonSettingsAdapter);
#endif /* HAVE_TRITON_NODEKIT */

  /** Applies an opacity value from a slider */
  class ApplyOpacity : public ControlEventHandler
  {
  public:
    ApplyOpacity(OceanNode* ocean, LabelControl* pctLabel)
      : ocean_(ocean),
        pctLabel_(pctLabel)
    {
    }

    ApplyOpacity(VisibleLayer* layer, LabelControl* pctLabel)
      : layer_(layer),
        pctLabel_(pctLabel)
    {
    }

    virtual void onValueChanged(Control* c, float value)
    {
      // Clamp between 0.f and 1.f
      value = simCore::sdkMax(0.f, simCore::sdkMin(1.f, value));
      if (ocean_.valid())
        ocean_->setAlpha(value);
      if (layer_.valid())
        layer_->setOpacity(value);
      if (pctLabel_.valid())
        pctLabel_->setText(Stringify() << static_cast<int>(value * 100.f) << "%");
    }

  private:
    osg::observer_ptr<OceanNode> ocean_;
    osg::observer_ptr<VisibleLayer> layer_;
    osg::observer_ptr<LabelControl> pctLabel_;
  };

  /** Applies a sea level value from a slider */
  class ApplySeaLevel : public ControlEventHandler
  {
  public:
    ApplySeaLevel(OceanNode* ocean, LabelControl* valueLabel)
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
    osg::observer_ptr<OceanNode> ocean_;
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
        sky_->setLighting(value);
    }

  private:
    osg::observer_ptr<SkyNode> sky_;
  };

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
      SilverLining::CloudLayer cloudLayer = SilverLining::CloudLayerFactory::Create(cloudType);
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

  Control* createMenu(OceanNode* simpleOcean, osgEarth::VisibleLayer* oceanLayer, SkyNode* skyNode, bool isTriton, bool isSilverLining)
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
    if (simpleOcean)
      opacitySlider = grid->setControl(1, row, new HSliderControl(-0.1f, 1.1f, 0.8f, new ApplyOpacity(simpleOcean, opacityPctLabel)));
    else
      opacitySlider = grid->setControl(1, row, new HSliderControl(-0.1f, 1.1f, 0.8f, new ApplyOpacity(oceanLayer, opacityPctLabel)));
    opacitySlider->setHorizFill(true, 250.0f);

    // Sea level
    ++row;
    grid->setControl(0, row, new LabelControl("Sea Level", TEXT_SIZE, WHITE));
    LabelControl* seaLevelLabel = grid->setControl(2, row, new LabelControl("0 m", TEXT_SIZE, WHITE));
    HSliderControl* seaLevelSlider = grid->setControl(1, row, new HSliderControl(-100.f, 100.f, 0.f, new ApplySeaLevel(simpleOcean, seaLevelLabel)));
    seaLevelSlider->setHorizFill(true, 250.0f);

    // Sky lighting
    ++row;
    grid->setControl(0, row, new LabelControl("Lighting", TEXT_SIZE));
    grid->setControl(1, row, new CheckBoxControl(true, new ToggleLighting(skyNode)));

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

  /** Factory for a sky node */
  SkyNode* makeSky(simVis::SceneManager* scene, bool useSilverLining, const std::string& slUser = "", const std::string& slLicense = "", const std::string& resourcePath = "")
  {
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

#ifdef HAVE_SILVERLINING_NODEKIT
      s_SlSettings->lensFlare()->set(true);

      // Configure clouds with the SilverLining callback settings
      s_SlSettings->addValue(s_CloudManager.get());
      return new SilverLining::SilverLiningNode(scene->getMapNode()->getMapSRS(), skyOptions, s_SlSettings.get());
#else
      return SkyNode::create(ConfigOptions(skyOptions), scene->getMapNode());
#endif /* HAVE_SILVERLINING_NODEKIT */
    }
    Config skyOptions;
    skyOptions.set("driver", "simple");
    return SkyNode::create(ConfigOptions(skyOptions), scene->getMapNode());
  }

#ifdef HAVE_TRITON_NODEKIT
  osgEarth::Triton::TritonLayer* makeTriton(const std::string& tritonUser = "", const std::string& tritonLicense = "", const std::string& resourcePath = "")
  {
    osgEarth::Triton::TritonOptions triton;
    if (!tritonUser.empty())
      triton.user() = tritonUser;
    if (!tritonLicense.empty())
      triton.licenseCode() = tritonLicense;
    if (!resourcePath.empty())
      triton.resourcePath() = resourcePath;

    triton.useHeightMap() = false;
    triton.maxAltitude() = 30000.0f;
    triton.renderBinNumber() = simVis::BIN_OCEAN;

#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,9,0)
    // Only newest osgEarth has callback support
    osgEarth::Triton::TritonLayer* rv = new osgEarth::Triton::TritonLayer(triton, s_TritonSettings.get());
#else
    osgEarth::Triton::TritonLayer* rv = new osgEarth::Triton::TritonLayer(triton);
#endif
    rv->setOpacity(0.8f);
    return rv;
  }
#endif

  /** Factory an ocean node */
  OceanNode* makeSimpleOcean(osgEarth::MapNode* mapNode)
  {
    osgEarth::Drivers::SimpleOcean::SimpleOceanOptions ocean;
    ocean.maxAltitude() = 30000.0f;
    ocean.lowFeatherOffset() = 0.0f;
    ocean.highFeatherOffset() = 1.0f;
    ocean.renderBinNumber() = simVis::BIN_OCEAN;
    OceanNode* rv = OceanNode::create(ocean, mapNode);
    rv->setAlpha(0.8f);
    return rv;
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

  // Construct a map.
  osg::ref_ptr<osgEarth::Map> map = new Map();

  // worldwide imagery layer:
  {
    TMSOptions options;
    options.url() = EXAMPLE_GLOBAL_IMAGERY_LAYER_TMS;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    map->addLayer(new ImageLayer("simdis.imagery", options));
#else
    map->addImageLayer(new ImageLayer("simdis.imagery", options));
#endif
  }

  // If we are testing the bathymetry offset, only load a Kauai inset.
  if (bathymetryOffset != 0.0f)
  {
    osgEarth::Drivers::MBTilesTileSourceOptions options;
    options.filename() = simExamples::getSampleDataPath() + "/terrain/" + EXAMPLE_ELEVATION_LAYER_DB;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    map->addLayer(new ElevationLayer("simdis.elevation.no.bathy", options));
#else
    map->addElevationLayer(new ElevationLayer("simdis.elevation.no.bathy", options));
#endif
  }
  else
  {
    // Otherwise load the global elevation layer with bathymetry.
    TMSOptions options;
    options.url() = EXAMPLE_ELEVATION_LAYER_TMS;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    map->addLayer(new ElevationLayer("simdis.elevation", options));
#else
    map->addElevationLayer(new ElevationLayer("simdis.elevation", options));
#endif
  }

  // start up a SIMDIS viewer
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // the data store houses the entity data model:
  simData::MemoryDataStore dataStore;
  simVis::ScenarioDataStoreAdapter adapter(&dataStore, scene->getScenario());

  // create our ship:
  s_shipId = createShip(dataStore);

  // add a sky to the scene.
  osg::ref_ptr<SkyNode> sky = makeSky(scene.get(), useSilverLining, sluser, sllicense, slpath);
  sky->attach(viewer->getMainView());
  sky->setDateTime(osgEarth::Util::DateTime(2014, 4, 22, 16.5));
  sky->setMinimumAmbient(simVis::Color::Gray);
  scene->setSkyNode(sky.get());

  // add an ocean surface to the scene.
  osg::ref_ptr<OceanNode> simpleOcean;
  osg::ref_ptr<osgEarth::VisibleLayer> tritonLayer;
#ifdef HAVE_TRITON_NODEKIT
  if (useTriton)
  {
    tritonLayer = makeTriton(tritonuser, tritonlicense, tritonpath);
    viewer->getSceneManager()->getMap()->addLayer(tritonLayer.get());
  }
  else
#endif
  {
    simpleOcean = makeSimpleOcean(scene->getMapNode());
    scene->setOceanNode(simpleOcean.get());
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
  osg::observer_ptr<simVis::PlatformNode> shipNode = scene->getScenario()->find<simVis::PlatformNode>(s_shipId);
  viewer->getMainView()->tetherCamera(shipNode.get());
  viewer->getMainView()->setFocalOffsets(80.0, -10.0, 2000.0);

  // install an on-screen menu
  Control* menu = createMenu(simpleOcean.get(), tritonLayer.get(), sky.get(), useTriton, useSilverLining);
  viewer->getMainView()->addOverlayControl(menu);

  // install the handler for the demo keys in the notify() above
  viewer->addEventHandler(new MenuHandler(viewer.get(), scene.get(), menu));

  viewer->installDebugHandlers();
  viewer->run();
}
