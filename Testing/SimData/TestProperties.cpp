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

#include "simCore/Common/SDKAssert.h"
#include "simData/DataTypeProperties.h"

namespace {

int testBeamProperties()
{
  int rv = 0;

  simData::BeamProperties prop;

  // Verify id field
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.clear_id();
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_id());

  // Verify hostId field
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.clear_hostid();
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_hostid());

  // Verify originaId field
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.clear_originalid();
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_originalid());

  // Verify type field
  rv += SDK_ASSERT(!prop.has_type());
  prop.set_type(simData::BeamProperties::Type::ABSOLUTE_POSITION);
  rv += SDK_ASSERT(prop.has_type());
  prop.clear_type();
  rv += SDK_ASSERT(!prop.has_type());
  prop.set_type(simData::BeamProperties::Type::BODY_RELATIVE);
  rv += SDK_ASSERT(prop.has_type());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_type());

  // Verify source field
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.clear_source();
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_source());

  // Verify MergeFrom
  simData::BeamProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify id field
  prop.set_id(1);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop.id() == prop2.id());
  rv += SDK_ASSERT(prop == prop2);

  // Verify hostId field
  prop.set_hostid(2);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop.hostid() == prop2.hostid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify originalId field
  prop.set_originalid(3);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop.originalid() == prop2.originalid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify type field
  prop.set_type(simData::BeamProperties::Type::TARGET);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.type() == simData::BeamProperties::Type::TARGET);
  rv += SDK_ASSERT(prop.type() == prop2.type());
  rv += SDK_ASSERT(prop == prop2);

  // Verify source field
  prop.set_source("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.source() == "Test");
  rv += SDK_ASSERT(prop.source() == prop2.source());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_id());
  rv += SDK_ASSERT(!prop2.has_hostid());
  rv += SDK_ASSERT(!prop2.has_originalid());
  rv += SDK_ASSERT(!prop2.has_type());
  rv += SDK_ASSERT(!prop2.has_source());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop2.type() == simData::BeamProperties::Type::TARGET);
  rv += SDK_ASSERT(prop2.source() == "Test");

  return rv;
}

int testClassificationProperties()
{
  int rv = 0;

  simData::ClassificationProperties prop;

  // Verify label field
  rv += SDK_ASSERT(!prop.has_label());
  prop.set_label("Test");
  rv += SDK_ASSERT(prop.has_label());
  prop.clear_label();
  rv += SDK_ASSERT(!prop.has_label());
  prop.set_label("Test");
  rv += SDK_ASSERT(prop.has_label());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_label());

  // Verify fontcolor field
  rv += SDK_ASSERT(!prop.has_fontcolor());
  prop.set_fontcolor(1);
  rv += SDK_ASSERT(prop.has_fontcolor());
  prop.clear_fontcolor();
  rv += SDK_ASSERT(!prop.has_fontcolor());
  prop.set_fontcolor(1);
  rv += SDK_ASSERT(prop.has_fontcolor());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_fontcolor());

  // Verify MergeFrom
  simData::ClassificationProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify label field
  prop.set_label("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.label() == "Test");
  rv += SDK_ASSERT(prop.label() == prop2.label());
  rv += SDK_ASSERT(prop == prop2);

  // Verify fontColor field
  prop.set_fontcolor(2);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.fontcolor() == 2);
  rv += SDK_ASSERT(prop.fontcolor() == prop2.fontcolor());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_label());
  rv += SDK_ASSERT(!prop2.has_fontcolor());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.label() == "Test");
  rv += SDK_ASSERT(prop2.fontcolor() == 2);

  return rv;
}

