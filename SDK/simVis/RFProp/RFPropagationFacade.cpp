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
#include "osg/Depth"
#include "osgEarth/Map"
#include "simCore/Calc/Angle.h"
#include "simCore/EM/AntennaPattern.h"
#include "simCore/Time/TimeClass.h"
#include "simNotify/Notify.h"
#include "simVis/Constants.h"
#include "simVis/Locator.h"
#include "simVis/LocatorNode.h"
#include "simVis/Types.h"
#include "simVis/RFProp/ArepsLoader.h"
#include "simVis/RFProp/CompositeColorProvider.h"
#include "simVis/RFProp/CompositeProfileProvider.h"
#include "simVis/RFProp/FallbackDataHelper.h"
#include "simVis/RFProp/OneWayPowerDataProvider.h"
#include "simVis/RFProp/PODProfileDataProvider.h"
#include "simVis/RFProp/ProfileManager.h"
#include "simVis/RFProp/Profile.h"
#include "simVis/RFProp/SNRDataProvider.h"
#include "simVis/RFProp/GradientColorProvider.h"
#include "simVis/RFProp/RFPropagationFacade.h"

namespace
{
const simVis::Color SIMDIS_ORANGE(1.0f, 0.5f, 0.0f, 1.0f); // darker than dark orange
const simVis::Color SIMDIS_CYAN(0.0f, 0.75f, 0.75f, 1.0f); // medium-dark cyan

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

std::string dataTypeToString(simRF::ProfileDataProvider::ThresholdType dataType)
{
  switch (dataType)
  {
  case simRF::ProfileDataProvider::THRESHOLDTYPE_POD:
    return "POD";
  case simRF::ProfileDataProvider::THRESHOLDTYPE_LOSS:
    return "Loss";
  case simRF::ProfileDataProvider::THRESHOLDTYPE_FACTOR:
    return "PPF";
  case simRF::ProfileDataProvider::THRESHOLDTYPE_SNR:
    return "SNR";
  case simRF::ProfileDataProvider::THRESHOLDTYPE_CNR:
    return "CNR";
  case simRF::ProfileDataProvider::THRESHOLDTYPE_ONEWAYPOWER:
    return "One-way power";
  case simRF::ProfileDataProvider::THRESHOLDTYPE_RECEIVEDPOWER:
    return "Received power";
  default:
    break;
  }
  return "?";
}
}

