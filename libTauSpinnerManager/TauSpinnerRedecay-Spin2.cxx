/*****************************************************************************
   Name    : TauSpinnerRedecay.cxx
   Package : 
   Author  : Elzbieta Richter-Was
   Created : June 2015
   
   DESCRIPTION:
   
   This class is test for TauSpinner weight
   COPIED from example by T. Przedzinski

*****************************************************************************/
// Include files

#define TauSpinnerRedecay_cxx

#include "TauSpinnerRedecay.h"
#include "../libHistoManager/HistoManager.h"
#include "../libEventManager/BuildTauSpinnerBranch.h"

// HepMC include file
#include "HepMC/GenEvent.h"

// TAUOLA header
#include "Tauola/Tauola.h"
#include "Tauola/TauolaHepMCEvent.h"

#include "TauSpinner/Particle.h"
#include "TauSpinner/SimpleParticle.h"
#include "TauSpinner/tau_reweight_lib.h"
#include "TauSpinner/nonSM.h"
#include "TauSpinner/vbfdistr.h"
#include "SPIN2/spin2distr.h"

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

// We take PDF distributions from TauSpinner for this test                                                                                                                      
namespace TauSpinner{
  extern double f(double x, int ID, double SS, double cmsene);
}


TauSpinnerRedecay* TauSpinnerRedecay::gTauSpinnerRedecay = 0;

TauSpinnerRedecay* TauSpinnerRedecay::getInstance()
{
  if (!gTauSpinnerRedecay) gTauSpinnerRedecay=new TauSpinnerRedecay();
  return gTauSpinnerRedecay;
}

const bool  DEBUG = 0;

/*
 We are using routine from 'distr.f' as an example of
 function for user-defined born calculation.
 We need to adjust conventions for signs etc.
 In our example: we  adapt convention for flavour of incoming quark
 and signs of spin states -- from helicity.
*/
 extern "C" double distjkwk_(int*, double*, double*, int*, int*, int*);

double nonSM_adopt(int ID, double S, double cost, int H1, int H2, int key)
{
  int ID2=3-ID;
  // for distjkwk_ conventions
  if(ID2<1) ID2=ID2+2;  // Since Feb 2016  we allow ID in range [1,5]
  if(ID2<1) ID2=ID2+2;
  int HH1= H1;   // to be fixed a bit later.
  int HH2=-H2;
  //  an example of use:
  return distjkwk_(&ID2, &S, &cost, &HH1, &HH2, &key);
  
  /*
      cout<<"             this function is dummy\n"<<endl;
      cout<<"             user must provide his own nonSM_born"<<endl;
      cout<<"             that is nonSM_adopt()"<<endl;
      exit(-1);
      return 1.0;
  */
}

/*
 We are using routine from 'ggHXtautau.f' as an example of
 function for user-defined Higgs born calculation.
 We need to adjust conventions for signs etc.
 In our example: we adapt convention for signs of spin states -- from helicity.
*/
extern "C" double disthjkwk_(double*, double*, int*, int*, int*);

double nonSM_adoptH(int ID, double S, double cost, int H1, int H2, int key)
{
  int HH1= H1;
  int HH2=-H1; // appropriate for X2 of spin 2
  
  if(key==0) HH2 = H1; // appropriate for Higgs
  
  //  an example of use:
  return disthjkwk_(&S, &cost, &HH1, &HH2, &key);
 
      /*
      cout<<"             this function is dummy\n"<<endl;
      cout<<"             user must provide his own nonSM_bornH"<<endl;
      cout<<"             that is nonSM_adoptH()"<<endl;
      exit(-1);
      return 1.0;
      */
}



/** Example function that can be used to modify/replace matrix element calculation of vbfdist  */
double vbfdistrModif(int I1, int I2, int I3, int I4, int H1, int H2, double P[6][4], int KEY, double res_modified)
{
    return res_modified;
}

/** Example function that can be used to modify/replace matrix element calculation of vbfdist */
double vbfdistrSPIN2(int I1, int I2, int I3, int I4, int H1, int H2, double P[6][4], int KEY, double spin2distr_result)
{
  double MX= spin2distr_(&I1,&I2,&I3,&I4, &H1, &H2, P, &KEY);
  return MX;
}

//replacement of default (not best quality) random number generator
TRandom gen;
  double randomik(){ 
    return gen.Rndm();
 }

//CONSTRUCTOR
//----------------------------------------------
TauSpinnerRedecay::TauSpinnerRedecay()
//----------------------------------------------
{

} 

//DESTRUCTOR
//----------------------------------------------
TauSpinnerRedecay::~TauSpinnerRedecay()
//----------------------------------------------
{

}