int testCoordinateFrameProperties()
{
  int rv = 0;

  simData::CoordinateFrameProperties prop;

  // Verify referencella field
  rv += SDK_ASSERT(!prop.has_referencella());
  prop.mutable_referencella()->set_lat(1.0);
  rv += SDK_ASSERT(prop.has_referencella());
  prop.clear_referencella();
  rv += SDK_ASSERT(!prop.has_referencella());
  prop.mutable_referencella()->set_lat(1.0);
  rv += SDK_ASSERT(prop.has_referencella());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_referencella());

  // Verify tangentplaneoffset field
  rv += SDK_ASSERT(!prop.has_tangentplaneoffset());
  prop.mutable_tangentplaneoffset()->set_tx(1.0);
  rv += SDK_ASSERT(prop.has_tangentplaneoffset());
  prop.clear_tangentplaneoffset();
  rv += SDK_ASSERT(!prop.has_tangentplaneoffset());
  prop.mutable_tangentplaneoffset()->set_tx(1.0);
  rv += SDK_ASSERT(prop.has_tangentplaneoffset());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_tangentplaneoffset());

  // Verify coordinatesystem field
  rv += SDK_ASSERT(!prop.has_coordinatesystem());
  prop.set_coordinatesystem(simData::CoordinateSystemProperties::ECEF);
  rv += SDK_ASSERT(prop.has_coordinatesystem());
  prop.clear_coordinatesystem();
  rv += SDK_ASSERT(!prop.has_coordinatesystem());
  prop.set_coordinatesystem(simData::CoordinateSystemProperties::ECEF);
  rv += SDK_ASSERT(prop.has_coordinatesystem());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_coordinatesystem());

  // Verify magneticvariance field
  rv += SDK_ASSERT(!prop.has_magneticvariance());
  prop.set_magneticvariance(simData::MagneticVarianceProperties::MV_TRUE);
  rv += SDK_ASSERT(prop.has_magneticvariance());
  prop.clear_magneticvariance();
  rv += SDK_ASSERT(!prop.has_magneticvariance());
  prop.set_magneticvariance(simData::MagneticVarianceProperties::MV_TRUE);
  rv += SDK_ASSERT(prop.has_magneticvariance());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_magneticvariance());

  // Verify verticaldatum field
  rv += SDK_ASSERT(!prop.has_verticaldatum());
  prop.set_verticaldatum(simData::VerticalDatumProperties::VD_MSL);
  rv += SDK_ASSERT(prop.has_verticaldatum());
  prop.clear_verticaldatum();
  rv += SDK_ASSERT(!prop.has_verticaldatum());
  prop.set_verticaldatum(simData::VerticalDatumProperties::VD_MSL);
  rv += SDK_ASSERT(prop.has_verticaldatum());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_verticaldatum());

  // Verify magneticvarianceuservalue field
  rv += SDK_ASSERT(!prop.has_magneticvarianceuservalue());
  prop.set_magneticvarianceuservalue(1.0);
  rv += SDK_ASSERT(prop.has_magneticvarianceuservalue());
  prop.clear_magneticvarianceuservalue();
  rv += SDK_ASSERT(!prop.has_magneticvarianceuservalue());
  prop.set_magneticvarianceuservalue(1.0);
  rv += SDK_ASSERT(prop.has_magneticvarianceuservalue());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_magneticvarianceuservalue());

  // Verify verticaldatumuservalue field
  rv += SDK_ASSERT(!prop.has_verticaldatumuservalue());
  prop.set_verticaldatumuservalue(1.0);
  rv += SDK_ASSERT(prop.has_verticaldatumuservalue());
  prop.clear_verticaldatumuservalue();
  rv += SDK_ASSERT(!prop.has_verticaldatumuservalue());
  prop.set_verticaldatumuservalue(1.0);
  rv += SDK_ASSERT(prop.has_verticaldatumuservalue());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_verticaldatumuservalue());

  // Verify ecireferencetime field
  rv += SDK_ASSERT(!prop.has_ecireferencetime());
  prop.set_ecireferencetime(1.0);
  rv += SDK_ASSERT(prop.has_ecireferencetime());
  prop.clear_ecireferencetime();
  rv += SDK_ASSERT(!prop.has_ecireferencetime());
  prop.set_ecireferencetime(1.0);
  rv += SDK_ASSERT(prop.has_ecireferencetime());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_ecireferencetime());

  // Verify MergeFrom
  simData::CoordinateFrameProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify referencella field
  rv += SDK_ASSERT(!prop.has_referencella());
  prop.mutable_referencella()->set_lat(1.0);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop.referencella().lat() == 1.0);
  rv += SDK_ASSERT(prop.referencella().lat() == prop2.referencella().lat());
  rv += SDK_ASSERT(prop == prop2);

  // Verify tangentplaneoffset field
  rv += SDK_ASSERT(!prop.has_tangentplaneoffset());
  prop.mutable_tangentplaneoffset()->set_tx(2.0);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop.tangentplaneoffset().tx() == 2.0);
  rv += SDK_ASSERT(prop.tangentplaneoffset().tx() == prop2.tangentplaneoffset().tx());
  rv += SDK_ASSERT(prop == prop2);

  // Verify coordinatesystem field
  prop.set_coordinatesystem(simData::CoordinateSystemProperties::ECEF);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.coordinatesystem() == simData::CoordinateSystemProperties::ECEF);
  rv += SDK_ASSERT(prop.coordinatesystem() == prop2.coordinatesystem());
  rv += SDK_ASSERT(prop == prop2);

  // Verify magneticvariance field
  prop.set_magneticvariance(simData::MagneticVarianceProperties::MV_USER);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.magneticvariance() == simData::MagneticVarianceProperties::MV_USER);
  rv += SDK_ASSERT(prop.magneticvariance() == prop2.magneticvariance());
  rv += SDK_ASSERT(prop == prop2);

  // Verify verticaldatum field
  prop.set_verticaldatum(simData::VerticalDatumProperties::VD_WGS84);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.verticaldatum() == simData::VerticalDatumProperties::VD_WGS84);
  rv += SDK_ASSERT(prop.verticaldatum() == prop2.verticaldatum());
  rv += SDK_ASSERT(prop == prop2);

  // Verify magneticvarianceuservalue field
  prop.set_magneticvarianceuservalue(1);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.magneticvarianceuservalue() == 1);
  rv += SDK_ASSERT(prop.magneticvarianceuservalue() == prop2.magneticvarianceuservalue());
  rv += SDK_ASSERT(prop == prop2);

  // Verify verticaldatumuservalue field
  prop.set_verticaldatumuservalue(2);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.verticaldatumuservalue() == 2);
  rv += SDK_ASSERT(prop.verticaldatumuservalue() == prop2.verticaldatumuservalue());
  rv += SDK_ASSERT(prop == prop2);

  // Verify ecireferencetime field
  prop.set_ecireferencetime(3);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.ecireferencetime() == 3);
  rv += SDK_ASSERT(prop.ecireferencetime() == prop2.ecireferencetime());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_referencella());
  rv += SDK_ASSERT(!prop2.has_tangentplaneoffset());
  rv += SDK_ASSERT(!prop2.has_coordinatesystem());
  rv += SDK_ASSERT(!prop2.has_magneticvariance());
  rv += SDK_ASSERT(!prop2.has_verticaldatum());
  rv += SDK_ASSERT(!prop2.has_magneticvarianceuservalue());
  rv += SDK_ASSERT(!prop2.has_verticaldatumuservalue());
  rv += SDK_ASSERT(!prop2.has_ecireferencetime());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.has_referencella());
  rv += SDK_ASSERT(prop2.referencella().lat() == 1.0);
  rv += SDK_ASSERT(prop2.has_tangentplaneoffset());
  rv += SDK_ASSERT(prop2.tangentplaneoffset().tx() == 2.0);
  rv += SDK_ASSERT(prop2.coordinatesystem() == simData::CoordinateSystemProperties::ECEF);
  rv += SDK_ASSERT(prop2.magneticvariance() == simData::MagneticVarianceProperties::MV_USER);
  rv += SDK_ASSERT(prop2.verticaldatum() == simData::VerticalDatumProperties::VD_WGS84);
  rv += SDK_ASSERT(prop2.magneticvarianceuservalue() == 1.0);
  rv += SDK_ASSERT(prop2.verticaldatumuservalue() == 2.0);
  rv += SDK_ASSERT(prop2.ecireferencetime() == 3.0);

  return rv;
}

