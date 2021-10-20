//! @file   TauSpinnerRedecay.h
//! @author Elzbieta Richter-Was <elzbieta.richter-was@cern.ch>
//! @date   created June 2015
#ifndef ANALTAUSPINNERREDECAY_H
#define ANALTAUSPINNERREDECAY_H

#include <TROOT.h>
#include "TLorentzVector.h"
#include "TVector.h"

// HepMC include file
#include "HepMC/GenEvent.h"

// TauSpinner include file
#include "TauSpinner/SimpleParticle.h"

using namespace TauSpinner;

//! @class TauSpinnerRedecay:
class TauSpinnerRedecay {

 private:

 public:
//! Constructor
   TauSpinnerRedecay();
   
//! Destructor
   ~TauSpinnerRedecay();

 private:
   static TauSpinnerRedecay* gTauSpinnerRedecay;

 public:
   //! Creates instance of the class   
   static TauSpinnerRedecay* getInstance();
   //! Executes subalgorithm:  ReadDataCards()   
   void Initialize(char *filename, int idhist, int mode, int optPDF);
   void Finalize();
   //! Executes subalgorithms to  
   void Decay(  HepMC::GenEvent *event );
   //! Algorithm for monitoring

   //! Histograms instantation 
   void CreateHistos( int idhist);

   //generic id for histograms
   int m_idHist;

/** Simple test
    only pritnout */
  void makeSimpleTestME2SPIN2();
  void calcTestME2SPIN2(int iter, double P[6][4]);
  
/** calcXsect
    Returns array W[2][2] */
   void calcXsectSPIN2(int IDPROD, SimpleParticle &p3, SimpleParticle &p4, SimpleParticle &sp_X,SimpleParticle &tau1, SimpleParticle &tau2, double (&W)[2][2], int KEY);

/** calcProdMatrix
    Returns array W[2][2] */
   void calcProdMatrixSPIN2(SimpleParticle &p3, SimpleParticle &p4, SimpleParticle &sp_X,SimpleParticle &tau1, SimpleParticle &tau2, double (&W)[2][2], int KEY, int ID1, int ID2, int ID3, int ID4, int pdfOpt);

/** calcSumME2
    Returns array W[2][2] */
 void calcSumME2SPIN2(SimpleParticle &p3, SimpleParticle &p4, SimpleParticle &sp_X,SimpleParticle &tau1, SimpleParticle &tau2, double (&W)[2][2], int KEY, int ID1, int ID2, int ID3, int ID4);

};

#endif

