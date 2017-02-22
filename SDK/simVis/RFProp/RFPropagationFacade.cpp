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
#include "osg/Depth"
#include "osgEarthUtil/ObjectLocator"
#include "simCore/Calc/Angle.h"
#include "simCore/EM/AntennaPattern.h"
#include "simNotify/Notify.h"
#include "simVis/Constants.h"
#include "simVis/RFProp/ArepsLoader.h"
#include "simVis/RFProp/OneWayPowerDataProvider.h"
#include "simVis/RFProp/PODProfileDataProvider.h"
#include "simVis/RFProp/ProfileManager.h"
#include "simVis/RFProp/Profile.h"
#include "simVis/RFProp/SNRDataProvider.h"
#include "simVis/RFProp/CompositeProfileProvider.h"
#include "simVis/RFProp/CompositeColorProvider.h"
#include "simVis/RFProp/GradientColorProvider.h"
#include "simVis/RFProp/RFPropagationFacade.h"

namespace
{

void setDefaultPODVector(simRF::PODVectorPtr podLossThresholds)
{
  // Set default detection thresholds - these are the same values used in SIMDIS 9
  {
    float pod[10] = {161.81f,   161.38f,   161.08f,   160.84f,   160.64f,   160.46f,   160.30f,   160.16f,   160.03f,   159.91f};
    podLossThresholds->insert(podLossThresholds->end(), pod, &pod[10]);
  }
  {
    float pod[10] = {159.79f,   159.68f,   159.58f,   159.48f,   159.39f,   159.29f,   159.21f,   159.12f,   159.04f,   158.96f};
    podLossThresholds->insert(podLossThresholds->end(), pod, &pod[10]);
  }
  {
    float pod[10] = {158.88f,   158.80f,   158.72f,   158.65f,   158.57f,   158.50f,   158.43f,   158.36f,   158.29f,   158.22f};
    podLossThresholds->insert(podLossThresholds->end(), pod, &pod[10]);
  }
  {
    float pod[10] = {158.15f,   158.08f,   158.01f,   157.95f,   157.88f,   157.81f,   157.75f,   157.68f,   157.61f,   157.54f};
    podLossThresholds->insert(podLossThresholds->end(), pod, &pod[10]);
  }
  {
    float pod[10] = {157.48f,   157.41f,   157.34f,   157.28f,   157.21f,   157.14f,   157.07f,   157.01f,   156.94f,   156.87f};
    podLossThresholds->insert(podLossThresholds->end(), pod, &pod[10]);
  }
  {
    float pod[10] = {156.80f,   156.73f,   156.66f,   156.58f,   156.51f,   156.44f,   156.36f,   156.29f,   156.21f,   156.13f};
    podLossThresholds->insert(podLossThresholds->end(), pod, &pod[10]);
  }
  {
    float pod[10] = {156.06f,   155.98f,   155.90f,   155.81f,   155.73f,   155.64f,   155.55f,   155.47f,   155.37f,   155.28f};
    podLossThresholds->insert(podLossThresholds->end(), pod, &pod[10]);
  }
  {
    float pod[10] = {155.18f,   155.08f,   154.98f,   154.88f,   154.77f,   154.66f,   154.54f,   154.42f,   154.30f,   154.17f};
    podLossThresholds->insert(podLossThresholds->end(), pod, &pod[10]);
  }
  {
    float pod[10] = {154.03f,   153.89f,   153.74f,   153.59f,   153.42f,   153.25f,   153.06f,   152.86f,   152.64f,   152.40f};
    podLossThresholds->insert(podLossThresholds->end(), pod, &pod[10]);
  }
  {
    float pod[10] = {152.14f,   151.86f,   151.53f,   151.16f,   150.73f,   150.20f,   149.53f,   148.60f,   147.04f,   147.04f};
    podLossThresholds->insert(podLossThresholds->end(), pod, &pod[10]);
  }
}
}