int testCustomRenderingProperties()
{
  int rv = 0;

  simData::CustomRenderingProperties prop;

  // Verify id field
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.clear_id();
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_id());

  // Verify hostId field
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.clear_hostid();
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_hostid());

  // Verify originaId field
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.clear_originalid();
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_originalid());

  // Verify renderer field
  rv += SDK_ASSERT(!prop.has_renderer());
  prop.set_renderer("Test");
  rv += SDK_ASSERT(prop.has_renderer());
  prop.clear_renderer();
  rv += SDK_ASSERT(!prop.has_renderer());
  prop.set_renderer("Test2");
  rv += SDK_ASSERT(prop.has_renderer());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_renderer());

  // Verify source field
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.clear_source();
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_source());

  // Verify MergeFrom
  simData::CustomRenderingProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify id field
  prop.set_id(1);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop.id() == prop2.id());
  rv += SDK_ASSERT(prop == prop2);

  // Verify hostId field
  prop.set_hostid(2);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop.hostid() == prop2.hostid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify originalId field
  prop.set_originalid(3);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop.originalid() == prop2.originalid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify renderer field
  prop.set_renderer("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.renderer() == "Test");
  rv += SDK_ASSERT(prop.renderer() == prop2.renderer());
  rv += SDK_ASSERT(prop == prop2);

  // Verify source field
  prop.set_source("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.source() == "Test");
  rv += SDK_ASSERT(prop.source() == prop2.source());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_id());
  rv += SDK_ASSERT(!prop2.has_hostid());
  rv += SDK_ASSERT(!prop2.has_originalid());
  rv += SDK_ASSERT(!prop2.has_renderer());
  rv += SDK_ASSERT(!prop2.has_source());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop2.renderer() == "Test");
  rv += SDK_ASSERT(prop2.source() == "Test");

  return rv;
}

