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

#include "EntityCommands.h"

namespace simData
{

SIMDATA_DEFINE_METHODS(BeamCommand);
SIMDATA_DEFINE_SUBFIELD_LIST(BeamCommand, updatePrefs_, updateprefs, BeamPrefs);
SIMDATA_DEFINE_FIELD(BeamCommand, time_, time, double, 0.0);
SIMDATA_DEFINE_FIELD(BeamCommand, isClearCommand_, isclearcommand, bool, false);

void BeamCommand::MergeFrom(const BeamCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(updatePrefs_, BeamPrefs, updateprefs);

  if (from.has_time())
    time_ = from.time_;

  if (from.has_isclearcommand())
    isClearCommand_ = from.isClearCommand_;
}

void BeamCommand::CopyFrom(const BeamCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(updatePrefs_, BeamPrefs, updateprefs);

  time_ = from.time_;
  isClearCommand_ = from.isClearCommand_;
}

bool BeamCommand::operator==(const BeamCommand& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(updatePrefs_, rhs);

  return ((time_ == rhs.time_) &&
    (isClearCommand_ == rhs.isClearCommand_));
}

void BeamCommand::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(updatePrefs_);
}

//-----------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(CustomRenderingCommand);
SIMDATA_DEFINE_SUBFIELD_LIST(CustomRenderingCommand, updatePrefs_, updateprefs, CustomRenderingPrefs);
SIMDATA_DEFINE_FIELD(CustomRenderingCommand, time_, time, double, 0.0);
SIMDATA_DEFINE_FIELD(CustomRenderingCommand, isClearCommand_, isclearcommand, bool, false);

void CustomRenderingCommand::MergeFrom(const CustomRenderingCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(updatePrefs_, CustomRenderingPrefs, updateprefs);

  if (from.has_time())
    time_ = from.time_;

  if (from.has_isclearcommand())
    isClearCommand_ = from.isClearCommand_;
}

void CustomRenderingCommand::CopyFrom(const CustomRenderingCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(updatePrefs_, CustomRenderingPrefs, updateprefs);

  time_ = from.time_;
  isClearCommand_ = from.isClearCommand_;
}

bool CustomRenderingCommand::operator==(const CustomRenderingCommand& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(updatePrefs_, rhs);

  return ((time_ == rhs.time_) &&
    (isClearCommand_ == rhs.isClearCommand_));
}

void CustomRenderingCommand::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(updatePrefs_);
}

//-----------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(GateCommand);
SIMDATA_DEFINE_SUBFIELD_LIST(GateCommand, updatePrefs_, updateprefs, GatePrefs);
SIMDATA_DEFINE_FIELD(GateCommand, time_, time, double, 0.0);
SIMDATA_DEFINE_FIELD(GateCommand, isClearCommand_, isclearcommand, bool, false);

void GateCommand::MergeFrom(const GateCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(updatePrefs_, GatePrefs, updateprefs);

  if (from.has_time())
    time_ = from.time_;

  if (from.has_isclearcommand())
    isClearCommand_ = from.isClearCommand_;
}

void GateCommand::CopyFrom(const GateCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(updatePrefs_, GatePrefs, updateprefs);

  time_ = from.time_;
  isClearCommand_ = from.isClearCommand_;
}

bool GateCommand::operator==(const GateCommand& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(updatePrefs_, rhs);

  return ((time_ == rhs.time_) &&
    (isClearCommand_ == rhs.isClearCommand_));
}

void GateCommand::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(updatePrefs_);
}

//-----------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(LaserCommand);
SIMDATA_DEFINE_SUBFIELD_LIST(LaserCommand, updatePrefs_, updateprefs, LaserPrefs);
SIMDATA_DEFINE_FIELD(LaserCommand, time_, time, double, 0.0);
SIMDATA_DEFINE_FIELD(LaserCommand, isClearCommand_, isclearcommand, bool, false);

void LaserCommand::MergeFrom(const LaserCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(updatePrefs_, LaserPrefs, updateprefs);

  if (from.has_time())
    time_ = from.time_;

  if (from.has_isclearcommand())
    isClearCommand_ = from.isClearCommand_;
}