namespace simRF
{
const int DEFAULT_TRANSPARENCY = 60;  // percentage, 0-100. 100 is fully transparent, 0 is opaque
const int DEFAULT_HISTORY = 30; // degrees

RFPropagationFacade::RFPropagationFacade(simData::ObjectId id, osg::Group* parent, osgEarth::Map* map)
 : id_(id),
   antennaHeightMeters_(0.0),
   rfParamsSet_(false),
   parent_(parent)
{
  // create locator
  if (map)
    locator_ = new osgEarth::Util::ObjectLocatorNode(map);
  // add locator to the parent node
  if (locator_.valid() && parent_.valid())
    parent_->addChild(locator_);

  profileManager_ = new simRF::ProfileManager();

  initializeDefaultColors_();
  colorProvider_ = new CompositeColorProvider();

  profileManager_->setColorProvider(colorProvider_);

  // set the default threshold type
  setThresholdType(simRF::ProfileDataProvider::THRESHOLDTYPE_POD);

  // set the default visualization mode
  setDrawMode(simRF::Profile::DRAWMODE_2D_HORIZONTAL);

  podLossThresholds_ = PODVectorPtr(new std::vector<float>);
  setDefaultPODVector(podLossThresholds_);

  radarParameters_ = RadarParametersPtr(new RadarParameters());

  // set default transparency
  setTransparency(DEFAULT_TRANSPARENCY);

  // default history (in degrees), note that history is defined as CCW from current bearing
  setHistory(DEFAULT_HISTORY);

  setDisplay(false);
  if (locator_)
    locator_->addChild(profileManager_);
}

RFPropagationFacade::~RFPropagationFacade()
{
  if (parent_.valid() && locator_.valid())
    parent_->removeChild(locator_.get());
}

int RFPropagationFacade::setModelType()
{
  // TODO
  return 1;
}

int RFPropagationFacade::setRadarParams(const simRF::RadarParameters& radarParams)
{
  // copy the struct
  *radarParameters_ = radarParams;
  // noise power in db = 10 log (kT/pw); for T, use standard ambient temperature: 17°C/290K
  radarParameters_->noisePowerdB = simCore::linear2dB(4e-15 / radarParams.pulseWidth_uSec) + radarParams.noiseFiguredB;
  radarParameters_->xmtPowerW = radarParams.xmtPowerKW * 1e03;
  rfParamsSet_ = true;
  return 0;
}

const RadarParametersPtr RFPropagationFacade::radarParams() const
{
  return radarParameters_;
}

int RFPropagationFacade::setPODLossThreshold(const std::vector<float>& podLoss)
{
  if (podLoss.size() != PODProfileDataProvider::POD_VECTOR_SIZE)
    return 1;
  // podLoss Vector of 100 positive(implicitly negative) Loss thresholds(dB) for a probability of detection from 0 % to 100 %;
  // podLoss must contain 100 elements, and elements are expected to be ordered as positive decreasing values (implicitly negative increasing)
  // copy all 100, inverting sign (from positive to negative threshold values)
  for (size_t i = 0; i < 100; ++i)
  {
    // if assert fails, a plug-in attempted to specify a POD vector containing negative thresholds
    assert(podLoss[i] >= 0);
    (*podLossThresholds_)[i] = -podLoss[i];
  }
  return 0;
}

const PODVectorPtr RFPropagationFacade::getPODLossThreshold() const
{
  return podLossThresholds_;
}

int RFPropagationFacade::setColorMap(simRF::ProfileDataProvider::ThresholdType type, const std::map<float, osg::Vec4>& colorMap)
{
  bool updateColorProvider = false;
  simRF::ProfileDataProvider::ThresholdType currentType = profileManager_ ? profileManager_->getThresholdType() : simRF::ProfileDataProvider::THRESHOLDTYPE_POD;
  switch (type)
  {
  // these types always share the same color map
  case simRF::ProfileDataProvider::THRESHOLDTYPE_CNR:
  case simRF::ProfileDataProvider::THRESHOLDTYPE_SNR:
  case simRF::ProfileDataProvider::THRESHOLDTYPE_FACTOR:
  case simRF::ProfileDataProvider::THRESHOLDTYPE_ONEWAYPOWER:
    colorMaps_[simRF::ProfileDataProvider::THRESHOLDTYPE_CNR] = colorMap;
    colorMaps_[simRF::ProfileDataProvider::THRESHOLDTYPE_SNR] = colorMap;
    colorMaps_[simRF::ProfileDataProvider::THRESHOLDTYPE_FACTOR] = colorMap;
    colorMaps_[simRF::ProfileDataProvider::THRESHOLDTYPE_ONEWAYPOWER] = colorMap;
    updateColorProvider = currentType == simRF::ProfileDataProvider::THRESHOLDTYPE_CNR || currentType == simRF::ProfileDataProvider::THRESHOLDTYPE_SNR
      || currentType == simRF::ProfileDataProvider::THRESHOLDTYPE_FACTOR || currentType == simRF::ProfileDataProvider::THRESHOLDTYPE_ONEWAYPOWER;
    break;
  default:
    updateColorProvider = currentType == type;
    colorMaps_[type] = colorMap;
    break;
  }
  if (colorProvider_ == NULL || profileManager_ == NULL)
    return 0;

  // update the color provider if the specified type is currently active
  if (updateColorProvider)
    colorProvider_->setGradientColorMap(colorMap);
  return 0;
}

int RFPropagationFacade::setSlotData(simRF::Profile* profile)
{
  if (profile == NULL)
    return 1;
  if (profileManager_ == NULL)
  {
    assert(0);
    return 1;
  }
  profileManager_->addProfile(profile);
  profileList_.push_back(profile);
  return 0;
}

const simRF::Profile* RFPropagationFacade::getSlotData(double azRad) const
{
  return (profileManager_ == NULL) ? NULL : profileManager_->getProfileByBearing(azRad);
}

int RFPropagationFacade::getInputFiles(const simCore::TimeStamp& time, std::vector<std::string>& filenames) const
{
  std::map<simCore::TimeStamp, std::vector<std::string> >::const_iterator it =
    arepsFilesetTimeMap_.find(time);
  if (it != arepsFilesetTimeMap_.end())
  {
    filenames = it->second;
    return 0;
  }
  return 1;
}

int RFPropagationFacade::setDisplay(bool onOff)
{
  if (locator_ == NULL)
    return 1;
  profileManager_->setDisplay(onOff);
  return 0;
}

bool RFPropagationFacade::display() const
{
  return profileManager_->display();
}

int RFPropagationFacade::setRangeTool(bool option)
{
  return 1;
}

int RFPropagationFacade::setAglActive(bool aglActive)
{
  if (profileManager_ != NULL)
    return 1;
  profileManager_->setAGL(aglActive);
  return 0;
}

bool RFPropagationFacade::aglActive() const
{
  if (profileManager_ == NULL)
    return false;

  return profileManager_->getAGL();
}

int RFPropagationFacade::setDrawMode(simRF::Profile::DrawMode drawMode)
{
  if (profileManager_ == NULL)
    return 1;

  profileManager_->setMode(drawMode);
  return 0;
}

simRF::Profile::DrawMode RFPropagationFacade::drawMode() const
{
  if (profileManager_ == NULL)
    return Profile::DRAWMODE_2D_HORIZONTAL;
  return profileManager_->getMode();
}

int RFPropagationFacade::setHeight(double height)
{
  if (profileManager_ == NULL)
    return 1;

  profileManager_->setHeight(height);
  return 0;
}

double RFPropagationFacade::height() const
{
  if (profileManager_ == NULL)
    return 0.0;

  return profileManager_->getHeight();
}

int RFPropagationFacade::setThicknessBySlots(int numSlots)
{
  if (profileManager_ == NULL)
    return 1;

  return profileManager_->setThicknessBySlots(numSlots);
}

int RFPropagationFacade::setThickness(double thickness)
{
  if (profileManager_ == NULL)
    return 1;

  profileManager_->setDisplayThickness(static_cast<float>(thickness));
  return 0;
}

double RFPropagationFacade::thickness() const
{
  if (profileManager_ == NULL)
    return 0.0;

  return profileManager_->getDisplayThickness();
}

int RFPropagationFacade::setHistory(int length)
{
  if (profileManager_ == NULL)
    return 1;

  profileManager_->setHistory(static_cast<double>(length) * simCore::DEG2RAD);
  return 0;
}

int RFPropagationFacade::history() const
{
  if (profileManager_ == NULL)
    return 0;

  return static_cast<int>(simCore::rint(profileManager_->getHistory() * simCore::RAD2DEG));
}

int RFPropagationFacade::setTransparency(int transparency)
{
  if (profileManager_)
    profileManager_->setAlpha(1.f - transparency * 0.01f);
  return 0;
}

int RFPropagationFacade::transparency() const
{
  // Add 0.5f to round correctly; subtract from 1.f to convert alpha to transparency
  if (profileManager_)
    return static_cast<int>(0.5f + 100.f * (1.f - profileManager_->getAlpha()));
  return 0;
}

int RFPropagationFacade::setThresholdMode(simRF::ColorProvider::ColorMode mode)
{
  if (profileManager_ == NULL)
    return 1;
  if (!colorProvider_)
  {
    assert(false);
    return 1;
  }
  colorProvider_->setMode(mode);

  return 0;
}

simRF::ColorProvider::ColorMode RFPropagationFacade::thresholdMode() const
{
  if (profileManager_ == NULL || colorProvider_ == NULL)
    return simRF::ColorProvider::COLORMODE_BELOW;
  return colorProvider_->getMode();
}

int RFPropagationFacade::setThresholdType(simRF::ProfileDataProvider::ThresholdType type)
{
  if (profileManager_ == NULL)
    return 1;

  profileManager_->setThresholdType(type);

  setGradientByThresholdType_(type);

  return 0;
}

simRF::ProfileDataProvider::ThresholdType RFPropagationFacade::thresholdType() const
{
  if (profileManager_ == NULL)
    return simRF::ProfileDataProvider::THRESHOLDTYPE_POD;

  return profileManager_->getThresholdType();
}

int RFPropagationFacade::setThresholdValue(int value)
{
  if (profileManager_ == NULL)
    return 1;
  if (!colorProvider_)
  {
    assert(false);
    return 1;
  }
  colorProvider_->setThreshold(value);
  return 0;
}

int RFPropagationFacade::threshold() const
{
  if (profileManager_ == NULL || colorProvider_ == NULL)
    return 0;
  return static_cast<int>(colorProvider_->getThreshold());
}

int RFPropagationFacade::setAboveColor(const osg::Vec4f& color)
{
  if (profileManager_ == NULL)
    return 1;
  if (!colorProvider_)
  {
    assert(false);
    return 1;
  }
  colorProvider_->setAboveColor(color);
  return 0;
}

int RFPropagationFacade::aboveColor(osg::Vec4f& color)
{
  if (profileManager_ == NULL)
    return 1;
  if (!colorProvider_)
  {
    assert(false);
    return 1;
  }
  color = colorProvider_->getAboveColor();
  return 0;
}

int RFPropagationFacade::setBelowColor(const osg::Vec4f& color)
{
  if (profileManager_ == NULL)
    return 1;
  if (!colorProvider_)
  {
    assert(false);
    return 1;
  }
  colorProvider_->setBelowColor(color);
  return 0;
}

int RFPropagationFacade::belowColor(osg::Vec4f& color)
{
  if (profileManager_ == NULL)
    return 1;
  if (!colorProvider_)
  {
    assert(false);
    return 1;
  }
  color = colorProvider_->getBelowColor();
  return 0;
}

int RFPropagationFacade::clearCache(bool reset)
{
  setDisplay(false);
  arepsFilesetTimeMap_.clear();
  profileList_.clear();
  // clear out old (data from) ProfileManager, create a new empty ProfileManager
  profileManager_ = new simRF::ProfileManager();
  return 0;
}

double RFPropagationFacade::getPOD(double azimRad, double gndRngMeters, double hgtMeters) const
{
  const simRF::CompositeProfileProvider* cProvider = getProfileProvider(azimRad);
  if (!cProvider)
  {
    SIM_WARN << "No profile found for beam at requested bearing" << std::endl;
    return 0.0;
  }

  // we want a POD data provider
  const simRF::ProfileDataProvider* provider =
    (cProvider->getProvider(simRF::ProfileDataProvider::THRESHOLDTYPE_POD));
  if (!provider)
  {
    SIM_WARN << "No POD data provider found for beam at requested bearing" << std::endl;
  }
  else if (gndRngMeters < provider->getMinRange() || gndRngMeters > provider->getMaxRange())
  {
    SIM_WARN << "POD Request outside of profile range limits" << std::endl;
  }
  else if (hgtMeters < provider->getMinHeight() || hgtMeters > provider->getMaxHeight())
  {
    SIM_WARN << "POD Request outside of profile height limits" << std::endl;
  }
  else
  {
    return provider->interpolateValue(hgtMeters, gndRngMeters);
  }
  return 0.0;
}

double RFPropagationFacade::getLoss(double azimRad, double gndRngMeters, double hgtMeters) const
{
  const simRF::CompositeProfileProvider* cProvider = getProfileProvider(azimRad);
  if (!cProvider)
  {
    SIM_WARN << "No profile found for beam at requested bearing" << std::endl;
    return simCore::SMALL_DB_VAL;
  }

  // we want a Loss data provider
  const simRF::ProfileDataProvider* provider =
    (cProvider->getProvider(simRF::ProfileDataProvider::THRESHOLDTYPE_LOSS));
  if (!provider)
  {
    SIM_WARN << "No Loss data provider found for beam at requested bearing" << std::endl;
  }
  else if (gndRngMeters < provider->getMinRange() || gndRngMeters > provider->getMaxRange())
  {
    SIM_WARN << "Loss Request outside of profile range limits" << std::endl;
  }
  else if (hgtMeters < provider->getMinHeight() || hgtMeters > provider->getMaxHeight())
  {
    SIM_WARN << "Loss Request outside of profile height limits" << std::endl;
  }
  else
  {
    double lossVal = provider->interpolateValue(hgtMeters, gndRngMeters);
    return (lossVal > simCore::SMALL_DB_VAL ? lossVal : simCore::SMALL_DB_VAL);
  }
  return simCore::SMALL_DB_VAL;
}

double RFPropagationFacade::getPPF(double azimRad, double gndRngMeters, double hgtMeters) const
{
  const simRF::CompositeProfileProvider* cProvider = getProfileProvider(azimRad);
  if (!cProvider)
  {
    SIM_WARN << "No profile found for beam at requested bearing" << std::endl;
    return simCore::SMALL_DB_VAL;
  }

  // we want a PPF data provider
  const simRF::ProfileDataProvider* provider =
    (cProvider->getProvider(simRF::ProfileDataProvider::THRESHOLDTYPE_FACTOR));
  if (!provider)
  {
    SIM_WARN << "No PPF data provider found for beam at requested bearing" << std::endl;
  }
  else if (gndRngMeters < provider->getMinRange() || gndRngMeters > provider->getMaxRange())
  {
    SIM_WARN << "PPF Request outside of profile range limits" << std::endl;
  }
  else if (hgtMeters < provider->getMinHeight() || hgtMeters > provider->getMaxHeight())
  {
    SIM_WARN << "PPF Request outside of profile height limits" << std::endl;
  }
  else
  {
    double ppfVal = provider->interpolateValue(hgtMeters, gndRngMeters);
    return (ppfVal > simCore::SMALL_DB_VAL ? ppfVal : simCore::SMALL_DB_VAL);
  }
  return simCore::SMALL_DB_VAL;
}

double RFPropagationFacade::getSNR(double azimRad, double slantRngMeters, double hgtMeters, double xmtGaindB, double rcvGaindB, double rcsSqm, double gndRngMeters) const
{
  const simRF::CompositeProfileProvider* cProvider = getProfileProvider(azimRad);
  if (!cProvider)
  {
    SIM_WARN << "No profile found for beam at requested bearing" << std::endl;
    return simCore::SMALL_DB_VAL;
  }

  const simRF::ProfileDataProvider* provider =
  (cProvider->getProvider(simRF::ProfileDataProvider::THRESHOLDTYPE_SNR));
  if (!provider)
  {
    SIM_WARN << "No SNR data provider found for beam at requested bearing" << std::endl;
  }
  else if (gndRngMeters < provider->getMinRange() || gndRngMeters > provider->getMaxRange())
  {
    SIM_WARN << "SNR Request outside of profile range limits" << std::endl;
  }
  else if (hgtMeters < provider->getMinHeight() || hgtMeters > provider->getMaxHeight())
  {
    SIM_WARN << "SNR Request outside of profile height limits" << std::endl;
  }
  else
  {
    const simRF::SNRDataProvider* snrProvider = dynamic_cast<const simRF::SNRDataProvider*>(provider);
    if (snrProvider)
      return snrProvider->getSNR(hgtMeters, gndRngMeters, slantRngMeters, xmtGaindB, rcvGaindB, rcsSqm);
  }
  // not found, return simCore::SMALL_DB_VAL as a near -infinity dB value
  return simCore::SMALL_DB_VAL;
}

double RFPropagationFacade::getCNR(double azimRad, double gndRngMeters) const
{
  const simRF::CompositeProfileProvider* cProvider = getProfileProvider(azimRad);
  if (!cProvider)
  {
    SIM_WARN << "No profile found for beam at requested bearing" << std::endl;
    return simCore::SMALL_DB_VAL;
  }

  // we want a CNR data provider
  const simRF::ProfileDataProvider* provider =
    (cProvider->getProvider(simRF::ProfileDataProvider::THRESHOLDTYPE_CNR));
  if (!provider)
  {
    SIM_WARN << "No CNR data provider found for beam at requested bearing" << std::endl;
  }
  else if (gndRngMeters < provider->getMinRange() || gndRngMeters > provider->getMaxRange())
  {
    SIM_WARN << "CNR Request outside of profile range limits" << std::endl;
  }
  else
  {
    return provider->interpolateValue(0.0, gndRngMeters);
  }
  // not found, return simCore::SMALL_DB_VAL as a near -infinity dB value
  return simCore::SMALL_DB_VAL;
}

double RFPropagationFacade::getOneWayPower(double azimRad, double slantRngMeters, double hgtMeters, double xmtGaindB, double gndRngMeters, double rcvGaindB) const
{
  const simRF::CompositeProfileProvider* cProvider = getProfileProvider(azimRad);
  if (!cProvider)
  {
    SIM_WARN << "No profile found for beam at requested bearing" << std::endl;
    return simCore::SMALL_DB_VAL;
  }

  const simRF::ProfileDataProvider* provider =
    (cProvider->getProvider(simRF::ProfileDataProvider::THRESHOLDTYPE_ONEWAYPOWER));
  if (!provider)
  {
    SIM_WARN << "No One-Way Power data provider found for beam at requested bearing" << std::endl;
  }
  else if (gndRngMeters < provider->getMinRange() || gndRngMeters > provider->getMaxRange())
  {
    SIM_WARN << "One-Way Power Request outside of profile range limits" << std::endl;
  }
  else if (hgtMeters < provider->getMinHeight() || hgtMeters > provider->getMaxHeight())
  {
    SIM_WARN << "One-Way Power Request outside of profile height limits" << std::endl;
  }
  else
  {
    const simRF::OneWayPowerDataProvider* owpProvider = dynamic_cast<const simRF::OneWayPowerDataProvider*>(provider);
    if (owpProvider)
      return owpProvider->getOneWayPower(hgtMeters, gndRngMeters, slantRngMeters, xmtGaindB, rcvGaindB);
  }
  // not found, return simCore::SMALL_DB_VAL as a near -infinity dB value
  return simCore::SMALL_DB_VAL;
}

double RFPropagationFacade::getReceivedPower(double azimRad, double slantRngMeters, double hgtMeters, double xmtGaindB, double rcvGaindB, double rcsSqm, double gndRngMeters) const
{
  const simRF::CompositeProfileProvider* cProvider = getProfileProvider(azimRad);
  if (!cProvider)
  {
    SIM_WARN << "No profile found for beam at requested bearing" << std::endl;
    return simCore::SMALL_DB_VAL;
  }

  const simRF::ProfileDataProvider* provider =
    (cProvider->getProvider(simRF::ProfileDataProvider::THRESHOLDTYPE_RECEIVEDPOWER));
  if (!provider)
  {
    SIM_WARN << "No Received Power data provider found for beam at requested bearing" << std::endl;
  }
  else if (gndRngMeters < provider->getMinRange() || gndRngMeters > provider->getMaxRange())
  {
    SIM_WARN << "Received Power Request outside of profile range limits" << std::endl;
  }
  else if (hgtMeters < provider->getMinHeight() || hgtMeters > provider->getMaxHeight())
  {
    SIM_WARN << "Received Power Request outside of profile height limits" << std::endl;
  }
  else
  {
    const simRF::TwoWayPowerDataProvider* twpProvider = dynamic_cast<const simRF::TwoWayPowerDataProvider*>(provider);
    if (twpProvider)
      return twpProvider->getTwoWayPower(hgtMeters, gndRngMeters, slantRngMeters, xmtGaindB, rcvGaindB, rcsSqm);
  }
  // not found, return simCore::SMALL_DB_VAL as a near -infinity dB value
  return simCore::SMALL_DB_VAL;
}

bool RFPropagationFacade::valid() const
{
  // TODO: SPR-167: in SIMDIS 9, valid == (rfParametersSet && podVectorSet && colorMapSet);
  return (profileManager_ != NULL && rfParamsSet_);
}

int RFPropagationFacade::loadArepsFiles(const simCore::TimeStamp& time, const std::vector<std::string>& filenames)
{
  const double timeAsDouble = time.secondsSinceRefYear().Double();
  profileManager_->addProfileMap(timeAsDouble);
  profileManager_->update(timeAsDouble);
  // TODO: we have to update the profileManager_ time to load data at specified time; should we restore previous time after load is completed?

  simRF::ArepsLoader arepsLoader(this);

  // TODO: SDK-53
  // it may be desirable to check that height min/max/num, range min/max/num, beam width, and antenna height values for the first file match values obtained from all subsequent files
  // loading 180 files is very slow, and there are no dependencies between files, so loading could be parallelized.

  // Process AREPS files
  for (size_t ii = 0; ii < filenames.size(); ii++)
  {
    osg::ref_ptr<simRF::Profile> profile = new simRF::Profile(new simRF::CompositeProfileProvider());
    if (0 != arepsLoader.loadFile(filenames[ii], *profile, ii == 0))
    {
      // failed to load a file
      profileManager_->removeProfileMap(timeAsDouble);
      return 1;
    }
    if (arepsFilesetTimeMap_.empty())
    {
      setAntennaHeight(arepsLoader.getAntennaHeight());
    }
    setSlotData(profile);
  }

  // store filenames to support getInputFiles()
  auto it = arepsFilesetTimeMap_.find(time);
  if (it == arepsFilesetTimeMap_.end())
    arepsFilesetTimeMap_[time] = filenames;
  else
    it->second.insert(it->second.end(), filenames.begin(), filenames.end());
  setDisplay(true);
  return 0;
}

const simRF::CompositeProfileProvider* RFPropagationFacade::getProfileProvider(double azimRad) const
{
  const simRF::Profile *profile = getSlotData(azimRad);
  if (profile)
    return dynamic_cast<const simRF::CompositeProfileProvider*>(profile->getDataProvider());
  return NULL;
}

void RFPropagationFacade::setAntennaHeight(float antennaHeightM)
{
  antennaHeightMeters_ = antennaHeightM;
  setPosition(profileManager_->getRefLat(), profileManager_->getRefLon());
}

float RFPropagationFacade::antennaHeight() const
{
  return antennaHeightMeters_;
}

float RFPropagationFacade::minHeight() const
{
  const Profile* profile = getProfile(0);
  if (profile == NULL)
    return 0.0f;
  const simRF::CompositeProfileProvider* cProvider = profile->getDataProvider();
  return (cProvider != NULL) ? cProvider->getMinHeight() : 0.0f;
}

float RFPropagationFacade::maxHeight() const
{
  const Profile* profile = getProfile(0);
  if (profile == NULL)
    return 0.0f;
  const simRF::CompositeProfileProvider* cProvider = profile->getDataProvider();
  return (cProvider != NULL) ? cProvider->getMaxHeight() : 0.0f;
}


double RFPropagationFacade::getBearing() const
{
  if (!profileManager_)
    return 0.0;
  return profileManager_->getBearing();
}

void RFPropagationFacade::setBearing(double bearing)
{
 if (profileManager_)
   profileManager_->setBearing(bearing);
}

void RFPropagationFacade::setElevation(double elevation)
{
 if (profileManager_)
   profileManager_->setElevAngle(elevation);
}

unsigned int RFPropagationFacade::numProfiles() const
{
  return profileList_.size();
}

const simRF::Profile* RFPropagationFacade::getProfile(unsigned int index) const
{
  return (profileList_.size() > index) ? profileList_.at(index) : NULL;
}

void RFPropagationFacade::setPosition(double latRad, double lonRad)
{
  profileManager_->setRefCoord(latRad, lonRad, antennaHeight());

  // locator takes lon/lat/alt, in degrees
  if (locator_)
  {
    locator_->getLocator()->setPosition(
      osg::Vec3d(lonRad * simCore::RAD2DEG, latRad * simCore::RAD2DEG, antennaHeight()));
  }
}

void RFPropagationFacade::initializeDefaultColors_()
{
  simRF::GradientColorProvider::ColorMap lossColors;
  lossColors[0.0f] =  osg::Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
  lossColors[110.0f] = osg::Vec4f(1.0f, 1.0f, 0.0f, 1.0f);
  lossColors[115.0f] = osg::Vec4f(1.0f, 0.0f, 1.0f, 1.0f);
  lossColors[120.0f] = osg::Vec4f(0.0f, 0.0f, 1.0f, 1.0f);
  lossColors[125.0f] = osg::Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
  lossColors[130.0f] = osg::Vec4f(1.0f, 0.5f, 0.0f, 1.0f);
  lossColors[135.0f] = osg::Vec4f(0.0f, 0.5f, 0.5f, 1.0f);
  lossColors[140.0f] = osg::Vec4f(0.0f, 0.5f, 0.0f, 1.0f);
  lossColors[145.0f] = osg::Vec4f(0.0f, 0.0f, 0.5f, 1.0f);
  lossColors[150.0f] = osg::Vec4f(0.0f, 0.75f, 0.75f, 1.0f);
  lossColors[155.0f] = osg::Vec4f(0.0f, 1.0f, 1.0f, 1.0f);
  lossColors[160.0f] = osg::Vec4f(0.5f, 0.0f, 0.5f, 1.0f);

  colorMaps_[simRF::ProfileDataProvider::THRESHOLDTYPE_LOSS] = lossColors;

  simRF::GradientColorProvider::ColorMap complexColors;
  complexColors[101.0f] = osg::Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
  complexColors[100.0f] = osg::Vec4f(1.0f, 1.0f, 0.0f, 1.0f);
  complexColors[80.0f] = osg::Vec4f(1.0f, 0.0f, 1.0f, 1.0f);
  complexColors[60.0f] = osg::Vec4f(0.0f, 0.0f, 1.0f, 1.0f);
  complexColors[40.0f] = osg::Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
  complexColors[20.0f] = osg::Vec4f(1.0f, 0.5f, 0.0f, 1.0f);
  complexColors[0.0f] = osg::Vec4f(0.0f, 0.5f, 0.5f, 1.0f);
  complexColors[-20.0f] = osg::Vec4f(0.0f, 0.5f, 0.0f, 1.0f);
  complexColors[-40.0f] = osg::Vec4f(0.0f, 0.0f, 0.5f, 1.0f);
  complexColors[-60.0f] = osg::Vec4f(0.75f, 0.75f, 0.75f, 1.0f);
  complexColors[-80.0f] = osg::Vec4f(0.0f, 1.0f, 1.0f, 1.0f);
  complexColors[-100.0f] = osg::Vec4f(0.5f, 0.0f, 0.5f, 1.0f);

  colorMaps_[simRF::ProfileDataProvider::THRESHOLDTYPE_SNR] = complexColors;
  colorMaps_[simRF::ProfileDataProvider::THRESHOLDTYPE_CNR] = complexColors;
  colorMaps_[simRF::ProfileDataProvider::THRESHOLDTYPE_ONEWAYPOWER] = complexColors;
  colorMaps_[simRF::ProfileDataProvider::THRESHOLDTYPE_FACTOR] = complexColors;

  // build a default color map
  defaultColors_[100.0f] = osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
  defaultColors_[90.0f] = osg::Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
  defaultColors_[80.0f] = osg::Vec4f(1.0f, 1.0f, 0.0f, 1.0f);
  defaultColors_[70.0f] = osg::Vec4f(1.0f, 0.0f, 1.0f, 1.0f);
  defaultColors_[60.0f] = osg::Vec4f(0.0f, 0.0f, 1.0f, 1.0f);
  defaultColors_[50.0f] = osg::Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
  defaultColors_[40.0f] = osg::Vec4f(1.0f, 0.5f, 0.0f, 1.0f);
  defaultColors_[30.0f] = osg::Vec4f(0.0f, 0.5f, 0.5f, 1.0f);
  defaultColors_[20.0f] = osg::Vec4f(0.0f, 0.5f, 0.0f, 1.0f);
  defaultColors_[10.0f] = osg::Vec4f(0.0f, 0.0f, 0.5f, 1.0f);
  defaultColors_[0.0f] = osg::Vec4f(0.75f, 0.75f, 0.75f, 1.0f);
}

void RFPropagationFacade::setGradientByThresholdType_(simRF::ProfileDataProvider::ThresholdType type)
{
  if (colorProvider_ == NULL)
    return;
  // apply the appropriate color map, or the default if we don't have one specified for this type
  std::map<simRF::ProfileDataProvider::ThresholdType, simRF::GradientColorProvider::ColorMap>::const_iterator foundColor = colorMaps_.find(type);
  if (foundColor != colorMaps_.end())
    colorProvider_->setGradientColorMap(foundColor->second);
  else
    colorProvider_->setGradientColorMap(defaultColors_);
}

void RFPropagationFacade::enableDepthBuffer(bool enable)
{
  if (!profileManager_.valid())
    return;
  osg::StateSet* stateset = profileManager_->getOrCreateStateSet();
  if (enable)
  {
    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    stateset->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, true));
  }
  else
  {
    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    stateset->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false));
  }
}

bool RFPropagationFacade::isDepthBufferEnabled() const
{
  if (!profileManager_.valid())
    return false;
  osg::StateSet* stateset = profileManager_->getStateSet();
  // Note the use of bitwise-AND (&) below is intentional
  return stateset != NULL &&
    ((stateset->getMode(GL_DEPTH_TEST) & osg::StateAttribute::ON) != 0);
}

}