//Initialize
//----------------------------------------------
void TauSpinnerRedecay::Initialize(char *filename, int idhist, int mode, int optPDF)
//----------------------------------------------
{

  //  Create histograms for this analysis
  m_idHist= idhist;
  CreateHistos(m_idHist);
  
  // Initialize Tauola
  Tauola::initialize();
  
  // Uncomment this line if producing sample
  // without polarization or spin correlations
  Tauola::spin_correlation.setAll(false);

  // change Random generator 
  gen.SetSeed(time(NULL));
  Tauola::setRandomGenerator( randomik );
  Tauola::setSeed(time(NULL), 0, 0);

  // Modify branching ratios so that only channels 1-4 are generated
  for(int i=1;i<30;i++) Tauola::setTauBr(i,0.0);
  Tauola::setTauBr(3,1.0);

  // Modify branching ratios so that K0 channels are generated
  /*
  for(int i=1;i<=22;i++) Tauola::setTauBr(i,0);
  //  for(int i=15;i<20;i++) Tauola::setTauBr(i,1.0);
  Tauola::setTauBr(15,1.0);
  Tauola::setTauBr(16,1.0);
  Tauola::setTauBr(19,1.0);
  Tauola::setTauBr(22,1.0);
  //  Tauola::setEtaK0sPi(0,0,0);
  */

  string name="cteq6ll.LHpdf";
  LHAPDF::initPDFSetByName(name);

  // PDFs reinicjalize
  if(optPDF == 1 ){
    string name="MSTW2008nnlo68cl.LHgrid";
    LHAPDF::initPDFSetByName(name);
  }
  if(optPDF == 2 ){
    string name="CT10nnlo.LHgrid";
    LHAPDF::initPDFSetByName(name);
  }
  if(optPDF == 3 ){
    string name="CT10nlo.LHgrid";
    LHAPDF::initPDFSetByName(name);
  }
  if(optPDF == 4 ){
    string name="NNPDF23_nnlo_as_0118.LHgrid";
    LHAPDF::initPDFSetByName(name);
  }

  if(DEBUG){
    const int SUBSET  = 0;
    const string NAME = "cteqll";
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
    cout << "number     = " << LHAPDF::numberPDF() << endl;
    cout << endl;
  }

  double CMSENE = 13000.0; // center of mass system energy.
                          // used in PDF calculation. For pp collisions only
  bool Ipp   = true;  // for pp collisions
  //bool Ipp = false; // otherwise (not prepared yet)

  int Ipol   = 1; // are input samples polarized?

  int nonSM2 = 1; // are we using nonSM calculations?
                  // at present we have checked only that for nonSM2 = 0
                  // all works as in the past. nonSM2 = 1 may simply mean 
                  // errors are switched on. More work is needed 
                  // for this option.
  int nonSMN = 0; // If we are using nonSM calculations we may want corrections 
                  // to shapes only: y/n  (1/0)

  if( mode == 1 ){

    CMSENE = 13000.0;      // center of mass system energy.
                           // used in PDF calculation. For pp collisions only
  }

  // Initialize TauSpinner
  TauSpinner::initialize_spinner(Ipp, Ipol, nonSM2, nonSMN,  CMSENE);

  int ref=1;      // EW scheme to be used for default vbf calculation.
  int variant =1; // EW scheme to be used in optional matrix element reweighting (nonSM2=1). Then
  //   for vbf calculation, declared above prototype method vbfdistrModif (or user function)
  //   will be used. At its disposal result of calculation with variant of  EW scheme will be available.
  vbfinit_(&ref,&variant);

  int QCDdefault=0; // QCD scheme to be used for default vbf calculation.
  int QCDvariant=0; // QCD scheme to be used in optional matrix element reweighting (nonSM2=1).
  setPDFOpt(QCDdefault,QCDvariant);

  // Set function that modifies/replaces Matrix Element calculation of vbfdistr
  // TauSpinner::set_vbfdistrModif(vbfdistrModif);

  // Set function that modifies/replaces Matrix Element calculation of vbfdistr with code of SPIN2
  spin2init_(&ref,&variant);
  TauSpinner::set_vbfdistrModif(vbfdistrSPIN2);

  std::cout << "initialised spin2init " << std::endl;
    

}

