//! @file   AnalTauSpinnerCP.h
//! @author Elzbieta Richter-Was <elzbieta.richter-was@cern.ch>
//! @date   created June 2015
#ifndef ANALTAUSPINNERCP_H
#define ANALTAUSPINNERCP_H

#include <TROOT.h>
#include "TLorentzVector.h"
#include "TVector.h"

// HepMC include file
#include "HepMC/GenEvent.h"

// TauSpinner include file
#include "TauSpinner/Particle.h"

//! @class AnalTauSpinnerCP:
class AnalTauSpinnerCP {

 private:

 public:
//! Constructor
   AnalTauSpinnerCP();
//! Destructor
   ~AnalTauSpinnerCP();

 private:
   static AnalTauSpinnerCP* gAnalTauSpinnerCP;

 public:
   //! Creates instance of the class   
   static AnalTauSpinnerCP* getInstance();
   //! Executes subalgorithm:  ReadDataCards()   
   void Initialize(char *filename, int idhist);
   void Finalize();
   //! Executes subalgorithms to  
   void Make( HepMC::GenEvent *hepmc, int analMode );
   //! Algorithm for monitoring
   void testRhoRhoZ( int idhist, HepMC::GenEvent *hepmc, float evtWeight );
   void testPiPiZ( int idhist, HepMC::GenEvent *hepmc, float evtWeight );
   void testRhoRho( int idhist, HepMC::GenEvent *hepmc, float evtWeight );
   void testA1A1(   int idhist, HepMC::GenEvent *hepmc, float evtWeight );
   void testA1Rho(   int idhist, HepMC::GenEvent *hepmc, float evtWeight );
   void NeusA1Rho(   int idhist, HepMC::GenEvent *hepmc, float evtWeight );
   void test_3p0n_1p1n(   int idhist, HepMC::GenEvent *hepmc, float evtWeight );
   void test_1p2n_1p1n(   int idhist, HepMC::GenEvent *hepmc, float evtWeight );
   void test_1p2n_v2_1p1n(   int idhist, HepMC::GenEvent *hepmc, float evtWeight );
   void test_3p0n_v2_1p1n(   int idhist, HepMC::GenEvent *hepmc, float evtWeight );

   //! Helper classes
   double P_norm_cross_product(TauSpinner::Particle v1, TauSpinner::Particle v2, double * result);
   double P_acoplanarAngle(TauSpinner::Particle p1, TauSpinner::Particle p2, TauSpinner::Particle p3, TauSpinner::Particle p4);
   double normalised_cross_product(double * v1, double * v2, double * result);
   double dot_product(double *v1, double *v2);
   double magnitude(double *v);
   void calcCosThetaPhiMustraal(TauSpinner::Particle tau_plus, TauSpinner::Particle tau_minus, TauSpinner::Particle p3,
				double& costheta1, double& costheta2, double& phi1, double& phi2, double& wt1, double& wt2);
   //! Histograms instantation 
   void CreateHistos( int idhist);

   //generic id for histograms
   int m_idHist;

   //    ClassDef(AnalTauSpinnerCP,1) // End of CLASS  AnalTauSpinnerCP
};

#endif

