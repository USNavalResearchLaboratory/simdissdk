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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgEarth/Controls"
#include "osgEarth/Color"
#include "osgEarth/LabelNode"
#include "osgEarth/ScreenSpaceLayout"

#include "simCore/Calc/MathConstants.h"
#include "simData/DataStore.h"
#include "simNotify/Notify.h"
#include "simVis/Entity.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simUtil/ExampleControls.h"

#ifdef ENABLE_DEPRECATED_SIMDISSDK_API
using namespace osgEarth;
using namespace osgEarth::Util::Controls;

namespace
{
  struct PlatformListData
  {
    PlatformListData() : selected_(nullptr) { }
    Control* selected_;
    std::map<simData::ObjectId, LabelControl*> labelsById_;
  };

  struct BeamListData
  {
    BeamListData() : selected_(nullptr) { }
    Control* selected_;
  };

  // tethers to a platform when the user clicks on a platform name.
  struct PlatformClick : public ControlEventHandler
  {
    PlatformClick(simData::ObjectId id, simVis::View* view, PlatformListData* data)
      : id_(id), view_(view), data_(data) { }

    void onClick(Control* control, int mbm)
    {
      if (data_->selected_)
        data_->selected_->clearBackColor();
      view_->tetherCamera(view_->getSceneManager()->getScenario()->find(id_));
      control->setBackColor(Color::Maroon);
      data_->selected_ = control;
    }

    simData::ObjectId    id_;
    simVis::View* view_;
    PlatformListData*    data_;
  };

  struct ClearTether : public ControlEventHandler
  {
    ClearTether(simVis::View* view, PlatformListData* data)
      : view_(view), data_(data) { }

    void onClick(Control* control, int mbm)
    {
      if (data_->selected_)
        data_->selected_->clearBackColor();

      view_->tetherCamera(nullptr);
      data_->selected_ = nullptr;
    }
    simVis::View* view_;
    PlatformListData*    data_;
  };

  struct BeamAntToggle : public ControlEventHandler
  {
    BeamAntToggle(simData::DataStore* ds, simData::ObjectId id, simVis::View* view, const std::string& antennaPattern)
      : ds_(ds), id_(id), view_(view), antennaPattern_(antennaPattern), origColorScale_(true), origFov_(-1.0f), origSense_(0.0f), origWeight_(true) { }

    void onValueChanged(Control* control, bool value)
    {
      simData::DataStore::Transaction xaction;
      simData::BeamPrefs* prefs = ds_->mutable_beamPrefs(id_, &xaction);

      prefs->set_drawtype(value ? simData::BeamPrefs::DrawType::ANTENNA_PATTERN : simData::BeamPrefs::DrawType::BEAM_3DB);

      if (value)
      {
        prefs->mutable_antennapattern()->set_type(simData::AntennaPatterns::Type::FILE);
        prefs->mutable_antennapattern()->set_filename(antennaPattern_);
        //prefs.set_gain(0);
        //prefs.set_frequency(7000);
        //prefs.set_horizontalwidth(0.15);
        //prefs.set_verticalwidth(0.113);

        //prefs.set_draw(true);

        origColorScale_ = prefs->colorscale();
        prefs->set_colorscale(true);

        origFov_ = prefs->fieldofview();
        if (origFov_ <= 0)
          prefs->set_fieldofview(M_PI_2);

        origSense_ = prefs->sensitivity();
        prefs->set_sensitivity(-50.0f);

        origWeight_ = prefs->weighting();
        prefs->set_weighting(true);
      }
      else
      {
        prefs->set_colorscale(origColorScale_);

        if (origFov_ > 0)
          prefs->set_fieldofview(origFov_);
        else
          prefs->clear_fieldofview();

        if (origSense_ != 0.0)
          prefs->set_sensitivity(origSense_);
        else
          prefs->clear_sensitivity();

        prefs->set_weighting(origWeight_);
      }

      xaction.complete(&prefs);
    }

    simData::DataStore*  ds_;
    simData::ObjectId    id_;
    simVis::View* view_;
    const std::string    antennaPattern_;

    bool  origColorScale_;
    float origFov_;
    float origSense_;
    bool  origWeight_;
  };

  struct RCSToggle : public ControlEventHandler
  {
    RCSToggle(simData::ObjectId id, simData::DataStore* dataStore)
      : id_(id), dataStore_(dataStore) { }

    void onValueChanged(Control* control, bool value)
    {
      simData::DataStore::Transaction xaction;
      simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(id_, &xaction);
      prefs->set_drawrcs(value);
      prefs->set_draw3drcs(value);
      xaction.complete(&prefs);
    }

    simData::ObjectId    id_;
    simData::DataStore*  dataStore_;
  };

