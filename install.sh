if test -e external; then
  echo ""
  echo "Directory 'external' exists. Remove it then re-run the script"
  echo ""
  exit
fi

#
# Get packages
#
mkdir external
cd external/
mkdir TAUOLA MC-TESTER LHAPDF HEPMC PYTHIA
cd TAUOLA/
wget https://tauolapp.web.cern.ch/tauolapp/resources/TAUOLA.1.1.5/TAUOLA.1.1.5.tar.gz
tar -xvzf TAUOLA.1.1.5.tar.gz
rm TAUOLA.1.1.5.tar.gz
cd ../MC-TESTER/
wget https://mc-tester.web.cern.ch/MC-TESTER/MC-TESTER-1.25.0.tar.gz
tar -xvzf MC-TESTER-1.25.0.tar.gz
rm MC-TESTER-1.25.0.tar.gz
cd ../HEPMC/
wget http://lcgapp.cern.ch/project/simu/HepMC/download/HepMC-2.06.09.tar.gz
tar -xvzf HepMC-2.06.09.tar.gz
rm HepMC-2.06.09.tar.gz
cd ../LHAPDF/
wget http://www.hepforge.org/archive/lhapdf/lhapdf-5.9.1.tar.gz
tar -xvzf lhapdf-5.9.1.tar.gz
rm lhapdf-5.9.1.tar.gz
cd ../PYTHIA/
wget https://pythia.org/download/pythia82/pythia8201.tgz
tar -xvzf pythia8201.tgz
rm pythia8201.tgz

#
# Build pythia
#
cd pythia8201/
./configure
make -j4

#
# Build HepMC
#
cd ../../HEPMC/HepMC-2.06.09/
./configure --with-momentum=GEV --with-length=MM --prefix=`pwd`/../HepMC
make install -j4

#
# Build MC-TESTER
#
cd ../../MC-TESTER/MC-TESTER/
./configure --with-HepMC=`pwd`/../../HEPMC/HepMC
make -j4

#
# Build LHAPDF
#
cd ../../LHAPDF/lhapdf-5.9.1/
./configure --prefix=`pwd`/../lhapdf
make install -j4
cd ../lhapdf/share/lhapdf/
mkdir PDFsets
cd PDFsets/
../../../bin/lhapdf-getdata MSTW2008nnlo90cl.LHgrid
../../../bin/lhapdf-getdata cteq6ll.LHpdf

#
# Build TAUOLA with TauSpinner
#
cd ../../../../../TAUOLA/TAUOLA/
./configure --with-tau-spinner --with-lhapdf=`pwd`/../../LHAPDF/lhapdf --with-mc-tester=`pwd`/../../MC-TESTER/MC-TESTER --with-hepmc=`pwd`/../../HEPMC/HepMC --with-pythia8=`pwd`/../../PYTHIA/pythia8201
make
source configure.paths.sh
cd TauSpinner/examples/
./configure --with-mc-tester=$MCTESTERLOCATION --with-hepmc=$HEPMCLOCATION
make
./tau-reweight-test.exe

#
# Build and run tests
#
cd ../../../../..
source setup.sh
cd tests
make

./pythia-generate.exe events.dat pythia.Z.conf 3 100
./tauspinner-test.exe events.dat