//Finalize
//----------------------------------------------
void TauSpinnerRedecay::Finalize()
//----------------------------------------------
{


}
//----------------------------------------------
void TauSpinnerRedecay::Decay( HepMC::GenEvent *event )
//----------------------------------------------
{

    // Convert units to GEV for Tauola++
    event->use_units(HepMC::Units::GEV,HepMC::Units::CM);


    TauolaHepMCEvent *tevent = new TauolaHepMCEvent(event);
    tevent->decayTaus();

    //    std::cout <<" electroweak wt= "<<Tauola::getEWwt() << " " << Tauola::getEWwt0() << "  " << Tauola::getEWwt()/Tauola::getEWwt0() <<  endl;


}

  
//---------------------------------------------------------------------------------------------------------------------------------------------------
  void TauSpinnerRedecay::calcXsectSPIN2(int IDPROD, SimpleParticle &p3, SimpleParticle &p4, SimpleParticle &sp_X,SimpleParticle &tau1, SimpleParticle &tau2, double (&W)[2][2], int KEY)
{
  /*
  // this may be necessary because of matrix element calculation may require absolute energy-momentum conservation!
  // FSR photons may need to be treated explicitely or with interpolation procedures.
  Particle P_QQ( p3.px()+p4.px()+tau1.px()+tau2.px(),
                 p3.py()+p4.py()+tau1.py()+tau2.py(),
                 p3.pz()+p4.pz()+tau1.pz()+tau2.pz(),
                 p3.e() +p4.e() +tau1.e() +tau2.e(), 0 );
  */
  Particle P_QQ( p3.px()+p4.px()+sp_X.px(),
                 p3.py()+p4.py()+sp_X.py(),
                 p3.pz()+p4.pz()+sp_X.pz(),
                 p3.e() +p4.e() +sp_X.e() , 0 );


  double SS = P_QQ.recalculated_mass()*P_QQ.recalculated_mass(); 

  double CMSENE = 13000.0;
  
  double x1x2  = SS/CMSENE/CMSENE;
  double x1Mx2 = P_QQ.pz()/CMSENE*2;
  
  double x1 = (  x1Mx2 + sqrt(x1Mx2*x1Mx2 + 4*x1x2) )/2;
  double x2 = ( -x1Mx2 + sqrt(x1Mx2*x1Mx2 + 4*x1x2) )/2;

  //---------------------------------------------------------------------------
  // Construct the matrix for FORTRAN function
  // NOTE: different order of indices than in FORTRAN!
  double P[6][4] = { { CMSENE/2*x1, 0.0,       0.0,          CMSENE/2*x1  },
                     { CMSENE/2*x2, 0.0,       0.0,         -CMSENE/2*x2  }, 
                     { p3.e(),      p3.px(),   p3.py(),      p3.pz()      }, 
                     { p4.e(),      p4.px(),   p4.py(),      p4.pz()      },
                     { tau1.e(),    tau1.px(), tau1.py(),    tau1.pz()    },
                     { tau2.e(),    tau2.px(), tau2.py(),    tau2.pz()    } };
 

  W[0][0]=0.0;
  W[0][1]=0.0;
  W[1][0]=0.0;
  W[1][1]=0.0;
  
  //
  // Calculate 'f' function for all x1 and all ID1, ID2
  //
  double  f_x1_ARRAY[11] = { 0.0 };
  double  f_x2_ARRAY[11] = { 0.0 };
  double *f_x1 = f_x1_ARRAY+5;     // This way f_x1[i],f_x2[i] can be used with 
  double *f_x2 = f_x2_ARRAY+5;     // i going from -5 to 5

  double SSfix = 91.188*91.188;
  double CMSENEfix = 91.188;
  for(int i=-5;i<=5;++i) {
    f_x1[i] = f(x1,i,SSfix,CMSENEfix);
    f_x2[i] = f(x2,i,SSfix,CMSENEfix);
    //    std::cout << "ERW: x, Q2, f(x,Q2), f(x,Q)=" << x1 << "  " << SS << "  " << f(x1,i,SS,CMSENE) << "  " << f(x1,i,sqrt(SS),CMSENE) << std::endl;
  }

  W[0][0]=0.0;
  W[0][1]=0.0;
  W[1][0]=0.0;
  W[1][1]=0.0;
  
  // these loops need to be cleaned from zero contributions! 
  for(int I1=-4;I1<=4;I1++){  // for test of single production process fix flavour
    for(int I2=-4;I2<=4;I2++){  // for test of single production process fix flavour
      for(int I3=-4;I3<=4;I3++){
	for(int I4=-4;I4<=4;I4++){
	  
          int ID1 = I1; if( ID1 == 0 ) ID1 = 21;
          int ID2 = I2; if( ID2 == 0 ) ID2 = 21;
          int ID3 = I3; if( ID3 == 0 ) ID3 = 21;
          int ID4 = I4; if( ID4 == 0 ) ID4 = 21;

	  //preselect production group
	  bool accept = false;
	  // any process 
          if( IDPROD == 0)  accept = true;
          // only GG processes
          if( IDPROD == 1 &&  ID1 == 21 && ID2 == 21 && abs (ID3) < 5 &&  abs (ID4) < 5 ) accept = true;
          // only GG processes ... details
          if( IDPROD == 110 &&  ID1 == 21 && ID2 == 21 ) {
	    accept = true;
            if( abs (ID3) != 2 || abs (ID4) != 2  ) accept = false;
	  }
          if( IDPROD == 111 &&  ID1 == 21 && ID2 == 21 ) {
	    accept = true;
            if( abs (ID3) != 1 || abs (ID4) != 1  ) accept = false;
	  }
          if( IDPROD == 112 &&  ID1 == 21 && ID2 == 21 ) {
	    accept = true;
            if( abs (ID3) != 3 || abs (ID4) != 3  ) accept = false;
	  }
          if( IDPROD == 113 &&  ID1 == 21 && ID2 == 21 ) {
	    accept = true;
            if( abs (ID3) != 4 || abs (ID4) != 4  ) accept = false;
	  }
          // only GQ processes
          if( IDPROD == 2 &&  ID1 == 21 && abs(ID2) < 5 ) accept = true ;
          if( IDPROD == 2 &&  ID2 == 21 && abs(ID1) < 5 ) accept = true ;
          // only GQ processes ... details
          if( (IDPROD == 210) && ( ID1 == 21) && ( ID2 < 5 ) && (ID2 > 0) ) accept = true;
          if( (IDPROD == 211) && ( ID1 == 21) && ( ID2 < 5 ) && (ID2 < 0) ) accept = true;
	  if( (IDPROD == 212) && ( ID2 == 21) && ( ID1 < 5 ) && (ID1 > 0) ) accept = true;
	  if( (IDPROD == 213) && ( ID2 == 21) && ( ID1 < 5 ) && (ID1 < 0) ) accept = true;
         // only QQ, QXQX processes
	  if( IDPROD == 3 &&  (ID1 * ID2 > 0 ) && abs (ID1) < 5 &&  abs (ID2) < 5  ) accept = true;
         // only QQ processes .. details
 	  if( IDPROD == 31 &&  ID1 > 0  &&  ID2 > 0  && abs (ID1) < 5 &&  abs (ID2) < 5  ) accept = true;
 	  if( IDPROD == 32 &&  ID1 < 0  &&  ID2 < 0  && abs (ID1) < 5 &&  abs (ID2) < 5  ) accept = true;
         // only QQ processes .. details
	  if( IDPROD == 310 &&  ID1 > 0 &&  ID2 > 0  && ID1 < 5 && ID2 < 5 && ID1 == ID2 ) accept = true;
	  if( IDPROD == 311 &&  ID1 > 0 &&  ID2 > 0  && ID1 < 5 && ID2 < 5 && ID1 != ID2 ) accept = true;
	  if( IDPROD == 312 &&  ID1 < 0 &&  ID2 < 0  && ID1 == ID2 ) accept = true;
	  if( IDPROD == 313 &&  ID1 < 0 &&  ID2 < 0  && ID1 != ID2 ) accept = true;
          // only QQ processes .. details
	  if( IDPROD == 320 &&  ID1 *  ID2 > 0  && abs(ID1) < 5 && abs(ID2) < 5 && abs(ID1) == abs(ID2) ) accept = true;
	  if( IDPROD == 321 &&  ID1 *  ID2 > 0  && abs(ID1) < 5 && abs(ID2) < 5 && abs(ID1) != abs(ID2) ) accept = true;
          // only QQ processes .. details
          if( IDPROD == 3101 &&  ID1 ==  2 &&  ID2 ==  2 &&  ID3 ==  2 &&  ID4 ==  2 ) accept = true;
          if( IDPROD == 3102 &&  ID1 == -2 &&  ID2 == -2 &&  ID3 == -2 &&  ID4 == -2  ) accept = true;
          if( IDPROD == 3103 &&  ID1 ==  2 &&  ID2 ==  2 ) accept = true;
          if( IDPROD == 3104 &&  ID1 == -2 &&  ID2 == -2 ) accept = true;
        // only QQX processes
          if( IDPROD == 4 &&  (ID1 * ID2 < 0 ) && abs (ID1) < 5 &&  abs (ID2) < 5  ) accept = true;
          // only QQX processes .. details
	  if( IDPROD == 420 &&  ID1 * ID2 < 0  && abs(ID1) < 5 && abs(ID2) < 5 && abs(ID1) == abs(ID2) ) accept = true;
	  if( IDPROD == 421 &&  ID1 * ID2 < 0  && abs(ID1) < 5 && abs(ID2) < 5 && abs(ID1) != abs(ID2) ) accept = true;
          // only QQX processes, first family in initial/final
          if( IDPROD == 41 &&  (ID1 * ID2 < 0 ) && abs(ID1) < 5 &&  abs(ID2) < 5 && abs(ID3) < 5 &&  abs(ID4) < 5 ) {
	    accept = true;
            if( abs (ID1) > 2 || abs (ID2) > 2 || abs (ID3) >2 || abs (ID4) > 2  ) accept = false;
	  }
          // only QQX processes, second family in initial/final state
          if( IDPROD == 42 &&  (ID1 * ID2 < 0 ) && abs(ID1) < 5 &&  abs(ID2) < 5 && abs(ID3) < 5 &&  abs(ID4) < 5 ) {
	    accept = true;
            if( abs (ID1) < 3 || abs (ID2) < 3 || abs (ID3) < 3 || abs (ID4) < 3  ) accept = false;
	  }
          // only QQX processes, mixed families in initial/final state
          if( IDPROD == 43 &&  ( ID1 * ID2 < 0) && abs(ID1) < 5 &&  abs(ID2) < 5 && abs(ID3) < 5 &&  abs(ID4) < 5 ) {
	    accept = true;
            if(  abs (ID1) < 3  &&  abs (ID2) < 3  &&  abs (ID3) < 3  &&  abs (ID4) < 3  ) accept = false;
            if(  abs (ID1) > 2  &&  abs (ID2) > 2  &&  abs (ID3) > 2  &&  abs (ID4) > 2  ) accept = false; 
	  }
          // only QQX processes, first family in initial state
          if( IDPROD == 44 &&  (ID1 * ID2 < 0 ) && abs(ID1) < 5 &&  abs(ID2) < 5 ) {
	    accept = true;
            if( abs (ID1) > 2 || abs (ID2) > 2 ) accept = false;
	  }
          // only QQX processes ... details
          if( IDPROD == 4101 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> U UX
            if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) != 2 || abs (ID4) != 2  ) accept = false;
            if( ID1 != 2 ) accept = false;
	  }
          if( IDPROD == 4102 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> U UX
            if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) != 2 || abs (ID4) != 2  ) accept = false;
            if( ID1 != -2 ) accept = false;
	  }
          if( IDPROD == 4103&&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> Q QX
            if( abs (ID1) != 2 || abs (ID2) != 2 ) accept = false;
            if( ID1 != 2 ) accept = false;
	  }
          if( IDPROD == 4104&&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only UX U --> Q QX
            if( abs (ID1) != 2 || abs (ID2) != 2 ) accept = false;
            if( ID1 != -2 ) accept = false;
	  }
          if( IDPROD == 4105&&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only D DX --> Q QX
            if( abs (ID1) != 1 || abs (ID2) != 1 ) accept = false;
            if( ID1 != 1 ) accept = false;
	  }
          if( IDPROD == 4106&&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only DX D --> Q QX
            if( abs (ID1) != 1 || abs (ID2) != 1 ) accept = false;
            if( ID1 != -1 ) accept = false;
	  }
          if( IDPROD == 4107&&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only Q QX --> Q QX
            if( ID1 < 0  ||  ID2 > 0) accept = false;
	  }
          if( IDPROD == 4108&&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only QX Q --> Q QX
            if( ID1 > 0  ||  ID2 < 0) accept = false;
	  }
           // only QQX processes ... details
          if( IDPROD == 410 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> U UX
            if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) != 2 || abs (ID4) != 2  ) accept = false;
	  }
          if( IDPROD == 411 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> C CX
            if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) != 4 || abs (ID4) != 4  ) accept = false;
	  }
          if( IDPROD == 412 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> S DX
            if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) != 3 || abs (ID4) != 1  ) accept = false;
	  }
          if( IDPROD == 413 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> D SX
            if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) != 1 || abs (ID4) != 3  ) accept = false;
	  }
          if( IDPROD == 414 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> D DX
            if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) != 1 || abs (ID4) != 1  ) accept = false;
	  }
          if( IDPROD == 415 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> S SX
             if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) != 3 || abs (ID4) != 3  ) accept = false;
	  }
          if( IDPROD == 416 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> Q QX 
            if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) != 21 || abs (ID4) != 21  ) accept = false;
	  }
          if( IDPROD == 417 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> G G 
            if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) == 21 || abs (ID4) == 21  ) accept = false;
	  }
          if( IDPROD == 418 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> J J  
            if( abs (ID1) != 2 || abs (ID2) != 2 ) accept = false;
	  }
          if( IDPROD == 419 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only D DX --> D DX  
            if( abs (ID1) != 1 || abs (ID2) != 1 || abs (ID3) != 1 || abs (ID4) != 1  ) accept = false;
	  }
          if( IDPROD == 511 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only D DX, DX D --> G G 
            if( abs (ID1) != 1 || abs (ID2) != 1 || abs (ID3) != 21 || abs (ID4) != 21  ) accept = false;
	  }
          if( IDPROD == 512 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only D DX --> G G 
            if( ID1 != 1 || ID2 != -1 || abs (ID3) != 21 || abs (ID4) != 21  ) accept = false;
	  }
          if( IDPROD == 513 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only DX D --> G G 
            if( ID1 != -1 || ID2 != 1 || abs (ID3) != 21 || abs (ID4) != 21  ) accept = false;
	  }
          if( IDPROD == 521 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX, UX U --> G G 
            if( abs (ID1) != 2 || abs (ID2) != 2 || abs (ID3) != 21 || abs (ID4) != 21  ) accept = false;
	  }
          if( IDPROD == 522 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only U UX --> G G 
            if( ID1 != 2 || ID2 != -2 || abs (ID3) != 21 || abs (ID4) != 21  ) accept = false;
	  }
          if( IDPROD == 523 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only UX U --> G G 
            if( ID1 != -2 || ID2 != 2 || abs (ID3) != 21 || abs (ID4) != 21  ) accept = false;
	  }
          if( IDPROD == 531 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only Q QX, QX Q --> G G 
            if( abs(ID1) > 4 || abs(ID2) > 4 || abs (ID3) != 21 || abs (ID4) != 21  ) accept = false;
	  }
          if( IDPROD == 532 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only Q QX  --> G G 
            if( abs(ID1) > 4 || abs(ID2) > 4 || ID1 < 0 || abs(ID3) != 21 || abs(ID4) != 21  ) accept = false;
		}
          if( IDPROD == 533 &&  (ID1 * ID2 < 0 ) ) {
	    accept = true;
	    // only QX Q  --> G G 
            if( abs(ID1) > 4 || abs(ID2) > 4 || ID2 < 0 || abs (ID3) != 21 || abs (ID4) != 21  ) accept = false;
	  }
          if( IDPROD == 541 ) {
	    accept = false;
	    // only U SX  --> D CX 
            if( ID1 ==  2 && ID2 == -3 && ID3 == 1 && ID4 == -4 ) accept = true;
            if( ID1 == -3 && ID2 ==  2 && ID3 == 1 && ID4 == -4 ) accept = true;
            if( ID1 ==  2 && ID2 == -3 && ID3 ==-4 && ID4 ==  1 ) accept = true;
            if( ID1 == -3 && ID2 ==  2 && ID3 ==-4 && ID4 ==  1 ) accept = true;
	  }

 	  if(accept ){
	    double result=0;	    
	    W[0][0] += f_x1[I1]*f_x2[I2]*vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1, -1, P, KEY,result);    // as in case of nonSM_adopt we change the sign for 
	    W[0][1] += f_x1[I1]*f_x2[I2]*vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1,  1, P, KEY,result);    // second tau helicity assuming without checking that
	    W[1][0] += f_x1[I1]*f_x2[I2]*vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1, -1, P, KEY,result);    // for VBF quantization frame conventions are the same.
	    W[1][1] += f_x1[I1]*f_x2[I2]*vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1,  1, P, KEY,result);	    
	  }	    
	}
      }
    }
  }
}


