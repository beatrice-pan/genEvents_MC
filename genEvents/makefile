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

EXAMPLES = pythia-generate.exe

all: $(EXAMPLES)

%.o: %.cxx 
	$(CC) $(CFLAGS) -c $< -o $@ -I$(PYTHIALOCATION)/include -I$(HEPMCLOCATION)/include -I$(PHOTOSLOCATION)/include -I$(TAUOLALOCATION)/include

%.exe: %.o  
	$(CC) $< -o $@ $(PYTHIALIBS) $(HEPMCLIBS) $(PHOTOSLIBS) $(TAUOLALIBS) $(LHAPDFLIBS) $(ROOTLIBS) -lgfortran ../external/TAUOLA/TAUOLA/lib/libTauolaCxxInterface.so ../external/TAUOLA/TAUOLA/lib/libTauolaFortran.so ../external/TAUOLA/TAUOLA/TauSpinner/lib/libTauolaTauSpinner.so ../external/TAUOLA/TAUOLA/TauSpinner/examples/example-VBF/SPIN2/lib/libSpin2.so  ../external/LHAPDF/lhapdf/lib/libLHAPDF.so -Wl,-rpath,$(PYTHIALOCATION)lib  -Wl,-rpath,$(HEPMCLOCATION)lib -Wl,-rpath,$(TAUOLALOCATION)lib  -Wl,-rpath,$(TAUOLALOCATION)TauSpinner/examples/example-VBF/SPIN2/lib  -Wl,-rpath,$(TAUOLALOCATION)TauSpinner/lib  -Wl,-rpath,$(LHAPDFLOCATION)lib -Wl,-rpath,$(PHOTOSLOCATION)lib

clean:
	rm -f *.o *~ *.exe
