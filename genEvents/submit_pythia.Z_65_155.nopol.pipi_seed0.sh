#!/bin/sh
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase;
alias setupATLAS='source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh';
setupATLAS;
localSetupROOT  5.34.25-x86_64-slc6-gcc48-opt;
cd /people/plgerichter/storage/testarea/Ela2021/TauSpinnerFrame

export TAUOLALOCATION=`pwd`/external/TAUOLA/TAUOLA/
export HEPMCLOCATION=`pwd`/external/HEPMC/HepMC/
export PYTHIALOCATION=`pwd`/external/PYTHIA/pythia8201
export MCTESTERLOCATION=`pwd`/external/MC-TESTER/MC-TESTER
export LHAPDFLOCATION=`pwd`/external/LHAPDF/lhapdf
export PYTHIA8DATA=${PYTHIALOCATION}/share/Pythia8/xmldoc

ROOTLIB=`root-config --libdir`

export LD_LIBRARY_PATH=${TAUOLALOCATION}/lib:${PREFIX}/lib:${HEPMCLOCATION}/lib:${PYTHIALOCATION}/lib/archive:${MCTESTERLOC
ATION}/lib:${ROOTLIB}:${LD_LIBRARY_PATH}

cd gen_ztautau
./pythia-generate.exe filesZtautau/pythia.Z_65_155.nopol.pipi.1M.seed0.hepmc pythia.Ztautau.HardProc_seed0.conf 0 0 3 3 1000000 > filesZtautau/pythia.Z_65_155.nopol.pipi.seed0.out