void  TauSpinnerRedecay::makeSimpleTestME2SPIN2()
{
 
  //---------------------------------------------------------------------------
  // Construct the matrix for FORTRAN function
  // NOTE: different order of indices than in FORTRAN!

  std::cout << "ERW: ----------------------------- " << std::endl;
  double P1[6][4]  = {{ 0.5000000E+03,  0.0,            0.0,            0.5000000E+03   },
                      { 0.5000000E+03,  0.0,            0.0,           -0.5000000E+03   }, 
                      { 0.8855009E+02, -0.2210038E+02,  0.4007979E+02, -0.7580437E+02   }, 
                      { 0.3283248E+03, -0.1038482E+03, -0.3019295E+03,  0.7649385E+02   },
                      { 0.1523663E+03, -0.1058795E+03, -0.9770827E+02,  0.4954769E+02   },
                      { 0.4307588E+03,  0.2318280E+03,  0.3595580E+03, -0.5023717E+02   } };
  calcTestME2SPIN2(1, P1);

  Particle p1         (  0.0,            0.0,            0.5000000E+03,  0.5000000E+03,   1  );
  Particle p2         (  0.0,            0.0,           -0.5000000E+03,  0.5000000E+03,  -1  ); 
  Particle p3         ( -0.2210038E+02,  0.4007979E+02, -0.7580437E+02,  0.8855009E+02,  2 ); 
  Particle p4         ( -0.1038482E+03, -0.3019295E+03,  0.7649385E+02,  0.3283248E+03,  -2  );
  Particle tau1       ( -0.1058795E+03, -0.9770827E+02,  0.4954769E+02,  0.1523663E+03,  15 );
  Particle tau2       (  0.2318280E+03,  0.3595580E+03, -0.5023717E+02,  0.4307588E+03,  -15  );

  std::cout << "ERW:  4-momenta before boost ----------------------------- " << std::endl;

  p1.print();
  p2.print();
  p3.print();
  p4.print();
  tau1.print();
  tau2.print();
  double x1=0.03;  // x1 for the first parton
  double xm = 1000.0;
  double cmsene = 13000.0;
  double x2=xm/cmsene*xm/cmsene/x1;    // x2 adjusted from  cms energy (13000), and virtuality of the whole system 1000 
  std::cout << "x1, x2= " << x1 << " " << x2 << std::endl;
  if (x2>=1.0) std::cout << "ERW: wrong x1, x2 cannot be defined (is above 1);  x2=" << x2 << std::endl;

  Particle P_Z0( tau1.px()+tau2.px(), tau1.py()+tau2.py(), tau1.pz()+tau2.pz(), tau1.e()+tau2.e(), 23 );
  P_Z0.print();
  double Q2 = P_Z0.recalculated_mass() *  P_Z0.recalculated_mass();
  std::cout << " pdfs = " << f(x1,0,Q2,cmsene) * f(x2,1,Q2,cmsene) << std::endl;

  Particle P_QQ( 0.0, 1.0e-10, (x1-x2)*cmsene/2.0, (x1+x2)*cmsene/2.0, 10 );
  P_QQ.print();
 
  p1.boostFromRestFrame(P_QQ);
  p2.boostFromRestFrame(P_QQ);
  p3.boostFromRestFrame(P_QQ);
  p4.boostFromRestFrame(P_QQ);
  P_Z0.boostFromRestFrame(P_QQ);
  tau1.boostFromRestFrame(P_QQ);
  tau2.boostFromRestFrame(P_QQ);

  std::cout << "ERW:  4-momenta after boost ----------------------------- " << std::endl;

  p1.print();
  p2.print();
  p3.print();
  p4.print();
  P_Z0.print();
  tau1.print();
  tau2.print();

  std::cout << "ERW: ----------------------------- " << std::endl;
  double P2[6][4]  = {{ 0.5000000E+03,  0.0,            0.0,            0.5000000E+03   },
                      { 0.5000000E+03,  0.0,            0.0,           -0.5000000E+03   }, 
                      { 0.1177462E+03,  -0.6070512E+02,   0.7123011E+02,   0.7145150E+02   }, 
                      { 0.3509495E+03,  -0.3178775E+02,   0.8393832E+02,   0.3392779E+03   },
                      { 0.3493321E+03,   0.1840069E+03,  -0.5152712E+02,  -0.2924315E+03   },
                      { 0.1819722E+03,  -0.9151401E+02,  -0.1036413E+03,  -0.1182978E+03   } };
  calcTestME2SPIN2(2, P2);
  std::cout << "ERW: ----------------------------- " << std::endl;
  double P3[6][4]  = {{ 0.5000000E+03,  0.0,            0.0,            0.5000000E+03   },
                      { 0.5000000E+03,  0.0,            0.0,           -0.5000000E+03   }, 
                      { 0.2586900E+03,   0.1324670E+03,  -0.1696171E+03,  -0.1435378E+03   }, 
                      { 0.1084567E+03,  -0.5735712E+02,  -0.2162482E+02,  -0.8947281E+02   },
                      { 0.4005742E+03,  -0.1580760E+03,   0.3563160E+03,   0.9223569E+02   },
                      { 0.2322791E+03,   0.8296613E+02,  -0.1650741E+03,   0.1407749E+03   } };
  calcTestME2SPIN2(3, P3);
  std::cout << "ERW: ----------------------------- " << std::endl;
  double P4[6][4]  = {{ 0.5000000E+03,  0.0,            0.0,            0.5000000E+03   },
                      { 0.5000000E+03,  0.0,            0.0,           -0.5000000E+03   }, 
                      { 0.1595700E+03,  -0.6917808E+02,  -0.1395175E+03,  -0.3481123E+02   }, 
                      { 0.2247758E+03,  -0.1360140E+03,   0.1650340E+03,  -0.6919641E+02   },
                      { 0.2508802E+03,   0.1447863E+01,   0.2499830E+03,  -0.2107335E+02   },
                      { 0.3647740E+03,   0.2037442E+03,  -0.2754995E+03,   0.1250810E+03   } };
  calcTestME2SPIN2(4, P4);

}

