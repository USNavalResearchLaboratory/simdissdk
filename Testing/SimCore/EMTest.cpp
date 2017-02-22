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
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/EM/Propagation.h"
#include "simCore/EM/RadarCrossSection.h"
#include "simCore/Calc/Angle.h"

#define EXAMPLE_RCS_FILE                  "fake_rcs_3.rcs"

namespace
{
#ifdef WIN32
const char PATH_SEP = '\\';
#else
const char PATH_SEP = '/';
#endif


/*
RCS files are organized into hierarchical containers stored under polarity, frequency and elevation.
A given polarity can have one or more frequencies associated with it.
A given frequency can have one or more elevation values, and an elevation can have one or more data pairings of azimuth and RCS values.

If the beam's polarity is not found in the RCS file, -300 dB is returned.
If an Unknown polarization is specified by the beam, the RCS pattern will use the first polarization found in the data structure.
Frequency uses a nearest neighbor lookup. Azimuth and elevation values are interpolated.
*/

  int rcsTest(int argc, char* argv[])
  {
    int rv = 0;
    std::string filepath;

    if (argc == 2)
    {
      filepath = argv[1];
    }
    else if (getenv("SIMDIS_SDK_FILE_PATH"))
    {
      const char *env = getenv("SIMDIS_SDK_FILE_PATH");
      rv += SDK_ASSERT(env[0] != '\0');
      filepath = env;
      filepath += "/data/rcs/";
      filepath.append(EXAMPLE_RCS_FILE);
    }
    else
    {
      std::cerr << "Skipping test, failed to find RCS file" << std::endl;
      return 0;
    }

    simCore::RadarCrossSection* rcsData_ = simCore::RcsFileParser::loadRCSFile(filepath);
    rv += SDK_ASSERT(rcsData_!=NULL);

    simCore::PolarityType polarity;
    float freqHz;
    double elevR;
    double azimR;
    float tableValue;   // correct value as determined by visual inspection of data file
    float rcsValue_dB;

    // If the beam's polarity is not found in the RCS file, -300 dB is returned. (regardless of other inputs)
    polarity = simCore::POLARITY_LINEAR;
    freqHz = 9000.0f;
    elevR = (simCore::DEG2RAD * 0.0f);
    azimR = (simCore::DEG2RAD * 10.0f);
    rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
    rv += SDK_ASSERT(rcsValue_dB == -300);

    polarity = simCore::POLARITY_LINEAR;
    freqHz = 30000.0f;
    elevR = (simCore::DEG2RAD * 0.0f);
    azimR = (simCore::DEG2RAD * 10.0f);
    rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
    rv += SDK_ASSERT(rcsValue_dB == -300);

    polarity = simCore::POLARITY_LINEAR;
    freqHz = 9000.0f;
    elevR = (simCore::DEG2RAD * -20.0f);
    azimR = (simCore::DEG2RAD * 10.0f);
    rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
    rv += SDK_ASSERT(rcsValue_dB == -300);


    // test exact match lookups
    polarity = simCore::POLARITY_VERTICAL;
    freqHz = 9000.0f;
    elevR = (simCore::DEG2RAD * 0.0f);
    azimR = (simCore::DEG2RAD * 10.0f);
    tableValue = 40.0f;
    rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
    rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));

    polarity = simCore::POLARITY_HORIZONTAL;
    freqHz = 9000.0f;
    elevR = (simCore::DEG2RAD * 0.0f);
    azimR = (simCore::DEG2RAD * 10.0f);
    tableValue = 30.0f;
    rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
    rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));


    // test return for unknown polarity:
    // If an Unknown polarization is specified by the beam, the RCS pattern will use the first polarization found in the data structure.
    polarity = simCore::POLARITY_UNKNOWN;
    freqHz = 9000.0f;
    elevR = (simCore::DEG2RAD * 0.0f);
    azimR = (simCore::DEG2RAD * 10.0f);
    tableValue = 30.0f;
    rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
    rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));

    // Frequency uses a nearest neighbor lookup.
    {
      polarity = simCore::POLARITY_VERTICAL;
      freqHz = 9000.0f;
      elevR = (simCore::DEG2RAD * 0.0f);
      azimR = (simCore::DEG2RAD * 10.0f);
      tableValue = 40.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));

      freqHz = 8000.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));

      freqHz = 11000.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));
    }
    {
      polarity = simCore::POLARITY_VERTICAL;
      freqHz = 13000.0f;
      elevR = (simCore::DEG2RAD * 0.0f);
      azimR = (simCore::DEG2RAD * 10.0f);
      tableValue = 46.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));

      freqHz = 11001.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));

      freqHz = 14000.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));
    }
    {
      polarity = simCore::POLARITY_VERTICAL;
      freqHz = 15000.0f;
      elevR = (simCore::DEG2RAD * 0.0f);
      azimR = (simCore::DEG2RAD * 10.0f);
      tableValue = 37.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));

      freqHz = 14001.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));

      freqHz = 15001.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));
    }


    // test elevation angles that are outside table limits
    {
      // min table value
      polarity = simCore::POLARITY_VERTICAL;
      freqHz = 9000.0f;
      azimR = (simCore::DEG2RAD * 5.0);
      elevR = (simCore::DEG2RAD * 0.0);
      tableValue = 38.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));
      // value below min value clamps to min value
      elevR = (simCore::DEG2RAD * -10.0);
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));
    }

    {
      // max table value
      polarity = simCore::POLARITY_VERTICAL;
      freqHz = 9000.0f;
      azimR = (simCore::DEG2RAD * 5.0);
      elevR = (simCore::DEG2RAD * 60.0);
      tableValue = 33.8f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));
      // value above max value clamps to max value
      elevR = (simCore::DEG2RAD * 70.0);
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue, 4e-06));
    }

    // test interpolation of elevation angle; interpolation operates on values in linear scale, not dB.
    {
      polarity = simCore::POLARITY_VERTICAL;
      freqHz = 9000.0f;
      azimR = simCore::DEG2RAD * 5.0;

      elevR = (simCore::DEG2RAD * 0.0);
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, 38.0f, 4e-06));
      float rcsValueL1 = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);

      elevR = (simCore::DEG2RAD * 30.0);
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, 35.8f, 4e-06));
      float rcsValueL2 = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);

      // casts to double are important, as (rcsValueL1 + rcsValueL2) / 2.0 leads to loss of precision on order of 3e-04
      double interpolatedValue = (static_cast<double>(rcsValueL1) + static_cast<double>(rcsValueL2)) / 2.0;

      // limits of float and extra calculation due to interpolation impose the 3e-04 precision limit
      elevR = (simCore::DEG2RAD * 15.0);
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      // this value determined from calculation, displayed here only to show that interpolation does not operate on dB scale  (38+35.8)/2 = 36.9
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, 37.0378418f, 4e-06));
      float rcsValueLI = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValueLI, interpolatedValue, 3e-04));

      elevR = (simCore::DEG2RAD * 375.0);
      rcsValueLI = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValueLI, interpolatedValue, 3e-04));

      elevR = (simCore::DEG2RAD * -345.0);
      rcsValueLI = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValueLI, interpolatedValue, 3e-04));
    }
    // once again, interpolation of elev angle, but with different values
    {
      polarity = simCore::POLARITY_VERTICAL;
      freqHz = 9000.0f;
      azimR = simCore::DEG2RAD * 5.0;

      elevR = (simCore::DEG2RAD * 30.0);
      float rcsValueL1 = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);

      elevR = (simCore::DEG2RAD * 60.0);
      float rcsValueL2 = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);

      // casts to double are important, as (rcsValueL1 + rcsValueL2) / 2.0 leads to loss of precision on order of 3e-04
      double interpolatedValue = (static_cast<double>(rcsValueL1) + static_cast<double>(rcsValueL2)) / 2.0;

      // limits of float and extra calculation due to interpolation impose the 3e-04 precision limit
      elevR = (simCore::DEG2RAD * 45.0);
      float rcsValueLI = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValueLI, interpolatedValue, 3e-04));

      elevR = (simCore::DEG2RAD * 405.0);
      rcsValueLI = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValueLI, interpolatedValue, 3e-04));

      elevR = (simCore::DEG2RAD * -315.0);
      rcsValueLI = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValueLI, interpolatedValue, 3e-04));
    }

    // test wrap around of azimuth angles
    {
      polarity = simCore::POLARITY_VERTICAL;
      freqHz = 9000.0f;
      elevR = (simCore::DEG2RAD * 0.0);

      azimR = (simCore::DEG2RAD * 5.0);
      tableValue = 38.0f;
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue));

      azimR = (simCore::DEG2RAD * 365.0);
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue));

      azimR = (simCore::DEG2RAD * -355.0);
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, tableValue));
    }

    // test interpolation of azimuth angle; interpolation operates on values in linear scale, not dB.
    {
      polarity = simCore::POLARITY_VERTICAL;
      freqHz = 9000.0f;
      elevR = (simCore::DEG2RAD * 0.0);

      azimR = (simCore::DEG2RAD * 5.0);
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, 38.0f));
      float rcsValueL1 = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);

      azimR = (simCore::DEG2RAD * 10.0);
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, 40.0f));
      float rcsValueL2 = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);

      // casts to double are important, as (rcsValueL1 + rcsValueL2) / 2.0 leads to loss of precision on order of 3e-04
      double interpolatedValue = (static_cast<double>(rcsValueL1) + static_cast<double>(rcsValueL2)) / 2.0;

      azimR = (simCore::DEG2RAD * 7.5);
      rcsValue_dB = rcsData_->RCSdB(freqHz, azimR, elevR, polarity);
      // this value determined from calculation, displayed here only to show that interpolation does not operate on dB scale
      rv += SDK_ASSERT(simCore::areEqual(rcsValue_dB, 39.1141281f, 4e-06));
      float rcsValueLI = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValueLI, interpolatedValue, 3e-04));
    }
    // again, with different values
    {
      polarity = simCore::POLARITY_VERTICAL;
      freqHz = 9000.0f;
      elevR = (simCore::DEG2RAD * 30.0);

      azimR = (simCore::DEG2RAD * 20.0);
      float rcsValueL1 = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);

      azimR = (simCore::DEG2RAD * 25.0);
      float rcsValueL2 = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);

      // casts to double are important, as (rcsValueL1 + rcsValueL2) / 2.0 leads to loss of precision on order of 3e-04
      double interpolatedValue = (static_cast<double>(rcsValueL1) + static_cast<double>(rcsValueL2)) / 2.0;

      azimR = (simCore::DEG2RAD * 22.5);
      float rcsValueLI = rcsData_->RCSsm(freqHz, azimR, elevR, polarity);
      rv += SDK_ASSERT(simCore::areEqual(rcsValueLI, interpolatedValue, 3e-04));
    }
    delete rcsData_;
    return rv;
  }
}