namespace simRF
{
const int DEFAULT_TRANSPARENCY = 60;  // percentage, 0-100. 100 is fully transparent, 0 is opaque
const int DEFAULT_HISTORY = 30; // degrees

RFPropagationFacade::RFPropagationFacade(osg::Group* parent, std::shared_ptr<simCore::DatumConvert> datumConvert)
  : antennaHeightMeters_(0.0),
  profileManager_(new simRF::ProfileManager(datumConvert)),
  parent_(parent)
{
  // add profileManager_ to the parent node
  if (profileManager_.valid() && parent_.valid())
    parent_->addChild(profileManager_);

  initializeColorProviders_();

  // set the threshold type, and update color provider accordingly
  setThresholdType(simRF::ProfileDataProvider::THRESHOLDTYPE_LOSS);

  // set the default visualization mode
  setDrawMode(simRF::Profile::DRAWMODE_2D_HORIZONTAL);

  podLossThresholds_ = PODVectorPtr(new std::vector<float>);
  setDefaultPODVector(podLossThresholds_);

  // do not create radarParameters_ until actually set

  // set default transparency
  setTransparency(DEFAULT_TRANSPARENCY);

  // default history (in degrees), note that history is defined as CCW from current bearing
  setHistory(DEFAULT_HISTORY);

  setDisplay(false);
}

RFPropagationFacade::~RFPropagationFacade()
{
  if (parent_.valid() && profileManager_.valid())
    parent_->removeChild(profileManager_.get());
}

int RFPropagationFacade::setModelType()
{
  // TODO
  return 1;
}

namespace
{
// can't use struct equality comparison: some params are calculated and not expected to be provided
bool areEqual(const simCore::RadarParameters& first, const simCore::RadarParameters& second)
{
  return (simCore::areEqual(first.freqMHz, second.freqMHz)
    && simCore::areEqual(first.antennaGaindBi, second.antennaGaindBi)
    && simCore::areEqual(first.noiseFiguredB, second.noiseFiguredB)
    && simCore::areEqual(first.pulseWidth_uSec, second.pulseWidth_uSec)
    && simCore::areEqual(first.systemLossdB, second.systemLossdB)
    && simCore::areEqual(first.xmtPowerKW, second.xmtPowerKW)
    && simCore::areEqual(first.hbwD, second.hbwD));
}
}

int RFPropagationFacade::setRadarParams(const simCore::RadarParameters& radarParams)
{
  if (radarParameters_)
  {
    if (areEqual(*radarParameters_.get(), radarParams))
      return 0;
    if (display() || !arepsFilesetTimeMap_.empty() || profileManager_->getProfile(0))
    {
      // facade is not in an initial state, disallow resetting params;
      // possibly reset cache; but probably need a dialog with user to do so.
      return 1;
    }
    simCore::RadarParameters* params = radarParameters_.get();
    params->antennaGaindBi = radarParams.antennaGaindBi;
    params->freqMHz = radarParams.freqMHz;
    params->hbwD = radarParams.hbwD;
    params->noiseFiguredB = radarParams.noiseFiguredB;
    //params->noisePowerdB = // calculated below
    params->pulseWidth_uSec = radarParams.pulseWidth_uSec;
    params->systemLossdB = radarParams.systemLossdB;
    params->xmtPowerKW = radarParams.xmtPowerKW;
    //params->xmtPowerW = // calculated below
  }
  else
    radarParameters_ = std::make_shared<simCore::RadarParameters>(radarParams);

  // noise power in db = 10 log (kT/pw); for T, use standard ambient temperature: 17°C/290K
  radarParameters_->noisePowerdB = simCore::linear2dB(4e-15 / radarParams.pulseWidth_uSec) + radarParams.noiseFiguredB;
  radarParameters_->xmtPowerW = radarParams.xmtPowerKW * 1e03;
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
  auto foundColorProvider = colorProviderMap_.find(type);
  if (foundColorProvider != colorProviderMap_.end())
    foundColorProvider->second->setGradientColorMap(colorMap);
  else
  {
    // some providers may not be available.
    defaultColorProvider_->setGradientColorMap(colorMap);
  }
  return 0;
}

int RFPropagationFacade::setSlotData(simRF::Profile* profile)
{
  if (profile == nullptr)
    return 1;
  const simCore::RadarParameters* rp = radarParameters_.get();
  if (rp == nullptr)
    return 1;
  if (!simCore::areEqual(profile->getHalfBeamWidth(), (rp->hbwD / 2. * simCore::DEG2RAD)))
    return 1;
  profileManager_->addProfile(profile);
  return 0;
}

const simRF::Profile* RFPropagationFacade::getSlotData(double azRad) const
{
  return profileManager_->getProfileByBearing(azRad);
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
  profileManager_->setAGL(aglActive);
  return 0;
}

bool RFPropagationFacade::aglActive() const
{
  return profileManager_->getAGL();
}

int RFPropagationFacade::setDrawMode(simRF::Profile::DrawMode drawMode)
{
  profileManager_->setMode(drawMode);
  return 0;
}

simRF::Profile::DrawMode RFPropagationFacade::drawMode() const
{
  return profileManager_->getMode();
}

int RFPropagationFacade::setHeight(double height)
{
  profileManager_->setHeight(height);
  return 0;
}

double RFPropagationFacade::height() const
{
  return profileManager_->getHeight();
}

int RFPropagationFacade::setThickness(unsigned int thickness)
{
  profileManager_->setDisplayThickness(thickness);
  return 0;
}

unsigned int RFPropagationFacade::thickness() const
{
  return profileManager_->getDisplayThickness();
}

int RFPropagationFacade::setHistory(int length)
{
  profileManager_->setHistory(length * simCore::DEG2RAD);
  return 0;
}

int RFPropagationFacade::history() const
{
  return static_cast<int>(simCore::rint(profileManager_->getHistory() * simCore::RAD2DEG));
}

int RFPropagationFacade::setTransparency(int transparency)
{
  profileManager_->setAlpha(1.f - transparency * 0.01f);
  return 0;
}

int RFPropagationFacade::transparency() const
{
  // Add 0.5f to round correctly; subtract from 1.f to convert alpha to transparency
  return static_cast<int>(0.5f + 100.f * (1.f - profileManager_->getAlpha()));
}

int RFPropagationFacade::setThresholdMode(simRF::ColorProvider::ColorMode mode)
{
  if (!currentColorProvider_)
  {
    assert(false);
    return 1;
  }
  currentColorProvider_->setMode(mode);
  return 0;
}

simRF::ColorProvider::ColorMode RFPropagationFacade::thresholdMode() const
{
  if (!currentColorProvider_)
    return simRF::ColorProvider::COLORMODE_BELOW;
  return currentColorProvider_->getMode();
}

int RFPropagationFacade::setThresholdType(simRF::ProfileDataProvider::ThresholdType type)
{
  profileManager_->setThresholdType(type);
  setColorProviderByThresholdType_(type);
  return 0;
}

simRF::ProfileDataProvider::ThresholdType RFPropagationFacade::thresholdType() const
{
  return profileManager_->getThresholdType();
}

int RFPropagationFacade::setThresholdValue(float value)
{
  if (!currentColorProvider_)
  {
    assert(false);
    return 1;
  }
  currentColorProvider_->setThreshold(value);
  return 0;
}

float RFPropagationFacade::threshold() const
{
  if (!currentColorProvider_)
    return 0.f;
  return currentColorProvider_->getThreshold();
}

int RFPropagationFacade::setAboveColor(const osg::Vec4f& color)
{
  if (!currentColorProvider_)
  {
    assert(false);
    return 1;
  }
  currentColorProvider_->setAboveColor(color);
  return 0;
}

int RFPropagationFacade::aboveColor(osg::Vec4f& color)
{
  if (!currentColorProvider_)
  {
    assert(false);
    return 1;
  }
  color = currentColorProvider_->getAboveColor();
  return 0;
}

int RFPropagationFacade::setBelowColor(const osg::Vec4f& color)
{
  if (!currentColorProvider_)
  {
    assert(false);
    return 1;
  }
  currentColorProvider_->setBelowColor(color);
  return 0;
}

int RFPropagationFacade::belowColor(osg::Vec4f& color)
{
  if (!currentColorProvider_)
  {
    assert(false);
    return 1;
  }
  color = currentColorProvider_->getBelowColor();
  return 0;
}

int RFPropagationFacade::clearCache(bool reset)
{
  setDisplay(false);
  arepsFilesetTimeMap_.clear();
  // clear out old (data from) ProfileManager, create a new empty ProfileManager
  profileManager_->reset();
  return 0;
}

double RFPropagationFacade::getPOD(double azimRad, double gndRngMeters, double hgtMeters) const
{
  std::string msg;
  const simRF::ProfileDataProvider* provider = getProfileDataProvider(
    simRF::ProfileDataProvider::THRESHOLDTYPE_POD,
    azimRad, gndRngMeters, hgtMeters, msg);
  if (provider)
  {
    return provider->interpolateValue(hgtMeters, gndRngMeters);
  }

  // TODO: POD is derived from Loss data, should be able to use losshelper to provide underlying data
  //if (lossDataHelper_)
  //{
  //  const double lossdB = lossDataHelper_->value(azimRad, gndRngMeters, hgtMeters);
  //  return (lossdB != simCore::SMALL_DB_VAL) ? getPOD_(-lossdB) : lossdB;
  //}

  SIM_WARN << "RFPropagationFacade::getPOD: " << msg << "\n";
  return 0.0;
}

void RFPropagationFacade::setLossDataHelper(std::unique_ptr<FallbackDataHelper> helper)
{
  lossDataHelper_ = std::move(helper);
  // rework data helper constructor;
  // TODO: bind, check error state, message on failure
}

double RFPropagationFacade::getLoss(double azimRad, double gndRngMeters, double hgtMeters) const
{
  std::string msg;
  const simRF::ProfileDataProvider* provider = getProfileDataProvider(
    simRF::ProfileDataProvider::THRESHOLDTYPE_LOSS,
    azimRad, gndRngMeters, hgtMeters, msg);
  if (provider)
  {
    const double lossVal = provider->interpolateValue(hgtMeters, gndRngMeters);
    return (lossVal > simCore::SMALL_DB_VAL ? lossVal : simCore::SMALL_DB_VAL);
  }
  if (lossDataHelper_)
  {
    const double lossdB = lossDataHelper_->value(azimRad, gndRngMeters, hgtMeters);
    if (lossdB != simCore::SMALL_DB_VAL)
      return lossdB;
  }
  SIM_WARN << "RFPropagationFacade::getLoss: " << msg << "\n";
  return simCore::SMALL_DB_VAL;
}

double RFPropagationFacade::getPPF(double azimRad, double gndRngMeters, double hgtMeters) const
{
  std::string msg;
  const simRF::ProfileDataProvider* provider = getProfileDataProvider(
    simRF::ProfileDataProvider::THRESHOLDTYPE_FACTOR,
    azimRad, gndRngMeters, hgtMeters, msg);
  if (provider)
  {
    const double ppf_dB = provider->interpolateValue(hgtMeters, gndRngMeters);
    return (ppf_dB > simCore::SMALL_DB_VAL ? ppf_dB : simCore::SMALL_DB_VAL);
  }

  if (lossDataHelper_)
  {
    const double lossdB = lossDataHelper_->value(azimRad, gndRngMeters, hgtMeters);
    const double slantRangeM = sqrt(simCore::square(gndRngMeters) + simCore::square(hgtMeters));
    const double ppf_dB = simCore::lossToPpf(slantRangeM, radarParameters_->freqMHz, lossdB);
    if (ppf_dB != simCore::SMALL_DB_VAL)
      return ppf_dB;
  }
  SIM_WARN << "RFPropagationFacade::getPPF: " << msg << "\n";
  return simCore::SMALL_DB_VAL;
}

double RFPropagationFacade::getSNR(double azimRad, double slantRngMeters, double hgtMeters, double xmtGaindB, double rcvGaindB, double rcsSqm, double gndRngMeters) const
{
  const double rcvPowerdB = getReceivedPower(azimRad, slantRngMeters, hgtMeters, xmtGaindB, rcvGaindB, rcsSqm, gndRngMeters);
  if (rcvPowerdB == simCore::SMALL_DB_VAL)
    return simCore::SMALL_DB_VAL;
  return (rcvPowerdB - radarParameters_->noisePowerdB);
}

double RFPropagationFacade::getCNR(double azimRad, double gndRngMeters) const
{
  std::string msg;
  const simRF::ProfileDataProvider* provider = getProfileDataProvider(
    simRF::ProfileDataProvider::THRESHOLDTYPE_CNR,
    azimRad, gndRngMeters, msg);
  if (provider)
  {
    return provider->interpolateValue(0.0, gndRngMeters);
  }
  SIM_WARN << "RFPropagationFacade::getCNR: " << msg << "\n";
  return simCore::SMALL_DB_VAL;
}

double RFPropagationFacade::getOneWayPower(double azimRad, double slantRngMeters, double hgtMeters, double xmtGaindB, double gndRngMeters, double rcvGaindB) const
{
  const double ppf_dB = getPPF(azimRad, gndRngMeters, hgtMeters);
  if (ppf_dB == simCore::SMALL_DB_VAL)
    return simCore::SMALL_DB_VAL;
  return simRF::OneWayPowerDataProvider::getOneWayPower(*radarParameters_, ppf_dB, slantRngMeters, xmtGaindB, rcvGaindB);
}

double RFPropagationFacade::getReceivedPower(double azimRad, double slantRngMeters, double hgtMeters, double xmtGaindB, double rcvGaindB, double rcsSqm, double gndRngMeters) const
{
  const double ppf_dB = getPPF(azimRad, gndRngMeters, hgtMeters);
  if (ppf_dB == simCore::SMALL_DB_VAL)
    return simCore::SMALL_DB_VAL;
  return simRF::TwoWayPowerDataProvider::getTwoWayPower(*radarParameters_, ppf_dB, slantRngMeters, xmtGaindB, rcvGaindB, rcsSqm);
}

bool RFPropagationFacade::valid() const
{
  // in SIMDIS 9, valid == (rfParametersSet && podVectorSet && colorMapSet);
  return (radarParameters_.get() != nullptr);
}

int RFPropagationFacade::loadArepsFiles(const simCore::TimeStamp& time, const std::vector<std::string>& filenames)
{
  const double timeAsDouble = time.secondsSinceRefYear().Double();

  // prepare the profileManager_ for addition of profiles
  profileManager_->addProfileMap(timeAsDouble);
  profileManager_->update(timeAsDouble);
  profileManager_->setSphericalEarth(true);

  // TODO: implementation of multiple timestamps of rfprop data per facade is notional.
  //    - needs conops to be fleshed out.
  //    - will need attention to ensure the implementation is consistent.
  //    - will have to update the profileManager_ time to load data at specified time; should we restore previous time after load is completed?

  // TODO: SDK-53
  // it may be desirable to check that height min/max/num, range min/max/num, beam width, and antenna height values for the first file match values obtained from all subsequent files
  // loading 180 files is very slow, and there are no dependencies between files, so loading could be parallelized.

  simRF::ArepsLoader arepsLoader(this);
  std::vector<std::string> filenamesAdded;
  bool loadingFirstFile = true;
  for (const auto& filename : filenames)
  {
    osg::ref_ptr<simRF::Profile> profile = new simRF::Profile(new simRF::CompositeProfileProvider());
    if (0 != arepsLoader.loadFile(filename, *profile, loadingFirstFile))
    {
      // arepsLoader provides the messaging on failure
      continue;
    }
    // adding slot can fail if hbw does not match expected value
    if (0 != setSlotData(profile.get()))
    {
      SIM_ERROR << "Could not add slot for AREPS file: " << filename << std::endl;
      continue;
    }
    // successfully loaded the file
    loadingFirstFile = false;
    filenamesAdded.push_back(filename);
    if (arepsFilesetTimeMap_.empty())
    {
      setAntennaHeight(arepsLoader.getAntennaHeight());
    }
  }

  if (filenamesAdded.empty())
  {
    profileManager_->removeProfileMap(timeAsDouble);
    return 1;
  }

  // store filenames to support getInputFiles()
  auto it = arepsFilesetTimeMap_.find(time);
  if (it == arepsFilesetTimeMap_.end())
    arepsFilesetTimeMap_[time] = filenamesAdded;
  else
    it->second.insert(it->second.end(), filenamesAdded.begin(), filenamesAdded.end());

  setDisplay(true);
  return 0;
}

const simRF::CompositeProfileProvider* RFPropagationFacade::getProfileProvider(double azimRad) const
{
  const simRF::Profile *profile = getSlotData(azimRad);
  return (profile ?
    dynamic_cast<const simRF::CompositeProfileProvider*>(profile->getDataProvider()) :
    nullptr);
}

const simRF::ProfileDataProvider* RFPropagationFacade::getProfileDataProvider(
  ProfileDataProvider::ThresholdType type,
  double azimRad, double gndRngMeters, std::string& msg) const
{
  const simRF::Profile *profile = getSlotData(azimRad);
  if (!profile)
  {
    msg = "No data found for beam at requested bearing";
    return nullptr;
  }
  const simRF::CompositeProfileProvider* cProvider =
    dynamic_cast<const simRF::CompositeProfileProvider*>(profile->getDataProvider());
  if (!cProvider)
  {
    msg = "No data found for beam at requested bearing";
    return nullptr;
  }
  const simRF::ProfileDataProvider* provider = cProvider->getProvider(type);
  if (!provider)
  {
    msg = "No " + dataTypeToString(type) + " data found for beam at requested bearing";
    return nullptr;
  }
  if (gndRngMeters < provider->getMinRange() || gndRngMeters > provider->getMaxRange())
  {
    msg = "Requested range is outside of " + dataTypeToString(type) +" data limits";
    return nullptr;
  }
  return provider;
}

const simRF::ProfileDataProvider* RFPropagationFacade::getProfileDataProvider(
  ProfileDataProvider::ThresholdType type,
  double azimRad, double gndRngMeters, double hgtMeters, std::string& msg) const
{
  const simRF::ProfileDataProvider* provider = getProfileDataProvider(type, azimRad, gndRngMeters, msg);
  if (!provider)
  {
    // msg already populated
    assert(!msg.empty());
    return nullptr;
  }
  if (hgtMeters < provider->getMinHeight() || hgtMeters > provider->getMaxHeight())
  {
    msg = "Requested height is outside of " + dataTypeToString(type) + " data limits";
    return nullptr;
  }
  return provider;
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
  if (profile == nullptr)
    return 0.0f;
  const simRF::CompositeProfileProvider* cProvider = profile->getDataProvider();
  return (cProvider != nullptr) ? cProvider->getMinHeight() : 0.0f;
}

float RFPropagationFacade::maxHeight() const
{
  const Profile* profile = getProfile(0);
  if (profile == nullptr)
    return 0.0f;
  const simRF::CompositeProfileProvider* cProvider = profile->getDataProvider();
  return (cProvider != nullptr) ? cProvider->getMaxHeight() : 0.0f;
}

unsigned int RFPropagationFacade::heightSteps() const
{
  const Profile* profile = getProfile(0);
  if (profile == nullptr)
    return 0;
  const simRF::CompositeProfileProvider* cProvider = profile->getDataProvider();
  return (cProvider != nullptr) ? cProvider->getNumHeights() : 0;
}


double RFPropagationFacade::getBearing() const
{
  return profileManager_->getBearing();
}

void RFPropagationFacade::setBearing(double bearing)
{
   profileManager_->setBearing(bearing);
}

void RFPropagationFacade::setElevation(double elevation)
{
   profileManager_->setElevAngle(elevation);
}

void RFPropagationFacade::setSphericalEarth(bool sphericalEarth)
{
  profileManager_->setSphericalEarth(sphericalEarth);
}

unsigned int RFPropagationFacade::numProfiles() const
{
  return profileManager_->getNumChildren();
}

const simRF::Profile* RFPropagationFacade::getProfile(unsigned int index) const
{
  return profileManager_->getProfile(index);
}

void RFPropagationFacade::setPosition(double latRad, double lonRad)
{
  profileManager_->setRefCoord(latRad, lonRad, antennaHeight());
}

void RFPropagationFacade::initializeColorProviders_()
{
  CompositeColorProvider* lossColorProvider = new CompositeColorProvider();
  colorProviderMap_[simRF::ProfileDataProvider::THRESHOLDTYPE_LOSS] = lossColorProvider;
  simRF::GradientColorProvider::ColorMap lossColors;
  lossColors[0.0f] = simVis::Color::Red;
  lossColors[110.0f] = simVis::Color::Yellow;
  lossColors[115.0f] = simVis::Color::Fuchsia;
  lossColors[120.0f] = simVis::Color::Blue;
  lossColors[125.0f] = simVis::Color::Lime;
  lossColors[130.0f] = SIMDIS_ORANGE;
  lossColors[135.0f] = simVis::Color::Teal;
  lossColors[140.0f] = simVis::Color::Green;
  lossColors[145.0f] = simVis::Color::Navy;
  lossColors[150.0f] = SIMDIS_CYAN;
  lossColors[155.0f] = simVis::Color::Aqua;
  lossColors[160.0f] = simVis::Color::Purple;
  lossColorProvider->setGradientColorMap(lossColors);
  lossColorProvider->setThreshold(150.f);

  simRF::GradientColorProvider::ColorMap complexColors;
  complexColors[101.0f] = simVis::Color::Red;
  complexColors[100.0f] = simVis::Color::Yellow;
  complexColors[80.0f] = simVis::Color::Fuchsia;
  complexColors[60.0f] = simVis::Color::Blue;
  complexColors[40.0f] = simVis::Color::Lime;
  complexColors[20.0f] = SIMDIS_ORANGE;
  complexColors[0.0f] = simVis::Color::Teal;
  complexColors[-20.0f] = simVis::Color::Green;
  complexColors[-40.0f] = simVis::Color::Navy;
  complexColors[-60.0f] = simVis::Color::Silver;
  complexColors[-80.0f] = simVis::Color::Aqua;
  complexColors[-100.0f] = simVis::Color::Purple;

  CompositeColorProvider* snrColorProvider = new CompositeColorProvider();
  colorProviderMap_[simRF::ProfileDataProvider::THRESHOLDTYPE_SNR] = snrColorProvider;
  snrColorProvider->setGradientColorMap(complexColors);
  snrColorProvider->setThreshold(0.f);

  CompositeColorProvider* cnrColorProvider = new CompositeColorProvider();
  colorProviderMap_[simRF::ProfileDataProvider::THRESHOLDTYPE_CNR] = cnrColorProvider;
  cnrColorProvider->setGradientColorMap(complexColors);
  cnrColorProvider->setThreshold(0.f);

  CompositeColorProvider* oneWayColorProvider = new CompositeColorProvider();
  colorProviderMap_[simRF::ProfileDataProvider::THRESHOLDTYPE_ONEWAYPOWER] = oneWayColorProvider;
  oneWayColorProvider->setGradientColorMap(complexColors);
  oneWayColorProvider->setThreshold(0.f);

  CompositeColorProvider* factorColorProvider = new CompositeColorProvider();
  colorProviderMap_[simRF::ProfileDataProvider::THRESHOLDTYPE_FACTOR] = factorColorProvider;
  factorColorProvider->setGradientColorMap(complexColors);
  factorColorProvider->setThreshold(0.f);


  // build a default color map
  simRF::GradientColorProvider::ColorMap defaultColors;
  defaultColors[100.0f] = simVis::Color::White;
  defaultColors[90.0f] = simVis::Color::Red;
  defaultColors[80.0f] = simVis::Color::Yellow;
  defaultColors[70.0f] = simVis::Color::Fuchsia;
  defaultColors[60.0f] = simVis::Color::Blue;
  defaultColors[50.0f] = simVis::Color::Lime;
  defaultColors[40.0f] = SIMDIS_ORANGE;
  defaultColors[30.0f] = simVis::Color::Teal;
  defaultColors[20.0f] = simVis::Color::Green;
  defaultColors[10.0f] = simVis::Color::Navy;
  defaultColors[0.0f] = simVis::Color::Silver;

  defaultColorProvider_ = new CompositeColorProvider();
  defaultColorProvider_->setGradientColorMap(defaultColors);
  defaultColorProvider_->setThreshold(50.f);
}

void RFPropagationFacade::setColorProviderByThresholdType_(simRF::ProfileDataProvider::ThresholdType type)
{
  auto foundColorProvider = colorProviderMap_.find(type);
  if (foundColorProvider != colorProviderMap_.end())
    currentColorProvider_ = foundColorProvider->second;
  else
  {
    // some providers may not be available.
    currentColorProvider_ = defaultColorProvider_;
  }
  profileManager_->setColorProvider(currentColorProvider_.get());
}

void RFPropagationFacade::enableDepthBuffer(bool enable)
{
  osg::StateSet* stateset = profileManager_->getOrCreateStateSet();
  if (enable)
    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
  else
    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
}

bool RFPropagationFacade::isDepthBufferEnabled() const
{
  const osg::StateSet* stateset = profileManager_->getStateSet();
  if (!stateset)
    return false;
  const osg::StateAttribute::GLModeValue values = stateset->getMode(GL_DEPTH_TEST);

  // Note the use of bitwise-AND (&) below is intentional
  return (values & osg::StateAttribute::ON) != 0;
}

}
