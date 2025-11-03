////////////////////////////////////////////////
// simCore/EM

%include "simCore/EM/Constants.h"
%include "simCore/EM/Decibel.h"
// TODO: template instantiations for Decibel.h methods

// TODO: Ignore getFreqMhzRange() due to overloaded enum args
%ignore simCore::getFreqMhzRange;
// simCore::getOneWayFreeSpaceRangeAndLoss()
%apply double* OUTPUT { double* fsLossDb };
%include "simCore/EM/Propagation.h"


// TODO: Implement
/*
// AntennaPattern::minMaxGain()
%apply float* OUTPUT { float* min, float* max };
%include "simCore/EM/AntennaPattern.h"

%include "simCore/EM/ElectroMagRange.h"
%include "simCore/EM/Propagation.h"
%include "simCore/EM/RadarCrossSection.h"
*/