int testGateProperties()
{
  int rv = 0;

  simData::GateProperties prop;

  // Verify id field
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.clear_id();
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_id());

  // Verify hostId field
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.clear_hostid();
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_hostid());

  // Verify originaId field
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.clear_originalid();
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_originalid());

  // Verify type field
  rv += SDK_ASSERT(!prop.has_type());
  prop.set_type(simData::GateProperties::Type::ABSOLUTE_POSITION);
  rv += SDK_ASSERT(prop.has_type());
  prop.clear_type();
  rv += SDK_ASSERT(!prop.has_type());
  prop.set_type(simData::GateProperties::Type::BODY_RELATIVE);
  rv += SDK_ASSERT(prop.has_type());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_type());

  // Verify source field
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.clear_source();
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_source());

  // Verify MergeFrom
  simData::GateProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify id field
  prop.set_id(1);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop.id() == prop2.id());
  rv += SDK_ASSERT(prop == prop2);

  // Verify hostId field
  prop.set_hostid(2);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop.hostid() == prop2.hostid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify originalId field
  prop.set_originalid(3);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop.originalid() == prop2.originalid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify type field
  prop.set_type(simData::GateProperties::Type::TARGET);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.type() == simData::GateProperties::Type::TARGET);
  rv += SDK_ASSERT(prop.type() == prop2.type());
  rv += SDK_ASSERT(prop == prop2);

  // Verify source field
  prop.set_source("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.source() == "Test");
  rv += SDK_ASSERT(prop.source() == prop2.source());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_id());
  rv += SDK_ASSERT(!prop2.has_hostid());
  rv += SDK_ASSERT(!prop2.has_originalid());
  rv += SDK_ASSERT(!prop2.has_type());
  rv += SDK_ASSERT(!prop2.has_source());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop2.type() == simData::GateProperties::Type::TARGET);
  rv += SDK_ASSERT(prop2.source() == "Test");

  return rv;
}

int testLaserProperties()
{
  int rv = 0;

  simData::LaserProperties prop;

  // Verify id field
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.clear_id();
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_id());

  // Verify hostId field
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.clear_hostid();
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_hostid());

  // Verify originaId field
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.clear_originalid();
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_originalid());

  // Verify coordinateSystem field
  rv += SDK_ASSERT(!prop.has_coordinatesystem());
  prop.set_coordinatesystem(simData::CoordinateSystemProperties::ECEF);
  rv += SDK_ASSERT(prop.has_coordinatesystem());
  prop.clear_coordinatesystem();
  rv += SDK_ASSERT(!prop.has_coordinatesystem());
  prop.set_coordinatesystem(simData::CoordinateSystemProperties::ECEF);
  rv += SDK_ASSERT(prop.has_coordinatesystem());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_coordinatesystem());

  // Verify azElRelativeToHostOri field
  rv += SDK_ASSERT(!prop.has_azelrelativetohostori());
  prop.set_azelrelativetohostori(true);
  rv += SDK_ASSERT(prop.has_azelrelativetohostori());
  prop.clear_azelrelativetohostori();
  rv += SDK_ASSERT(!prop.has_azelrelativetohostori());
  prop.set_azelrelativetohostori(false);
  rv += SDK_ASSERT(prop.has_azelrelativetohostori());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_azelrelativetohostori());

  // Verify source field
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.clear_source();
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_source());

  // Verify MergeFrom
  simData::LaserProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify id field
  prop.set_id(1);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop.id() == prop2.id());
  rv += SDK_ASSERT(prop == prop2);

  // Verify hostId field
  prop.set_hostid(2);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop.hostid() == prop2.hostid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify originalId field
  prop.set_originalid(3);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop.originalid() == prop2.originalid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify coordinateSystem field
  prop.set_coordinatesystem(simData::CoordinateSystemProperties::LLA);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.coordinatesystem() == simData::CoordinateSystemProperties::LLA);
  rv += SDK_ASSERT(prop.coordinatesystem() == prop2.coordinatesystem());
  rv += SDK_ASSERT(prop == prop2);

  // Verify azElRelativeToHostOri field
  prop.set_azelrelativetohostori(true);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.azelrelativetohostori() == true);
  rv += SDK_ASSERT(prop.azelrelativetohostori() == prop2.azelrelativetohostori());
  rv += SDK_ASSERT(prop == prop2);

  // Verify source field
  prop.set_source("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.source() == "Test");
  rv += SDK_ASSERT(prop.source() == prop2.source());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_id());
  rv += SDK_ASSERT(!prop2.has_hostid());
  rv += SDK_ASSERT(!prop2.has_originalid());
  rv += SDK_ASSERT(!prop2.has_coordinatesystem());
  rv += SDK_ASSERT(!prop2.has_azelrelativetohostori());
  rv += SDK_ASSERT(!prop2.has_source());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop2.coordinatesystem() == simData::CoordinateSystemProperties::LLA);
  rv += SDK_ASSERT(prop2.azelrelativetohostori() == true);
  rv += SDK_ASSERT(prop2.source() == "Test");

  return rv;
}

