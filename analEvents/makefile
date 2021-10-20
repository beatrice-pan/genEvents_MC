include paths.inc

PYTHIALIBS= -L$(PYTHIALOCATION)lib -lpythia8 #-llhapdfdummy -lpythia8tohepmc
HEPMCLIBS=  -L$(HEPMCLOCATION)lib -lHepMC
PHOTOSLIBS= -L$(PHOTOSLOCATION)lib -lPhotospp -lPhotosppHepMC $(HEPMC_LIBS)
LHAPDFLIBS= -L$(LHAPDFLOCATION)lib -lLHAPDF
TAUOLALIBS=  -L$(TAUOLALOCATION)lib -L$(TAUOLALOCATION)TauSpinner/lib  -lTauolaTauSpinner -lTauolaFortran -lTauolaCxxInterface
export PYTHIA8DATA=${PYTHIALOCATION}/share/Pythia8/xmldoc
SPIN2LIBS=  -L$(TAUOLALOCATION)TauSpinner/examples/example-VBF/SPIN2/lib/ -lSpin2

#====================================================================

ROOTLIBS := $(shell root-config --libs) 

#======================================================================
LIBDIR= $(shell root-config --libdir)
INCDIR= $(shell root-config --incdir)
#  Root  libraries for linker
RLIB=   -L$(LIBDIR) -lCore -lCint -lHist -lGraf -lGraf3d -lGpad -lTree -lRint \
	-lPostscript -lMatrix -lPhysics -lm -ldl -lpthread -rdynamic
#  Fortran libraries for linker (just in case)g
FLIB=   -lf2c

#======================================================================
CC= g++
CFLAGS= -g3 -O2 -fPIC -Wall -I$(INCDIR)
#======================================================================

#EXAMPLES = pythia8-example.exe
#EXAMPLES = hepmcfile-example.exe
#EXAMPLES = hepmcfile-testCP.exe
EXAMPLES = hepmcfile-testCPalpha.exe
#EXAMPLES = pythia-generate.exe
#EXAMPLES = hepmcfile-testDY.exe
#EXAMPLES = hepmcfile-testDY_test.exe

#EXAMPLES = lhefile-example.exe
#EXAMPLES = lhefile-shower.exe

#EXAMPLES = ntupfile-example.exe
#EXAMPLES = ntupZpolAis-example.exe
#EXAMPLES = ntupZ3DAis-example.exe
#EXAMPLES = ntupZpol13TeV-example.exe
#EXAMPLES = ntupZpolAisEWPO-example.exe
#EXAMPLES = ntupZpolAis2020-example.exe
#EXAMPLES = ntupZpolAis2020-test.exe
#EXAMPLES = ntupZpolAis2020-testNTUP.exe
#EXAMPLES = ntupZpolAis2020-testNTUPEWwt.exe

all: $(EXAMPLES)

%.o: %.cxx
	$(CC) $(CFLAGS) -c $< -o $@ -I$(PYTHIALOCATION)/include -I$(HEPMCLOCATION)/include -I$(PHOTOSLOCATION)/include -I$(TAUOLALOCATION)/include

lib:
	cd ../libAnalAlgo; make
	cd ../libEventManager; make
	cd ../libTauSpinnerManager; make
	cd ../libHistoManager; make

%.exe: %.o  lib
	$(CC) $< -o $@ $(PYTHIALIBS) $(HEPMCLIBS) $(PHOTOSLIBS) $(TAUOLALIBS) $(LHAPDFLIBS) ../libAnalAlgo/libLibAnalAlgo.so  ../libEventManager/libLibEventManager.so   ../libTauSpinnerManager/libLibTauSpinnerManager.so  ../libHistoManager/libLibHistoManager.so  $(ROOTLIBS) -lgfortran ../external/TAUOLA/TAUOLA/lib/libTauolaCxxInterface.so ../external/TAUOLA/TAUOLA/lib/libTauolaFortran.so ../external/TAUOLA/TAUOLA/TauSpinner/lib/libTauolaTauSpinner.so ../external/TAUOLA/TAUOLA/TauSpinner/examples/example-VBF/SPIN2/lib/libSpin2.so  ../external/LHAPDF/lhapdf/lib/libLHAPDF.so -Wl,-rpath,$(PYTHIALOCATION)lib  -Wl,-rpath,$(HEPMCLOCATION)lib -Wl,-rpath,$(TAUOLALOCATION)lib  -Wl,-rpath,$(TAUOLALOCATION)TauSpinner/examples/example-VBF/SPIN2/lib  -Wl,-rpath,$(TAUOLALOCATION)TauSpinner/lib  -Wl,-rpath,$(LHAPDFLOCATION)lib -Wl,-rpath,$(PHOTOSLOCATION)lib

clean:
	rm -f *.o *~ *.exe