  struct DynScaleToggle : public ControlEventHandler
  {
    DynScaleToggle(simData::ObjectId id, simData::DataStore* ds)
      : id_(id), ds_(ds) { }

    void onValueChanged(Control* control, bool value)
    {
      simData::DataStore::Transaction xaction;
      simData::PlatformPrefs* prefs = ds_->mutable_platformPrefs(id_, &xaction);
      prefs->set_dynamicscale(value);
      xaction.complete(&prefs);
    }

    simData::ObjectId    id_;
    simData::DataStore*  ds_;
  };

  struct DynScaleToggleGlobal : public ControlEventHandler
  {
    DynScaleToggleGlobal(simVis::View* view, simData::DataStore* dataStore)
      : dataStore_(dataStore), view_(view) { }

    void onValueChanged(Control* control, bool value)
    {
      simData::DataStore::IdList ids;
      dataStore_->idList(&ids, simData::PLATFORM);

      for (simData::DataStore::IdList::const_iterator i = ids.begin(); i != ids.end(); ++i)
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(*i, &xaction);
        prefs->set_dynamicscale(value);
        xaction.complete(&prefs);
      }
    }

    simData::DataStore*  dataStore_;
    simVis::View* view_;
  };

  struct LabelToggleGlobal : public ControlEventHandler
  {
    explicit LabelToggleGlobal(simData::DataStore* dataStore) : dataStore_(dataStore) { }

    void onValueChanged(Control* control, bool value)
    {
      simData::DataStore::IdList ids;
      dataStore_->idList(&ids, simData::PLATFORM);

      for (simData::DataStore::IdList::const_iterator i = ids.begin(); i != ids.end(); ++i)
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(*i, &xaction);
        prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(value);
        xaction.complete(&prefs);
      }
    }
    simData::DataStore*  dataStore_;
  };

  struct DeclutterToggle : public ControlEventHandler
  {
    void onValueChanged(Control* control, bool value)
    {
      osgEarth::ScreenSpaceLayout::setDeclutteringEnabled(value);
    }
  };

  // listens for new platforms and adds them to the platform control list.
  struct NewPlatformListener : public simData::DataStore::DefaultListener
  {
    NewPlatformListener(Container* container, simVis::View* view, PlatformListData* data)
      : container_(container), view_(view), data_(data) { }

    virtual void onAddEntity(simData::DataStore *ds, simData::ObjectId newId, simData::ObjectType ot)
    {
      if (ot != simData::PLATFORM)
        return;

      simData::DataStore::Transaction xaction;
      const simData::PlatformProperties *props = ds->platformProperties(newId, &xaction);

      simData::DataStore::Transaction xactionPrefs;
      const simData::PlatformPrefs *prefs = ds->platformPrefs(newId, &xactionPrefs);

      ControlVector row;

      std::string name = prefs->commonprefs().name();
      LabelControl* label = new LabelControl(name, 14);
      label->setActiveColor(0, 0, 1, 1);
      label->addEventHandler(new PlatformClick(props->id(), view_, data_));
      row.push_back(label);

      //Control* dynamicScale = new CheckBoxControl();
      //dynamicScale->addEventHandler( new DynScaleToggle(props->id(), view_) );
      //row.push_back( dynamicScale );

      Control* rcs = new CheckBoxControl();
      rcs->addEventHandler(new RCSToggle(props->id(), ds));
      row.push_back(rcs);

      container_->addControls(row);

      // insert the label into a lookup table so we can change it later
      data_->labelsById_[newId] = label;
    }

    Container*           container_;
    simVis::View* view_;
    PlatformListData*    data_;
  };

  struct ControlPrefsListener
  {
    virtual ~ControlPrefsListener() {}
    virtual void notifyPrefsChange(simData::DataStore &ds, simData::ObjectId id) =0;
  };

  struct ControlPrefsNotification : public simData::DataStore::DefaultListener
  {
    explicit ControlPrefsNotification(ControlPrefsListener* listener) : listener_(listener) { }

    virtual void onPrefsChange(simData::DataStore* ds, simData::ObjectId id) override
    {
      if (ds && ds->objectType(id) == simData::PLATFORM)
        listener_->notifyPrefsChange(*ds, id);
    }
    ControlPrefsListener* listener_ = nullptr;
  };

  struct PlatformListControl : public VBox, public ControlPrefsListener
  {
    PlatformListControl(
      simVis::View* view,
      simData::DataStore*  dataStore)
    {
      Grid* grid = new Grid();
      grid->setChildHorizAlign(Control::ALIGN_CENTER);
      grid->setChildVertAlign(Control::ALIGN_CENTER);

      grid->setControl(0, 0, new LabelControl("Platform", 14, Color::Yellow));
      //setControl(1,0,new LabelControl("DS", 14, Color::Yellow));
      grid->setControl(1, 0, new LabelControl("RCS", 14, Color::Yellow));

      dataStore->addListener(simData::DataStore::ListenerPtr(
        new NewPlatformListener(grid, view, &data_)));

      this->addControl(grid);

      LabelControl* instr = new LabelControl("Clear tether", 14, simVis::Color::Yellow);
      instr->setActiveColor(Color::Blue);
      instr->addEventHandler(new ClearTether(view, &data_));
      this->addControl(instr);

      HBox* dynScaleBox = this->addControl(new HBox());
      dynScaleBox->addControl(new CheckBoxControl(false, new DynScaleToggleGlobal(view, dataStore)));
      dynScaleBox->addControl(new LabelControl("Dynamic Scaling", 14));

      HBox* labelBox = this->addControl(new HBox());
      labelBox->addControl(new CheckBoxControl(false, new LabelToggleGlobal(dataStore)));
      labelBox->addControl(new LabelControl("Show Labels", 14));

      HBox* declutterBox = this->addControl(new HBox());
      declutterBox->addControl(new CheckBoxControl(true, new DeclutterToggle()));
      declutterBox->addControl(new LabelControl("Declutter Labels", 14));

      setPadding(10);
      setBackColor(0, 0, 0, 0.5);
      setAbsorbEvents(true);

      // set up a notification listener to capture name change events.
      dataStore->addListener(simData::DataStore::ListenerPtr(new ControlPrefsNotification(this)));
    }

    void notifyPrefsChange(simData::DataStore &ds, simData::ObjectId id)
    {
      LabelControl* label = data_.labelsById_[id];

      simData::DataStore::Transaction xaction;
      const simData::PlatformPrefs* prefs = ds.platformPrefs(id, &xaction);
      label->setText(prefs->commonprefs().name());
      xaction.complete(&prefs);
    }

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simUtil"; }

    /** Return the class name */
    virtual const char* className() const { return "PlatformListControl"; }

    PlatformListData data_;
  };

  // listens for new beams and adds them to the beam control list.
  struct NewBeamListener : public simData::DataStore::DefaultListener
  {
    NewBeamListener(Container* container, simVis::View* view, BeamListData* data,
      const std::string& antennaPattern,
      simData::DataStore* dataStore)
      : container_(container), view_(view), data_(data), antennaPattern_(antennaPattern), dataStore_(dataStore) { }

    virtual void onAddEntity(simData::DataStore *ds, simData::ObjectId newId, simData::ObjectType ot)
    {
      if (ot != simData::BEAM)
        return;

      simData::ObjectId hostId = 0;
      {
        simData::DataStore::Transaction x;
        const simData::BeamProperties *props = ds->beamProperties(newId, &x);
        hostId = props->hostid();
      }

      ControlVector row;

      // access the name of the host platform
      std::string name;
      {
        simData::DataStore::Transaction xpp;
        const simData::PlatformPrefs* platformPrefs = dataStore_->platformPrefs(hostId, &xpp);
        name = platformPrefs->commonprefs().name() + " :: " + name;
        xpp.complete(&platformPrefs);
      }

      Control* label = new LabelControl(name, 14);
      label->setActiveColor(0, 0, 1, 1);
      row.push_back(label);

      Control* ant = new CheckBoxControl();
      ant->addEventHandler(new BeamAntToggle(dataStore_, newId, view_, antennaPattern_));
      row.push_back(ant);

      container_->addControls(row);
    }

    Container*           container_;
    simVis::View* view_;
    BeamListData*        data_;
    const std::string    antennaPattern_;
    simData::DataStore*  dataStore_;
  };

  struct BeamListControl : public VBox
  {
    BeamListControl(
      simVis::View* view,
      simData::DataStore*  dataStore,
      const std::string&   antennaPattern)
    {
      Grid* grid = new Grid();
      grid->setChildHorizAlign(Control::ALIGN_CENTER);
      grid->setChildVertAlign(Control::ALIGN_CENTER);

      grid->setControl(0, 0, new LabelControl("Beam", 14, Color::Yellow));
      grid->setControl(1, 0, new LabelControl("Antenna Pattern", 14, Color::Yellow));

      dataStore->addListener(simData::DataStore::ListenerPtr(
        new NewBeamListener(grid, view, &data_, antennaPattern, dataStore)));

      this->addControl(grid);

      setPadding(10);
      setBackColor(0, 0, 0, 0.5);
      setAbsorbEvents(true);
    }

    BeamListData data_;
  };

  struct VCRTimeHandler : public osgGA::GUIEventHandler
  {
    VCRTimeHandler(
      simData::DataStore* dataStore,
      HSliderControl*     slider,
      LabelControl*       label,
      double*             simTime,
      bool*               isPlaying)
    {
      dataStore_ = dataStore;
      timeSlider_ = slider;
      timeLabel_  = label;
      isPlaying_ = isPlaying;
      simTime_   = simTime;
      startTime_ = -1.0;
      lastTime_  = 0.0;
      *simTime_  = 0.0;
    }

    bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
    {
      if (ea.getEventType() != osgGA::GUIEventAdapter::FRAME)
        return false;

      const double now = ea.getTime();

      if (*isPlaying_)
      {
        *simTime_ += (now - lastTime_);
        timeSlider_->setValue(*simTime_, false);

        dataStore_->update(*simTime_);

        std::stringstream ss;
        ss << *simTime_;
        timeLabel_->setText(ss.str());
      }
      lastTime_ = now;

      return false;
    }

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simUtil"; }

    /** Return the class name */
    virtual const char* className() const { return "VCRTimeHandler"; }

  protected:
    double startTime_, lastTime_;
    simData::DataStore* dataStore_;
    HSliderControl*     timeSlider_;
    LabelControl*       timeLabel_;
    bool*               isPlaying_;
    double*             simTime_;
  };

  struct PlayStopToggle : public ControlEventHandler
  {
    explicit PlayStopToggle(bool* isPlaying): isPlaying_(isPlaying) {}
    void onClick(Control* button, int mbmask)
    {
      *isPlaying_ = !(*isPlaying_);
      LabelControl* label = static_cast<LabelControl*>(button);
      label->setText((*isPlaying_) ? "Stop" : "Play");
    }
    bool* isPlaying_;
  };

  struct TimeSlide : public ControlEventHandler
  {
    TimeSlide(double* simTime, simData::DataStore* dataStore, LabelControl* readout)
    : simTime_(simTime),
      dataStore_(dataStore),
      readout_(readout)
    {
    }

    void onValueChanged(Control* control, float value)
    {
      *simTime_ = value;
      dataStore_->update(*simTime_);
      std::stringstream buf;
      buf << (*simTime_);
      readout_->setText(buf.str());
    }

    double*             simTime_;
    simData::DataStore* dataStore_;
    LabelControl*       readout_;
  };

  struct VCRControl : public HBox
  {
    VCRControl(
      simVis::View* view,
      simData::DataStore*  dataStore)
    {
      this->setBackColor(0, 0, 0, 0.5);
      this->setPadding(10);
      this->setAbsorbEvents(true);

      isPlaying_ = false;

      LabelControl* playStop = new LabelControl("Play");
      playStop->setActiveColor(Color::Blue);
      playStop->addEventHandler(new PlayStopToggle(&isPlaying_));
      this->addControl(playStop);

      LabelControl* readout = new LabelControl("0.0");

      HSliderControl* slider = new HSliderControl(0.0, 1000.0, 0.0);
      slider->setSize(400, 20);
      slider->addEventHandler(new TimeSlide(&simTime_, dataStore, readout));
      this->addControl(slider);

      this->addControl(readout);

      view->addEventHandler(new VCRTimeHandler(dataStore, slider, readout, &simTime_, &isPlaying_));
    }

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simUtil"; }

    /** Return the class name */
    virtual const char* className() const { return "VCRControl"; }

    double simTime_;
    bool   isPlaying_;
  };
}