int testLobGroupProperties()
{
  int rv = 0;

  simData::LobGroupProperties prop;

  // Verify id field
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.clear_id();
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_id());

  // Verify hostId field
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.clear_hostid();
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_hostid());

  // Verify originaId field
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.clear_originalid();
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_originalid());

  // Verify coordinateSystem field
  rv += SDK_ASSERT(!prop.has_coordinatesystem());
  prop.set_coordinatesystem(simData::CoordinateSystemProperties::ECEF);
  rv += SDK_ASSERT(prop.has_coordinatesystem());
  prop.clear_coordinatesystem();
  rv += SDK_ASSERT(!prop.has_coordinatesystem());
  prop.set_coordinatesystem(simData::CoordinateSystemProperties::ECEF);
  rv += SDK_ASSERT(prop.has_coordinatesystem());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_coordinatesystem());

  // Verify azElRelativeToHostOri field
  rv += SDK_ASSERT(!prop.has_azelrelativetohostori());
  prop.set_azelrelativetohostori(true);
  rv += SDK_ASSERT(prop.has_azelrelativetohostori());
  prop.clear_azelrelativetohostori();
  rv += SDK_ASSERT(!prop.has_azelrelativetohostori());
  prop.set_azelrelativetohostori(false);
  rv += SDK_ASSERT(prop.has_azelrelativetohostori());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_azelrelativetohostori());

  // Verify source field
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.clear_source();
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_source());

  // Verify MergeFrom
  simData::LobGroupProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify id field
  prop.set_id(1);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop.id() == prop2.id());
  rv += SDK_ASSERT(prop == prop2);

  // Verify hostId field
  prop.set_hostid(2);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop.hostid() == prop2.hostid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify originalId field
  prop.set_originalid(3);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop.originalid() == prop2.originalid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify coordinateSystem field
  prop.set_coordinatesystem(simData::CoordinateSystemProperties::LLA);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.coordinatesystem() == simData::CoordinateSystemProperties::LLA);
  rv += SDK_ASSERT(prop.coordinatesystem() == prop2.coordinatesystem());
  rv += SDK_ASSERT(prop == prop2);

  // Verify azElRelativeToHostOri field
  prop.set_azelrelativetohostori(true);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.azelrelativetohostori() == true);
  rv += SDK_ASSERT(prop.azelrelativetohostori() == prop2.azelrelativetohostori());
  rv += SDK_ASSERT(prop == prop2);

  // Verify source field
  prop.set_source("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.source() == "Test");
  rv += SDK_ASSERT(prop.source() == prop2.source());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_id());
  rv += SDK_ASSERT(!prop2.has_hostid());
  rv += SDK_ASSERT(!prop2.has_originalid());
  rv += SDK_ASSERT(!prop2.has_coordinatesystem());
  rv += SDK_ASSERT(!prop2.has_azelrelativetohostori());
  rv += SDK_ASSERT(!prop2.has_source());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop2.coordinatesystem() == simData::CoordinateSystemProperties::LLA);
  rv += SDK_ASSERT(prop2.azelrelativetohostori() == true);
  rv += SDK_ASSERT(prop2.source() == "Test");

  return rv;
}

int testPlatformProperties()
{
  int rv = 0;

  simData::PlatformProperties prop;

  // Verify coordinateframe field
  rv += SDK_ASSERT(!prop.has_coordinateframe());
  prop.mutable_coordinateframe()->set_magneticvarianceuservalue(1.0);
  rv += SDK_ASSERT(prop.has_coordinateframe());
  prop.clear_coordinateframe();
  rv += SDK_ASSERT(!prop.has_coordinateframe());
  prop.mutable_coordinateframe()->set_magneticvarianceuservalue(1.0);
  rv += SDK_ASSERT(prop.has_coordinateframe());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_coordinateframe());

  // Verify id field
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.clear_id();
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_id());

  // Verify originaId field
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.clear_originalid();
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_originalid());

  // Verify source field
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.clear_source();
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_source());

  // Verify MergeFrom
  simData::PlatformProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify coordinateframe field
  rv += SDK_ASSERT(!prop.has_coordinateframe());
  prop.mutable_coordinateframe()->set_magneticvarianceuservalue(1.0);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop.coordinateframe().magneticvarianceuservalue() == 1.0);
  rv += SDK_ASSERT(prop.coordinateframe().magneticvarianceuservalue() == prop2.coordinateframe().magneticvarianceuservalue());
  rv += SDK_ASSERT(prop == prop2);

  // Verify id field
  prop.set_id(1);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop.id() == prop2.id());
  rv += SDK_ASSERT(prop == prop2);

  // Verify originalId field
  prop.set_originalid(3);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop.originalid() == prop2.originalid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify source field
  prop.set_source("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.source() == "Test");
  rv += SDK_ASSERT(prop.source() == prop2.source());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_coordinateframe());
  rv += SDK_ASSERT(!prop2.has_id());
  rv += SDK_ASSERT(!prop2.has_originalid());
  rv += SDK_ASSERT(!prop2.has_source());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.has_coordinateframe());
  rv += SDK_ASSERT(prop2.coordinateframe().magneticvarianceuservalue() == 1.0);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop2.source() == "Test");

  return rv;
}