/*******************************************************************************/
  void  TauSpinnerRedecay::calcTestME2SPIN2(int iter, double P1[6][4])
{

  double ME2ref, ME2;
  int ID1, ID2, ID3, ID4;

  int KEY=0;

  // case GD -> GD
  ID1 = 21; ID2 = 1; ID3 = 21; ID4 = 1;

  if(iter == 1) ME2ref =  1.5961579933867344E-010;
  if(iter == 2) ME2ref =  3.8749742050329834E-010;
  if(iter == 3) ME2ref =  5.0434636937545397E-012;
  if(iter == 4) ME2ref =  7.9077184257060427E-012;

  double result=0;
  ME2    =   vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1, -1, P1,  KEY, result) 
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1,  1, P1,  KEY, result)  
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1,  1, P1,  KEY, result) 
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1, -1, P1,  KEY, result);  

  std::cout << "ERW: ME2 GD->GD  (ref)     = " << ME2ref << std::endl;
  std::cout << "ERW: ME2 GD->GD            = " << ME2 << "       ratio to ref = " << ME2/ME2ref  << std::endl;

  // case GU -> GU
  ID1 = 21; ID2 = 2; ID3 = 21; ID4 = 2;

  if(iter == 1) ME2ref =  2.9195503763051040E-010;

  ME2    =   vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1, -1, P1,  KEY,result) 
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1,  1, P1,  KEY,result)  
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1,  1, P1,  KEY,result) 
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1, -1, P1,  KEY,result);  

  if(iter == 1)std::cout << "ERW: ME2 GU->GU  (ref)     = " << ME2ref << std::endl;
  if(iter == 1)std::cout << "ERW: ME2 GU->GU            = " << ME2 << "       ratio to ref = " << ME2/ME2ref  << std::endl;

  // case DD -> DD
  ID1 = 1; ID2 = 1; ID3 = 1; ID4 = 1;

  if(iter == 1) ME2ref =  3.3953129762581284E-017;
  if(iter == 2) ME2ref =  6.0054072781075002E-017;
  if(iter == 3) ME2ref =  3.9707833415682912E-018;
  if(iter == 4) ME2ref =  1.5342177192807347E-018;

  ME2    =   vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1, -1, P1,  KEY, result) 
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1,  1, P1,  KEY, result)  
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1,  1, P1,  KEY, result) 
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1, -1, P1,  KEY, result);  

  std::cout << "ERW: ME2 DD->DD  (ref)     = " << ME2ref << std::endl;
  std::cout << "ERW: ME2 DD->DD            = " << ME2 << "       ratio to ref = " << ME2/ME2ref  << std::endl;

  // case UU -> UU
  ID1 = 2; ID2 = 2; ID3 = 2; ID4 = 2;

  if(iter == 1) ME2ref =  1.9412924824120248E-017;
  if(iter == 2) ME2ref =  4.0830132078559964E-017;
  if(iter == 3) ME2ref =  2.1297931556738857E-018;
  if(iter == 4) ME2ref =  7.9215386777281075E-019;

  ME2    =   vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1, -1, P1,  KEY, result) 
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1,  1, P1,  KEY, result)  
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1,  1, P1,  KEY, result) 
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1, -1, P1,  KEY, result);  

  std::cout << "ERW: ME2 UU->UU  (ref)     = " << ME2ref << std::endl;
  std::cout << "ERW: ME2 UU->UU            = " << ME2 << "       ratio to ref = " << ME2/ME2ref  << std::endl;

  // case DDX -> CCX
  ID1 = 1; ID2 = -1; ID3 = 4; ID4 = -4;

  if(iter == 1) ME2ref =  3.6780119739685137E-020;
  if(iter == 2) ME2ref =  5.2280923900274694E-018;
  if(iter == 3) ME2ref =  2.9001589049209953E-019;
  if(iter == 4) ME2ref =  5.8509026904432882E-020;

  ME2    =   vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1, -1, P1,  KEY, result) 
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4,  1,  1, P1,  KEY, result)  
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1,  1, P1,  KEY, result) 
           + vbfdistrSPIN2(ID1,ID2,ID3,ID4, -1, -1, P1,  KEY, result);  

  std::cout << "ERW: ME2 DDX->CCX  (ref)   = " << ME2ref << std::endl;
  std::cout << "ERW: ME2 DDX->CCX          = " << ME2 << "       ratio to ref = " << ME2/ME2ref  << std::endl;

}



