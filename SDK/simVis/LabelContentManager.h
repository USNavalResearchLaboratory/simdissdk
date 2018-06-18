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

#ifndef SIMVIS_LABEL_CONTENT_MANAGER_H
#define SIMVIS_LABEL_CONTENT_MANAGER_H

#include <osg/Referenced>
#include <osg/ref_ptr>
#include "simData/DataTypes.h"
#include "simData/ObjectId.h"

namespace simVis
{
  /** Callback for the user to create custom label content for a platform.  */
  class LabelContentCallback : public osg::Referenced
  {
  public:
    /**
    * Returns a platform label content based on the given preference and update
    * @param prefs Preferences for the platform; must be valid
    * @param lastUpdate Location of platform; must be valid
    * @param fields Display fields to use when forming the display string
    * @return A label content based on the given preference and update; does not include platform name/alias
    */
    virtual std::string createString(const simData::PlatformPrefs& prefs, const simData::PlatformUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields) = 0;

    /**
    * Returns a beam label content based on the given preference and update
    * @param prefs Preferences for the beam; must be valid
    * @param lastUpdate Location of beam; must be valid
    * @param fields Display fields to use when forming the display string
    * @return A label content based on the given preference and update; does not include beam name/alias
    */
    virtual std::string createString(const simData::BeamPrefs& prefs, const simData::BeamUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields) = 0;

    /**
    * Returns a gate label content based on the given preference and update
    * @param prefs Preferences for the gate; must be valid
    * @param lastUpdate Location of gate; must be valid
    * @param fields Display fields to use when forming the display string
    * @return A label content based on the given preference and update; does not include gate name/alias
    */
    virtual std::string createString(const simData::GatePrefs& prefs, const simData::GateUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields) = 0;

    /**
    * Returns a laser label content based on the given preference and update
    * @param prefs Preferences for the laser; must be valid
    * @param lastUpdate Location of laser; must be valid
    * @param fields Display fields to use when forming the display string
    * @return A label content based on the given preference and update; does not include laser name/alias
    */
    virtual std::string createString(const simData::LaserPrefs& prefs, const simData::LaserUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields) = 0;

    /**
    * Returns a LOB Group label content based on the given preference and update
    * @param prefs Preferences for the LOB Group; must be valid
    * @param lastUpdate Location of LOB Group; must be valid
    * @param fields Display fields to use when forming the display string
    * @return A label content based on the given preference and update; does not include LOB Group name/alias
    */
    virtual std::string createString(const simData::LobGroupPrefs& prefs, const simData::LobGroupUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields) = 0;

    /**
    * Returns a projector label content based on the given preference and update
    * @param prefs Preferences for the projector; must be valid
    * @param lastUpdate Location of projector; must be valid
    * @param fields Display fields to use when forming the display string
    * @return A label content based on the given preference and update; does not include projector name/alias
    */
    virtual std::string createString(const simData::ProjectorPrefs& prefs, const simData::ProjectorUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields) = 0;

#ifdef ENABLE_CUSTOM_RENDERING
    /**
    * Returns a custom rendering label content based on the given preference
    * @param id Since there is no update for custom rendering, need the id get information for the text string
    * @param prefs Preferences for the custom rendering; must be valid
    * @param fields Display fields to use when forming the display string
    * @return A label content based on the given preference; does not include custom rendering name/alias
    */
    virtual std::string createString(simData::ObjectId id, const simData::CustomRenderingPrefs& prefs, const simData::LabelPrefs_DisplayFields& fields) = 0;
#endif

  protected:
    virtual ~LabelContentCallback() {}
  };

  /** Abstract class for creating callbacks for the different entity types */
  class SDKVIS_EXPORT LabelContentManager : public osg::Referenced
  {
  public:

    /** Callback for entity */
    virtual LabelContentCallback* createLabelContentCallback(simData::ObjectId id) = 0;

  protected:
    virtual ~LabelContentManager() {}
  };

  /** Returns "" for platform  */
  class NullEntityCallback : public simVis::LabelContentCallback
  {
  public:
    NullEntityCallback() {}

    virtual std::string createString(const simData::PlatformPrefs& prefs, const simData::PlatformUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields)
    {
      return "";
    }

    virtual std::string createString(const simData::BeamPrefs& prefs, const simData::BeamUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields)
    {
      return "";
    }

    virtual std::string createString(const simData::GatePrefs& prefs, const simData::GateUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields)
    {
      return "";
    }

    virtual std::string createString(const simData::LaserPrefs& prefs, const simData::LaserUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields)
    {
      return "";
    }

    virtual std::string createString(const simData::LobGroupPrefs& prefs, const simData::LobGroupUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields)
    {
      return "";
    }

    virtual std::string createString(const simData::ProjectorPrefs& prefs, const simData::ProjectorUpdate& lastUpdate, const simData::LabelPrefs_DisplayFields& fields)
    {
      return "";
    }

#ifdef ENABLE_CUSTOM_RENDERING
    virtual std::string createString(simData::ObjectId id, const simData::CustomRenderingPrefs& prefs, const simData::LabelPrefs_DisplayFields& fields)
    {
      return "";
    }
#endif

  protected:
    virtual ~NullEntityCallback() {}
  };

  /** Null object implementation for Null Label Content Manager */
  class NullLabelContentManager : public simVis::LabelContentManager
  {
  public:
    NullLabelContentManager() {}

    virtual simVis::LabelContentCallback* createLabelContentCallback(simData::ObjectId id)
    {
      return new NullEntityCallback();
    }

  protected:
    virtual ~NullLabelContentManager() {}

  };
}

#endif


