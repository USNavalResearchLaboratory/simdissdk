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

/** Beam commands */
class SDKDATA_EXPORT BeamCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(BeamCommand);
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, BeamPrefs);
  SIMDATA_DECLARE_FIELD(time_, time, double);
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Custom rendering commands */
class SDKDATA_EXPORT CustomRenderingCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(CustomRenderingCommand);
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, CustomRenderingPrefs);
  SIMDATA_DECLARE_FIELD(time_, time, double);
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Gate commands */
class SDKDATA_EXPORT GateCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(GateCommand);
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, GatePrefs);
  SIMDATA_DECLARE_FIELD(time_, time, double);
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Laser commands */
class SDKDATA_EXPORT LaserCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(LaserCommand);
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, LaserPrefs);
  SIMDATA_DECLARE_FIELD(time_, time, double);
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** LOB Group commands */
class SDKDATA_EXPORT LobGroupCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(LobGroupCommand);
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, LobGroupPrefs);
  SIMDATA_DECLARE_FIELD(time_, time, double);
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Platform commands */
class SDKDATA_EXPORT PlatformCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(PlatformCommand);
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, PlatformPrefs);
  SIMDATA_DECLARE_FIELD(time_, time, double);
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

/** Projector commands */
class SDKDATA_EXPORT ProjectorCommand : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(ProjectorCommand);
  SIMDATA_DECLARE_SUBFIELD_LIST(updatePrefs_, updateprefs, ProjectorPrefs);
  SIMDATA_DECLARE_FIELD(time_, time, double);
  SIMDATA_DECLARE_FIELD(isClearCommand_, isclearcommand, bool);
};

}

#endif
