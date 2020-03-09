////////////////////////////////////////////////
// simCore/LUT
%include "simCore/LUT/LUT1.h"
%include "simCore/LUT/LUT2.h"
%include "simCore/LUT/InterpTable.h"

%template(intLUT1) simCore::LUT::LUT1<int>;
%template(doubleLUT1) simCore::LUT::LUT1<double>;
%template(intIndex) simCore::LUT::index<int>;
%template(doubleIndex) simCore::LUT::index<double>;
%template(intLowValue) simCore::LUT::lowValue<int>;
%template(doubleLowValue) simCore::LUT::lowValue<double>;
// TODO: interpolate()

%template(intLUT2) simCore::LUT::LUT2<int>;
%template(doubleLUT2) simCore::LUT::LUT2<double>;
// TODO: index()
// TODO: nearValue()
// TODO: interpolate()

// TODO: BilinearInterpolate class
%template(intInterpTableLimitException) simCore::InterpTableLimitException<int>;
%template(doubleInterpTableLimitException) simCore::InterpTableLimitException<double>;
%template(intInterpTable) simCore::InterpTable<int>;
%template(doubleInterpTable) simCore::InterpTable<double>;
// TODO: NearestLookup()
// TODO: BilinearLookup()
// TODO: BilinearLookupNoException()