int testTwoWayRcvdPowerFreeSpace()
{
  try
  {
    double rv = 0;
    int err = 0;
    std::cout << "  testTwoWayRcvdPowerFreeSpace..." << std::endl;

    // example from EW & Radar Handbook (https://ewhdbks.mugu.navy.mil/two-way-mono.htm)
    rv = simCore::getRcvdPowerFreeSpace(31000, 5000, 10000, 45, 40, 9, 5, false);
    std::cout << "two-way: " << rv << std::endl;
    err += SDK_ASSERT(simCore::areEqual(rv, -107.52, 0.05));
    rv = simCore::getRcvdPowerFreeSpace(31000, 7000, 10000, 45, 40, 9, 5, false);
    std::cout << "two-way: " << rv << std::endl;
    err += SDK_ASSERT(simCore::areEqual(rv, -110.44, 0.05));
    return err;
  }
  catch (std::exception const & ex)
  {
    std::cerr << "\n< EXC > The following exception was raised:\n\t " << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "\n< EXC > An unexpected exception was raised!\n" << std::endl;
  }
  return 1;
}

int testOneWayRcvdPowerFreeSpace()
{
  try
  {
    double rv = 0;
    int err = 0;
    std::cout << "  testOneWayRcvdPowerFreeSpace..." << std::endl;

    // example from EW & Radar Handbook (https://ewhdbks.mugu.navy.mil/one-way.htm)
    rv = simCore::getRcvdPowerFreeSpace(31000, 5000, 10000, 45, 0, 1, 5, true);
    std::cout << "one-way: " << rv << std::endl;
    err += SDK_ASSERT(simCore::areEqual(rv, -56.25, 0.05));
    rv = simCore::getRcvdPowerFreeSpace(31000, 7000, 10000, 45, 0, 1, 5, true);
    std::cout << "one-way: " << rv << std::endl;
    err += SDK_ASSERT(simCore::areEqual(rv, -59.17, 0.05));
    return err;
  }
  catch (std::exception const & ex)
  {
    std::cerr << "\n< EXC > The following exception was raised:\n\t " << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "\n< EXC > An unexpected exception was raised!\n" << std::endl;
  }
  return 1;
}


int EMTest(int argc, char* argv[])
{
  int rv = 0;

  rv += rcsTest(argc, argv);
  rv += testTwoWayRcvdPowerFreeSpace();
  rv += testOneWayRcvdPowerFreeSpace();

  std::cout << "EMTests " << ((rv == 0) ? "Passed" : "Failed") << std::endl;

  return rv;
}
