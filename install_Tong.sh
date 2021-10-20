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
mkdir TAUOLA MC-TESTER LHAPDF HEPMC PYTHIA PHOTOS
cd TAUOLA/
wget https://tauolapp.web.cern.ch/tauolapp/resources/TAUOLA.1.1.8/TAUOLA.1.1.8.tar.gz
tar -xvzf TAUOLA.1.1.8.tar.gz
rm TAUOLA.1.1.8.tar.gz
cd ../MC-TESTER/
wget https://mc-tester.web.cern.ch/MC-TESTER/MC-TESTER-1.25.0.tar.gz
tar -xvzf MC-TESTER-1.25.0.tar.gz
rm MC-TESTER-1.25.0.tar.gz
cd ../HEPMC/
wget https://hepmc.web.cern.ch/hepmc/releases/hepmc2.06.11.tgz
tar zxvf hepmc2.06.11.tgz
rm hepmc2.06.11.tgz
cd ../LHAPDF/
wget https://lhapdf.hepforge.org/downloads/LHAPDF-6.3.0.tar.gz
tar -xvzf LHAPDF-6.3.0.tar.gz
rm LHAPDF-6.3.0.tar.gz
cd ../PHOTOS/
wget http://photospp.web.cern.ch/photospp/resources/PHOTOS.3.61/PHOTOS.3.61.tar.gz
tar -xvzf PHOTOS.3.61.tar.gz
rm PHOTOS.3.61.tar.gz
cd ../PYTHIA/
wget https://pythia.org/download/pythia83/pythia8306.tgz
tar -xvzf pythia8306.tgz
rm pythia8306.tgz

#
# Build pythia
#
cd pythia8306/
./configure
make -j4

#
# Build HepMC
#
cd ../../HEPMC/HepMC-2.06.11/
./configure --with-momentum=GEV --with-length=MM --prefix=`pwd`/../HepMC
make install -j4

#
# Build PHOTOS
#
cd ../../PHOTOS/PHOTOS/
./configure --with-hepmc=/home/pantong/gen_events/external/HEPMC/HepMC
make

#
# Build MC-TESTER
#
# cd ../../MC-TESTER/MC-TESTER/
# ./configure --with-HepMC=`pwd`/../../HEPMC/HepMC
# make -j4

#
# Build LHAPDF
#
cd ../../LHAPDF/LHAPDF-6.3.0/
./configure --prefix=`pwd`/../lhapdf
make install -j4
cd ../lhapdf/share/LHAPDF/
###############################################
#IMPORTANT INFORMATION ABOUT PDF SETS

#LHAPDF no longer bundles PDF set data in the package tarball.
#The sets are instead all stored online at
#  http://lhapdfsets.web.cern.ch/lhapdfsets/current/
#and you should install those that you wish to use into
#  /home/pantong/gen_events/external/LHAPDF/LHAPDF-6.3.0/../lhapdf/share/LHAPDF

#The downloadable PDF sets are packaged as tarballs, which
#must be expanded to be used. The simplest way to do this is with
#the 'lhapdf' script, e.g. to install the CT10nlo PDF set:
#  lhapdf install CT10nlo
# The same effect can be achieved manually with, e.g.:
#  wget http://lhapdfsets.web.cern.ch/lhapdfsets/current/CT10nlo.tar.gz -O- | tar xz -C /home/pantong/gen_events/external/LHAPDF/LHAPDF-6.3.0/../lhapdf/share/LHAPDF

###############################################
# ../../../bin/lhapdf-getdata MSTW2008nnlo90cl.LHgrid
# ../../../bin/lhapdf-getdata cteq6ll.LHpdf

wget http://lhapdfsets.web.cern.ch/lhapdfsets/current/MSTW2008nnlo90cl.tar.gz -O- | tar xz -C /home/pantong/gen_events/external/LHAPDF/LHAPDF-6.3.0/../lhapdf/share/LHAPDF
wget http://lhapdfsets.web.cern.ch/lhapdfsets/current/@UNVALIDATED/cteq6.tar.gz -O- | tar xz -C /home/pantong/gen_events/external/LHAPDF/LHAPDF-6.3.0/../lhapdf/share/LHAPDF

#
# Build TAUOLA with TauSpinner
#
cd ../../../../../TAUOLA/TAUOLA/
./configure --with-tau-spinner --with-lhapdf=`pwd`/../../LHAPDF/lhapdf --with-hepmc=`pwd`/../../HEPMC/HepMC --with-pythia8=`pwd`/../../PYTHIA/pythia8306 --without-hepmc3
make
source configure.paths.sh
cd TauSpinner/examples/
./configure --with-hepmc=$HEPMCLOCATION
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

