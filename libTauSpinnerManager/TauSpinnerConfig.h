//! @file   TauSpinnerConfig.h
//! @author Elzbieta Richter-Was <elzbieta.richter-was@cern.ch>
//! @date   created June 2015
#ifndef ANALTAUSPINNERCONFIG_H
#define ANALTAUSPINNERCONFIG_H

#include <TROOT.h>
#include "TLorentzVector.h"
#include "TVector.h"

// HepMC include file
#include "HepMC/GenEvent.h"

//! @class TauSpinnerConfig:
class TauSpinnerConfig {

 private:

 public:
//! Constructor
   TauSpinnerConfig();
//! Destructor
   ~TauSpinnerConfig();

 private:
   static TauSpinnerConfig* gTauSpinnerConfig;

 public:
   //! Creates instance of the class   
   static TauSpinnerConfig* getInstance();
   //! Executes subalgorithm:  ReadDataCards()   
   void Initialize(char *filename, int idhist, int mode);
   void Finalize();
   //! Executes subalgorithms to  
   void Make();
   //! Algorithm for monitoring

   //! Histograms instantation 
   void CreateHistos( int idhist);

   //generic id for histograms
   int m_idHist;

};

#endif