//---------------------------------------------------------------------------------------------------------------------------------------------------
void  TauSpinnerRedecay::calcSumME2SPIN2(SimpleParticle &p3, SimpleParticle &p4, SimpleParticle &sp_X,SimpleParticle &tau1, SimpleParticle &tau2, double (&W)[2][2], int KEY,
                    int ID1, int ID2, int ID3, int ID4)
{
  /*
  // this may be necessary because of matrix element calculation may require absolute energy-momentum conservation!
  // FSR photons may need to be treated explicitely or with interpolation procedures.
  Particle P_QQ( p3.px()+p4.px()+tau1.px()+tau2.px(),
                 p3.py()+p4.py()+tau1.py()+tau2.py(),
                 p3.pz()+p4.pz()+tau1.pz()+tau2.pz(),
                 p3.e() +p4.e() +tau1.e() +tau2.e(), 0 );
  */
  Particle P_QQ( p3.px()+p4.px()+sp_X.px(),
                 p3.py()+p4.py()+sp_X.py(),
                 p3.pz()+p4.pz()+sp_X.pz(),
                 p3.e() +p4.e() +sp_X.e() , 0 );


  double SS = P_QQ.recalculated_mass()*P_QQ.recalculated_mass(); 

  double CMSENE = 13000.0;
  double x1x2   = SS/CMSENE/CMSENE;
  double x1Mx2  = P_QQ.pz()/CMSENE*2;
  
  double x1 = (  x1Mx2 + sqrt(x1Mx2*x1Mx2 + 4*x1x2) )/2;
  double x2 = ( -x1Mx2 + sqrt(x1Mx2*x1Mx2 + 4*x1x2) )/2;

  //---------------------------------------------------------------------------
  // Construct the matrix for FORTRAN function
  // NOTE: different order of indices than in FORTRAN!
  double P[6][4] = { { CMSENE/2*x1, 0.0,       0.0,          CMSENE/2*x1  },
                     { CMSENE/2*x2, 0.0,       0.0,         -CMSENE/2*x2  }, 
                     { p3.e(),      p3.px(),   p3.py(),      p3.pz()      }, 
                     { p4.e(),      p4.px(),   p4.py(),      p4.pz()      },
                     { tau1.e(),    tau1.px(), tau1.py(),    tau1.pz()    },
                     { tau2.e(),    tau2.px(), tau2.py(),    tau2.pz()    } };
 
  double result=0;
  W[0][0] = vbfdistrSPIN2(ID1,ID2,ID3,ID4, 1, -1, P, KEY, result);   
  W[0][1] = vbfdistrSPIN2(ID1,ID2,ID3,ID4, 1,  1, P, KEY, result);  
  W[1][0] = vbfdistrSPIN2(ID1,ID2,ID3,ID4,-1, -1, P, KEY, result); 
  W[1][1] = vbfdistrSPIN2(ID1,ID2,ID3,ID4,-1,  1, P, KEY, result);

  double  sumW =(W[0][0]+W[0][1]+ W[1][0]+W[1][1]); 
  if( sumW < 0 ){
    std::cout << "ERW: ==== " << std::endl; 
    std::cout << "ERW: W[]  = " << W[0][0] << " " << W[0][1] << " " << W[1][0] << " " << W[1][1]	<< std::endl; 
    std::cout << "ERW: ==== " << std::endl; 
    std::cout << "ERW: p1   = " << CMSENE/2*x1 << "  " << 0.0 << "  " << 0.0 << "  " <<   CMSENE/2*x1 	<< std::endl; 
    std::cout << "ERW: p2   = " << CMSENE/2*x2 << "  " << 0.0 << "  " << 0.0 << "  " <<  -CMSENE/2*x2 	<< std::endl; 
    std::cout << "ERW:  X   = " << sp_X.e()  << "  " << sp_X.px()  << "  " << sp_X.py()  << "  " <<  sp_X.pz() 	<< std::endl; 
    std::cout << "ERW: p3   = " << p3.e() << "  " << p3.px() << "  " << p3.py() << "  " <<  p3.pz() 	<< std::endl; 
    std::cout << "ERW: p4   = " << p4.e() << "  " << p4.px() << "  " << p4.py() << "  " <<  p4.pz() 	<< std::endl; 
    std::cout << "ERW: tau1 = " << tau1.e() << "  " << tau1.px() << "  " << tau1.py() << "  " <<  tau1.pz() 	<< std::endl; 
    std::cout << "ERW: tau2 = " << tau2.e() << "  " << tau2.px() << "  " << tau2.py() << "  " <<  tau2.pz() 	<< std::endl; 
    std::cout << "ERW: ==== " << std::endl; 
    std::cout << "ERW:   p1+p2= " << P[0][0]+P[1][0] <<  "  " <<  P[0][1]+P[1][1] << "  "  <<  P[0][2]+P[1][2] <<  "  "  <<  P[0][3]+P[1][3]	<< std::endl;
    std::cout << "ERW: X+p3+p4= " << P[2][0]+P[3][0]+P[4][0]+P[5][0] <<  "  " <<  P[2][1]+P[3][1]+P[4][1]+P[5][1] << "  " 
                                  << P[2][2]+P[3][2]+P[4][2]+P[5][2] <<  "  " <<  P[2][3]+P[3][3]+P[4][3]+P[5][3] << std::endl;

    double p3sqr = p3.e()*p3.e()-p3.px()*p3.px()-p3.py()*p3.py()-p3.pz()*p3.pz();
    double p4sqr = p4.e()*p4.e()-p4.px()*p4.px()-p4.py()*p4.py()-p4.pz()*p4.pz();
    std::cout << "ERW: p3^2, p4^2 =" <<  p3sqr  << "  " <<  p4sqr << std::endl;
  }

}


