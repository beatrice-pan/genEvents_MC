include ../main/paths.inc

##############################################################################
#----------------------------------------------------------------------
#  Execute from command line:
#	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PWD}
#	export LD_LIBRARY_PATH
#  in order to add "./" to load library path
#----------------------------------------------------------------------
.SUFFIXES:  .o .c .cxx .f .l  .s  .sh  .h  .a
#======================================================================
RPATH = $(ROOTSYS)
#  Root  libraries for linker
RLIB=   -L$(RPATH)/lib -lCore -lCint -lHist -lGraf -lGraf3d -lGpad -lTree -lRint \
	-lPostscript -lMatrix -lPhysics -lm -ldl -lpthread -rdynamic
#  Fortran libraries for linker (just in case)
FLIB=   -lf2c
ROOTINC = $(shell root-config --incdir)
RLIB = $(shell root-config --gflags)

#======================================================================
#####  c++ compiler
CXX         =  g++
#CXX         =  insure
#CXX         =  codewizard
CLD         =  $(CXX)
#CXXFLAGS    = -g -ansi -pedantic
#CXXFLAGS    = -O -Wall -fPIC -D_REENTRANT -I/home/jadach/root/include
CXXFLAGS     = -g -O  -ansi -Wall -fPIC -D_REENTRANT -I$(ROOTINC)  -I$(HEPMCLOCATION)/include -I$(TAUOLALOCATION)/include -I$(LHAPDFLOCATION)/include -I$(TAUSPINNERLOCATION)/examples/example-VBF/SPIN2/include

#CXXFLAGS     = -g -O -ansi -Wall -fPIC -D_REENTRANT -I/home/erichter/downloads/root_v5.34/include -I/home/erichter/Athena15/TauAnalFrame_temp/external/hepmc269/include  -I/home/erichter/Athena15/TauAnalFrame_temp/external/TAUOLA/include -I/home/erichter/Athena15/TauAnalFrame_temp/external/lhapdf589/include -I/usr/include/i386-linux-gnu

#CXXFLAGS    =  -g
#======================================================================
#####  f77 compiler
# forbid underscore completely       -fno-second-underscore
# forbid double underscore only      -fno-underscoring
FFLAGS  =  -O -fno-second-underscore -fno-automatic -ffixed-line-length-132 -fbackslash 
F77 = g77
#  Fortran libraries for linker
FLIB=   -lf2c
CLF =  $(F77)
#--------------------------------------------------------------------
.cxx.o:
	$(CXX) $(CXXFLAGS) -c $<
.f.o:
	$(F77) $(FFLAGS) -c $<
#====================================================================
all: lib
#==============================================================================
# Local shared library for all programs
SLIB1 = LibTauSpinnerManager
$(SLIB1)_HED= TauSpinnerConfig.h  TauSpinnerRedecay.h 
$(SLIB1)_OBJ= TauSpinnerConfig.o  TauSpinnerRedecay.o 

#$(SLIB1)Dict.cxx: $($(SLIB1)_HED) $(SLIB1)LinkDef.h
#	@echo "Generating dictionary $@..."
#	@rootcint -f $@ -c $(CXXINCLUDES) $^
lib$(SLIB1).so:    $($(SLIB1)_OBJ) 
	g++ -shared -o lib$(SLIB1).so -O  $($(SLIB1)_OBJ)
lib$(SLIB1).a:     $($(SLIB1)_OBJ) 
	ar cr lib$(SLIB1).a   $($(SLIB1)_OBJ) 
lib: lib$(SLIB1).so lib$(SLIB1).a 

#====================================================================
##############################################################################
clean:
	rm -rf lib$(SLIB1).so lib$(SLIB1).a $($(SLIB1)_OBJ) $(SLIB1)Dict.*
