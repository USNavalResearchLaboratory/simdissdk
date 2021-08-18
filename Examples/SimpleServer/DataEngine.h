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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SS_DATAENGINE_H
#define SS_DATAENGINE_H

#include <vector>
#include "osg/Referenced"
#include "osg/observer_ptr"

namespace simData { class DataStore; }
namespace simVis { class ScenarioManager; }

namespace SimpleServer {

class CirclingPlatform;
class CyclingTargetBeam;
class DataGenerator;

/** Inserts "Simple Server" scenario data into the Data Store provided */
class DataEngine : public osg::Referenced
{
public:
  DataEngine(simData::DataStore& dataStore, simVis::ScenarioManager& scenario);

protected:
  virtual ~DataEngine();

private:
  void generateData_();

  simData::DataStore& dataStore_;
  simVis::ScenarioManager& scenario_;
  class GenerateDataTimer;
  osg::ref_ptr<GenerateDataTimer> generateDataTimer_;

  std::vector<osg::ref_ptr<CirclingPlatform> > platforms_;
  double lastCreateTime_;
  osg::ref_ptr<CyclingTargetBeam> targetBeam_;

  std::vector<osg::ref_ptr<DataGenerator> > generators_;
};

}

#endif /* SS_DATAENGINE_H */
