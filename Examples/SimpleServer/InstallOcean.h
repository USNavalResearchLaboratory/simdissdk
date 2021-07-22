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
#ifndef SS_INSTALLOCEAN_H
#define SS_INSTALLOCEAN_H

#include <string>

namespace osg { class ArgumentParser; }
namespace simVis { class SceneManager; }

namespace SimpleServer {

/** Encapsulates routines for installing an ocean to the scene. */
class InstallOcean
{
public:
  InstallOcean();

  /** Detect configuration from arguments */
  void set(osg::ArgumentParser& args);

  /** Do not configure an ocean on install */
  void setNone();
  /** Configure to use Simple ocean, with provided bathymetry offset (positive value, or 0 for none) */
  void setSimple(double bathymetryOffset);
  /** Configure to use Triton ocean, with provided license details and bathymetry offset */
  void setTriton(double bathymetryOffset, const std::string& user, const std::string& license, const std::string& resourcePath="");

  /** Installs the ocean on the scene provided */
  void install(simVis::SceneManager& scene);

private:
  enum OceanType {
    NONE,
    SIMPLE,
    TRITON
  };

  OceanType type_;
  double bathymetryOffset_;
  std::string user_;
  std::string license_;
  std::string resourcePath_;
};

}

#endif /* SS_INSTALLOCEAN_H */