#endif

//----------------------------------------------------------------------

osgEarth::Util::Controls::Control* simExamples::createPlatformListControl(simVis::View* view, simData::DataStore* dataStore)
{
#ifdef ENABLE_DEPRECATED_SIMDISSDK_API
  Control* control = new PlatformListControl(view, dataStore);
  control->setHorizAlign(Control::ALIGN_RIGHT);
  control->setVertAlign(Control::ALIGN_TOP);
  return control;
#else
  return nullptr;
#endif
}

osgEarth::Util::Controls::Control* simExamples::createBeamListControl(simVis::View* view, simData::DataStore* dataStore, const std::string& antennaPattern)
{
#ifdef ENABLE_DEPRECATED_SIMDISSDK_API
  Control* control = new BeamListControl(view, dataStore, antennaPattern);
  control->setHorizAlign(Control::ALIGN_LEFT);
  control->setVertAlign(Control::ALIGN_BOTTOM);
  control->setMargin(osgEarth::Util::Controls::Gutter(0, 0, 50, 0));
  return control;
#else
  return nullptr;
#endif
}

osgEarth::Util::Controls::Control* simExamples::createVCRControl(simVis::View* view, simData::DataStore* dataStore)
{
#ifdef ENABLE_DEPRECATED_SIMDISSDK_API
  Control* control = new VCRControl(view, dataStore);
  control->setHorizAlign(Control::ALIGN_LEFT);
  control->setVertAlign(Control::ALIGN_BOTTOM);
  return control;
#else
  return nullptr;
#endif
}