int testProjectorProperties()
{
  int rv = 0;

  simData::ProjectorProperties prop;

  // Verify id field
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.clear_id();
  rv += SDK_ASSERT(!prop.has_id());
  prop.set_id(1);
  rv += SDK_ASSERT(prop.has_id());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_id());

  // Verify hostId field
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.clear_hostid();
  rv += SDK_ASSERT(!prop.has_hostid());
  prop.set_hostid(1);
  rv += SDK_ASSERT(prop.has_hostid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_hostid());

  // Verify originaId field
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.clear_originalid();
  rv += SDK_ASSERT(!prop.has_originalid());
  prop.set_originalid(1);
  rv += SDK_ASSERT(prop.has_originalid());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_originalid());

  // Verify source field
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.clear_source();
  rv += SDK_ASSERT(!prop.has_source());
  prop.set_source("Test");
  rv += SDK_ASSERT(prop.has_source());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_source());

  // Verify MergeFrom
  simData::ProjectorProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify id field
  prop.set_id(1);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop.id() == prop2.id());
  rv += SDK_ASSERT(prop == prop2);

  // Verify hostId field
  prop.set_hostid(2);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop.hostid() == prop2.hostid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify originalId field
  prop.set_originalid(3);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop.originalid() == prop2.originalid());
  rv += SDK_ASSERT(prop == prop2);

  // Verify source field
  prop.set_source("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.source() == "Test");
  rv += SDK_ASSERT(prop.source() == prop2.source());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_id());
  rv += SDK_ASSERT(!prop2.has_hostid());
  rv += SDK_ASSERT(!prop2.has_originalid());
  rv += SDK_ASSERT(!prop2.has_source());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.id() == 1);
  rv += SDK_ASSERT(prop2.hostid() == 2);
  rv += SDK_ASSERT(prop2.originalid() == 3);
  rv += SDK_ASSERT(prop2.source() == "Test");

  return rv;
}

int testReferenceProperties()
{
  int rv = 0;

  simData::ReferenceProperties prop;

  // Verify latitude field
  rv += SDK_ASSERT(!prop.has_lat());
  prop.set_lat(1.0);
  rv += SDK_ASSERT(prop.has_lat());
  prop.clear_lat();
  rv += SDK_ASSERT(!prop.has_lat());
  prop.set_lat(1.0);
  rv += SDK_ASSERT(prop.has_lat());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_lat());

  // Verify longitude field
  rv += SDK_ASSERT(!prop.has_lon());
  prop.set_lon(1.0);
  rv += SDK_ASSERT(prop.has_lon());
  prop.clear_lon();
  rv += SDK_ASSERT(!prop.has_lon());
  prop.set_lon(1.0);
  rv += SDK_ASSERT(prop.has_lon());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_lon());

  // Verify altitude field
  rv += SDK_ASSERT(!prop.has_alt());
  prop.set_alt(1.0);
  rv += SDK_ASSERT(prop.has_alt());
  prop.clear_alt();
  rv += SDK_ASSERT(!prop.has_alt());
  prop.set_alt(1.0);
  rv += SDK_ASSERT(prop.has_alt());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_alt());

  // Verify MergeFrom
  simData::ReferenceProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify latitude field
  prop.set_lat(1.0);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.lat() == 1.0);
  rv += SDK_ASSERT(prop.lat() == prop2.lat());
  rv += SDK_ASSERT(prop == prop2);

  // Verify longitude field
  prop.set_lon(2.0);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.lon() == 2.0);
  rv += SDK_ASSERT(prop.lon() == prop2.lon());
  rv += SDK_ASSERT(prop == prop2);

  // Verify altitude field
  prop.set_alt(3.0);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.alt() == 3.0);
  rv += SDK_ASSERT(prop.alt() == prop2.alt());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_lat());
  rv += SDK_ASSERT(!prop2.has_lon());
  rv += SDK_ASSERT(!prop2.has_alt());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.lat() == 1.0);
  rv += SDK_ASSERT(prop2.lon() == 2.0);
  rv += SDK_ASSERT(prop2.alt() == 3.0);

  return rv;
}