void LaserCommand::CopyFrom(const LaserCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(updatePrefs_, LaserPrefs, updateprefs);

  time_ = from.time_;
  isClearCommand_ = from.isClearCommand_;
}

bool LaserCommand::operator==(const LaserCommand& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(updatePrefs_, rhs);

  return ((time_ == rhs.time_) &&
    (isClearCommand_ == rhs.isClearCommand_));
}

void LaserCommand::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(updatePrefs_);
}

//-----------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(LobGroupCommand);
SIMDATA_DEFINE_SUBFIELD_LIST(LobGroupCommand, updatePrefs_, updateprefs, LobGroupPrefs);
SIMDATA_DEFINE_FIELD(LobGroupCommand, time_, time, double, 0.0);
SIMDATA_DEFINE_FIELD(LobGroupCommand, isClearCommand_, isclearcommand, bool, false);

void LobGroupCommand::MergeFrom(const LobGroupCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(updatePrefs_, LobGroupPrefs, updateprefs);

  if (from.has_time())
    time_ = from.time_;

  if (from.has_isclearcommand())
    isClearCommand_ = from.isClearCommand_;
}

void LobGroupCommand::CopyFrom(const LobGroupCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(updatePrefs_, LobGroupPrefs, updateprefs);

  time_ = from.time_;
  isClearCommand_ = from.isClearCommand_;
}

bool LobGroupCommand::operator==(const LobGroupCommand& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(updatePrefs_, rhs);

  return ((time_ == rhs.time_) &&
    (isClearCommand_ == rhs.isClearCommand_));
}

void LobGroupCommand::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(updatePrefs_);
}

//-----------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(PlatformCommand);
SIMDATA_DEFINE_SUBFIELD_LIST(PlatformCommand, updatePrefs_, updateprefs, PlatformPrefs);
SIMDATA_DEFINE_FIELD(PlatformCommand, time_, time, double, 0.0);
SIMDATA_DEFINE_FIELD(PlatformCommand, isClearCommand_, isclearcommand, bool, false);

void PlatformCommand::MergeFrom(const PlatformCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(updatePrefs_, PlatformPrefs, updateprefs);

  if (from.has_time())
    time_ = from.time_;

  if (from.has_isclearcommand())
    isClearCommand_ = from.isClearCommand_;
}

void PlatformCommand::CopyFrom(const PlatformCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(updatePrefs_, PlatformPrefs, updateprefs);

  time_ = from.time_;
  isClearCommand_ = from.isClearCommand_;
}

bool PlatformCommand::operator==(const PlatformCommand& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(updatePrefs_, rhs);

  return ((time_ == rhs.time_) &&
    (isClearCommand_ == rhs.isClearCommand_));
}

void PlatformCommand::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(updatePrefs_);
}

//-----------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(ProjectorCommand);
SIMDATA_DEFINE_SUBFIELD_LIST(ProjectorCommand, updatePrefs_, updateprefs, ProjectorPrefs);
SIMDATA_DEFINE_FIELD(ProjectorCommand, time_, time, double, 0.0);
SIMDATA_DEFINE_FIELD(ProjectorCommand, isClearCommand_, isclearcommand, bool, false);

void ProjectorCommand::MergeFrom(const ProjectorCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(updatePrefs_, ProjectorPrefs, updateprefs);

  if (from.has_time())
    time_ = from.time_;

  if (from.has_isclearcommand())
    isClearCommand_ = from.isClearCommand_;
}

void ProjectorCommand::CopyFrom(const ProjectorCommand& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(updatePrefs_, ProjectorPrefs, updateprefs);

  time_ = from.time_;
  isClearCommand_ = from.isClearCommand_;
}

bool ProjectorCommand::operator==(const ProjectorCommand& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(updatePrefs_, rhs);

  return ((time_ == rhs.time_) &&
    (isClearCommand_ == rhs.isClearCommand_));
}

void ProjectorCommand::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(updatePrefs_);
}

}
