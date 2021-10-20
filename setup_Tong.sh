#!/bin/sh

export TAUOLALOCATION=`pwd`/external/TAUOLA/TAUOLA/
export HEPMCLOCATION=`pwd`/external/HEPMC/HepMC/
export PHOTOSLOCATION=`pwd`/external/PHOTOS/PHOTOS/
export PYTHIALOCATION=`pwd`/external/PYTHIA/pythia8201
# export MCTESTERLOCATION=`pwd`/external/MC-TESTER/MC-TESTER
export LHAPDFLOCATION=`pwd`/external/LHAPDF/lhapdf
export PYTHIA8DATA=${PYTHIALOCATION}/share/Pythia8/xmldoc

ROOTLIB=`root-config --libdir`

# Examples have these paths hardcoded during compilation
# Nonetheless, this line might be useful for any other programs
# that user might want to compile
export LD_LIBRARY_PATH=${TAUOLALOCATION}/lib:${PREFIX}/lib:${HEPMCLOCATION}/lib:$PHOTOSLOCATION}/lib:${PYTHIALOCATION}/lib/archive:${MCTESTERLOCATION}/lib:${ROOTLIB}:${LD_LIBRARY_PATH}