int testScenarioProperties()
{
  int rv = 0;

  simData::ScenarioProperties prop;

  // Verify coordinateframe field
  rv += SDK_ASSERT(!prop.has_coordinateframe());
  prop.mutable_coordinateframe()->set_magneticvarianceuservalue(1.0);
  rv += SDK_ASSERT(prop.has_coordinateframe());
  prop.clear_coordinateframe();
  rv += SDK_ASSERT(!prop.has_coordinateframe());
  prop.mutable_coordinateframe()->set_magneticvarianceuservalue(1.0);
  rv += SDK_ASSERT(prop.has_coordinateframe());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_coordinateframe());

  // Verify classification field
  rv += SDK_ASSERT(!prop.has_classification());
  prop.mutable_classification()->set_label("Test");
  rv += SDK_ASSERT(prop.has_classification());
  prop.clear_classification();
  rv += SDK_ASSERT(!prop.has_classification());
  prop.mutable_classification()->set_label("Test");
  rv += SDK_ASSERT(prop.has_classification());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_classification());

  // Verify soundFile field
  rv += SDK_ASSERT(!prop.has_soundfile());
  prop.mutable_soundfile()->set_starttime(1.0);
  rv += SDK_ASSERT(prop.has_soundfile());
  prop.clear_soundfile();
  rv += SDK_ASSERT(!prop.has_soundfile());
  prop.mutable_soundfile()->set_starttime(1.0);
  rv += SDK_ASSERT(prop.has_soundfile());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_soundfile());

  // Verify gogFile field
  rv += SDK_ASSERT(prop.gogfile_size() == 0);
  *prop.add_gogfile() = "Test";
  rv += SDK_ASSERT(prop.gogfile_size() == 1);
  rv += SDK_ASSERT(prop.gogfile(0) == "Test");
  prop.clear_gogfile();
  rv += SDK_ASSERT(prop.gogfile_size() == 0);
  prop.mutable_gogfile()->push_back("Test");
  rv += SDK_ASSERT(prop.gogfile_size() == 1);
  rv += SDK_ASSERT(prop.gogfile(0) == "Test");
  prop.Clear();
  rv += SDK_ASSERT(prop.gogfile_size() == 0);

  // Verify MergeFrom
  simData::ScenarioProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify coordinateframe field
  prop.mutable_coordinateframe()->set_magneticvarianceuservalue(1.0);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.coordinateframe().magneticvarianceuservalue() == 1.0);
  rv += SDK_ASSERT(prop.coordinateframe().magneticvarianceuservalue() == prop2.coordinateframe().magneticvarianceuservalue());
  rv += SDK_ASSERT(prop == prop2);

  // Verify classification field
  prop.mutable_classification()->set_label("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.classification().label() == "Test");
  rv += SDK_ASSERT(prop.classification().label() == prop2.classification().label());
  rv += SDK_ASSERT(prop == prop2);

  // Verify soundFile field
  prop.mutable_soundfile()->set_starttime(2.0);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.soundfile().starttime() == 2.0);
  rv += SDK_ASSERT(prop.soundfile().starttime() == prop2.soundfile().starttime());
  rv += SDK_ASSERT(prop == prop2);

  // Verify gogFile field
  *prop.add_gogfile() = "Test2";
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.gogfile(0) == "Test2");
  rv += SDK_ASSERT(prop.gogfile(0) == prop2.gogfile(0));
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_coordinateframe());
  rv += SDK_ASSERT(!prop2.has_classification());
  rv += SDK_ASSERT(!prop2.has_soundfile());
  rv += SDK_ASSERT(prop2.gogfile_size() == 0);
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.has_coordinateframe());
  rv += SDK_ASSERT(prop2.has_classification());
  rv += SDK_ASSERT(prop2.has_soundfile());
  rv += SDK_ASSERT(prop2.gogfile_size() == 1);

  return rv;
}

