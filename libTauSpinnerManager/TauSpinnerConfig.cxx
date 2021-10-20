/*****************************************************************************
   Name    : TauSpinnerConfig.cxx
   Package : 
   Author  : Elzbieta Richter-Was
   Created : June 2015
   
   DESCRIPTION:
   
   This class is test for TauSpinner weight
   COPIED from example by T. Przedzinski

*****************************************************************************/
// Include files

#define TauSpinnerConfig_cxx

#include "TauSpinnerConfig.h"
#include "../libHistoManager/HistoManager.h"
#include "../libEventManager/BuildTauSpinnerBranch.h"

// HepMC include file
#include "HepMC/GenEvent.h"

// TAUOLA header
#include "Tauola/Tauola.h"

#include "TauSpinner/Particle.h"
#include "TauSpinner/SimpleParticle.h"
#include "TauSpinner/tau_reweight_lib.h"
#include "TauSpinner/nonSM.h"

#include <stdio.h>
#include <math.h> 

#include "TLorentzVector.h"

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TMath.h>
#include <TObject.h>
#include <TRandom.h>

using namespace std;
using namespace Tauolapp;
using namespace TauSpinner;


TauSpinnerConfig* TauSpinnerConfig::gTauSpinnerConfig = 0;

TauSpinnerConfig* TauSpinnerConfig::getInstance()
{
  if (!gTauSpinnerConfig) gTauSpinnerConfig=new TauSpinnerConfig();
  return gTauSpinnerConfig;
}

const bool  DEBUG = 0;


TRandom gen1;

  double randomik1(){
 
  return gen1.Rndm();

 }

//CONSTRUCTOR
//----------------------------------------------
TauSpinnerConfig::TauSpinnerConfig()
//----------------------------------------------
{

} 

//DESTRUCTOR
//----------------------------------------------
TauSpinnerConfig::~TauSpinnerConfig()
//----------------------------------------------
{

}

//Initialize
//----------------------------------------------
void TauSpinnerConfig::Initialize(char *filename, int idhist, int mode)
//----------------------------------------------
{

  //  Create histograms for this analysis
  m_idHist= idhist;
  CreateHistos(m_idHist);
  
  // Initialize Tauola
  Tauola::initialize();
  
  // Uncomment this line if producing sample
  // without polarization or spin correlations
  if( mode != 0 ) 
    Tauola::spin_correlation.setAll(false);

  // change Random generator 
  gen1.SetSeed(time(NULL));
  Tauola::setRandomGenerator( randomik1 );
  Tauola::setSeed(time(NULL), 0, 0);

  //  string name="CT10nlo.LHgrid";
  //  LHAPDF::initPDFSet("CT10nlo", LHAPDF::LHGRID, 0);
  LHAPDF::initPDFSet("NNPDF23_nnlo_as_0118", LHAPDF::LHGRID, 0);
 

  if(DEBUG){
    const int SUBSET = 0;
    const string NAME = "CT10";
    const double Q = 10.0, mz = 91.2;
    cout << "alphas(mz) = " << LHAPDF::alphasPDF(mz) << endl;
    cout << "qcdlam4    = " << LHAPDF::getLam4(SUBSET) << endl;
    cout << "qcdlam5    = " << LHAPDF::getLam5(SUBSET) << endl;
    cout << "orderPDF   = " << LHAPDF::getOrderPDF() << endl;
    cout << "xmin       = " << LHAPDF::getXmin(SUBSET) << endl;
    cout << "xmax       = " << LHAPDF::getXmax(SUBSET) << endl;
    cout << "q2min      = " << LHAPDF::getQ2min(SUBSET) << endl;
    cout << "q2max      = " << LHAPDF::getQ2max(SUBSET) << endl;
    cout << "orderalfas = " << LHAPDF::getOrderAlphaS() << endl;
    cout << "num flav   = " << LHAPDF::getNf() << endl;
    cout << "name       = " << NAME << endl;
    cout << "number     = " << LHAPDF::numberPDF() << endl;
    cout << endl;
  }


  double CMSENE = 13000.0; // center of mass system energy.
                          // used in PDF calculation. For pp collisions only
  bool Ipp   = true;  // for pp collisions
  //bool Ipp = false; // otherwise (not prepared yet)

  int Ipol   = 1; // are input samples polarized?

  int nonSM2 = 0; // are we using nonSM calculations?
                  // at present we have checked only that for nonSM2 = 0
                  // all works as in the past. nonSM2 = 1 may simply mean 
                  // errors are switched on. More work is needed 
                  // for this option.
  int nonSMN = 0; // If we are using nonSM calculations we may want corrections 
                  // to shapes only: y/n  (1/0)
  
  // Initialize TauSpinner
  TauSpinner::initialize_spinner(Ipp, Ipol, nonSM2, nonSMN,  CMSENE);

  if( mode == 1 ){
    // Initialize transverse spin effects of TauSpinner
    setHiggsParametersTR(-1.0, 1.0, 0.0, 0.0); // for scalar H
  }

  if( mode == 2 ){
    // Initialize transverse spin effects of TauSpinner
    setHiggsParametersTR( 1.0,-1.0, 0.0, 0.0); // for pseudo-scalar H
  }

  // Multipliers for  components of transverse density matrix of DY
  //                  (Rxx,Ryy,Rxy,Ryx)
  setZgamMultipliersTR(1., 1., 1., 1. );
  if( mode == 3 )
    setZgamMultipliersTR(1., 0., 1., 1. );
  if( mode == 4 )
    setZgamMultipliersTR(0., 1., 1., 1. );
  if( mode == 5 )
    setZgamMultipliersTR(1., -1., 1., 1. );


}

//Finalize
//----------------------------------------------
void TauSpinnerConfig::Finalize()
//----------------------------------------------
{


}
//----------------------------------------------
void TauSpinnerConfig::Make()
//----------------------------------------------
{

}

//----------------------------------------------------
// Define  histograms for analysis
//----------------------------------------------------
//
void TauSpinnerConfig::CreateHistos( int idhist)  {

  
  HistoManager *hmg  = HistoManager::getInstance();

  Text_t namehist[500];


}

