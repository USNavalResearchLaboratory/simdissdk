#!/usr/bin/env bash

writeHeader()
{
  echo "/* -*- mode: c++ -*- */" > $1
  echo "/****************************************************************************" >> $1
  echo " *****                                                                  *****" >> $1
  echo " *****                   Classification: UNCLASSIFIED                   *****" >> $1
  echo " *****                    Classified By:                                *****" >> $1
  echo " *****                    Declassify On:                                *****" >> $1
  echo " *****                                                                  *****" >> $1
  echo " ****************************************************************************" >> $1
  echo " *" >> $1
  echo " *" >> $1
  echo " * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div." >> $1
  echo " *               EW Modeling & Simulation, Code 5773" >> $1
  echo " *               4555 Overlook Ave." >> $1
  echo " *               Washington, D.C. 20375-5339" >> $1
  echo " *" >> $1
  echo " * License for source code at https://simdis.nrl.navy.mil/License.aspx" >> $1
  echo " *" >> $1
  echo " * The U.S. Government retains all rights to use, duplicate, distribute," >> $1
  echo " * disclose, or release this software." >> $1
  echo " *" >> $1
  echo " */" >> $1
  echo "#ifndef SIMDISSDK_${2}_H" >> $1
  echo "#define SIMDISSDK_${2}_H" >> $1
  echo "" >> $1
  echo "#ifdef _MSC_VER" >> $1
  echo "#pragma message( __FILE__ \": warning <DEPR>: File is deprecated and will be removed in a future release.\" )" >> $1
  echo "#else" >> $1
  echo "#warning File is deprecated and will be removed in a future release." >> $1
  echo "#endif" >> $1

  echo "" >> $1
}

addIncludes()
{
  sort -f $1.inc >> $1
  rm -f $1.inc
}

writeFooter()
{
  echo "" >> $1
  echo "#endif /* SIMDISSDK_${2}_H */" >> $1
}

# simVis
writeHeader simVis.h SIMVIS
echo "#include \"simVis/osgEarthVersion.h\"" >> simVis.h.inc
find simVis -name '*.h' | sort -f | grep -v "simVis/Shaders.h" | grep -v "simVis/DBFormat.h" | grep -v "simVis/DBOptions.h" | grep -v "simVis/DB/" | sed 's/^/#include "/' | sed 's/$/"/' >> simVis.h.inc
addIncludes simVis.h
writeFooter simVis.h SIMVIS

# simUtil
writeHeader simUtil.h SIMUTIL
echo "// simUtil/SilverLiningSettings.h is intentionally omitted to avoid commonly missing 3rd party library" >> simUtil.h
echo "// simUtil/TritonSettings.h is intentionally omitted to avoid commonly missing 3rd party library" >> simUtil.h
find simUtil -name '*.h' | sort -f | grep -v "simUtil/DbConfigurationFile" | grep -v "simUtil/SilverLiningSettings.h" | grep -v "simUtil/TritonSettings.h" | sed 's/^/#include "/' | sed 's/$/"/' >> simUtil.h.inc
addIncludes simUtil.h
writeFooter simUtil.h SIMUTIL

# simCore
writeHeader simCore.h SIMCORE
echo "#include \"simCore/Common/Version.h\"" >> simCore.h.inc
find simCore -name '*.h' | sort -f | grep -v "inttypes.h" | grep -v "stdint.h" | grep -v "HighPerformanceGraphics.h" | sed 's/^/#include "/' | sed 's/$/"/' >> simCore.h.inc
addIncludes simCore.h
writeFooter simCore.h SIMCORE

# simNotify
writeHeader simNotify.h SIMNOTIFY
find simNotify -name '*.h' | sort -f | sed 's/^/#include "/' | sed 's/$/"/' >> simNotify.h.inc
addIncludes simNotify.h
writeFooter simNotify.h SIMNOTIFY

# simData
writeHeader simData.h SIMDATA
find simData -name '*.h' | sort -f | grep -v "TimeContainerDeque.h" | grep -v "simData.pb.h" | grep -v "\-inl.h" | sed 's/^/#include "/' | sed 's/$/"/' >> simData.h.inc
addIncludes simData.h
writeFooter simData.h SIMDATA

# simQt
writeHeader simQt.h SIMQT
find simQt -name '*.h' | sort -f | sed 's/^/#include "/' | sed 's/$/"/' >> simQt.h.inc
addIncludes simQt.h
writeFooter simQt.h SIMQT