//---------------------------------------------------------------------------------------------------------------------------------------------------
void  TauSpinnerRedecay::calcProdMatrixSPIN2(SimpleParticle &p3, SimpleParticle &p4, SimpleParticle &sp_X,SimpleParticle &tau1, SimpleParticle &tau2, double (&W)[2][2], int KEY,
                    int ID1, int ID2, int ID3, int ID4, int pdfOpt)
{
  /*
  // this may be necessary because of matrix element calculation may require absolute energy-momentum conservation!
  // FSR photons may need to be treated explicitely or with interpolation procedures.
  Particle P_QQ( p3.px()+p4.px()+tau1.px()+tau2.px(),
                 p3.py()+p4.py()+tau1.py()+tau2.py(),
                 p3.pz()+p4.pz()+tau1.pz()+tau2.pz(),
                 p3.e() +p4.e() +tau1.e() +tau2.e(), 0 );
  */
  Particle P_QQ( p3.px()+p4.px()+sp_X.px(),
                 p3.py()+p4.py()+sp_X.py(),
                 p3.pz()+p4.pz()+sp_X.pz(),
                 p3.e() +p4.e() +sp_X.e() , 0 );


  double SS = P_QQ.recalculated_mass()*P_QQ.recalculated_mass(); 
  
  double CMSENE = 13000.0;
  double x1x2   = SS/CMSENE/CMSENE;
  double x1Mx2  = P_QQ.pz()/CMSENE*2;
  
  double x1 = (  x1Mx2 + sqrt(x1Mx2*x1Mx2 + 4*x1x2) )/2;
  double x2 = ( -x1Mx2 + sqrt(x1Mx2*x1Mx2 + 4*x1x2) )/2;
  
  //---------------------------------------------------------------------------
  // Construct the matrix for FORTRAN function
  // NOTE: different order of indices than in FORTRAN!
  double P[6][4] = { { CMSENE/2*x1, 0.0,       0.0,          CMSENE/2*x1  },
                     { CMSENE/2*x2, 0.0,       0.0,         -CMSENE/2*x2  }, 
                     { p3.e(),      p3.px(),   p3.py(),      p3.pz()      }, 
                     { p4.e(),      p4.px(),   p4.py(),      p4.pz()      },
                     { tau1.e(),    tau1.px(), tau1.py(),    tau1.pz()    },
                     { tau2.e(),    tau2.px(), tau2.py(),    tau2.pz()    } };
 

  //
  // Calculate 'f' function for all x1 and all ID1, ID2
  //
  double  f_x1_ARRAY[11] = { 0.0 };
  double  f_x2_ARRAY[11] = { 0.0 };
  double *f_x1 = f_x1_ARRAY+5;     // This way f_x1[i],f_x2[i] can be used with 
  double *f_x2 = f_x2_ARRAY+5;     // i going from -5 to 5


  Particle P_X(  sp_X.px(), sp_X.py(), sp_X.pz(), sp_X.e() , 0 );


  double Q2  =  P_X.recalculated_mass()*P_X.recalculated_mass();
  double PT2 =  sp_X.px()* sp_X.px() +  sp_X.py()*  sp_X.py(); 

  if(pdfOpt == 0){
    for(int i=-5;i<=5;++i) {
      f_x1[i] = f(x1,i,Q2,CMSENE);
      f_x2[i] = f(x2,i,Q2,CMSENE);
    }
  } else if (pdfOpt == 1) {
    for(int i=-5;i<=5;++i) {
      f_x1[i] = f(x1,i,PT2,CMSENE);
      f_x2[i] = f(x2,i,PT2,CMSENE);
    }
  } else if (pdfOpt == 2) {
    double PT24 = PT2/4.;
    for(int i=-5;i<=5;++i) {
      f_x1[i] = f(x1,i,PT24,CMSENE);
      f_x2[i] = f(x2,i,PT24,CMSENE);
    }
  }


  int I1=ID1;
  int I2=ID2;
  if( I1 == 21) I1=0;
  if( I2 == 21) I2=0;

  double result;
  W[0][0] = f_x1[I1]*f_x2[I2]*vbfdistrSPIN2(ID1,ID2,ID3,ID4, 1, -1, P, KEY, result);   
  W[0][1] = f_x1[I1]*f_x2[I2]*vbfdistrSPIN2(ID1,ID2,ID3,ID4, 1,  1, P, KEY, result);  
  W[1][0] = f_x1[I1]*f_x2[I2]*vbfdistrSPIN2(ID1,ID2,ID3,ID4,-1, -1, P, KEY, result); 
  W[1][1] = f_x1[I1]*f_x2[I2]*vbfdistrSPIN2(ID1,ID2,ID3,ID4,-1,  1, P, KEY, result);

  double  sumW =(W[0][0]+W[0][1]+ W[1][0]+W[1][1]); 
  if( sumW < 0 ){
    std::cout << "ERW: f1*f2= " << f_x1[I1]*f_x2[I2]  << "  x1=" << x1 << " x2=" << x2 << " f1=" << f_x1[I1] << " f2=" << f_x2[I1]	<< std::endl; 
    std::cout << "ERW: ==== " << std::endl; 
    std::cout << "ERW: W[]  = " << W[0][0] << " " << W[0][1] << " " << W[1][0] << " " << W[1][1]	<< std::endl; 
    std::cout << "ERW: ==== " << std::endl; 
    std::cout << "ERW: p1   = " << CMSENE/2*x1 << "  " << 0.0 << "  " << 0.0 << "  " <<   CMSENE/2*x1 	<< std::endl; 
    std::cout << "ERW: p2   = " << CMSENE/2*x2 << "  " << 0.0 << "  " << 0.0 << "  " <<  -CMSENE/2*x2 	<< std::endl; 
    std::cout << "ERW:  X   = " << sp_X.e()  << "  " << sp_X.px()  << "  " << sp_X.py()  << "  " <<  sp_X.pz() 	<< std::endl; 
    std::cout << "ERW: p3   = " << p3.e() << "  " << p3.px() << "  " << p3.py() << "  " <<  p3.pz() 	<< std::endl; 
    std::cout << "ERW: p4   = " << p4.e() << "  " << p4.px() << "  " << p4.py() << "  " <<  p4.pz() 	<< std::endl; 
    std::cout << "ERW: tau1 = " << tau1.e() << "  " << tau1.px() << "  " << tau1.py() << "  " <<  tau1.pz() 	<< std::endl; 
    std::cout << "ERW: tau2 = " << tau2.e() << "  " << tau2.px() << "  " << tau2.py() << "  " <<  tau2.pz() 	<< std::endl; 
    std::cout << "ERW: ==== " << std::endl; 
    std::cout << "ERW:   p1+p2= " << P[0][0]+P[1][0] <<  "  " <<  P[0][1]+P[1][1] << "  "  <<  P[0][2]+P[1][2] <<  "  "  <<  P[0][3]+P[1][3]	<< std::endl;
    std::cout << "ERW: X+p3+p4= " << P[2][0]+P[3][0]+P[4][0]+P[5][0] <<  "  " <<  P[2][1]+P[3][1]+P[4][1]+P[5][1] << "  " 
                                  << P[2][2]+P[3][2]+P[4][2]+P[5][2] <<  "  " <<  P[2][3]+P[3][3]+P[4][3]+P[5][3] << std::endl;

    double p3sqr = p3.e()*p3.e()-p3.px()*p3.px()-p3.py()*p3.py()-p3.pz()*p3.pz();
    double p4sqr = p4.e()*p4.e()-p4.px()*p4.px()-p4.py()*p4.py()-p4.pz()*p4.pz();
    std::cout << "ERW: p3^2, p4^2 =" <<  p3sqr  << "  " <<  p4sqr << std::endl;
  }

}


//----------------------------------------------------
// Define  histograms for analysis
//----------------------------------------------------
//
void TauSpinnerRedecay::CreateHistos( int idhist)  {

  
  HistoManager *hmg  = HistoManager::getInstance();

  Text_t namehist[500];


}

