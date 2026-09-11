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
#ifndef SIMDATA_ENTITY_COMMANDS_H
#define SIMDATA_ENTITY_COMMANDS_H

#include "EntityPreferences.h"

namespace simData
{

/** Timed update to beam prefs; commands override prefs */
class SDKDATA_EXPORT BeamCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(BeamCommand);
  /// The prefs message that will contain the command's settings
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, BeamPrefs);
  /// The time at which the command should be executed
  SIMDATA_DECLARE_FIELD(time_, time, double);
  /// Indicates that the command will clear the values that are set in updatePrefs from the command set
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Timed update to Custom Rendering prefs; commands override prefs */
class SDKDATA_EXPORT CustomRenderingCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(CustomRenderingCommand);
  /// The prefs message that will contain the command's settings
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, CustomRenderingPrefs);
  /// The time at which the command should be executed
  SIMDATA_DECLARE_FIELD(time_, time, double);
  /// Indicates that the command will clear the values that are set in updatePrefs from the command set
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Timed update to gate prefs; commands override prefs */
class SDKDATA_EXPORT GateCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(GateCommand);
  /// The prefs message that will contain the command's settings
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, GatePrefs);
  /// The time at which the command should be executed
  SIMDATA_DECLARE_FIELD(time_, time, double);
  /// Indicates that the command will clear the values that are set in updatePrefs from the command set
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Timed update to laser prefs; commands override prefs */
class SDKDATA_EXPORT LaserCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(LaserCommand);
  /// The prefs message that will contain the command's settings
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, LaserPrefs);
  /// The time at which the command should be executed
  SIMDATA_DECLARE_FIELD(time_, time, double);
  /// Indicates that the command will clear the values that are set in updatePrefs from the command set
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Timed update to line of bearing (LOB) group prefs; commands override prefs */
class SDKDATA_EXPORT LobGroupCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(LobGroupCommand);
  /// The prefs message that will contain the command's settings
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, LobGroupPrefs);
  /// The time at which the command should be executed
  SIMDATA_DECLARE_FIELD(time_, time, double);
  /// Indicates that the command will clear the values that are set in updatePrefs from the command set
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Timed update to platform prefs; commands override prefs */
class SDKDATA_EXPORT PlatformCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(PlatformCommand);
  /// The prefs message that will contain the command's settings
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, PlatformPrefs);
  /// The time at which the command should be executed
  SIMDATA_DECLARE_FIELD(time_, time, double);
  /// Indicates that the command will clear the values that are set in updatePrefs from the command set
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Timed update to projector prefs; commands override prefs */
class SDKDATA_EXPORT ProjectorCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(ProjectorCommand);
  /// The prefs message that will contain the command's settings
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, ProjectorPrefs);
  /// The time at which the command should be executed
  SIMDATA_DECLARE_FIELD(time_, time, double);
  /// Indicates that the command will clear the values that are set in updatePrefs from the command set
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

}

#endif