int testSoundProperties()
{
  int rv = 0;

  simData::SoundFileProperties prop;

  // Verify filename field
  rv += SDK_ASSERT(!prop.has_filename());
  prop.set_filename("Test");
  rv += SDK_ASSERT(prop.has_filename());
  prop.clear_filename();
  rv += SDK_ASSERT(!prop.has_filename());
  prop.set_filename("Test");
  rv += SDK_ASSERT(prop.has_filename());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_filename());

  // Verify startTime field
  rv += SDK_ASSERT(!prop.has_starttime());
  prop.set_starttime(1);
  rv += SDK_ASSERT(prop.has_starttime());
  prop.clear_starttime();
  rv += SDK_ASSERT(!prop.has_starttime());
  prop.set_starttime(1);
  rv += SDK_ASSERT(prop.has_starttime());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_starttime());

  // Verify endTime field
  rv += SDK_ASSERT(!prop.has_endtime());
  prop.set_endtime(1);
  rv += SDK_ASSERT(prop.has_endtime());
  prop.clear_endtime();
  rv += SDK_ASSERT(!prop.has_endtime());
  prop.set_endtime(1);
  rv += SDK_ASSERT(prop.has_endtime());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_endtime());

  // Verify MergeFrom
  simData::SoundFileProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify filename field
  prop.set_filename("Test");
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.filename() == "Test");
  rv += SDK_ASSERT(prop.filename() == prop2.filename());
  rv += SDK_ASSERT(prop == prop2);

  // Verify startTime field
  prop.set_starttime(1);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.starttime() == 1);
  rv += SDK_ASSERT(prop.starttime() == prop2.starttime());
  rv += SDK_ASSERT(prop == prop2);

  // Verify endTime field
  prop.set_endtime(2);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.endtime() == 2);
  rv += SDK_ASSERT(prop.endtime() == prop2.endtime());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_filename());
  rv += SDK_ASSERT(!prop2.has_starttime());
  rv += SDK_ASSERT(!prop2.has_endtime());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.filename() == "Test");
  rv += SDK_ASSERT(prop2.starttime() == 1);
  rv += SDK_ASSERT(prop2.endtime() == 2);

  return rv;
}

int testTangentPlaneOffsetsProperties()
{
  int rv = 0;

  simData::TangentPlaneOffsetsProperties prop;

  // Verify tx field
  rv += SDK_ASSERT(!prop.has_tx());
  prop.set_tx(1.0);
  rv += SDK_ASSERT(prop.has_tx());
  prop.clear_tx();
  rv += SDK_ASSERT(!prop.has_tx());
  prop.set_tx(1.0);
  rv += SDK_ASSERT(prop.has_tx());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_tx());

  // Verify ty field
  rv += SDK_ASSERT(!prop.has_ty());
  prop.set_ty(1.0);
  rv += SDK_ASSERT(prop.has_ty());
  prop.clear_ty();
  rv += SDK_ASSERT(!prop.has_ty());
  prop.set_ty(1.0);
  rv += SDK_ASSERT(prop.has_ty());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_ty());

  // Verify angle field
  rv += SDK_ASSERT(!prop.has_angle());
  prop.set_angle(1.0);
  rv += SDK_ASSERT(prop.has_angle());
  prop.clear_angle();
  rv += SDK_ASSERT(!prop.has_angle());
  prop.set_angle(1.0);
  rv += SDK_ASSERT(prop.has_angle());
  prop.Clear();
  rv += SDK_ASSERT(!prop.has_angle());

  // Verify MergeFrom
  simData::TangentPlaneOffsetsProperties prop2;

  // Start both as empty
  rv += SDK_ASSERT(prop == prop2);

  // Verify tx field
  prop.set_tx(1.0);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.tx() == 1.0);
  rv += SDK_ASSERT(prop.tx() == prop2.tx());
  rv += SDK_ASSERT(prop == prop2);

  // Verify ty field
  prop.set_ty(2.0);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.ty() == 2);
  rv += SDK_ASSERT(prop.ty() == prop2.ty());
  rv += SDK_ASSERT(prop == prop2);

  // Verify angle field
  prop.set_angle(3);
  rv += SDK_ASSERT(prop != prop2);
  prop2.MergeFrom(prop);
  rv += SDK_ASSERT(prop2.angle() == 3);
  rv += SDK_ASSERT(prop.angle() == prop2.angle());
  rv += SDK_ASSERT(prop == prop2);

  // Verify Copy
  prop2.Clear();
  rv += SDK_ASSERT(!prop2.has_tx());
  rv += SDK_ASSERT(!prop2.has_ty());
  rv += SDK_ASSERT(!prop2.has_angle());
  prop2.CopyFrom(prop);
  rv += SDK_ASSERT(prop == prop2);
  rv += SDK_ASSERT(prop2.tx() == 1.0);
  rv += SDK_ASSERT(prop2.ty() == 2.0);
  rv += SDK_ASSERT(prop2.angle() == 3.0);

  return rv;
}

}

int TestProperties(int argc, char* argv[])
{
  int rv = 0;

  rv += testBeamProperties();
  rv += testClassificationProperties();
  rv += testCoordinateFrameProperties();
  rv += testCustomRenderingProperties();
  rv += testGateProperties();
  rv += testLaserProperties();
  rv += testLobGroupProperties();
  rv += testPlatformProperties();
  rv += testProjectorProperties();
  rv += testReferenceProperties();
  rv += testScenarioProperties();
  rv += testSoundProperties();
  rv += testTangentPlaneOffsetsProperties();

  return rv;
}
