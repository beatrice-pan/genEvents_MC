/*****************************************************************************
   Name    : AnalTauSpinnerCP.cxx
   Package : 
   Author  : Elzbieta Richter-Was
   Created : June 2015
   
   DESCRIPTION:
   
   This class is test for TauSpinner CP weights
   COPIED from example by T. Przedzinski

*****************************************************************************/
// Include files

#define AnalTauSpinnerCP_cxx

#include "AnalTauSpinnerCP.h"
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
#include <TVector3.h>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TMath.h>
#include <TObject.h>
#include <TRandom.h>

using namespace std;
using namespace Tauolapp;
using namespace TauSpinner;

// We take sigborn and PDF distributions from TauSpinner for this test
namespace TauSpinner{
  extern double sigborn(int ID, double SS, double costhe);
  extern double f(double x, int ID, double SS, double cmsene);
}


pair<double,double> get_costheta_and_mass(Particle &q, Particle &anti_q, Particle &taum, Particle &taup);
pair<double,double*> calculate_costhe_and_WID(double S, Particle &taum, Particle &taup);


AnalTauSpinnerCP* AnalTauSpinnerCP::gAnalTauSpinnerCP = 0;

AnalTauSpinnerCP* AnalTauSpinnerCP::getInstance()
{
  if (!gAnalTauSpinnerCP) gAnalTauSpinnerCP=new AnalTauSpinnerCP();
  return gAnalTauSpinnerCP;
}

const bool  BENCHMARK  = 0;
const bool  DEBUG      = 0;
const bool  outNNlab   = 0;
const bool  outNN      = 0;
const bool  a1a1Frame_zRotate = 0;
const bool  a1a1Frame = 0;

const int   NuesOption  = 0;

const float kPI = 3.14159265359;

//CONSTRUCTOR
//----------------------------------------------
AnalTauSpinnerCP::AnalTauSpinnerCP()
//----------------------------------------------
{

} 

//DESTRUCTOR
//----------------------------------------------
AnalTauSpinnerCP::~AnalTauSpinnerCP()
//----------------------------------------------
{

}

//Initialize
//----------------------------------------------
void AnalTauSpinnerCP::Initialize(char *filename, int idhist)
//----------------------------------------------
{

  //  Create histograms for this analysis
  m_idHist= idhist;
  CreateHistos(m_idHist+1000000);
  
}

//Finalize
//----------------------------------------------
void AnalTauSpinnerCP::Finalize()
//----------------------------------------------
{


}
//----------------------------------------------
void AnalTauSpinnerCP::Make(HepMC::GenEvent *hepmc, int analMode)
//----------------------------------------------
{

  HistoManager  *hmg   = HistoManager::getInstance();

  BuildTauSpinnerBranch  *evtmg = BuildTauSpinnerBranch::getInstance();

  float evtWeight = 1.0;

  //monitor cutflow
  int idhist = m_idHist + 1000000; 
  float accFlag = 0;
  hmg->GetHistoTH1F(idhist)->Fill( accFlag, evtWeight );
 

  //
  // Analyze the event
  //

  // prepare branch for TauSpinner weight
  SimpleParticle p1, p2, X, tau, tau2; // SimpleParticle consist of 4 momentum and pdgId.
  vector<SimpleParticle> tau_daughters, tau2_daughters;

    float WT = 1.0;

    // hepmc->print();
    // search for tau decay in the event
    int status =  evtmg->buildSimpleBranch(hepmc, X, tau, tau2, tau_daughters, tau2_daughters);


    // shake events
    X.setPx( X.px()+1.e-6);
    X.setPy( X.py()-1.e-6);
    tau.setPx( tau.px()+1.e-6);
    tau.setPy( tau.py()-1.e-6);
    tau2.setPx( tau2.px()+1.e-6);
    tau2.setPy( tau2.py()-1.e-6);

    if(status == 2) {
      

      if( X.pdgid()==25 || X.pdgid()==36 || X.pdgid()==22 || X.pdgid()==23|| X.pdgid()==0 ){

	// NOT WEIGHTED

	hmg->GetHistoTH1F(idhist+ 4000+1)->Fill( 1.0, evtWeight );      

	// Now calculate weight for required model
	// NOTE: if any component of transverse density matrix for DY has been turned on
	//       using setZgamMultipliersTR, weight WT will contain transverse part
	//       of the spin amplitude (calculated from tables table1-1.txt table2-2.txt
	//       which must be stored in the same directory as the program)


        WT = 1.0;

	// Enforce (for a moment) it is Higgs boson  
	if( analMode == 2)
	  X.setPdgid(25);

        if( analMode != 0) 
	  WT = TauSpinner::calculateWeightFromParticlesH(X, tau, tau2, tau_daughters, tau2_daughters);

	hmg->GetHistoTH1F(idhist+ 1000+1)->Fill( WT, evtWeight );      

	// average polarization
        double polBorn = getTauSpin();
        hmg->GetHistoTH1F(idhist+ 1000+2)->Fill(  polBorn, WT * evtWeight );

        double Xmass = sqrt(X.e()*X.e()-X.px()*X.px()-X.py()*X.py()-X.pz()*X.pz());
	hmg->GetHistoTH1F(idhist+ 1000+13)->Fill( Xmass, evtWeight );

  	// average weight as function of mass
	hmg->GetHistoTH1F(idhist+ 1000+14)->Fill( Xmass, WT * evtWeight );


	testRhoRhoZ(idhist+  1000, hepmc, WT * evtWeight);
	testPiPiZ(idhist+  7000, hepmc, WT * evtWeight);
	//	testRhoRho(idhist+  1000, hepmc, WT * evtWeight);
	//	testA1A1  (idhist+  2000, hepmc, WT * evtWeight);
	//      testA1Rho (idhist+  3000, hepmc, WT * evtWeight);
	//	NeusA1Rho (idhist+  3000, hepmc, WT * evtWeight);
	//	test_3p0n_1p1n (idhist+  3000, hepmc, WT * evtWeight);
	//      test_1p2n_1p1n (idhist+  4000, hepmc, WT * evtWeight);
	//      test_1p2n_v2_1p1n (idhist+  5000, hepmc, WT * evtWeight);
	//      test_3p0n_v2_1p1n (idhist+  6000, hepmc, WT * evtWeight);
      }    else    {
	
	cout<<"WARNING: Unexpected PDG for tau mother: "<<X.pdgid()<<endl;
	
      }
	  
    }
       
}


// with lifetime calculation
//----------------------------------------------
void AnalTauSpinnerCP::testPiPiZ( int idhist, HepMC::GenEvent *hepmc, float evtWeight )
//----------------------------------------------
{

  HistoManager  *hmg = HistoManager::getInstance(); 
  BuildTauSpinnerBranch  *evtmg = BuildTauSpinnerBranch::getInstance();


  // prepare branch for TauSpinner weight
  SimpleParticle sp_X, sp_tau, sp_tau2; // SimpleParticle consist of 4 momentum and pdgId.
  vector<SimpleParticle> sp_tau_daughters, sp_tau2_daughters;

   // search for tau decay in the event
    int status =  evtmg->buildSimpleBranch(hepmc, sp_X, sp_tau, sp_tau2, sp_tau_daughters, sp_tau2_daughters);

    if(status == 2) {

      float accFlag = 0;
      hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

      vector<Particle> tau_pi,  tau2_pi ;
      vector<Particle> tau_rho, tau2_rho ;
      vector< vector<Particle> > tau_rho_daughters, tau2_rho_daughters ;

      double etmiss_px = 0;
      double etmiss_py = 0;
 

       // Create list of tau daughters and reconstruct rho resonances
       for(unsigned int i=0; i<sp_tau_daughters.size(); i++) {
	 if(abs(sp_tau_daughters[i].pdgid()) == 16){
	   etmiss_px += sp_tau_daughters[i].px();
	   etmiss_py += sp_tau_daughters[i].py();
	 }
	 if( abs(sp_tau_daughters[i].pdgid()) == 211 ){
	   Particle pp(sp_tau_daughters[i].px(),
		       sp_tau_daughters[i].py(),
		       sp_tau_daughters[i].pz(),
		       sp_tau_daughters[i].e(),
		       sp_tau_daughters[i].pdgid() );
	   tau_pi.push_back(pp);
	   for(unsigned int ii=0; ii<sp_tau_daughters.size(); ii++) {
	     if( abs(sp_tau_daughters[ii].pdgid()) == 111 ){
	       vector<Particle> temp;
	       Particle p1(sp_tau_daughters[i].px() ,
			   sp_tau_daughters[i].py(),
			   sp_tau_daughters[i].pz(),
			   sp_tau_daughters[i].e(),
			   sp_tau_daughters[i].pdgid());
	       temp.push_back(p1);
	       Particle p2(sp_tau_daughters[ii].px() ,
			   sp_tau_daughters[ii].py(),
			   sp_tau_daughters[ii].pz(),
			   sp_tau_daughters[ii].e(),
			   sp_tau_daughters[ii].pdgid());
	       temp.push_back(p2);
	       tau_pi.push_back(p2);
	       Particle pp(sp_tau_daughters[i].px()+sp_tau_daughters[ii].px() ,
			   sp_tau_daughters[i].py()+sp_tau_daughters[ii].py(),
			   sp_tau_daughters[i].pz()+sp_tau_daughters[ii].pz(),
			   sp_tau_daughters[i].e() +sp_tau_daughters[ii].e(),
			   0 );
	       tau_rho.push_back(pp);
	       tau_rho_daughters.push_back(temp);
	     }
	   }
	 }
       }
       if(DEBUG){
	 std::cout << "tau_rho.size = " << tau_rho.size() << std::endl;
	 if( tau_rho.size() > 0) {
	   std::cout << "tau_rho_daughters[0].size =" << tau_rho_daughters[0].size() << std::endl;
	   tau_rho_daughters[0][0].print();
	   tau_rho_daughters[0][1].print();
	 }
       }

   
       // Create list of tau2 daughters and reconstruct rho resonances
       for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) {
	 if(abs(sp_tau2_daughters[i].pdgid()) == 16){
	   etmiss_px += sp_tau2_daughters[i].px();
	   etmiss_py += sp_tau2_daughters[i].py();
	 }
	 if( abs(sp_tau2_daughters[i].pdgid()) == 211 ){
	   Particle pp(sp_tau2_daughters[i].px(),
		       sp_tau2_daughters[i].py(),
		       sp_tau2_daughters[i].pz(),
		       sp_tau2_daughters[i].e(),
		       sp_tau2_daughters[i].pdgid() );
	   tau2_pi.push_back(pp);
	   for(unsigned int ii=0; ii<sp_tau2_daughters.size(); ii++) {
	     if( abs(sp_tau2_daughters[ii].pdgid()) == 111 ){
	       vector<Particle> temp;
	       Particle p1(sp_tau2_daughters[i].px() ,
			   sp_tau2_daughters[i].py(),
			   sp_tau2_daughters[i].pz(),
			   sp_tau2_daughters[i].e(),
			   sp_tau2_daughters[i].pdgid());
	       temp.push_back(p1);
	       Particle p2(sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[ii].e(),
			   sp_tau2_daughters[ii].pdgid());
	       temp.push_back(p2);
	       tau2_pi.push_back(p2);
	       Particle pp(sp_tau2_daughters[i].px()+sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[i].py()+sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[i].pz()+sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[i].e() +sp_tau2_daughters[ii].e(),
			   0 );
	       tau2_rho.push_back(pp);
	       tau2_rho_daughters.push_back(temp);
	     }
	   }
	 }
       }
       if(DEBUG){
	 std::cout << "tau2_rho.size = " << tau2_rho.size() << std::endl;
	 if( tau2_rho.size() > 0) {
	   std::cout << "tau2_rho_daughters[0].size =" << tau2_rho_daughters[0].size() << std::endl;
	   tau2_rho_daughters[0][0].print();
	   tau2_rho_daughters[0][1].print();
	 }
       }

       if( tau_rho.size() == 0 &&  tau2_rho.size() == 0 ){
	 	 
	 float accFlag = 1;
	 hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );
	 
	 // here work on approximating tau momenta frm the measurement of difference
	 // between primary and secondary vertex positions
	 
	 
	 Particle tau     ( sp_tau.px() ,
			    sp_tau.py(),
			    sp_tau.pz(),
			    sp_tau.e(),
			    0);
	 
	 Particle tau2     ( sp_tau2.px(),
			     sp_tau2.py(),
			     sp_tau2.pz(),
			     sp_tau2.e(),
			     0);
	 
	// Build the tau-tau system resonances
	 Particle tautau( sp_tau.px()+sp_tau2.px(), 
			  sp_tau.py()+sp_tau2.py(),
			  sp_tau.pz()+sp_tau2.pz(),
			  sp_tau.e() +sp_tau2.e(), 0);	 
	 	 


	// calculate angles between (tau, tau_pi) and (tau2, tau2_pi) planes
	    
	    // acoplanarity between rho planes
	    tau.boostToRestFrame(tautau);
	    tau_pi[0].boostToRestFrame(tautau);
	    tau2.boostToRestFrame(tautau);
	    tau2_pi[0].boostToRestFrame(tautau);

	    if( BENCHMARK ){
	      std::cout << "-- after boost to tautau frame  " << std::endl;
	      std::cout<< "--- tau_pi  --" << std::endl;
	      tau_pi[0].print();
	      std::cout<< "--- tau2_pi  --" << std::endl;
	      tau2_pi[0].print();
	    } 
	    
	    //normal to the plane spanned by tau, tau_pi  
	    double n_1[3];
	    P_norm_cross_product(tau,tau_pi[0],n_1);
	    //normal to the plane spanned by tau2, tau2_pi  
	    double n_2[3];
	    P_norm_cross_product(tau2,tau2_pi[0],n_2);	
	    // calculate the acoplanarity angle
	    double phi_star = acos(dot_product(n_1,n_2));
	    // calculate the acoplanarity angle in range (0, 2pi)
	    double n_3[3];
	    P_norm_cross_product(tau_pi[0],tau2_pi[0],n_3);
	    double v_tau[3];
            v_tau[0] = tau.px(); v_tau[1] = tau.py(); v_tau[2] = tau.pz();
	    double sign = dot_product(n_3,v_tau);
	    double phi_star_sign = phi_star;
	    if( sign > 0)
	      phi_star_sign = 2* kPI - phi_star; 

	    // alpha angles in the lab frame
	    TVector3  tlv3_Ez(0.0, 0.0, 1.0 );
	    
	    TVector3  tlv3_tau_pi(tau_pi[0].px(), tau_pi[0].py(), tau_pi[0].pz() );
	    TVector3  tlv3_tau(tau.px(), tau.py(), tau.pz() );
	    TVector3 cross1_pi = (tlv3_Ez.Cross(tlv3_tau)).Unit();
	    TVector3 cross2_pi = (tlv3_tau_pi.Cross(tlv3_tau)).Unit();
	    double alpha_pi = TMath::ACos(TMath::Abs(cross1_pi * cross2_pi));


	    //std::cout << "phi_star = " << phi_star << std::endl;

	    int isFilter =1;
	    //	    hmg->GetHistoTH1F(idhist+121)->Fill( phi_star, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+121)->Fill( phi_star_sign, isFilter * evtWeight );	  
	    if(  alpha_pi > kPI/4.){
                   hmg->GetHistoTH1F(idhist+125)->Fill( phi_star_sign, isFilter * evtWeight );
	    } else {
                   hmg->GetHistoTH1F(idhist+126)->Fill( phi_star_sign, isFilter * evtWeight );
	    }

       }
    }
}

// with lifetime calculation
//----------------------------------------------
void AnalTauSpinnerCP::testRhoRhoZ( int idhist, HepMC::GenEvent *hepmc, float evtWeight )
//----------------------------------------------
{

  HistoManager  *hmg = HistoManager::getInstance(); 
  BuildTauSpinnerBranch  *evtmg = BuildTauSpinnerBranch::getInstance();


  // prepare branch for TauSpinner weight
  SimpleParticle sp_X, sp_tau, sp_tau2; // SimpleParticle consist of 4 momentum and pdgId.
  vector<SimpleParticle> sp_tau_daughters, sp_tau2_daughters;

   // search for tau decay in the event
    int status =  evtmg->buildSimpleBranch(hepmc, sp_X, sp_tau, sp_tau2, sp_tau_daughters, sp_tau2_daughters);

    if(status == 2) {

      float accFlag = 0;
      hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

      vector<Particle> tau_pi,  tau2_pi ;
      vector<Particle> tau_rho, tau2_rho ;
      vector< vector<Particle> > tau_rho_daughters, tau2_rho_daughters ;

      double etmiss_px = 0;
      double etmiss_py = 0;
 

       // Create list of tau daughters and reconstruct rho resonances
       for(unsigned int i=0; i<sp_tau_daughters.size(); i++) {
	 if(abs(sp_tau_daughters[i].pdgid()) == 16){
	   etmiss_px += sp_tau_daughters[i].px();
	   etmiss_py += sp_tau_daughters[i].py();
	 }
	 if( abs(sp_tau_daughters[i].pdgid()) == 211 ){
	   Particle pp(sp_tau_daughters[i].px(),
		       sp_tau_daughters[i].py(),
		       sp_tau_daughters[i].pz(),
		       sp_tau_daughters[i].e(),
		       sp_tau_daughters[i].pdgid() );
	   tau_pi.push_back(pp);
	   for(unsigned int ii=0; ii<sp_tau_daughters.size(); ii++) {
	     if( abs(sp_tau_daughters[ii].pdgid()) == 111 ){
	       vector<Particle> temp;
	       Particle p1(sp_tau_daughters[i].px() ,
			   sp_tau_daughters[i].py(),
			   sp_tau_daughters[i].pz(),
			   sp_tau_daughters[i].e(),
			   sp_tau_daughters[i].pdgid());
	       temp.push_back(p1);
	       Particle p2(sp_tau_daughters[ii].px() ,
			   sp_tau_daughters[ii].py(),
			   sp_tau_daughters[ii].pz(),
			   sp_tau_daughters[ii].e(),
			   sp_tau_daughters[ii].pdgid());
	       temp.push_back(p2);
	       tau_pi.push_back(p2);
	       Particle pp(sp_tau_daughters[i].px()+sp_tau_daughters[ii].px() ,
			   sp_tau_daughters[i].py()+sp_tau_daughters[ii].py(),
			   sp_tau_daughters[i].pz()+sp_tau_daughters[ii].pz(),
			   sp_tau_daughters[i].e() +sp_tau_daughters[ii].e(),
			   0 );
	       tau_rho.push_back(pp);
	       tau_rho_daughters.push_back(temp);
	     }
	   }
	 }
       }
       if(DEBUG){
	 std::cout << "tau_rho.size = " << tau_rho.size() << std::endl;
	 if( tau_rho.size() > 0) {
	   std::cout << "tau_rho_daughters[0].size =" << tau_rho_daughters[0].size() << std::endl;
	   tau_rho_daughters[0][0].print();
	   tau_rho_daughters[0][1].print();
	 }
       }

   
       // Create list of tau2 daughters and reconstruct rho resonances
       for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) {
	 if(abs(sp_tau2_daughters[i].pdgid()) == 16){
	   etmiss_px += sp_tau2_daughters[i].px();
	   etmiss_py += sp_tau2_daughters[i].py();
	 }
	 if( abs(sp_tau2_daughters[i].pdgid()) == 211 ){
	   Particle pp(sp_tau2_daughters[i].px(),
		       sp_tau2_daughters[i].py(),
		       sp_tau2_daughters[i].pz(),
		       sp_tau2_daughters[i].e(),
		       sp_tau2_daughters[i].pdgid() );
	   tau2_pi.push_back(pp);
	   for(unsigned int ii=0; ii<sp_tau2_daughters.size(); ii++) {
	     if( abs(sp_tau2_daughters[ii].pdgid()) == 111 ){
	       vector<Particle> temp;
	       Particle p1(sp_tau2_daughters[i].px() ,
			   sp_tau2_daughters[i].py(),
			   sp_tau2_daughters[i].pz(),
			   sp_tau2_daughters[i].e(),
			   sp_tau2_daughters[i].pdgid());
	       temp.push_back(p1);
	       Particle p2(sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[ii].e(),
			   sp_tau2_daughters[ii].pdgid());
	       temp.push_back(p2);
	       tau2_pi.push_back(p2);
	       Particle pp(sp_tau2_daughters[i].px()+sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[i].py()+sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[i].pz()+sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[i].e() +sp_tau2_daughters[ii].e(),
			   0 );
	       tau2_rho.push_back(pp);
	       tau2_rho_daughters.push_back(temp);
	     }
	   }
	 }
       }
       if(DEBUG){
	 std::cout << "tau2_rho.size = " << tau2_rho.size() << std::endl;
	 if( tau2_rho.size() > 0) {
	   std::cout << "tau2_rho_daughters[0].size =" << tau2_rho_daughters[0].size() << std::endl;
	   tau2_rho_daughters[0][0].print();
	   tau2_rho_daughters[0][1].print();
	 }
       }

       if( tau_rho.size() == 1 &&  tau2_rho.size() == 1 ){
	 	 
	 float accFlag = 1;
	 hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

	 // here work on approximating tau momenta frm the measurement of difference
	 // between primary and secondary vertex positions
	 
	 SimpleParticle sp_tau_reco, sp_tau2_reco;
	 double tau_mass = 1.77;
	 
	 double tau_lifetime = Tauola::tau_lifetime;
	 double lifetime = tau_lifetime * (-log( Tauola::randomDouble() ));

	 double ene = sqrt( tau_mass*tau_mass
			  + sp_tau.px()*lifetime/tau_lifetime * sp_tau.px()*lifetime/tau_lifetime 
			  + sp_tau.py()*lifetime/tau_lifetime * sp_tau.py()*lifetime/tau_lifetime
			  + sp_tau.pz()*lifetime/tau_lifetime * sp_tau.pz()*lifetime/tau_lifetime);
	 // assuming that position of primary and secondary vertex is known and taking average tau lifetime
	 // we can recalculate tau momenta

	 sp_tau_reco.setPx(sp_tau.px()*lifetime/tau_lifetime);
	 sp_tau_reco.setPy(sp_tau.py()*lifetime/tau_lifetime);
	 sp_tau_reco.setPz(sp_tau.pz()*lifetime/tau_lifetime);
	 sp_tau_reco.setE(ene);

	Particle tau     ( sp_tau.px() ,
			   sp_tau.py(),
			   sp_tau.pz(),
			   sp_tau.e(),
			   0);
	Particle tau_reco( sp_tau_reco.px() ,
			   sp_tau_reco.py(),
			   sp_tau_reco.pz(),
			   sp_tau_reco.e(),
			   0);

	 lifetime = tau_lifetime * (-log( Tauola::randomDouble() ));
	 ene = sqrt( tau_mass*tau_mass
			  + sp_tau2.px()*lifetime/tau_lifetime * sp_tau2.px()*lifetime/tau_lifetime 
			  + sp_tau2.py()*lifetime/tau_lifetime * sp_tau2.py()*lifetime/tau_lifetime
			  + sp_tau2.pz()*lifetime/tau_lifetime * sp_tau2.pz()*lifetime/tau_lifetime);

	 sp_tau2_reco.setPx(sp_tau2.px()*lifetime/tau_lifetime);
	 sp_tau2_reco.setPy(sp_tau2.py()*lifetime/tau_lifetime);
	 sp_tau2_reco.setPz(sp_tau2.pz()*lifetime/tau_lifetime);
	 sp_tau2_reco.setE(ene);

	Particle tau2     ( sp_tau2.px() ,
			    sp_tau2.py(),
			    sp_tau2.pz(),
			    sp_tau2.e(),
			    0);

	Particle tau2_reco( sp_tau2_reco.px() ,
			    sp_tau2_reco.py(),
			    sp_tau2_reco.pz(),
			    sp_tau2_reco.e(),
			    0);

	 // how good is this approximation
	if( DEBUG ){
	 printf(" sp_tau       = %lf  %lf  %lf  %lf \n", 
	   	  sp_tau.px(), sp_tau.py(), sp_tau.pz(), sp_tau.e());
	 printf(" sp_tau_reco  = %lf  %lf  %lf  %lf \n", 
	   	  sp_tau_reco.px(), sp_tau_reco.py(), sp_tau_reco.pz(), sp_tau_reco.e());
	 printf(" sp_tau2      = %lf  %lf  %lf  %lf \n", 
	   	  sp_tau2.px(), sp_tau2.py(), sp_tau2.pz(), sp_tau2.e());
	 printf(" sp_tau2_reco = %lf  %lf  %lf  %lf \n", 
	   	  sp_tau2_reco.px(), sp_tau2_reco.py(), sp_tau2_reco.pz(), sp_tau2_reco.e());
	}
	 

	 
	 if( outNNlab ){
	   printf("TUPLE  %lf  \n", evtWeight);
	   for(unsigned int i=0; i<sp_tau_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau_daughters[i].px(), sp_tau_daughters[i].py(), sp_tau_daughters[i].pz(), sp_tau_daughters[i].e(), sp_tau_daughters[i].pdgid());
	   for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau2_daughters[i].px(), sp_tau2_daughters[i].py(), sp_tau2_daughters[i].pz(), sp_tau2_daughters[i].e(), sp_tau2_daughters[i].pdgid());
	   //	  return;
	 }
	 
	 // Build the rho-rho system resonances
	 Particle rhorho( tau_rho[0].px()+tau2_rho[0].px(), 
			  tau_rho[0].py()+tau2_rho[0].py(),
			  tau_rho[0].pz()+tau2_rho[0].pz(),
			  tau_rho[0].e() +tau2_rho[0].e(), 0.0);	 
	 
	 if( BENCHMARK ){
	   std::cout<< "--- tau_rho --" << std::endl;
	   tau_rho[0].print();
	   std::cout<< "--- tau2_rho --" << std::endl;
	   tau2_rho[0].print();
	   std::cout<< "--- tau_rho tau2_rho system--" << std::endl;
	   rhorho.print();
	 }
	 
	int isFilter =1;
	double tau_rho_pTlab = sqrt( tau_rho[0].px()*tau_rho[0].px()+ tau_rho[0].py()*tau_rho[0].py() );
	double tau2_rho_pTlab = sqrt( tau2_rho[0].px()*tau2_rho[0].px()+ tau2_rho[0].py()*tau2_rho[0].py() );
	if(  tau_rho_pTlab < 20.0 ||  tau2_rho_pTlab < 20.0 ) isFilter =0;
	for(unsigned int i=0; i<tau_rho_daughters.size(); i++) {
	  double pT = sqrt( tau_pi[i].px()* tau_pi[i].px()+ tau_pi[i].py()* tau_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        
	for(unsigned int i=0; i<tau2_rho_daughters.size(); i++) {
	  double pT = sqrt( tau2_pi[i].px()* tau2_pi[i].px()+ tau2_pi[i].py()* tau2_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        

	if( BENCHMARK )
	  std::cout<< "--- filter = " <<  isFilter << std::endl;

	// calculate angles between normal to rho planes
	double  costheta[1], theta[1];
	int index = -1;

	for(int i = 0; i< tau_rho.size() ; i++){
	  for(int ii = 0; ii< tau2_rho.size() ; ii++){
	    
	    if( BENCHMARK ){
	      std::cout << " " << std::endl;
	      std::cout << " --- NEXT rho-rho pair --" << std::endl;
	      std::cout << " " << std::endl;
	    }
	    
	    
	    Particle rho_1pi(
			     tau_rho_daughters[i][0].px(),
			     tau_rho_daughters[i][0].py(),
			     tau_rho_daughters[i][0].pz(),
			     tau_rho_daughters[i][0].e(),0);
	    Particle rho_2pi(
			     tau_rho_daughters[i][1].px(),
			     tau_rho_daughters[i][1].py(),
			     tau_rho_daughters[i][1].pz(),
			     tau_rho_daughters[i][1].e(),0);
	    Particle rho(
			 tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			 tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			 tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			 tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
	    Particle rho2_1pi(
			      tau2_rho_daughters[ii][0].px(),
			      tau2_rho_daughters[ii][0].py(),
			      tau2_rho_daughters[ii][0].pz(),
			      tau2_rho_daughters[ii][0].e(),0);
	    Particle rho2_2pi(
			      tau2_rho_daughters[ii][1].px(),
			      tau2_rho_daughters[ii][1].py(),
			      tau2_rho_daughters[ii][1].pz(),
			      tau2_rho_daughters[ii][1].e(),0);
 	    Particle rho2(
			  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    
	    if( BENCHMARK ){
	      std::cout << "-- still in the LAB frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }

	    // alpha angles in the lab frame
	    TVector3  tlv3_Ez(0.0, 0.0, 1.0 );
	    
	    TVector3  tlv3_rho_1pi(rho_1pi.px(), rho_1pi.py(), rho_1pi.pz() );
	    TVector3  tlv3_rho(rho.px(), rho.py(), rho.pz() );
	    TVector3 cross1_rho = (tlv3_Ez.Cross(tlv3_rho)).Unit();
	    TVector3 cross2_rho = (tlv3_rho_1pi.Cross(tlv3_rho)).Unit();
	    double alpha_rho = TMath::ACos(TMath::Abs(cross1_rho * cross2_rho));

            // alpha angles in the lab frame
            Particle e_z(0.0,0.0,1.0,1.0,0);
	    double alpha_rho_1 = P_acoplanarAngle(e_z,rho,rho,rho_1pi);	    

	    // trying out Mustraal frame
	    
            double jetE = sqrt(sp_X.px()*sp_X.px() + sp_X.py()*sp_X.py() + sp_X.pz()*sp_X.pz());
            Particle jet(-sp_X.px(), -sp_X.py(), -sp_X.pz(), jetE, 0);
            double costheta1, costheta2, phi1, phi2, wt1, wt2;
            calcCosThetaPhiMustraal(rho, rho2, jet, costheta1, costheta2, phi1, phi2, wt1, wt2);
	    double rnd = Tauola::randomDouble();
            double sintheta1 = sqrt( 1 - costheta1*costheta1);
            double sintheta2 = sqrt( 1 - costheta2*costheta2);
	    Particle beam;
	    if( wt1/(wt1+wt2) < rnd ){
	      beam.setPx( sintheta1 * cos(phi1) );
              beam.setPy( sintheta1 * sin(phi1) );
              beam.setPz( costheta1 );
	    } else {
	      beam.setPx( sintheta2 * cos(phi2) );
              beam.setPy( sintheta2 * sin(phi2) );
              beam.setPz( costheta2 );
	    }
 	    double alpha_rho_2 = P_acoplanarAngle(e_z,beam,rho,rho_1pi);	    
	   

	    // acoplanarity between rho planes in the rho-rho plane
	    
	    rho_1pi.boostToRestFrame(rhorho);
	    rho_2pi.boostToRestFrame(rhorho);
	    rho2_1pi.boostToRestFrame(rhorho);
	    rho2_2pi.boostToRestFrame(rhorho);

           // alpha angles in the lab rhorho frame
           // e_z.boostToRestFrame(rhorho);
           // rho.boostToRestFrame(rhorho);
	   // double alpha_rho_2 = P_acoplanarAngle(e_z,rho,rho,rho_1pi);	    
  
	    // std::cout << " alpha_rho " << alpha_rho << " " << alpha_rho_1 << " " << alpha_rho_2 << std::endl;

	    hmg->GetHistoTH1F(idhist+191)->Fill(alpha_rho, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+192)->Fill(alpha_rho_1, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+193)->Fill(alpha_rho_2, isFilter * evtWeight );

	    double HHpHHm_0, HHpHHm_1;
	    getHHpHHm(HHpHHm_0, HHpHHm_1);
	    //std::cout << "from analysis HHpHHm " << HHpHHm_0 << " " << HHpHHm_1 << std::endl;

	    hmg->GetHistoTH1F(idhist+181)->Fill(alpha_rho, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+182)->Fill(alpha_rho, HHpHHm_0 * isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+183)->Fill(alpha_rho, HHpHHm_1 * isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+184)->Fill(alpha_rho_1, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+185)->Fill(alpha_rho_1, HHpHHm_0 * isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+186)->Fill(alpha_rho_1, HHpHHm_1 * isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+187)->Fill(alpha_rho_2, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+188)->Fill(alpha_rho_2, HHpHHm_0 * isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+189)->Fill(alpha_rho_2, HHpHHm_1 * isFilter * evtWeight );

	    
	    if( BENCHMARK ){
	      std::cout << "-- after boost to rho-rho frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    } 
	    
	    //normal to the plane spanned by rho-> pi+ pi-  
	    double n_1[3];
	    P_norm_cross_product(rho_1pi,rho_2pi,n_1);
	    //normal to the plane spanned by rho2-> pi+ pi0  
	    double n_2[3];
	    P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);	
	    // calculate the acoplanarity angle
	    index++;
	    costheta[index] = dot_product(n_1,n_2);
	    theta[index] = P_acoplanarAngle(rho_1pi, rho_2pi, rho2_1pi, rho2_2pi);

	    if( BENCHMARK ) {
	      std::cout<< "--- normal to the tau_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
	      std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
	      std::cout<< "--- cos( acoplanarity angle ) --" << std::endl;
	      std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
	      std::cout<< "--- acoplanarity angle --" << std::endl;
	      std::cout<< "Theta = " <<  theta[index] << std::endl;
	    }

	    double theta4Pi = theta[index];
	    double y1 = (rho_1pi.e()-rho_2pi.e())/(rho_1pi.e()+rho_2pi.e());
	    double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    
	    hmg->GetHistoTH1F(idhist+121)->Fill( theta[index], isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+122)->Fill( rho.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+123)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+124)->Fill( theta4Pi, isFilter * evtWeight );
	    // transverse spin correlations
	    double Rxx, Ryy, Rxy, Ryx;
	    getZgamParametersTR(Rxx, Ryy, Rxy, Ryx);

	    if(    (  alpha_rho_1 <    kPI/8.) 
                || (  alpha_rho_1 > 7* kPI/8. && alpha_rho_1 < 9* kPI/8.)
		|| (  alpha_rho_1 > 15* kPI/8.) ){
	      hmg->GetHistoTH1F(idhist+125)->Fill( theta[index], isFilter * evtWeight );
	      hmg->GetHistoTH1F(idhist+135)->Fill( theta4Pi, isFilter * evtWeight );
	    } else {
	      hmg->GetHistoTH1F(idhist+126)->Fill( theta[index], isFilter * evtWeight );
	      hmg->GetHistoTH1F(idhist+136)->Fill( theta4Pi, isFilter * evtWeight );
	    }

	    if(  alpha_rho > kPI/4.){
                   hmg->GetHistoTH1F(idhist+194)->Fill( theta4Pi, isFilter * evtWeight );
	    } else {
                   hmg->GetHistoTH1F(idhist+195)->Fill( theta4Pi, isFilter * evtWeight );
	    }

	    if(    (  alpha_rho_1 <    kPI/4.) 
                || (  alpha_rho_1 > 3* kPI/4. && alpha_rho_1 < 5* kPI/8.) 
                || (  alpha_rho_1 > 7* kPI/8.) ){
                   hmg->GetHistoTH1F(idhist+196)->Fill( theta4Pi, isFilter * evtWeight );
	    } else {
                   hmg->GetHistoTH1F(idhist+197)->Fill( theta4Pi, isFilter * evtWeight );
	    }

	    if(    (  alpha_rho_2 <    kPI/4.) 
                || (  alpha_rho_2 > 3* kPI/4. && alpha_rho_2 < 5* kPI/8.) 
                || (  alpha_rho_2 > 7* kPI/8.) ){
                   hmg->GetHistoTH1F(idhist+198)->Fill( theta4Pi, isFilter * evtWeight );
	    } else {
                   hmg->GetHistoTH1F(idhist+199)->Fill( theta4Pi, isFilter * evtWeight );
	    }

	  }

	  
	  //here verify original and reco tau
	  tau.boostToRestFrame(rhorho);
	  tau2.boostToRestFrame(rhorho);
	  tau_reco.boostToRestFrame(rhorho);
	  tau2_reco.boostToRestFrame(rhorho);

	    

	  if (DEBUG ){	  
	    std::cout<< "--- original taus in the rho-rho frame --" << std::endl;
	    tau.print();
	    tau2.print();
	    std::cout << "angle phi, theta=" << tau.getAnglePhi() << "  " << tau.getAngleTheta() << std::endl;
	    std::cout << "angle phi, theta=" << tau2.getAnglePhi() << "  " << tau2.getAngleTheta() << std::endl;
	    std::cout<< "--- reco taus in the rho-rho frame --" << std::endl;
	    tau_reco.print();
	    tau2_reco.print();
	    std::cout << "angle phi, theta=" << tau_reco.getAnglePhi() << "  " << tau_reco.getAngleTheta() << std::endl;
	    std::cout << "angle phi, theta=" << tau2_reco.getAnglePhi() << "  " << tau2_reco.getAngleTheta() << std::endl;
	  }
	  

	}
       }
    }
    
}

// with lifetime calculation
//----------------------------------------------
void AnalTauSpinnerCP::testRhoRho( int idhist, HepMC::GenEvent *hepmc, float evtWeight )
//----------------------------------------------
{

  HistoManager  *hmg = HistoManager::getInstance(); 
  BuildTauSpinnerBranch  *evtmg = BuildTauSpinnerBranch::getInstance();


  // prepare branch for TauSpinner weight
  SimpleParticle sp_X, sp_tau, sp_tau2; // SimpleParticle consist of 4 momentum and pdgId.
  vector<SimpleParticle> sp_tau_daughters, sp_tau2_daughters;

   // search for tau decay in the event
    int status =  evtmg->buildSimpleBranch(hepmc, sp_X, sp_tau, sp_tau2, sp_tau_daughters, sp_tau2_daughters);

    if(status == 2) {

      float accFlag = 0;
      hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

      vector<Particle> tau_pi,  tau2_pi ;
      vector<Particle> tau_rho, tau2_rho ;
      vector< vector<Particle> > tau_rho_daughters, tau2_rho_daughters ;

      double etmiss_px = 0;
      double etmiss_py = 0;
 

       // Create list of tau daughters and reconstruct rho resonances
       for(unsigned int i=0; i<sp_tau_daughters.size(); i++) {
	 if(abs(sp_tau_daughters[i].pdgid()) == 16){
	   etmiss_px += sp_tau_daughters[i].px();
	   etmiss_py += sp_tau_daughters[i].py();
	 }
	 if( abs(sp_tau_daughters[i].pdgid()) == 211 ){
	   Particle pp(sp_tau_daughters[i].px(),
		       sp_tau_daughters[i].py(),
		       sp_tau_daughters[i].pz(),
		       sp_tau_daughters[i].e(),
		       sp_tau_daughters[i].pdgid() );
	   tau_pi.push_back(pp);
	   for(unsigned int ii=0; ii<sp_tau_daughters.size(); ii++) {
	     if( abs(sp_tau_daughters[ii].pdgid()) == 111 ){
	       vector<Particle> temp;
	       Particle p1(sp_tau_daughters[i].px() ,
			   sp_tau_daughters[i].py(),
			   sp_tau_daughters[i].pz(),
			   sp_tau_daughters[i].e(),
			   sp_tau_daughters[i].pdgid());
	       temp.push_back(p1);
	       Particle p2(sp_tau_daughters[ii].px() ,
			   sp_tau_daughters[ii].py(),
			   sp_tau_daughters[ii].pz(),
			   sp_tau_daughters[ii].e(),
			   sp_tau_daughters[ii].pdgid());
	       temp.push_back(p2);
	       tau_pi.push_back(p2);
	       Particle pp(sp_tau_daughters[i].px()+sp_tau_daughters[ii].px() ,
			   sp_tau_daughters[i].py()+sp_tau_daughters[ii].py(),
			   sp_tau_daughters[i].pz()+sp_tau_daughters[ii].pz(),
			   sp_tau_daughters[i].e() +sp_tau_daughters[ii].e(),
			   0 );
	       tau_rho.push_back(pp);
	       tau_rho_daughters.push_back(temp);
	     }
	   }
	 }
       }
       if(DEBUG){
	 std::cout << "tau_rho.size = " << tau_rho.size() << std::endl;
	 if( tau_rho.size() > 0) {
	   std::cout << "tau_rho_daughters[0].size =" << tau_rho_daughters[0].size() << std::endl;
	   tau_rho_daughters[0][0].print();
	   tau_rho_daughters[0][1].print();
	 }
       }

   
       // Create list of tau2 daughters and reconstruct rho resonances
       for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) {
	 if(abs(sp_tau2_daughters[i].pdgid()) == 16){
	   etmiss_px += sp_tau2_daughters[i].px();
	   etmiss_py += sp_tau2_daughters[i].py();
	 }
	 if( abs(sp_tau2_daughters[i].pdgid()) == 211 ){
	   Particle pp(sp_tau2_daughters[i].px(),
		       sp_tau2_daughters[i].py(),
		       sp_tau2_daughters[i].pz(),
		       sp_tau2_daughters[i].e(),
		       sp_tau2_daughters[i].pdgid() );
	   tau2_pi.push_back(pp);
	   for(unsigned int ii=0; ii<sp_tau2_daughters.size(); ii++) {
	     if( abs(sp_tau2_daughters[ii].pdgid()) == 111 ){
	       vector<Particle> temp;
	       Particle p1(sp_tau2_daughters[i].px() ,
			   sp_tau2_daughters[i].py(),
			   sp_tau2_daughters[i].pz(),
			   sp_tau2_daughters[i].e(),
			   sp_tau2_daughters[i].pdgid());
	       temp.push_back(p1);
	       Particle p2(sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[ii].e(),
			   sp_tau2_daughters[ii].pdgid());
	       temp.push_back(p2);
	       tau2_pi.push_back(p2);
	       Particle pp(sp_tau2_daughters[i].px()+sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[i].py()+sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[i].pz()+sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[i].e() +sp_tau2_daughters[ii].e(),
			   0 );
	       tau2_rho.push_back(pp);
	       tau2_rho_daughters.push_back(temp);
	     }
	   }
	 }
       }
       if(DEBUG){
	 std::cout << "tau2_rho.size = " << tau2_rho.size() << std::endl;
	 if( tau2_rho.size() > 0) {
	   std::cout << "tau2_rho_daughters[0].size =" << tau2_rho_daughters[0].size() << std::endl;
	   tau2_rho_daughters[0][0].print();
	   tau2_rho_daughters[0][1].print();
	 }
       }

       if( tau_rho.size() == 1 &&  tau2_rho.size() == 1 ){
	 	 
	 float accFlag = 1;
	 hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

	 // here work on approximating tau momenta frm the measurement of difference
	 // between primary and secondary vertex positions
	 
	 SimpleParticle sp_tau_reco, sp_tau2_reco;
	 double tau_mass = 1.77;
	 
	 double tau_lifetime = Tauola::tau_lifetime;
	 double lifetime = tau_lifetime * (-log( Tauola::randomDouble() ));

	 double ene = sqrt( tau_mass*tau_mass
			  + sp_tau.px()*lifetime/tau_lifetime * sp_tau.px()*lifetime/tau_lifetime 
			  + sp_tau.py()*lifetime/tau_lifetime * sp_tau.py()*lifetime/tau_lifetime
			  + sp_tau.pz()*lifetime/tau_lifetime * sp_tau.pz()*lifetime/tau_lifetime);
	 // assuming that position of primary and secondary vertex is known and taking average tau lifetime
	 // we can recalculate tau momenta

	 sp_tau_reco.setPx(sp_tau.px()*lifetime/tau_lifetime);
	 sp_tau_reco.setPy(sp_tau.py()*lifetime/tau_lifetime);
	 sp_tau_reco.setPz(sp_tau.pz()*lifetime/tau_lifetime);
	 sp_tau_reco.setE(ene);

	Particle tau     ( sp_tau.px() ,
			   sp_tau.py(),
			   sp_tau.pz(),
			   sp_tau.e(),
			   0);
	Particle tau_reco( sp_tau_reco.px() ,
			   sp_tau_reco.py(),
			   sp_tau_reco.pz(),
			   sp_tau_reco.e(),
			   0);

	 lifetime = tau_lifetime * (-log( Tauola::randomDouble() ));
	 ene = sqrt( tau_mass*tau_mass
			  + sp_tau2.px()*lifetime/tau_lifetime * sp_tau2.px()*lifetime/tau_lifetime 
			  + sp_tau2.py()*lifetime/tau_lifetime * sp_tau2.py()*lifetime/tau_lifetime
			  + sp_tau2.pz()*lifetime/tau_lifetime * sp_tau2.pz()*lifetime/tau_lifetime);

	 sp_tau2_reco.setPx(sp_tau2.px()*lifetime/tau_lifetime);
	 sp_tau2_reco.setPy(sp_tau2.py()*lifetime/tau_lifetime);
	 sp_tau2_reco.setPz(sp_tau2.pz()*lifetime/tau_lifetime);
	 sp_tau2_reco.setE(ene);

	Particle tau2     ( sp_tau2.px() ,
			    sp_tau2.py(),
			    sp_tau2.pz(),
			    sp_tau2.e(),
			    0);

	Particle tau2_reco( sp_tau2_reco.px() ,
			    sp_tau2_reco.py(),
			    sp_tau2_reco.pz(),
			    sp_tau2_reco.e(),
			    0);

	 // how good is this approximation
	if( DEBUG ){
	 printf(" sp_tau       = %lf  %lf  %lf  %lf \n", 
	   	  sp_tau.px(), sp_tau.py(), sp_tau.pz(), sp_tau.e());
	 printf(" sp_tau_reco  = %lf  %lf  %lf  %lf \n", 
	   	  sp_tau_reco.px(), sp_tau_reco.py(), sp_tau_reco.pz(), sp_tau_reco.e());
	 printf(" sp_tau2      = %lf  %lf  %lf  %lf \n", 
	   	  sp_tau2.px(), sp_tau2.py(), sp_tau2.pz(), sp_tau2.e());
	 printf(" sp_tau2_reco = %lf  %lf  %lf  %lf \n", 
	   	  sp_tau2_reco.px(), sp_tau2_reco.py(), sp_tau2_reco.pz(), sp_tau2_reco.e());
	}
	 

	 
	 if( outNNlab ){
	   printf("TUPLE  %lf  \n", evtWeight);
	   for(unsigned int i=0; i<sp_tau_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau_daughters[i].px(), sp_tau_daughters[i].py(), sp_tau_daughters[i].pz(), sp_tau_daughters[i].e(), sp_tau_daughters[i].pdgid());
	   for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau2_daughters[i].px(), sp_tau2_daughters[i].py(), sp_tau2_daughters[i].pz(), sp_tau2_daughters[i].e(), sp_tau2_daughters[i].pdgid());
	   //	  return;
	 }
	 
	 // Build the rho-rho system resonances
	 Particle rhorho( tau_rho[0].px()+tau2_rho[0].px(), 
			  tau_rho[0].py()+tau2_rho[0].py(),
			  tau_rho[0].pz()+tau2_rho[0].pz(),
			  tau_rho[0].e() +tau2_rho[0].e(), 0.0);	 
	 
	 if( BENCHMARK ){
	   std::cout<< "--- tau_rho --" << std::endl;
	   tau_rho[0].print();
	   std::cout<< "--- tau2_rho --" << std::endl;
	   tau2_rho[0].print();
	   std::cout<< "--- tau_rho tau2_rho system--" << std::endl;
	   rhorho.print();
	 }
	 
	int isFilter =1;
	double tau_rho_pTlab = sqrt( tau_rho[0].px()*tau_rho[0].px()+ tau_rho[0].py()*tau_rho[0].py() );
	double tau2_rho_pTlab = sqrt( tau2_rho[0].px()*tau2_rho[0].px()+ tau2_rho[0].py()*tau2_rho[0].py() );
	if(  tau_rho_pTlab < 20.0 ||  tau2_rho_pTlab < 20.0 ) isFilter =0;
	for(unsigned int i=0; i<tau_rho_daughters.size(); i++) {
	  double pT = sqrt( tau_pi[i].px()* tau_pi[i].px()+ tau_pi[i].py()* tau_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        
	for(unsigned int i=0; i<tau2_rho_daughters.size(); i++) {
	  double pT = sqrt( tau2_pi[i].px()* tau2_pi[i].px()+ tau2_pi[i].py()* tau2_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        

	if( BENCHMARK )
	  std::cout<< "--- filter = " <<  isFilter << std::endl;

	// calculate angles between normal to rho planes
	double  costheta[1], theta[1];
	int index = -1;

	for(int i = 0; i< tau_rho.size() ; i++){
	  for(int ii = 0; ii< tau2_rho.size() ; ii++){
	    
	    if( BENCHMARK ){
	      std::cout << " " << std::endl;
	      std::cout << " --- NEXT rho-rho pair --" << std::endl;
	      std::cout << " " << std::endl;
	    }
	    
	    
	    Particle rho_1pi(
			     tau_rho_daughters[i][0].px(),
			     tau_rho_daughters[i][0].py(),
			     tau_rho_daughters[i][0].pz(),
			     tau_rho_daughters[i][0].e(),0);
	    Particle rho_2pi(
			     tau_rho_daughters[i][1].px(),
			     tau_rho_daughters[i][1].py(),
			     tau_rho_daughters[i][1].pz(),
			     tau_rho_daughters[i][1].e(),0);
	    Particle rho(
			 tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			 tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			 tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			 tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
	    Particle rho2_1pi(
			      tau2_rho_daughters[ii][0].px(),
			      tau2_rho_daughters[ii][0].py(),
			      tau2_rho_daughters[ii][0].pz(),
			      tau2_rho_daughters[ii][0].e(),0);
	    Particle rho2_2pi(
			      tau2_rho_daughters[ii][1].px(),
			      tau2_rho_daughters[ii][1].py(),
			      tau2_rho_daughters[ii][1].pz(),
			      tau2_rho_daughters[ii][1].e(),0);
	    Particle rho2(
			  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    
	    if( BENCHMARK ){
	      std::cout << "-- still in the LAB frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }

	    /*
	    // alpha angles in the lab frame
	    TVector3  tlv3_Ez(0.0, 0.0, 1.0 );
	    
	    TVector3  tlv3_rho_1pi(rho_1pi.px(), rho_1pi.py(), rho_1pi.pz() );
	    TVector3  tlv3_rho(rho.px(), rho.py(), rho.pz() );
	    TVector3 cross1_rho = (tlv3_Ez.Cross(tlv3_rho)).Unit();
	    TVector3 cross2_rho = (tlv3_rho_1pi.Cross(tlv3_rho)).Unit();
	    
	    double alpha_rho = TMath::ACos(TMath::Abs(cross1_rho * cross2_rho));
	    */

	    //normal to the plane spanned by tau1->rho-  
	    double n_1prod[3];
	    P_norm_cross_product(tau,rho,n_1prod);
	    //normal to the plane spanned by rho-> pi-  
	    double n_2prod[3];
	    P_norm_cross_product(rho,rho_1pi,n_2prod);	
	    // calculate the acoplanarity angle
	    double alpha_rho = P_acoplanarAngle(tau,rho,rho,rho_1pi);

	    
	    // acoplanarity between rho planes
	    rho_1pi.boostToRestFrame(rhorho);
	    rho_2pi.boostToRestFrame(rhorho);
	    rho2_1pi.boostToRestFrame(rhorho);
	    rho2_2pi.boostToRestFrame(rhorho);


	    if( BENCHMARK ){
	      std::cout << "-- after boost to rho-rho frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    } 
	    
	    //normal to the plane spanned by rho-> pi+ pi-  
	    double n_1[3];
	    P_norm_cross_product(rho_1pi,rho_2pi,n_1);
	    //normal to the plane spanned by rho2-> pi+ pi0  
	    double n_2[3];
	    P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);	
	    // calculate the acoplanarity angle
	    index++;
	    costheta[index] = dot_product(n_1,n_2);
	    theta[index] = P_acoplanarAngle(rho_1pi, rho_2pi, rho2_1pi, rho2_2pi);

	    if( BENCHMARK ) {
	      std::cout<< "--- normal to the tau_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
	      std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
	      std::cout<< "--- cos( acoplanarity angle ) --" << std::endl;
	      std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
	      std::cout<< "--- acoplanarity angle --" << std::endl;
	      std::cout<< "Theta = " <<  theta[index] << std::endl;
	    }

	    double theta4Pi = theta[index];
	    double y1 = (rho_1pi.e()-rho_2pi.e())/(rho_1pi.e()+rho_2pi.e());
	    double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    
	    hmg->GetHistoTH1F(idhist+121)->Fill( theta[index], isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+122)->Fill( rho.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+123)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+124)->Fill( theta4Pi, isFilter * evtWeight );

	    if( alpha_rho < kPI/4. || alpha_rho > 3./4.* kPI ){
                   hmg->GetHistoTH1F(idhist+125)->Fill( theta[index], isFilter * evtWeight );
	    } else {
                   hmg->GetHistoTH1F(idhist+126)->Fill( theta[index], isFilter * evtWeight );
	    }
	    hmg->GetHistoTH1F(idhist+127)->Fill(alpha_rho - kPI/2., isFilter * evtWeight );
	  }
	  
	  //here verify original and reco tau
	  tau.boostToRestFrame(rhorho);
	  tau2.boostToRestFrame(rhorho);
	  tau_reco.boostToRestFrame(rhorho);
	  tau2_reco.boostToRestFrame(rhorho);

	    

	  if (DEBUG ){	  
	    std::cout<< "--- original taus in the rho-rho frame --" << std::endl;
	    tau.print();
	    tau2.print();
	    std::cout << "angle phi, theta=" << tau.getAnglePhi() << "  " << tau.getAngleTheta() << std::endl;
	    std::cout << "angle phi, theta=" << tau2.getAnglePhi() << "  " << tau2.getAngleTheta() << std::endl;
	    std::cout<< "--- reco taus in the rho-rho frame --" << std::endl;
	    tau_reco.print();
	    tau2_reco.print();
	    std::cout << "angle phi, theta=" << tau_reco.getAnglePhi() << "  " << tau_reco.getAngleTheta() << std::endl;
	    std::cout << "angle phi, theta=" << tau2_reco.getAnglePhi() << "  " << tau2_reco.getAngleTheta() << std::endl;
	  }
	  

	}
       }
    }
    
}

//----------------------------------------------
void AnalTauSpinnerCP::test_3p0n_1p1n( int idhist, HepMC::GenEvent *hepmc, float evtWeight )
//----------------------------------------------
{

  HistoManager  *hmg = HistoManager::getInstance();
 
  BuildTauSpinnerBranch  *evtmg = BuildTauSpinnerBranch::getInstance();


  // prepare branch for TauSpinner weight
  SimpleParticle sp_X, sp_tau, sp_tau2; // SimpleParticle consist of 4 momentum and pdgId.
  vector<SimpleParticle> sp_tau_daughters, sp_tau2_daughters;

   // search for tau decay in the event
    int status =  evtmg->buildSimpleBranch(hepmc, sp_X, sp_tau, sp_tau2, sp_tau_daughters, sp_tau2_daughters);

    if(status == 2) {

      float accFlag = 0;
      hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

      hmg->GetHistoTH1F(idhist+1)->Fill( evtWeight, 1.0 );

      vector<Particle> tau_pi,  tau2_pi ;
      vector<Particle> tau_rho, tau2_rho ;
      vector<Particle> tau_other_pi;
      vector< vector<Particle> > tau_rho_daughters, tau2_rho_daughters ;

      double etmiss_px = 0;
      double etmiss_py = 0;
 
     // Create list of tau daughters and reconstruct a1 resonances
      for(unsigned int i=0; i<sp_tau_daughters.size(); i++) {
	if(abs(sp_tau_daughters[i].pdgid()) == 16){
	  etmiss_px += sp_tau_daughters[i].px();
	  etmiss_py += sp_tau_daughters[i].py();
	}
	if( abs(sp_tau_daughters[i].pdgid()) == 211 ){
	  Particle pp(sp_tau_daughters[i].px(),
		      sp_tau_daughters[i].py(),
		      sp_tau_daughters[i].pz(),
		      sp_tau_daughters[i].e(),
		      sp_tau_daughters[i].pdgid() );
	  tau_pi.push_back(pp);
	  for(unsigned int ii=0; ii<sp_tau_daughters.size(); ii++) {
	    if( abs(sp_tau_daughters[ii].pdgid()) == 211 ){
	      if( sp_tau_daughters[i].pdgid() * sp_tau_daughters[ii].pdgid()< 0 && ii > i ){
		vector<Particle> temp;
		Particle p1(sp_tau_daughters[i].px() ,
			    sp_tau_daughters[i].py(),
			    sp_tau_daughters[i].pz(),
			    sp_tau_daughters[i].e(),
			    sp_tau_daughters[i].pdgid());
		temp.push_back(p1);
		Particle p2(sp_tau_daughters[ii].px() ,
			    sp_tau_daughters[ii].py(),
			    sp_tau_daughters[ii].pz(),
			    sp_tau_daughters[ii].e(),
			    sp_tau_daughters[ii].pdgid());
		temp.push_back(p2);
		Particle pp(sp_tau_daughters[i].px()+sp_tau_daughters[ii].px() ,
			    sp_tau_daughters[i].py()+sp_tau_daughters[ii].py(),
			    sp_tau_daughters[i].pz()+sp_tau_daughters[ii].pz(),
			    sp_tau_daughters[i].e() +sp_tau_daughters[ii].e(),
			    0 );
		tau_rho.push_back(pp);
		tau_rho_daughters.push_back(temp);
	      } else if ( ii != i ){
		Particle p(sp_tau_daughters[ii].px() ,
			   sp_tau_daughters[ii].py(),
			   sp_tau_daughters[ii].pz(),
			   sp_tau_daughters[ii].e(),
			   sp_tau_daughters[ii].pdgid());
		tau_other_pi.push_back(p);
	      }
	    }
	  }
	}
      }
       if(DEBUG){
	std::cout << "tau_rho.size = " << tau_rho.size() << std::endl;
	if( tau_rho.size() > 0) {
	  std::cout << "tau_rho_daughters[0].size =" << tau_rho_daughters[0].size() << std::endl;
	  tau_rho_daughters[0][0].print();
	  tau_rho_daughters[0][1].print();
	}
	if( tau_rho.size() > 1) {
	  std::cout << "tau_rho_daughters[1].size =" << tau_rho_daughters[1].size() << std::endl;
	  tau_rho_daughters[1][0].print();
	  tau_rho_daughters[1][1].print();
	}
	for(unsigned int i=0; i<tau_other_pi.size(); i++) {
	  std::cout << "tau_other_pi.size =" << tau_other_pi.size() << std::endl;
	  tau_other_pi[i].print();
	}
       }

       // Create list of tau2 daughters and reconstruct rho resonances
       for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) {
	 if(abs(sp_tau2_daughters[i].pdgid()) == 16){
	   etmiss_px += sp_tau2_daughters[i].px();
	   etmiss_py += sp_tau2_daughters[i].py();
	 }
	 if( abs(sp_tau2_daughters[i].pdgid()) == 211 ){
	   Particle pp(sp_tau2_daughters[i].px(),
		       sp_tau2_daughters[i].py(),
		       sp_tau2_daughters[i].pz(),
		       sp_tau2_daughters[i].e(),
		       sp_tau2_daughters[i].pdgid() );
	   tau2_pi.push_back(pp);
	   for(unsigned int ii=0; ii<sp_tau2_daughters.size(); ii++) {
	     if( abs(sp_tau2_daughters[ii].pdgid()) == 111 ){
	       vector<Particle> temp;
	       Particle p1(sp_tau2_daughters[i].px() ,
			   sp_tau2_daughters[i].py(),
			   sp_tau2_daughters[i].pz(),
			   sp_tau2_daughters[i].e(),
			   sp_tau2_daughters[i].pdgid());
	       temp.push_back(p1);
	       Particle p2(sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[ii].e(),
			   sp_tau2_daughters[ii].pdgid());
	       temp.push_back(p2);
	       tau2_pi.push_back(p2);
	       Particle pp(sp_tau2_daughters[i].px()+sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[i].py()+sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[i].pz()+sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[i].e() +sp_tau2_daughters[ii].e(),
			   0 );
	       tau2_rho.push_back(pp);
	       tau2_rho_daughters.push_back(temp);
	     }
	   }
	 }
       }
       if(DEBUG){
	 std::cout << "tau2_rho.size = " << tau2_rho.size() << std::endl;
	 if( tau2_rho.size() > 0) {
	   std::cout << "tau2_rho_daughters[0].size =" << tau2_rho_daughters[0].size() << std::endl;
	   tau2_rho_daughters[0][0].print();
	   tau2_rho_daughters[0][1].print();
	 }
       }

       //       std::cout << "a1rho events: " << tau_pi.size() << " " <<  tau2_rho.size() << std::endl;

       if( tau_pi.size() == 3 &&  tau2_rho.size() == 1 ){
	 
	 
	 float accFlag = 1;
	 hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );
	 
	 if( outNNlab ){
	   printf("TUPLE  %lf  \n", evtWeight);
	   for(unsigned int i=0; i<sp_tau_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau_daughters[i].px(), sp_tau_daughters[i].py(), sp_tau_daughters[i].pz(), sp_tau_daughters[i].e(), sp_tau_daughters[i].pdgid());
	   for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau2_daughters[i].px(), sp_tau2_daughters[i].py(), sp_tau2_daughters[i].pz(), sp_tau2_daughters[i].e(), sp_tau2_daughters[i].pdgid());
	   //	  return;
	 }
	 
	 // Build the a1 resonances
	 Particle tau_a1 ( tau_pi[0].px()+tau_pi[1].px()+tau_pi[2].px(), 
			   tau_pi[0].py()+tau_pi[1].py()+tau_pi[2].py(),
			   tau_pi[0].pz()+tau_pi[1].pz()+tau_pi[2].pz(),
			   tau_pi[0].e() +tau_pi[1].e() +tau_pi[2].e(), 0.0);
	 
	 // Build the a1-rho system resonances
	 Particle a1rho ( tau_a1.px()+tau2_rho[0].px(), 
			  tau_a1.py()+tau2_rho[0].py(),
			  tau_a1.pz()+tau2_rho[0].pz(),
			  tau_a1.e() +tau2_rho[0].e(), 0.0);	 
	 if( BENCHMARK ){
	   std::cout<< "--- tau_a1 --" << std::endl;
	   tau_a1.print();
	   std::cout<< "--- tau2_rho --" << std::endl;
	   tau2_rho[0].print();
	   std::cout<< "--- tau_a1 tau2_rho system--" << std::endl;
	   a1rho.print();
	 }
	 
	int isFilter =1;
	double tau_a1_pTlab = sqrt( tau_a1.px()*tau_a1.px()+ tau_a1.py()*tau_a1.py() );
	double tau2_rho_pTlab = sqrt( tau2_rho[0].px()*tau2_rho[0].px()+ tau2_rho[0].py()*tau2_rho[0].py() );
	if(  tau_a1_pTlab < 20.0 ||  tau2_rho_pTlab < 20.0 ) isFilter =0;
	for(unsigned int i=0; i<tau_pi.size(); i++) {
	  double pT = sqrt( tau_pi[i].px()* tau_pi[i].px()+ tau_pi[i].py()* tau_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        
	for(unsigned int i=0; i<tau2_rho_daughters.size(); i++) {
	  double pT = sqrt( tau2_pi[i].px()* tau2_pi[i].px()+ tau2_pi[i].py()* tau2_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        

	if( BENCHMARK )
	  std::cout<< "--- filter = " <<  isFilter << std::endl;


	// calculate angles between normal to rho planes
	double  costheta[4], theta[4];
	int index = -1;

	if( BENCHMARK )
	  std::cout<< "--- loping over possible configuration of the rho-rho pairs-- "<< std::endl;

	for(int i = 0; i< tau_rho.size() ; i++){
	  for(int ii = 0; ii< tau2_rho.size() ; ii++){
	    
	    if( BENCHMARK ){
	      std::cout << " " << std::endl;
	      std::cout << " --- NEXT rho-rho pair --" << std::endl;
	      std::cout << " " << std::endl;
	    }
	    
	    
	    Particle rho_1pi(
			     tau_rho_daughters[i][0].px(),
			     tau_rho_daughters[i][0].py(),
			     tau_rho_daughters[i][0].pz(),
			     tau_rho_daughters[i][0].e(),0);
	    Particle rho_2pi(
			     tau_rho_daughters[i][1].px(),
			     tau_rho_daughters[i][1].py(),
			     tau_rho_daughters[i][1].pz(),
			     tau_rho_daughters[i][1].e(),0);
	    Particle rho(
			 tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			 tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			 tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			 tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
	    Particle other_pi(
			      tau_other_pi[i].px(),
			      tau_other_pi[i].py(),
			      tau_other_pi[i].pz(),
			      tau_other_pi[i].e(),0);
	    Particle rho2_1pi(
			      tau2_rho_daughters[ii][0].px(),
			      tau2_rho_daughters[ii][0].py(),
			      tau2_rho_daughters[ii][0].pz(),
			      tau2_rho_daughters[ii][0].e(),0);
	    Particle rho2_2pi(
			      tau2_rho_daughters[ii][1].px(),
			      tau2_rho_daughters[ii][1].py(),
			      tau2_rho_daughters[ii][1].pz(),
			      tau2_rho_daughters[ii][1].e(),0);
	    Particle rho2(
			  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    Particle rho_rho (
			      tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px() +  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			      tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py() +  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			      tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz() +  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			      tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e()  +  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    
	    if( BENCHMARK ){
	      std::cout << "-- still in the LAB frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    // acoplanarity between rho planes
	    rho_1pi.boostToRestFrame(rho_rho);
	    rho_2pi.boostToRestFrame(rho_rho);
	    rho2_1pi.boostToRestFrame(rho_rho);
	    rho2_2pi.boostToRestFrame(rho_rho);

	    if( BENCHMARK ){	    
	      std::cout << "-- after boost to rho-rho frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    //normal to the plane spanned by rho-> pi+ pi-  
	    double n_1[3];
	    P_norm_cross_product(rho_1pi,rho_2pi,n_1);
	    //normal to the plane spanned by rho2-> pi+ pi0  
	    double n_2[3];
	    P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);	
	    // calculate the acoplanarity angle
	    index++;
	    costheta[index] = dot_product(n_1,n_2);
	    theta[index] = P_acoplanarAngle(rho_1pi, rho_2pi, rho2_1pi, rho2_2pi);

	    if( BENCHMARK ){
	      std::cout<< "--- normal to the tau_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
	      std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
	      std::cout<< "--- cos( acoplanarity angle ) --" << std::endl;
	      std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
	      std::cout<< "--- acoplanarity angle --" << std::endl;
	      std::cout<< "Theta = " <<  theta[index] << std::endl;
	    }

	    double theta4Pi = theta[index];
	    double y1 = (rho_1pi.e()-rho_2pi.e())/(rho_1pi.e()+rho_2pi.e());
	    double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    
	    hmg->GetHistoTH1F(idhist+121)->Fill( theta[index], isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+122)->Fill( rho.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+123)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+124)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+144)->Fill( theta4Pi, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+145)->Fill( theta4Pi, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+146)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+127)->Fill(y1, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+128)->Fill(y2, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+129)->Fill(y1*y2, isFilter * evtWeight );
	    

	  }
	}
    
	
	if( BENCHMARK )
	  std::cout<< "--- looping over possible configuration of the a1-rho  pairs-- "<< std::endl;
	
	for(int i = 0; i<tau_rho.size(); i++){
	  for(int ii = 0; ii<tau2_rho.size(); ii++){
	    
	    if( BENCHMARK ){	    
	      std::cout << " " << std::endl;
	      std::cout << " --- NEXT a1-rho pair --" << std::endl;
	      std::cout << " " << std::endl;
	    }
	    
	    Particle rho_1pi(
			     tau_rho_daughters[i][0].px(),
			     tau_rho_daughters[i][0].py(),
			     tau_rho_daughters[i][0].pz(),
			     tau_rho_daughters[i][0].e(),0);
	    Particle rho_2pi(
			     tau_rho_daughters[i][1].px(),
			     tau_rho_daughters[i][1].py(),
			     tau_rho_daughters[i][1].pz(),
			     tau_rho_daughters[i][1].e(),0);
	    Particle rho(
			 tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			 tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			 tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			 tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
	    Particle other_pi(
			      tau_other_pi[i].px(),
			      tau_other_pi[i].py(),
			      tau_other_pi[i].pz(),
			      tau_other_pi[i].e(),0);
	    Particle rho2_1pi(
			      tau2_rho_daughters[ii][0].px(),
			      tau2_rho_daughters[ii][0].py(),
			      tau2_rho_daughters[ii][0].pz(),
			      tau2_rho_daughters[ii][0].e(),0);
	    Particle rho2_2pi(
			      tau2_rho_daughters[ii][1].px(),
			      tau2_rho_daughters[ii][1].py(),
			      tau2_rho_daughters[ii][1].pz(),
			      tau2_rho_daughters[ii][1].e(),0);
	    Particle rho2(
			  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    Particle a1_rho (
			     tau_a1.px() +  tau2_rho[ii].px(),
			     tau_a1.py() +  tau2_rho[ii].py(),
			     tau_a1.pz() +  tau2_rho[ii].pz(),
			     tau_a1.e()  +  tau2_rho[ii].e(), 0);
	    
	    if( BENCHMARK ){
	      std::cout << "-- still in the LAB frame  " << std::endl;
	      std::cout<< "--- tau_a1 components --" << std::endl;
	      rho.print();
	      other_pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    // acoplanarity between a1-rho planes
	    rho.boostToRestFrame(a1_rho);
	    other_pi.boostToRestFrame(a1_rho);
	    rho2_1pi.boostToRestFrame(a1_rho);
	    rho2_2pi.boostToRestFrame(a1_rho);
	    
	    if( BENCHMARK ){
	      std::cout << "-- after boost to a1-rho frame  " << std::endl;
	      std::cout<< "--- tau_a1 components --" << std::endl;
	      rho.print();
	      other_pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    //normal to the plane spanned by rho-> pi+ pi-  
	    double n_1[3];
	    P_norm_cross_product(rho,other_pi,n_1);
	    //normal to the plane spanned by rho2-> pi+ pi-  
	    double n_2[3];
	    P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);	
	    // calculate the acoplanarity angle
	    index++;
	    costheta[index] = dot_product(n_1,n_2);
	    theta[index] = P_acoplanarAngle(rho, other_pi, rho2_1pi, rho2_2pi);

	    if( BENCHMARK ){
	      std::cout<< "--- normal to the tau_a1 plane --" << std::endl;
	      std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
	      std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
	      std::cout<< "--- cos(acoplanarity angle) --" << std::endl;
	      std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
	      std::cout<< "--- acoplanarity angle --" << std::endl;
	      std::cout<< "Theta = " <<  theta[index] << std::endl;
	    }

	    double theta4Pi_v1 = theta[index];
	    double theta4Pi_v2 = theta[index];
	    double theta4Pi_v3 = theta[index];

	    double x1 = (pow(tau_a1.recalculated_mass(),2)- pow(other_pi.recalculated_mass(),2)+pow(rho.recalculated_mass(),2))  
	      /( 2 * pow(tau_a1.recalculated_mass(),2));

	    double y1_v1 = (rho.e()-other_pi.e())/(rho.e()+other_pi.e()) - x1;
	    
	    double y1_v2 = rho.e()/(rho.e()+other_pi.e());
	    if( y1_v2 > x1 ) y1_v2 = -y1_v2;

	    double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	    
	    if( y1_v1*y2 < 0) theta4Pi_v1 = theta4Pi_v1 + 2 * kPI;
	    if( y1_v2*y2 < 0) theta4Pi_v2 = theta4Pi_v2 + 2 * kPI;
	    if( y1_v2*y2 > 0) theta4Pi_v3 = theta4Pi_v3 + 2 * kPI;
	    	    
	    hmg->GetHistoTH1F(idhist+131)->Fill( theta[index], isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+132)->Fill( tau_a1.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+133)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+134)->Fill( theta4Pi_v1, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+135)->Fill( theta4Pi_v2, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+136)->Fill( theta4Pi_v3, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+144)->Fill( theta4Pi_v1, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+145)->Fill( theta4Pi_v2, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+146)->Fill( theta4Pi_v3, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+137)->Fill( y1_v1, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+138)->Fill( y1_v2, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+139)->Fill( y1_v1 * y2, isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+140)->Fill( y1_v2 * y2, isFilter * evtWeight );

	    
	  }
	}
	
       }
    }
    
}
//----------------------------------------------
void AnalTauSpinnerCP::test_3p0n_v2_1p1n( int idhist, HepMC::GenEvent *hepmc, float evtWeight )
//----------------------------------------------
{

  HistoManager  *hmg = HistoManager::getInstance();
 
  BuildTauSpinnerBranch  *evtmg = BuildTauSpinnerBranch::getInstance();


  // prepare branch for TauSpinner weight
  SimpleParticle sp_X, sp_tau, sp_tau2; // SimpleParticle consist of 4 momentum and pdgId.
  vector<SimpleParticle> sp_tau_daughters, sp_tau2_daughters;

   // search for tau decay in the event
    int status =  evtmg->buildSimpleBranch(hepmc, sp_X, sp_tau, sp_tau2, sp_tau_daughters, sp_tau2_daughters);

    if(status == 2) {

      float accFlag = 0;
      hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

      hmg->GetHistoTH1F(idhist+1)->Fill( evtWeight, 1.0 );

      vector<Particle> tau_pi,  tau2_pi ;
      vector<Particle> tau_rho, tau2_rho ;
      vector<Particle> tau_other_pi;
      vector< vector<Particle> > tau_rho_daughters, tau2_rho_daughters ;

      double etmiss_px = 0;
      double etmiss_py = 0;
 
     // Create list of tau daughters and reconstruct a1 resonances
      for(unsigned int i=0; i<sp_tau_daughters.size(); i++) {
	if(abs(sp_tau_daughters[i].pdgid()) == 16){
	  etmiss_px += sp_tau_daughters[i].px();
	  etmiss_py += sp_tau_daughters[i].py();
	}
	if( abs(sp_tau_daughters[i].pdgid()) == 211 ){
	  Particle pp(sp_tau_daughters[i].px(),
		      sp_tau_daughters[i].py(),
		      sp_tau_daughters[i].pz(),
		      sp_tau_daughters[i].e(),
		      sp_tau_daughters[i].pdgid() );
	  tau_pi.push_back(pp);
	  double pT_i = sqrt(   sp_tau_daughters[i].px()*sp_tau_daughters[i].px()
				  +  sp_tau_daughters[i].py()*sp_tau_daughters[i].py() );
	  for(unsigned int ii=i+1; ii<sp_tau_daughters.size(); ii++) {
	    if( abs(sp_tau_daughters[ii].pdgid()) == 211 ){
	      double pT_ii = sqrt(   sp_tau_daughters[ii].px()*sp_tau_daughters[ii].px()
				  +  sp_tau_daughters[ii].py()*sp_tau_daughters[ii].py() );
	      for(unsigned int iii=0; iii<sp_tau_daughters.size(); iii++) {
		 if( abs(sp_tau_daughters[iii].pdgid()) == 211 && iii != i && iii != ii ){
		   double pT_iii = sqrt(   sp_tau_daughters[iii].px()*sp_tau_daughters[iii].px()
					+  sp_tau_daughters[iii].py()*sp_tau_daughters[iii].py() );

		   if(  pT_i <pT_iii &&  pT_ii <pT_iii ) { 
		     vector<Particle> temp;
		     Particle p1(sp_tau_daughters[i].px() ,
				 sp_tau_daughters[i].py(),
				 sp_tau_daughters[i].pz(),
				 sp_tau_daughters[i].e(),
				 sp_tau_daughters[i].pdgid());
		     temp.push_back(p1);
		     Particle p2(sp_tau_daughters[ii].px() ,
				 sp_tau_daughters[ii].py(),
				 sp_tau_daughters[ii].pz(),
				 sp_tau_daughters[ii].e(),
				 sp_tau_daughters[ii].pdgid());
		     temp.push_back(p2);
		     Particle pp(sp_tau_daughters[i].px()+sp_tau_daughters[ii].px() ,
				 sp_tau_daughters[i].py()+sp_tau_daughters[ii].py(),
				 sp_tau_daughters[i].pz()+sp_tau_daughters[ii].pz(),
				 sp_tau_daughters[i].e() +sp_tau_daughters[ii].e(),
				 0 );
		     tau_rho.push_back(pp);
		     tau_rho_daughters.push_back(temp);
		   } else if ( ii != i ){
		     Particle p(sp_tau_daughters[ii].px() ,
				sp_tau_daughters[ii].py(),
				sp_tau_daughters[ii].pz(),
				sp_tau_daughters[ii].e(),
				sp_tau_daughters[ii].pdgid());
		     tau_other_pi.push_back(p);
		   }
		 }
	      }
	    }
	  }
	}
      }
       if(DEBUG){
	std::cout << "tau_rho.size = " << tau_rho.size() << std::endl;
	if( tau_rho.size() > 0) {
	  std::cout << "tau_rho_daughters[0].size =" << tau_rho_daughters[0].size() << std::endl;
	  tau_rho_daughters[0][0].print();
	  tau_rho_daughters[0][1].print();
	}
	if( tau_rho.size() > 1) {
	  std::cout << "tau_rho_daughters[1].size =" << tau_rho_daughters[1].size() << std::endl;
	  tau_rho_daughters[1][0].print();
	  tau_rho_daughters[1][1].print();
	}
	for(unsigned int i=0; i<tau_other_pi.size(); i++) {
	  std::cout << "tau_other_pi.size =" << tau_other_pi.size() << std::endl;
	  tau_other_pi[i].print();
	}
       }

       // Create list of tau2 daughters and reconstruct rho resonances
       for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) {
	 if(abs(sp_tau2_daughters[i].pdgid()) == 16){
	   etmiss_px += sp_tau2_daughters[i].px();
	   etmiss_py += sp_tau2_daughters[i].py();
	 }
	 if( abs(sp_tau2_daughters[i].pdgid()) == 211 ){
	   Particle pp(sp_tau2_daughters[i].px(),
		       sp_tau2_daughters[i].py(),
		       sp_tau2_daughters[i].pz(),
		       sp_tau2_daughters[i].e(),
		       sp_tau2_daughters[i].pdgid() );
	   tau2_pi.push_back(pp);
	   for(unsigned int ii=0; ii<sp_tau2_daughters.size(); ii++) {
	     if( abs(sp_tau2_daughters[ii].pdgid()) == 111 ){
	       vector<Particle> temp;
	       Particle p1(sp_tau2_daughters[i].px() ,
			   sp_tau2_daughters[i].py(),
			   sp_tau2_daughters[i].pz(),
			   sp_tau2_daughters[i].e(),
			   sp_tau2_daughters[i].pdgid());
	       temp.push_back(p1);
	       Particle p2(sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[ii].e(),
			   sp_tau2_daughters[ii].pdgid());
	       temp.push_back(p2);
	       tau2_pi.push_back(p2);
	       Particle pp(sp_tau2_daughters[i].px()+sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[i].py()+sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[i].pz()+sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[i].e() +sp_tau2_daughters[ii].e(),
			   0 );
	       tau2_rho.push_back(pp);
	       tau2_rho_daughters.push_back(temp);
	     }
	   }
	 }
       }
       if(DEBUG){
	 std::cout << "tau2_rho.size = " << tau2_rho.size() << std::endl;
	 if( tau2_rho.size() > 0) {
	   std::cout << "tau2_rho_daughters[0].size =" << tau2_rho_daughters[0].size() << std::endl;
	   tau2_rho_daughters[0][0].print();
	   tau2_rho_daughters[0][1].print();
	 }
       }

       //       std::cout << "a1rho events: " << tau_pi.size() << " " <<  tau2_rho.size() << std::endl;

       if( tau_pi.size() == 3 &&  tau2_rho.size() == 1 ){
	 
	 
	 float accFlag = 1;
	 hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );
	 
	 if( outNNlab ){
	   printf("TUPLE  %lf  \n", evtWeight);
	   for(unsigned int i=0; i<sp_tau_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau_daughters[i].px(), sp_tau_daughters[i].py(), sp_tau_daughters[i].pz(), sp_tau_daughters[i].e(), sp_tau_daughters[i].pdgid());
	   for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau2_daughters[i].px(), sp_tau2_daughters[i].py(), sp_tau2_daughters[i].pz(), sp_tau2_daughters[i].e(), sp_tau2_daughters[i].pdgid());
	   //	  return;
	 }
	 
	 // Build the a1 resonances
	 Particle tau_a1 ( tau_pi[0].px()+tau_pi[1].px()+tau_pi[2].px(), 
			   tau_pi[0].py()+tau_pi[1].py()+tau_pi[2].py(),
			   tau_pi[0].pz()+tau_pi[1].pz()+tau_pi[2].pz(),
			   tau_pi[0].e() +tau_pi[1].e() +tau_pi[2].e(), 0.0);
	 
	 // Build the a1-rho system resonances
	 Particle a1rho ( tau_a1.px()+tau2_rho[0].px(), 
			  tau_a1.py()+tau2_rho[0].py(),
			  tau_a1.pz()+tau2_rho[0].pz(),
			  tau_a1.e() +tau2_rho[0].e(), 0.0);	 
	 if( BENCHMARK ){
	   std::cout<< "--- tau_a1 --" << std::endl;
	   tau_a1.print();
	   std::cout<< "--- tau2_rho --" << std::endl;
	   tau2_rho[0].print();
	   std::cout<< "--- tau_a1 tau2_rho system--" << std::endl;
	   a1rho.print();
	 }
	 
	int isFilter =1;
	double tau_a1_pTlab = sqrt( tau_a1.px()*tau_a1.px()+ tau_a1.py()*tau_a1.py() );
	double tau2_rho_pTlab = sqrt( tau2_rho[0].px()*tau2_rho[0].px()+ tau2_rho[0].py()*tau2_rho[0].py() );
	if(  tau_a1_pTlab < 20.0 ||  tau2_rho_pTlab < 20.0 ) isFilter =0;
	for(unsigned int i=0; i<tau_pi.size(); i++) {
	  double pT = sqrt( tau_pi[i].px()* tau_pi[i].px()+ tau_pi[i].py()* tau_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        
	for(unsigned int i=0; i<tau2_rho_daughters.size(); i++) {
	  double pT = sqrt( tau2_pi[i].px()* tau2_pi[i].px()+ tau2_pi[i].py()* tau2_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        

	if( BENCHMARK )
	  std::cout<< "--- filter = " <<  isFilter << std::endl;


	// calculate angles between normal to rho planes
	double  costheta[4], theta[4];
	int index = -1;

	if( BENCHMARK )
	  std::cout<< "--- loping over possible configuration of the rho-rho pairs-- "<< std::endl;

	for(int i = 0; i< tau_rho.size() ; i++){
	  for(int ii = 0; ii< tau2_rho.size() ; ii++){
	    
	    if( BENCHMARK ){
	      std::cout << " " << std::endl;
	      std::cout << " --- NEXT rho-rho pair --" << std::endl;
	      std::cout << " " << std::endl;
	    }
	    
	    
	    Particle rho_1pi(
			     tau_rho_daughters[i][0].px(),
			     tau_rho_daughters[i][0].py(),
			     tau_rho_daughters[i][0].pz(),
			     tau_rho_daughters[i][0].e(),0);
	    Particle rho_2pi(
			     tau_rho_daughters[i][1].px(),
			     tau_rho_daughters[i][1].py(),
			     tau_rho_daughters[i][1].pz(),
			     tau_rho_daughters[i][1].e(),0);
	    Particle rho(
			 tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			 tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			 tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			 tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
	    Particle other_pi(
			      tau_other_pi[i].px(),
			      tau_other_pi[i].py(),
			      tau_other_pi[i].pz(),
			      tau_other_pi[i].e(),0);
	    Particle rho2_1pi(
			      tau2_rho_daughters[ii][0].px(),
			      tau2_rho_daughters[ii][0].py(),
			      tau2_rho_daughters[ii][0].pz(),
			      tau2_rho_daughters[ii][0].e(),0);
	    Particle rho2_2pi(
			      tau2_rho_daughters[ii][1].px(),
			      tau2_rho_daughters[ii][1].py(),
			      tau2_rho_daughters[ii][1].pz(),
			      tau2_rho_daughters[ii][1].e(),0);
	    Particle rho2(
			  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    Particle rho_rho (
			      tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px() +  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			      tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py() +  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			      tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz() +  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			      tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e()  +  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    
	    if( BENCHMARK ){
	      std::cout << "-- still in the LAB frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    // acoplanarity between rho planes
	    rho_1pi.boostToRestFrame(rho_rho);
	    rho_2pi.boostToRestFrame(rho_rho);
	    rho2_1pi.boostToRestFrame(rho_rho);
	    rho2_2pi.boostToRestFrame(rho_rho);

	    if( BENCHMARK ){	    
	      std::cout << "-- after boost to rho-rho frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    //normal to the plane spanned by rho-> pi+ pi-  
	    double n_1[3];
	    P_norm_cross_product(rho_1pi,rho_2pi,n_1);
	    //normal to the plane spanned by rho2-> pi+ pi0  
	    double n_2[3];
	    P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);	
	    // calculate the acoplanarity angle
	    index++;
	    costheta[index] = dot_product(n_1,n_2);
	    theta[index] = P_acoplanarAngle(rho_1pi, rho_2pi, rho2_1pi, rho2_2pi);

	    if( BENCHMARK ){
	      std::cout<< "--- normal to the tau_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
	      std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
	      std::cout<< "--- cos( acoplanarity angle ) --" << std::endl;
	      std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
	      std::cout<< "--- acoplanarity angle --" << std::endl;
	      std::cout<< "Theta = " <<  theta[index] << std::endl;
	    }

	    double theta4Pi = theta[index];
	    double y1 = (rho_1pi.e()-rho_2pi.e())/(rho_1pi.e()+rho_2pi.e());
	    double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    
	    hmg->GetHistoTH1F(idhist+121)->Fill( theta[index], isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+122)->Fill( rho.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+123)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+124)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+154)->Fill( theta4Pi, isFilter * evtWeight );
	    double thetaFlip =   theta[index];
	    if( y1*y2 > 0)
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    if( y1*y2 < 0){
	      thetaFlip =   theta[index] - kPI;
	      if (thetaFlip < 0)  thetaFlip = thetaFlip + 2 * kPI;
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    }
	    
	  }
	}
    
	
	if( BENCHMARK )
	  std::cout<< "--- looping over possible configuration of the a1-rho  pairs-- "<< std::endl;
	
	for(int i = 0; i<tau_rho.size(); i++){
	  for(int ii = 0; ii<tau2_rho.size(); ii++){
	    
	    if( BENCHMARK ){	    
	      std::cout << " " << std::endl;
	      std::cout << " --- NEXT a1-rho pair --" << std::endl;
	      std::cout << " " << std::endl;
	    }
	    
	    Particle rho_1pi(
			     tau_rho_daughters[i][0].px(),
			     tau_rho_daughters[i][0].py(),
			     tau_rho_daughters[i][0].pz(),
			     tau_rho_daughters[i][0].e(),0);
	    Particle rho_2pi(
			     tau_rho_daughters[i][1].px(),
			     tau_rho_daughters[i][1].py(),
			     tau_rho_daughters[i][1].pz(),
			     tau_rho_daughters[i][1].e(),0);
	    Particle rho(
			 tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			 tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			 tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			 tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
	    Particle other_pi(
			      tau_other_pi[i].px(),
			      tau_other_pi[i].py(),
			      tau_other_pi[i].pz(),
			      tau_other_pi[i].e(),0);
	    Particle rho2_1pi(
			      tau2_rho_daughters[ii][0].px(),
			      tau2_rho_daughters[ii][0].py(),
			      tau2_rho_daughters[ii][0].pz(),
			      tau2_rho_daughters[ii][0].e(),0);
	    Particle rho2_2pi(
			      tau2_rho_daughters[ii][1].px(),
			      tau2_rho_daughters[ii][1].py(),
			      tau2_rho_daughters[ii][1].pz(),
			      tau2_rho_daughters[ii][1].e(),0);
	    Particle rho2(
			  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    Particle a1_rho (
			     tau_a1.px() +  tau2_rho[ii].px(),
			     tau_a1.py() +  tau2_rho[ii].py(),
			     tau_a1.pz() +  tau2_rho[ii].pz(),
			     tau_a1.e()  +  tau2_rho[ii].e(), 0);
	    
	    if( BENCHMARK ){
	      std::cout << "-- still in the LAB frame  " << std::endl;
	      std::cout<< "--- tau_a1 components --" << std::endl;
	      rho.print();
	      other_pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    // acoplanarity between a1-rho planes
	    rho.boostToRestFrame(a1_rho);
	    other_pi.boostToRestFrame(a1_rho);
	    rho2_1pi.boostToRestFrame(a1_rho);
	    rho2_2pi.boostToRestFrame(a1_rho);
	    
	    if( BENCHMARK ){
	      std::cout << "-- after boost to a1-rho frame  " << std::endl;
	      std::cout<< "--- tau_a1 components --" << std::endl;
	      rho.print();
	      other_pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    //normal to the plane spanned by rho-> pi+ pi-  
	    double n_1[3];
	    P_norm_cross_product(rho,other_pi,n_1);
	    //normal to the plane spanned by rho2-> pi+ pi-  
	    double n_2[3];
	    P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);	
	    // calculate the acoplanarity angle
	    index++;
	    costheta[index] = dot_product(n_1,n_2);
	    theta[index] = P_acoplanarAngle(rho, other_pi, rho2_1pi, rho2_2pi);

	    if( BENCHMARK ){
	      std::cout<< "--- normal to the tau_a1 plane --" << std::endl;
	      std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
	      std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
	      std::cout<< "--- cos(acoplanarity angle) --" << std::endl;
	      std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
	      std::cout<< "--- acoplanarity angle --" << std::endl;
	      std::cout<< "Theta = " <<  theta[index] << std::endl;
	    }

	    double theta4Pi = theta[index];
	    double thetaPi =   theta[index] -  kPI;
	    if (thetaPi < 0)  thetaPi = thetaPi + 2 * kPI;

	    double x1 = (pow(tau_a1.recalculated_mass(),2)- pow(other_pi.recalculated_mass(),2)+pow(rho.recalculated_mass(),2))  
	      /( 2 * pow(tau_a1.recalculated_mass(),2));

	    // new
	    double y1 = (rho.e()-other_pi.e())/(rho.e()+other_pi.e()) - x1;	    
	    //	old
	    //  double y1 = rho.e()/(rho.e()+other_pi.e());
	    //  if( y1 > x1 ) y1 = -y1;

	    double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    	    
	    hmg->GetHistoTH1F(idhist+131)->Fill( theta[index], isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+132)->Fill( tau_a1.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+133)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+134)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+141)->Fill( thetaPi, isFilter * evtWeight );
	    theta4Pi = thetaPi;
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    hmg->GetHistoTH1F(idhist+144)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+154)->Fill( theta4Pi, isFilter * evtWeight );

	    double thetaFlip =   theta[index];
	    if( y1*y2 < 0)
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    if( y1*y2 > 0){
	      thetaFlip =   theta[index] - kPI;
	      if (thetaFlip < 0)  thetaFlip = thetaFlip + 2 * kPI;
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    }

	    
	  }
	}
	
       }
    }
    
}
//----------------------------------------------
void AnalTauSpinnerCP::test_1p2n_v2_1p1n( int idhist, HepMC::GenEvent *hepmc, float evtWeight )
//----------------------------------------------
{

  HistoManager  *hmg = HistoManager::getInstance();
 
  BuildTauSpinnerBranch  *evtmg = BuildTauSpinnerBranch::getInstance();


  // prepare branch for TauSpinner weight
  SimpleParticle sp_X, sp_tau, sp_tau2; // SimpleParticle consist of 4 momentum and pdgId.
  vector<SimpleParticle> sp_tau_daughters, sp_tau2_daughters;

   // search for tau decay in the event
    int status =  evtmg->buildSimpleBranch(hepmc, sp_X, sp_tau, sp_tau2, sp_tau_daughters, sp_tau2_daughters);

    if(status == 2) {

      float accFlag = 0;
      hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

      hmg->GetHistoTH1F(idhist+1)->Fill( evtWeight, 1.0 );

      vector<Particle> tau_pi,  tau2_pi ;
      vector<Particle> tau_rho, tau2_rho ;
      vector<Particle> tau_other_pi0;
      vector< vector<Particle> > tau_rho_daughters, tau2_rho_daughters ;

      double etmiss_px = 0;
      double etmiss_py = 0;
 
     // Create list of tau daughters and reconstruct a1 resonances
      for(unsigned int i=0; i<sp_tau_daughters.size(); i++) {
	if(abs(sp_tau_daughters[i].pdgid()) == 16){
	  etmiss_px += sp_tau_daughters[i].px();
	  etmiss_py += sp_tau_daughters[i].py();
	}
	if( abs(sp_tau_daughters[i].pdgid()) == 211 ){
	  Particle pp(sp_tau_daughters[i].px(),
		      sp_tau_daughters[i].py(),
		      sp_tau_daughters[i].pz(),
		      sp_tau_daughters[i].e(),
		      sp_tau_daughters[i].pdgid() );
	  tau_pi.push_back(pp);
	  for(unsigned int ii=0; ii<sp_tau_daughters.size(); ii++) {
	    if( abs(sp_tau_daughters[ii].pdgid()) == 111 ){
	      vector<Particle> temp;
	      Particle p1(sp_tau_daughters[i].px() ,
			  sp_tau_daughters[i].py(),
			  sp_tau_daughters[i].pz(),
			  sp_tau_daughters[i].e(),
			  sp_tau_daughters[i].pdgid());
	      temp.push_back(p1);
	      Particle p2(sp_tau_daughters[ii].px() ,
			  sp_tau_daughters[ii].py(),
			  sp_tau_daughters[ii].pz(),
			  sp_tau_daughters[ii].e(),
			  sp_tau_daughters[ii].pdgid());
	      temp.push_back(p2);
	      Particle pp(sp_tau_daughters[i].px()+sp_tau_daughters[ii].px() ,
			  sp_tau_daughters[i].py()+sp_tau_daughters[ii].py(),
			  sp_tau_daughters[i].pz()+sp_tau_daughters[ii].pz(),
			  sp_tau_daughters[i].e() +sp_tau_daughters[ii].e(),
			  0 );
	      tau_rho.push_back(pp);
	      tau_rho_daughters.push_back(temp);
	      for(unsigned int iii=0; iii<sp_tau_daughters.size(); iii++) {
		if( abs(sp_tau_daughters[iii].pdgid()) == 111 && ii!=iii ){
		  Particle p(sp_tau_daughters[iii].px() ,
			     sp_tau_daughters[iii].py(),
			     sp_tau_daughters[iii].pz(),
			     sp_tau_daughters[iii].e(),
			     sp_tau_daughters[iii].pdgid());
		  tau_other_pi0.push_back(p);
		}
	      }
	    }
	  }
	}
      }
       if(DEBUG){
	std::cout << "tau_rho.size = " << tau_rho.size() << std::endl;
	if( tau_rho.size() > 0) {
	  std::cout << "tau_rho_daughters[0].size =" << tau_rho_daughters[0].size() << std::endl;
	  tau_rho_daughters[0][0].print();
	  tau_rho_daughters[0][1].print();
	}
	if( tau_rho.size() > 1) {
	  std::cout << "tau_rho_daughters[1].size =" << tau_rho_daughters[1].size() << std::endl;
	  tau_rho_daughters[1][0].print();
	  tau_rho_daughters[1][1].print();
	}
	for(unsigned int i=0; i<tau_other_pi0.size(); i++) {
	  std::cout << "tau_other_pi0.size =" << tau_other_pi0.size() << std::endl;
	  tau_other_pi0[i].print();
	}
       }

       // Create list of tau2 daughters and reconstruct rho resonances
       for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) {
	 if(abs(sp_tau2_daughters[i].pdgid()) == 16){
	   etmiss_px += sp_tau2_daughters[i].px();
	   etmiss_py += sp_tau2_daughters[i].py();
	 }
	 if( abs(sp_tau2_daughters[i].pdgid()) == 211 ){
	   Particle pp(sp_tau2_daughters[i].px(),
		       sp_tau2_daughters[i].py(),
		       sp_tau2_daughters[i].pz(),
		       sp_tau2_daughters[i].e(),
		       sp_tau2_daughters[i].pdgid() );
	   tau2_pi.push_back(pp);
	   for(unsigned int ii=0; ii<sp_tau2_daughters.size(); ii++) {
	     if( abs(sp_tau2_daughters[ii].pdgid()) == 111 ){
	       vector<Particle> temp;
	       Particle p1(sp_tau2_daughters[i].px() ,
			   sp_tau2_daughters[i].py(),
			   sp_tau2_daughters[i].pz(),
			   sp_tau2_daughters[i].e(),
			   sp_tau2_daughters[i].pdgid());
	       temp.push_back(p1);
	       Particle p2(sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[ii].e(),
			   sp_tau2_daughters[ii].pdgid());
	       temp.push_back(p2);
	       tau2_pi.push_back(p2);
	       Particle pp(sp_tau2_daughters[i].px()+sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[i].py()+sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[i].pz()+sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[i].e() +sp_tau2_daughters[ii].e(),
			   0 );
	       tau2_rho.push_back(pp);
	       tau2_rho_daughters.push_back(temp);
	     }
	   }
	 }
       }
       if(DEBUG){
	 std::cout << "tau2_rho.size = " << tau2_rho.size() << std::endl;
	 if( tau2_rho.size() > 0) {
	   std::cout << "tau2_rho_daughters[0].size =" << tau2_rho_daughters[0].size() << std::endl;
	   tau2_rho_daughters[0][0].print();
	   tau2_rho_daughters[0][1].print();
	 }
       }

       //       std::cout << "a1rho events: " << tau_pi.size() << " " <<  tau2_rho.size() << std::endl;

       if( tau_pi.size() == 1 &&  tau_other_pi0.size() == 2 &&  tau2_rho.size() == 1 ){
	 
	 
	 float accFlag = 1;
	 hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );
	 
	 if( outNNlab ){
	   printf("TUPLE  %lf  \n", evtWeight);
	   for(unsigned int i=0; i<sp_tau_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau_daughters[i].px(), sp_tau_daughters[i].py(), sp_tau_daughters[i].pz(), sp_tau_daughters[i].e(), sp_tau_daughters[i].pdgid());
	   for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau2_daughters[i].px(), sp_tau2_daughters[i].py(), sp_tau2_daughters[i].pz(), sp_tau2_daughters[i].e(), sp_tau2_daughters[i].pdgid());
	   //	  return;
	 }
	 
	 // Build the a1 resonances
	 Particle tau_a1 ( tau_other_pi0[0].px()+tau_rho[0].px(), 
			   tau_other_pi0[0].py()+tau_rho[0].py(),
			   tau_other_pi0[0].pz()+tau_rho[0].pz(),
			   tau_other_pi0[0].e() +tau_rho[0].e() , 0.0);
	 
	 // Build the a1-rho system resonances
	 Particle a1rho ( tau_a1.px()+tau2_rho[0].px(), 
			  tau_a1.py()+tau2_rho[0].py(),
			  tau_a1.pz()+tau2_rho[0].pz(),
			  tau_a1.e() +tau2_rho[0].e(), 0.0);	 
	 if( BENCHMARK ){
	   std::cout<< "--- tau_a1 --" << std::endl;
	   tau_a1.print();
	   std::cout<< "--- tau2_rho --" << std::endl;
	   tau2_rho[0].print();
	   std::cout<< "--- tau_a1 tau2_rho system--" << std::endl;
	   a1rho.print();
	 }
	 
	int isFilter =1;
	double tau_a1_pTlab = sqrt( tau_a1.px()*tau_a1.px()+ tau_a1.py()*tau_a1.py() );
	double tau2_rho_pTlab = sqrt( tau2_rho[0].px()*tau2_rho[0].px()+ tau2_rho[0].py()*tau2_rho[0].py() );
	if(  tau_a1_pTlab < 20.0 ||  tau2_rho_pTlab < 20.0 ) isFilter =0;
	for(unsigned int i=0; i<tau_pi.size(); i++) {
	  double pT = sqrt( tau_pi[i].px()* tau_pi[i].px()+ tau_pi[i].py()* tau_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        
	for(unsigned int i=0; i<tau2_rho_daughters.size(); i++) {
	  double pT = sqrt( tau2_pi[i].px()* tau2_pi[i].px()+ tau2_pi[i].py()* tau2_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        

	if( BENCHMARK )
	  std::cout<< "--- filter = " <<  isFilter << std::endl;


	// calculate angles between normal to rho planes
	double  costheta[4], theta[4];
	int index = -1;

	if( BENCHMARK )
	  std::cout<< "--- loping over possible configuration of the rho-rho pairs-- "<< std::endl;

	for(int i = 0; i< tau_rho.size() ; i++){
	  for(int ii = 0; ii< tau2_rho.size() ; ii++){
	    
	    if( BENCHMARK ){
	      std::cout << " " << std::endl;
	      std::cout << " --- NEXT rho-rho pair --" << std::endl;
	      std::cout << " " << std::endl;
	    }
	    
	    
	    Particle rho_1pi(
			     tau_rho_daughters[i][0].px(),
			     tau_rho_daughters[i][0].py(),
			     tau_rho_daughters[i][0].pz(),
			     tau_rho_daughters[i][0].e(),0);
	    Particle rho_2pi(
			     tau_rho_daughters[i][1].px(),
			     tau_rho_daughters[i][1].py(),
			     tau_rho_daughters[i][1].pz(),
			     tau_rho_daughters[i][1].e(),0);
	    Particle rho(
			 tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			 tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			 tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			 tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
	    Particle other_pi0(
			      tau_other_pi0[i].px(),
			      tau_other_pi0[i].py(),
			      tau_other_pi0[i].pz(),
			      tau_other_pi0[i].e(),0);
	    Particle rho2_1pi(
			      tau2_rho_daughters[ii][0].px(),
			      tau2_rho_daughters[ii][0].py(),
			      tau2_rho_daughters[ii][0].pz(),
			      tau2_rho_daughters[ii][0].e(),0);
	    Particle rho2_2pi(
			      tau2_rho_daughters[ii][1].px(),
			      tau2_rho_daughters[ii][1].py(),
			      tau2_rho_daughters[ii][1].pz(),
			      tau2_rho_daughters[ii][1].e(),0);
	    Particle rho2(
			  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    Particle rho_rho (
			      tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px() +  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			      tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py() +  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			      tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz() +  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			      tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e()  +  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    
	    if( BENCHMARK ){
	      std::cout << "-- still in the LAB frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    // acoplanarity between rho planes
	    rho_1pi.boostToRestFrame(rho_rho);
	    rho_2pi.boostToRestFrame(rho_rho);
	    rho2_1pi.boostToRestFrame(rho_rho);
	    rho2_2pi.boostToRestFrame(rho_rho);

	    if( BENCHMARK ){	    
	      std::cout << "-- after boost to rho-rho frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    //normal to the plane spanned by rho-> pi+ pi-  
	    double n_1[3];
	    P_norm_cross_product(rho_1pi,rho_2pi,n_1);
	    //normal to the plane spanned by rho2-> pi+ pi0  
	    double n_2[3];
	    P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);	
	    // calculate the acoplanarity angle
	    index++;
	    costheta[index] = dot_product(n_1,n_2);
	    theta[index] = P_acoplanarAngle(rho_1pi, rho_2pi, rho2_1pi, rho2_2pi);

	    if( BENCHMARK ){
	      std::cout<< "--- normal to the tau_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
	      std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
	      std::cout<< "--- cos( acoplanarity angle ) --" << std::endl;
	      std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
	      std::cout<< "--- acoplanarity angle --" << std::endl;
	      std::cout<< "Theta = " <<  theta[index] << std::endl;
	    }

	    double theta4Pi = theta[index];
	    double y1 = (rho_1pi.e()-rho_2pi.e())/(rho_1pi.e()+rho_2pi.e());
	    double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    
	    hmg->GetHistoTH1F(idhist+121)->Fill( theta[index], isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+122)->Fill( rho.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+123)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+124)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+154)->Fill( theta4Pi, isFilter * evtWeight );

	    double thetaFlip =   theta[index];
	    if( y1*y2 > 0)
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    if( y1*y2 < 0){
	      thetaFlip =   theta[index] - kPI;
	      if (thetaFlip < 0)  thetaFlip = thetaFlip + 2 * kPI;
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    }


	  }
	}
    
	
	if( BENCHMARK )
	  std::cout<< "--- looping over possible configuration of the a1-rho  pairs-- "<< std::endl;
	
	for(int i = 0; i<tau_rho.size(); i++){
	  for(int ii = 0; ii<tau2_rho.size(); ii++){
	    
	    if( BENCHMARK ){	    
	      std::cout << " " << std::endl;
	      std::cout << " --- NEXT a1-rho pair --" << std::endl;
	      std::cout << " " << std::endl;
	    }
	    
	    Particle rho_1pi(
			     tau_rho_daughters[i][0].px(),
			     tau_rho_daughters[i][0].py(),
			     tau_rho_daughters[i][0].pz(),
			     tau_rho_daughters[i][0].e(),0);
	    Particle rho_2pi(
			     tau_rho_daughters[i][1].px(),
			     tau_rho_daughters[i][1].py(),
			     tau_rho_daughters[i][1].pz(),
			     tau_rho_daughters[i][1].e(),0);
	    Particle rho(
			 tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			 tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			 tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			 tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
	    Particle other_pi0(
			      tau_other_pi0[i].px(),
			      tau_other_pi0[i].py(),
			      tau_other_pi0[i].pz(),
			      tau_other_pi0[i].e(),0);
	    Particle rho2_1pi(
			      tau2_rho_daughters[ii][0].px(),
			      tau2_rho_daughters[ii][0].py(),
			      tau2_rho_daughters[ii][0].pz(),
			      tau2_rho_daughters[ii][0].e(),0);
	    Particle rho2_2pi(
			      tau2_rho_daughters[ii][1].px(),
			      tau2_rho_daughters[ii][1].py(),
			      tau2_rho_daughters[ii][1].pz(),
			      tau2_rho_daughters[ii][1].e(),0);
	    Particle rho2(
			  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    Particle a1_rho (
			     tau_a1.px() +  tau2_rho[ii].px(),
			     tau_a1.py() +  tau2_rho[ii].py(),
			     tau_a1.pz() +  tau2_rho[ii].pz(),
			     tau_a1.e()  +  tau2_rho[ii].e(), 0);
	    
	    if( BENCHMARK ){
	      std::cout << "-- still in the LAB frame  " << std::endl;
	      std::cout<< "--- tau_a1 components --" << std::endl;
	      rho.print();
	      other_pi0.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    // acoplanarity between a1-rho planes
	    rho.boostToRestFrame(a1_rho);
	    other_pi0.boostToRestFrame(a1_rho);
	    rho2_1pi.boostToRestFrame(a1_rho);
	    rho2_2pi.boostToRestFrame(a1_rho);
	    
	    if( BENCHMARK ){
	      std::cout << "-- after boost to a1-rho frame  " << std::endl;
	      std::cout<< "--- tau_a1 components --" << std::endl;
	      rho.print();
	      other_pi0.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    //normal to the plane spanned by rho-> pi+ pi-  
	    double n_1[3];
	    P_norm_cross_product(rho,other_pi0,n_1);
	    //normal to the plane spanned by rho2-> pi+ pi-  
	    double n_2[3];
	    P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);	
	    // calculate the acoplanarity angle
	    index++;
	    costheta[index] = dot_product(n_1,n_2);
	    theta[index] = P_acoplanarAngle(rho, other_pi0, rho2_1pi, rho2_2pi);

	    if( BENCHMARK ){
	      std::cout<< "--- normal to the tau_a1 plane --" << std::endl;
	      std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
	      std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
	      std::cout<< "--- cos(acoplanarity angle) --" << std::endl;
	      std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
	      std::cout<< "--- acoplanarity angle --" << std::endl;
	      std::cout<< "Theta = " <<  theta[index] << std::endl;
	    }

	    double theta4Pi = theta[index];
	    double thetaPi =   theta[index] -  kPI;
	    if (thetaPi < 0)  thetaPi = thetaPi + 2 * kPI;

	    double y1 = rho.e()/(rho.e()+other_pi0.e());
	    double x1 = (pow(tau_a1.recalculated_mass(),2)- pow(other_pi0.recalculated_mass(),2)+pow(rho.recalculated_mass(),2))  
	      /( 2 * pow(tau_a1.recalculated_mass(),2));
	    if( y1 > x1 ) y1 = -y1;

	    //	    double y1 = (rho.e()-other_pi0.e())/(rho.e()+other_pi0.e());

	    double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    	    
	    hmg->GetHistoTH1F(idhist+131)->Fill( theta[index], isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+132)->Fill( tau_a1.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+133)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+134)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+141)->Fill( thetaPi, isFilter * evtWeight );
	    theta4Pi = thetaPi;
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    hmg->GetHistoTH1F(idhist+144)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+154)->Fill( theta4Pi, isFilter * evtWeight );	    

	    double thetaFlip =   theta[index];
	    if( y1*y2 < 0)
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    if( y1*y2 > 0){
	      thetaFlip =   theta[index] - kPI;
	      if (thetaFlip < 0)  thetaFlip = thetaFlip + 2 * kPI;
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    }

	    
	  }
	}
	
       }
    }
    
}

//----------------------------------------------
void AnalTauSpinnerCP::test_1p2n_1p1n( int idhist, HepMC::GenEvent *hepmc, float evtWeight )
//----------------------------------------------
{

  HistoManager  *hmg = HistoManager::getInstance();
 
  BuildTauSpinnerBranch  *evtmg = BuildTauSpinnerBranch::getInstance();


  // prepare branch for TauSpinner weight
  SimpleParticle sp_X, sp_tau, sp_tau2; // SimpleParticle consist of 4 momentum and pdgId.
  vector<SimpleParticle> sp_tau_daughters, sp_tau2_daughters;

   // search for tau decay in the event
    int status =  evtmg->buildSimpleBranch(hepmc, sp_X, sp_tau, sp_tau2, sp_tau_daughters, sp_tau2_daughters);

    if(status == 2) {

      float accFlag = 0;
      hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

      hmg->GetHistoTH1F(idhist+1)->Fill( evtWeight, 1.0 );

      vector<Particle> tau_pi, tau_pi0,  tau2_pi ;
      vector<Particle> tau_rho, tau2_rho ;
      vector<Particle> tau_other_pi;
      vector< vector<Particle> > tau_rho_daughters, tau2_rho_daughters ;

      double etmiss_px = 0;
      double etmiss_py = 0;
 
     // Create list of tau daughters and reconstruct a1 resonances
      for(unsigned int i=0; i<sp_tau_daughters.size(); i++) {
	if(abs(sp_tau_daughters[i].pdgid()) == 16){
	  etmiss_px += sp_tau_daughters[i].px();
	  etmiss_py += sp_tau_daughters[i].py();
	}
	if( abs(sp_tau_daughters[i].pdgid()) == 211 ){
	  Particle pp(sp_tau_daughters[i].px(),
		      sp_tau_daughters[i].py(),
		      sp_tau_daughters[i].pz(),
		      sp_tau_daughters[i].e(),
		      sp_tau_daughters[i].pdgid() );
	  tau_pi.push_back(pp);
	}
	if( abs(sp_tau_daughters[i].pdgid()) == 111 ){
	  for(unsigned int ii=i+1; ii<sp_tau_daughters.size(); ii++) {
	    if( abs(sp_tau_daughters[ii].pdgid()) == 111 ){
		vector<Particle> temp;
		Particle p1(sp_tau_daughters[i].px() ,
			    sp_tau_daughters[i].py(),
			    sp_tau_daughters[i].pz(),
			    sp_tau_daughters[i].e(),
			    sp_tau_daughters[i].pdgid());
		temp.push_back(p1);
		tau_pi0.push_back(p1);
		Particle p2(sp_tau_daughters[ii].px() ,
			    sp_tau_daughters[ii].py(),
			    sp_tau_daughters[ii].pz(),
			    sp_tau_daughters[ii].e(),
			    sp_tau_daughters[ii].pdgid());
		temp.push_back(p2);
		tau_pi0.push_back(p2);
		Particle pp(sp_tau_daughters[i].px()+sp_tau_daughters[ii].px() ,
			    sp_tau_daughters[i].py()+sp_tau_daughters[ii].py(),
			    sp_tau_daughters[i].pz()+sp_tau_daughters[ii].pz(),
			    sp_tau_daughters[i].e() +sp_tau_daughters[ii].e(),
			    0 );
		tau_rho.push_back(pp);
		tau_rho_daughters.push_back(temp);
	    }
	  }
	}
      }
      if(DEBUG){
	std::cout << "tau_rho.size = " << tau_rho.size() << std::endl;
	if( tau_rho.size() > 0) {
	  std::cout << "tau_rho_daughters[0].size =" << tau_rho_daughters[0].size() << std::endl;
	  tau_rho_daughters[0][0].print();
	  tau_rho_daughters[0][1].print();
	}
	if( tau_rho.size() > 1) {
	  std::cout << "tau_rho_daughters[1].size =" << tau_rho_daughters[1].size() << std::endl;
	  tau_rho_daughters[1][0].print();
	  tau_rho_daughters[1][1].print();
	}
	for(unsigned int i=0; i<tau_pi.size(); i++) {
	  std::cout << "tau_pi.size =" << tau_pi.size() << std::endl;
	  tau_pi[i].print();
	}
      }

      // Create list of tau2 daughters and reconstruct rho resonances
      for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) {
	if(abs(sp_tau2_daughters[i].pdgid()) == 16){
	  etmiss_px += sp_tau2_daughters[i].px();
	  etmiss_py += sp_tau2_daughters[i].py();
	}
	if( abs(sp_tau2_daughters[i].pdgid()) == 211 ){
	  Particle pp(sp_tau2_daughters[i].px(),
		      sp_tau2_daughters[i].py(),
		      sp_tau2_daughters[i].pz(),
		      sp_tau2_daughters[i].e(),
		      sp_tau2_daughters[i].pdgid() );
	  tau2_pi.push_back(pp);
	  for(unsigned int ii=0; ii<sp_tau2_daughters.size(); ii++) {
	    if( abs(sp_tau2_daughters[ii].pdgid()) == 111 ){
	      vector<Particle> temp;
	      Particle p1(sp_tau2_daughters[i].px() ,
			  sp_tau2_daughters[i].py(),
			  sp_tau2_daughters[i].pz(),
			  sp_tau2_daughters[i].e(),
			  sp_tau2_daughters[i].pdgid());
	      temp.push_back(p1);
	      Particle p2(sp_tau2_daughters[ii].px() ,
			  sp_tau2_daughters[ii].py(),
			  sp_tau2_daughters[ii].pz(),
			  sp_tau2_daughters[ii].e(),
			  sp_tau2_daughters[ii].pdgid());
	      temp.push_back(p2);
	      tau2_pi.push_back(p2);
	      Particle pp(sp_tau2_daughters[i].px()+sp_tau2_daughters[ii].px() ,
			  sp_tau2_daughters[i].py()+sp_tau2_daughters[ii].py(),
			  sp_tau2_daughters[i].pz()+sp_tau2_daughters[ii].pz(),
			  sp_tau2_daughters[i].e() +sp_tau2_daughters[ii].e(),
			  0 );
	      tau2_rho.push_back(pp);
	      tau2_rho_daughters.push_back(temp);
	    }
	  }
	 }
       }
       if(DEBUG){
	 std::cout << "tau2_rho.size = " << tau2_rho.size() << std::endl;
	 if( tau2_rho.size() > 0) {
	   std::cout << "tau2_rho_daughters[0].size =" << tau2_rho_daughters[0].size() << std::endl;
	   tau2_rho_daughters[0][0].print();
	   tau2_rho_daughters[0][1].print();
	 }
       }

       //       std::cout << "a1rho events: " << tau_pi.size() << " " <<  tau2_rho.size() << std::endl;
       
       if( tau_pi.size() == 1 &&  tau2_rho.size() == 1 ){
	 
	 
	 float accFlag = 1;
	 hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );
	 
	 if( outNNlab ){
	   printf("TUPLE  %lf  \n", evtWeight);
	   for(unsigned int i=0; i<sp_tau_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau_daughters[i].px(), sp_tau_daughters[i].py(), sp_tau_daughters[i].pz(), sp_tau_daughters[i].e(), sp_tau_daughters[i].pdgid());
	   for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau2_daughters[i].px(), sp_tau2_daughters[i].py(), sp_tau2_daughters[i].pz(), sp_tau2_daughters[i].e(), sp_tau2_daughters[i].pdgid());
	   //	  return;
	 }
	 
	 // Build the a1 resonances
	 Particle tau_a1 ( tau_pi[0].px()+tau_pi0[0].px()+tau_pi0[1].px(), 
			   tau_pi[0].py()+tau_pi0[0].py()+tau_pi0[1].py(),
			   tau_pi[0].pz()+tau_pi0[0].pz()+tau_pi0[1].pz(),
			   tau_pi[0].e() +tau_pi0[0].e() +tau_pi0[1].e(), 0.0);
	 
	 // Build the a1-rho system resonances
	 Particle a1rho ( tau_a1.px()+tau2_rho[0].px(), 
			  tau_a1.py()+tau2_rho[0].py(),
			  tau_a1.pz()+tau2_rho[0].pz(),
			  tau_a1.e() +tau2_rho[0].e(), 0.0);	 
	 if( BENCHMARK ){
	   std::cout<< "--- tau_a1 --" << std::endl;
	   tau_a1.print();
	   std::cout<< "--- tau2_rho --" << std::endl;
	   tau2_rho[0].print();
	   std::cout<< "--- tau_a1 tau2_rho system--" << std::endl;
	   a1rho.print();
	 }
	 
	int isFilter =1;
	double tau_a1_pTlab = sqrt( tau_a1.px()*tau_a1.px()+ tau_a1.py()*tau_a1.py() );
	double tau2_rho_pTlab = sqrt( tau2_rho[0].px()*tau2_rho[0].px()+ tau2_rho[0].py()*tau2_rho[0].py() );
	if(  tau_a1_pTlab < 20.0 ||  tau2_rho_pTlab < 20.0 ) isFilter =0;
	for(unsigned int i=0; i<tau_pi.size(); i++) {
	  double pT = sqrt( tau_pi[i].px()* tau_pi[i].px()+ tau_pi[i].py()* tau_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        
	for(unsigned int i=0; i<tau2_rho_daughters.size(); i++) {
	  double pT = sqrt( tau2_pi[i].px()* tau2_pi[i].px()+ tau2_pi[i].py()* tau2_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        

	if( BENCHMARK )
	  std::cout<< "--- filter = " <<  isFilter << std::endl;


	// calculate angles between normal to rho planes
	double  costheta[4], theta[4];
	int index = -1;

	if( BENCHMARK )
	  std::cout<< "--- loping over possible configuration of the rho-rho pairs-- "<< std::endl;

	for(int i = 0; i< tau_rho.size() ; i++){
	  for(int ii = 0; ii< tau2_rho.size() ; ii++){
	    
	    if( BENCHMARK ){
	      std::cout << " " << std::endl;
	      std::cout << " --- NEXT rho-rho pair --" << std::endl;
	      std::cout << " " << std::endl;
	    }
	    
	    
	    Particle rho_1pi(
			     tau_rho_daughters[i][0].px(),
			     tau_rho_daughters[i][0].py(),
			     tau_rho_daughters[i][0].pz(),
			     tau_rho_daughters[i][0].e(),0);
	    Particle rho_2pi(
			     tau_rho_daughters[i][1].px(),
			     tau_rho_daughters[i][1].py(),
			     tau_rho_daughters[i][1].pz(),
			     tau_rho_daughters[i][1].e(),0);
	    Particle rho(
			 tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			 tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			 tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			 tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
	    Particle other_pi(
			      tau_pi[i].px(),
			      tau_pi[i].py(),
			      tau_pi[i].pz(),
			      tau_pi[i].e(),0);
	    Particle rho2_1pi(
			      tau2_rho_daughters[ii][0].px(),
			      tau2_rho_daughters[ii][0].py(),
			      tau2_rho_daughters[ii][0].pz(),
			      tau2_rho_daughters[ii][0].e(),0);
	    Particle rho2_2pi(
			      tau2_rho_daughters[ii][1].px(),
			      tau2_rho_daughters[ii][1].py(),
			      tau2_rho_daughters[ii][1].pz(),
			      tau2_rho_daughters[ii][1].e(),0);
	    Particle rho2(
			  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    Particle rho_rho (
			      tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px() +  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			      tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py() +  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			      tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz() +  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			      tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e()  +  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    
	    if( BENCHMARK ){
	      std::cout << "-- still in the LAB frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    // acoplanarity between rho planes
	    rho_1pi.boostToRestFrame(rho_rho);
	    rho_2pi.boostToRestFrame(rho_rho);
	    rho2_1pi.boostToRestFrame(rho_rho);
	    rho2_2pi.boostToRestFrame(rho_rho);

	    if( BENCHMARK ){	    
	      std::cout << "-- after boost to rho-rho frame  " << std::endl;
	      std::cout<< "--- tau_rho components --" << std::endl;
	      rho_1pi.print();
	      rho_2pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    //normal to the plane spanned by rho-> pi+ pi-  
	    double n_1[3];
	    P_norm_cross_product(rho_1pi,rho_2pi,n_1);
	    //normal to the plane spanned by rho2-> pi+ pi0  
	    double n_2[3];
	    P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);	
	    // calculate the acoplanarity angle
	    index++;
	    costheta[index] = dot_product(n_1,n_2);
	    theta[index] = P_acoplanarAngle(rho_1pi, rho_2pi, rho2_1pi, rho2_2pi);

	    if( BENCHMARK ){
	      std::cout<< "--- normal to the tau_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
	      std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
	      std::cout<< "--- cos( acoplanarity angle ) --" << std::endl;
	      std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
	      std::cout<< "--- acoplanarity angle --" << std::endl;
	      std::cout<< "Theta = " <<  theta[index] << std::endl;
	    }

	    double theta4Pi = theta[index];
	    double y1 = (rho_1pi.e()-rho_2pi.e())/(rho_1pi.e()+rho_2pi.e());
	    double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    
	    hmg->GetHistoTH1F(idhist+121)->Fill( theta[index], isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+122)->Fill( rho.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+123)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+124)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+154)->Fill( theta4Pi, isFilter * evtWeight );

	    double thetaFlip =   theta[index];
	    if( y1*y2 > 0)
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    if( y1*y2 < 0){
	      thetaFlip =   theta[index] - kPI;
	      if (thetaFlip < 0)  thetaFlip = thetaFlip + 2 * kPI;
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    }
	    
	  }
	}
    
	
	if( BENCHMARK )
	  std::cout<< "--- looping over possible configuration of the a1-rho  pairs-- "<< std::endl;
	
	for(int i = 0; i<tau_rho.size(); i++){
	  for(int ii = 0; ii<tau2_rho.size(); ii++){
	    
	    if( BENCHMARK ){	    
	      std::cout << " " << std::endl;
	      std::cout << " --- NEXT a1-rho pair --" << std::endl;
	      std::cout << " " << std::endl;
	    }
	    
	    Particle rho_1pi(
			     tau_rho_daughters[i][0].px(),
			     tau_rho_daughters[i][0].py(),
			     tau_rho_daughters[i][0].pz(),
			     tau_rho_daughters[i][0].e(),0);
	    Particle rho_2pi(
			     tau_rho_daughters[i][1].px(),
			     tau_rho_daughters[i][1].py(),
			     tau_rho_daughters[i][1].pz(),
			     tau_rho_daughters[i][1].e(),0);
	    Particle rho(
			 tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			 tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			 tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			 tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
	    Particle other_pi(
			      tau_pi[i].px(),
			      tau_pi[i].py(),
			      tau_pi[i].pz(),
			      tau_pi[i].e(),0);
	    Particle rho2_1pi(
			      tau2_rho_daughters[ii][0].px(),
			      tau2_rho_daughters[ii][0].py(),
			      tau2_rho_daughters[ii][0].pz(),
			      tau2_rho_daughters[ii][0].e(),0);
	    Particle rho2_2pi(
			      tau2_rho_daughters[ii][1].px(),
			      tau2_rho_daughters[ii][1].py(),
			      tau2_rho_daughters[ii][1].pz(),
			      tau2_rho_daughters[ii][1].e(),0);
	    Particle rho2(
			  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	    Particle a1_rho (
			     tau_a1.px() +  tau2_rho[ii].px(),
			     tau_a1.py() +  tau2_rho[ii].py(),
			     tau_a1.pz() +  tau2_rho[ii].pz(),
			     tau_a1.e()  +  tau2_rho[ii].e(), 0);
	    
	    if( BENCHMARK ){
	      std::cout << "-- still in the LAB frame  " << std::endl;
	      std::cout<< "--- tau_a1 components --" << std::endl;
	      rho.print();
	      other_pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    // acoplanarity between a1-rho planes
	    rho.boostToRestFrame(a1_rho);
	    other_pi.boostToRestFrame(a1_rho);
	    rho2_1pi.boostToRestFrame(a1_rho);
	    rho2_2pi.boostToRestFrame(a1_rho);
	    
	    if( BENCHMARK ){
	      std::cout << "-- after boost to a1-rho frame  " << std::endl;
	      std::cout<< "--- tau_a1 components --" << std::endl;
	      rho.print();
	      other_pi.print();
	      std::cout<< "--- tau2_rho components --" << std::endl;
	      rho2_1pi.print();
	      rho2_2pi.print();
	    }
	    
	    //normal to the plane spanned by rho-> pi+ pi-  
	    double n_1[3];
	    P_norm_cross_product(rho,other_pi,n_1);
	    //normal to the plane spanned by rho2-> pi+ pi-  
	    double n_2[3];
	    P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);	
	    // calculate the acoplanarity angle
	    index++;
	    costheta[index] = dot_product(n_1,n_2);
	    theta[index] = P_acoplanarAngle(rho, other_pi, rho2_1pi, rho2_2pi);

	    if( BENCHMARK ){
	      std::cout<< "--- normal to the tau_a1 plane --" << std::endl;
	      std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
	      std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
	      std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
	      std::cout<< "--- cos(acoplanarity angle) --" << std::endl;
	      std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
	      std::cout<< "--- acoplanarity angle --" << std::endl;
	      std::cout<< "Theta = " <<  theta[index] << std::endl;
	    }

	    double theta4Pi = theta[index];
	    double thetaPi =   theta[index] -  kPI;
	    if (thetaPi < 0)  thetaPi = thetaPi + 2 * kPI;


	    double y1 = rho.e()/(rho.e()+other_pi.e());
	    double x1 = (pow(tau_a1.recalculated_mass(),2)- pow(other_pi.recalculated_mass(),2)+pow(rho.recalculated_mass(),2))  
	      /( 2 * pow(tau_a1.recalculated_mass(),2));
	    if( y1 > x1 ) y1 = -y1;

	    //	    double y1 = (rho.e()-other_pi.e())/(rho.e()+other_pi.e());

	    double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    	    
	    hmg->GetHistoTH1F(idhist+131)->Fill( theta[index], isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+132)->Fill( tau_a1.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+133)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );
	    hmg->GetHistoTH1F(idhist+134)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+141)->Fill( thetaPi, isFilter * evtWeight );
	    theta4Pi = thetaPi;
	    if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;
	    hmg->GetHistoTH1F(idhist+144)->Fill( theta4Pi, isFilter * evtWeight );

	    hmg->GetHistoTH1F(idhist+154)->Fill( theta4Pi, isFilter * evtWeight );	    

	    double thetaFlip =   theta[index];
	    if( y1*y2 < 0)
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    if( y1*y2 > 0){
	      thetaFlip =   theta[index] - kPI;
	      if (thetaFlip < 0)  thetaFlip = thetaFlip + 2 * kPI;
	      hmg->GetHistoTH1F(idhist+151)->Fill( thetaFlip, isFilter * evtWeight );
	    }
	    
	  }
	}
	
       }
    }
    
}

// here only to get estimate on neutrinos momenta
//----------------------------------------------
void AnalTauSpinnerCP::NeusA1Rho( int idhist, HepMC::GenEvent *hepmc, float evtWeight )
//----------------------------------------------
{

  HistoManager  *hmg = HistoManager::getInstance();
 
  BuildTauSpinnerBranch  *evtmg = BuildTauSpinnerBranch::getInstance();


  // prepare branch for TauSpinner weight
  SimpleParticle sp_X, sp_tau, sp_tau2; // SimpleParticle consist of 4 momentum and pdgId.
  vector<SimpleParticle> sp_tau_daughters, sp_tau2_daughters;

   // search for tau decay in the event
    int status =  evtmg->buildSimpleBranch(hepmc, sp_X, sp_tau, sp_tau2, sp_tau_daughters, sp_tau2_daughters);

    if(status == 2) {

      float accFlag = 0;
      hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

      hmg->GetHistoTH1F(idhist+1)->Fill( evtWeight, 1.0 );

      vector<Particle> tau_nu,  tau2_nu ;
      vector<Particle> tau_pi,  tau2_pi ;
      vector<Particle> tau_rho, tau2_rho ;
      vector<Particle> tau_other_pi;
      vector< vector<Particle> > tau_rho_daughters, tau2_rho_daughters ;

      double etmiss_px = 0;
      double etmiss_py = 0;
 
     // Create list of tau daughters and reconstruct a1 resonances
      for(unsigned int i=0; i<sp_tau_daughters.size(); i++) {
	if(abs(sp_tau_daughters[i].pdgid()) == 16){
	  etmiss_px += sp_tau_daughters[i].px();
	  etmiss_py += sp_tau_daughters[i].py();
	  Particle pp(sp_tau_daughters[i].px(),
		      sp_tau_daughters[i].py(),
		      sp_tau_daughters[i].pz(),
		      sp_tau_daughters[i].e(),
		      sp_tau_daughters[i].pdgid() );
	  tau_nu.push_back(pp);
	}
	if( abs(sp_tau_daughters[i].pdgid()) == 211 ){
	  Particle pp(sp_tau_daughters[i].px(),
		      sp_tau_daughters[i].py(),
		      sp_tau_daughters[i].pz(),
		      sp_tau_daughters[i].e(),
		      sp_tau_daughters[i].pdgid() );
	  tau_pi.push_back(pp);
	  for(unsigned int ii=0; ii<sp_tau_daughters.size(); ii++) {
	    if( abs(sp_tau_daughters[ii].pdgid()) == 211 ){
	      if( sp_tau_daughters[i].pdgid() * sp_tau_daughters[ii].pdgid()< 0 && ii > i ){
		vector<Particle> temp;
		Particle p1(sp_tau_daughters[i].px() ,
			    sp_tau_daughters[i].py(),
			    sp_tau_daughters[i].pz(),
			    sp_tau_daughters[i].e(),
			    sp_tau_daughters[i].pdgid());
		temp.push_back(p1);
		Particle p2(sp_tau_daughters[ii].px() ,
			    sp_tau_daughters[ii].py(),
			    sp_tau_daughters[ii].pz(),
			    sp_tau_daughters[ii].e(),
			    sp_tau_daughters[ii].pdgid());
		temp.push_back(p2);
		Particle pp(sp_tau_daughters[i].px()+sp_tau_daughters[ii].px() ,
			    sp_tau_daughters[i].py()+sp_tau_daughters[ii].py(),
			    sp_tau_daughters[i].pz()+sp_tau_daughters[ii].pz(),
			    sp_tau_daughters[i].e() +sp_tau_daughters[ii].e(),
			    0 );
		tau_rho.push_back(pp);
		tau_rho_daughters.push_back(temp);
	      } else if ( ii != i ){
		Particle p(sp_tau_daughters[ii].px() ,
			   sp_tau_daughters[ii].py(),
			   sp_tau_daughters[ii].pz(),
			   sp_tau_daughters[ii].e(),
			   sp_tau_daughters[ii].pdgid());
		tau_other_pi.push_back(p);
	      }
	    }
	  }
	}
      }
       if(DEBUG){
	std::cout << "tau_rho.size = " << tau_rho.size() << std::endl;
	if( tau_rho.size() > 0) {
	  std::cout << "tau_rho_daughters[0].size =" << tau_rho_daughters[0].size() << std::endl;
	  tau_rho_daughters[0][0].print();
	  tau_rho_daughters[0][1].print();
	}
	if( tau_rho.size() > 1) {
	  std::cout << "tau_rho_daughters[1].size =" << tau_rho_daughters[1].size() << std::endl;
	  tau_rho_daughters[1][0].print();
	  tau_rho_daughters[1][1].print();
	}
	for(unsigned int i=0; i<tau_other_pi.size(); i++) {
	  std::cout << "tau_other_pi.size =" << tau_other_pi.size() << std::endl;
	  tau_other_pi[i].print();
	}
      }

       // Create list of tau2 daughters and reconstruct rho resonances
       for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) {
	 if(abs(sp_tau2_daughters[i].pdgid()) == 16){
	   etmiss_px += sp_tau2_daughters[i].px();
	   etmiss_py += sp_tau2_daughters[i].py();
	  Particle pp(sp_tau2_daughters[i].px(),
		      sp_tau2_daughters[i].py(),
		      sp_tau2_daughters[i].pz(),
		      sp_tau2_daughters[i].e(),
		      sp_tau2_daughters[i].pdgid() );
	  tau2_nu.push_back(pp);
	 }
	 if( abs(sp_tau2_daughters[i].pdgid()) == 211 ){
	   Particle pp(sp_tau2_daughters[i].px(),
		       sp_tau2_daughters[i].py(),
		       sp_tau2_daughters[i].pz(),
		       sp_tau2_daughters[i].e(),
		       sp_tau2_daughters[i].pdgid() );
	   tau2_pi.push_back(pp);
	   for(unsigned int ii=0; ii<sp_tau2_daughters.size(); ii++) {
	     if( abs(sp_tau2_daughters[ii].pdgid()) == 111 ){
	       vector<Particle> temp;
	       Particle p1(sp_tau2_daughters[i].px() ,
			   sp_tau2_daughters[i].py(),
			   sp_tau2_daughters[i].pz(),
			   sp_tau2_daughters[i].e(),
			   sp_tau2_daughters[i].pdgid());
	       temp.push_back(p1);
	       Particle p2(sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[ii].e(),
			   sp_tau2_daughters[ii].pdgid());
	       temp.push_back(p2);
	       tau2_pi.push_back(p2);
	       Particle pp(sp_tau2_daughters[i].px()+sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[i].py()+sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[i].pz()+sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[i].e() +sp_tau2_daughters[ii].e(),
			   0 );
	       tau2_rho.push_back(pp);
	       tau2_rho_daughters.push_back(temp);
	     }
	   }
	 }
       }
       if(DEBUG){
	 std::cout << "tau2_rho.size = " << tau2_rho.size() << std::endl;
	 if( tau2_rho.size() > 0) {
	   std::cout << "tau2_rho_daughters[0].size =" << tau2_rho_daughters[0].size() << std::endl;
	   tau2_rho_daughters[0][0].print();
	   tau2_rho_daughters[0][1].print();
	 }
       }

       if( tau_pi.size() == 3 &&  tau2_rho.size() == 1 ){
	 
	 
	 float accFlag = 1;
	 hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );
	 
	 if( outNNlab ){
	   printf("TUPLE  %lf  \n", evtWeight);
	   for(unsigned int i=0; i<sp_tau_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau_daughters[i].px(), sp_tau_daughters[i].py(), sp_tau_daughters[i].pz(), sp_tau_daughters[i].e(), sp_tau_daughters[i].pdgid());
	   for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) 
	     printf(" %lf  %lf  %lf  %lf %d\n", 
		    sp_tau2_daughters[i].px(), sp_tau2_daughters[i].py(), sp_tau2_daughters[i].pz(), sp_tau2_daughters[i].e(), sp_tau2_daughters[i].pdgid());
	   //	  return;
	 }
	 
	 // Build the a1 resonances
	 Particle tau_a1 ( tau_pi[0].px()+tau_pi[1].px()+tau_pi[2].px(), 
			   tau_pi[0].py()+tau_pi[1].py()+tau_pi[2].py(),
			   tau_pi[0].pz()+tau_pi[1].pz()+tau_pi[2].pz(),
			   tau_pi[0].e() +tau_pi[1].e() +tau_pi[2].e(), 0.0);
	 
	 // Build the a1-rho system resonances
	 Particle a1rho ( tau_a1.px()+tau2_rho[0].px(), 
			  tau_a1.py()+tau2_rho[0].py(),
			  tau_a1.pz()+tau2_rho[0].pz(),
			  tau_a1.e() +tau2_rho[0].e(), 0.0);	 
	 if( BENCHMARK ){
	   std::cout<< "NeusA1Rho: LAB frame --- tau_a1 --" << std::endl;
	   tau_a1.print();
	   std::cout<< "NeusA1Rho: LAB frame --- tau_nu --" << std::endl;
	   tau_nu[0].print();
	   std::cout<< "NeusA1Rho: LAB frame --- tau2_rho --" << std::endl;
	   tau2_rho[0].print();
	   std::cout<< "NeusA1Rho: LAB frame --- tau2_nu --" << std::endl;
	   tau2_nu[0].print();
	   std::cout<< "NeusA1Rho: LAB frame --- tau_a1 tau2_rho system--" << std::endl;
	   a1rho.print();
	 }

	 	 
	int isFilter =1;
	double tau_a1_pTlab = sqrt( tau_a1.px()*tau_a1.px()+ tau_a1.py()*tau_a1.py() );
	double tau2_rho_pTlab = sqrt( tau2_rho[0].px()*tau2_rho[0].px()+ tau2_rho[0].py()*tau2_rho[0].py() );
	if(  tau_a1_pTlab < 20.0 ||  tau2_rho_pTlab < 20.0 ) isFilter =0;
	for(unsigned int i=0; i<tau_pi.size(); i++) {
	  double pT = sqrt( tau_pi[i].px()* tau_pi[i].px()+ tau_pi[i].py()* tau_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        
	for(unsigned int i=0; i<tau2_rho_daughters.size(); i++) {
	  double pT = sqrt( tau2_pi[i].px()* tau2_pi[i].px()+ tau2_pi[i].py()* tau2_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        

	if( BENCHMARK )
	  std::cout<< "--- filter = " <<  isFilter << std::endl;

	Particle tau_h  =  tau_a1;
	Particle tau2_h =  tau2_rho[0];
	
	 // boost and rotate to the a1-rho frame
	 Particle btau_h =  tau_a1;
	 btau_h.boostToRestFrame(a1rho);
	 double phi = btau_h.getAnglePhi();
	 btau_h.rotateXY(-phi);
	 double theta   = btau_h.getAngleTheta();
	 btau_h.rotateXZ(kPI-theta);

	 Particle btau_nu =  tau_nu[0];
	 btau_nu.boostToRestFrame(a1rho);
	 btau_nu.rotateXY(-phi);
	 btau_nu.rotateXZ(kPI-theta);
	 
	 Particle btau2_nu =  tau2_nu[0];
	 btau2_nu.boostToRestFrame(a1rho);
	 btau2_nu.rotateXY(-phi);
	 btau2_nu.rotateXZ(kPI-theta);
	 
	 Particle btau2_h =  tau2_rho[0];
	 btau2_h.boostToRestFrame(a1rho);
	 btau2_h.rotateXY(-phi);
	 btau2_h.rotateXZ(kPI-theta);

	 if( BENCHMARK ){
	   std::cout<< "NeusA1Rho: a1rho frame --- tau_a1 --" << std::endl;
	   btau_h.print();
	   std::cout<< "NeusA1Rho: a1rho frame --- tau_nu --" << std::endl;
	   btau_nu.print();
	   std::cout<< "NeusA1Rho: a1rho frame --- tau2_rho --" << std::endl;
	   btau2_h.print();
	   std::cout<< "NeusA1Rho: a1rho frame --- tau2_nu --" << std::endl;
	   btau2_nu.print();
	   std::cout<< "NeusA1Rho: a1rho frame --- tau_a1 tau2_rho system--" << std::endl;
	 }



	 // calculate exact a1, a2
 
	 double alf1_cms = btau_h.pz()/(btau_h.pz()+btau_nu.pz());
	 double alf2_cms = btau2_h.pz()/(btau2_h.pz()+btau2_nu.pz());

	 double tau_nu_length = sqrt( tau_nu[0].px()*tau_nu[0].px() + tau_nu[0].py()*tau_nu[0].py()+ tau_nu[0].pz()*tau_nu[0].pz());
	 double tau_h_length  = sqrt( tau_h.px()*tau_h.px() + tau_h.py()*tau_h.py()+ tau_h.pz()*tau_h.pz() );

	 double tau2_nu_length = sqrt( tau2_nu[0].px()*tau2_nu[0].px() + tau2_nu[0].py()*tau2_nu[0].py()+ tau2_nu[0].pz()*tau2_nu[0].pz());
	 double tau2_h_length  = sqrt( tau2_h.px()*tau2_h.px() + tau2_h.py()*tau2_h.py()+ tau2_h.pz()*tau2_h.pz() );

	 double alf1_lab = tau_h_length/(tau_nu_length+tau_h_length);
	 double alf2_lab = tau2_h_length/(tau2_nu_length+tau2_h_length);

	 double x1_cms = 1./(1+alf1_cms);
	 double x2_cms = 1./(1+alf2_cms);

	 double x1_lab = 1./(1+alf1_lab);
	 double x2_lab = 1./(1+alf2_lab);
	 
 
         // calculate approximate a1, a2 using collinear approximation, calculation in the laboratory frame
         // but then will be used in the trust frame, should be the same
       
	 // define ETmissX, ETmissY
	 double ETmissX = tau_nu[0].px()+tau2_nu[0].px();
	 double ETmissY = tau_nu[0].py()+tau2_nu[0].py();
	 // here for tests only
         // double ETmissX = alf1_lab*tau_h.px()+alf2_lab*tau2_h.px();
	 // double ETmissY = alf1_lab*tau_h.py()+alf2_lab*tau2_h.py();

         // define masses
	 double mH   = 125.0;
	 double mtau = 1.77;
	 // here for tests only
         // double mtau = 0.0;
         // double mH   = sqrt(2*(1+alf1_lab)*(1+alf2_lab)* (tau_h.e()*tau2_h.e()-tau_h.px()*tau2_h.px()-tau_h.py()*tau2_h.py()-tau_h.pz()*tau2_h.pz()));

	 double alf2_approx, alf1_approx;
	 
	 // option A  (using ETmissX, ETmissY )
	 if( NuesOption == 1 ) {
          alf2_approx = ( -ETmissX*tau_h.py() + ETmissY * tau_h.px())/( tau_h.px()*tau2_h.py()-tau2_h.px()*tau_h.py() );
          alf1_approx = (  ETmissX - alf2_approx * tau2_h.px())/tau_h.px();
	 }

         // option B (using ETmissX, ETmissY, mH, mtau )
	 if( NuesOption == 2 ) {
	   alf2_approx = ( -ETmissX*tau_h.py() + ETmissY * tau_h.px())/( tau_h.px()*tau2_h.py()-tau2_h.px()*tau_h.py() );
	   alf1_approx = (mH*mH/2. - mtau*mtau)/(tau_h.e()*tau2_h.e()-tau_h.px()*tau2_h.px()-tau_h.py()*tau2_h.py()-tau_h.pz()*tau2_h.pz())/(1+alf2_approx)-1;
	 }

	 // option C (using ETmissX, ETmissY, mH, mtau )
	 if( NuesOption == 3 ){
	   alf1_approx = ( -ETmissX*tau2_h.py() + ETmissY * tau2_h.px())/( tau2_h.px()*tau_h.py()-tau_h.px()*tau2_h.py() );
	   alf2_approx = (mH*mH/2. - mtau*mtau)/(tau_h.e()*tau2_h.e()-tau_h.px()*tau2_h.px()-tau_h.py()*tau2_h.py()-tau_h.pz()*tau2_h.pz())/(1+alf1_approx)-1;
	 }

	 if( BENCHMARK ){
	  std::cout<< "NeusA1Rho: --- alf1_lab, alf2_lab       = " <<  alf1_lab << "  " << alf2_lab << std::endl;
	  std::cout<< "NeusA1Rho: --- alf1_cms, alf2_cms       = " <<  alf1_cms << "  " << alf2_cms << std::endl;
	  std::cout<< "NeusA1Rho: --- alf1_approx, alf2_approx = " <<  alf1_approx << "  " << alf2_approx << std::endl;
	 }

	 
	 double x1_approx = 1./(1+alf1_approx);
	 double x2_approx = 1./(1+alf2_approx);

	 // reject events if no solution found
	 //	 if( x1_lab < 0 ||  x2_lab < 0 ) isFilter = 0;	 
	 //	 if( x1_lab > 1 ||  x2_lab > 1 ) isFilter = 0;	 
	 //	 if( x1_cms < 0 ||  x2_cms < 0 ) isFilter = 0;	 
	 //	 if( x1_cms > 1 ||  x2_cms > 1 ) isFilter = 0;	 
	 if( x1_approx > 1 ||  x2_approx > 1 ) isFilter = 0;	 

	 // here monitor correlations
	 hmg->GetHistoTH2F(idhist+161)->Fill(x1_cms, x1_approx, isFilter * evtWeight );
	 hmg->GetHistoTH2F(idhist+162)->Fill(x2_cms, x2_approx, isFilter * evtWeight );
	 hmg->GetHistoTH2F(idhist+171)->Fill(x1_lab, x1_approx, isFilter * evtWeight );
	 hmg->GetHistoTH2F(idhist+172)->Fill(x2_lab, x2_approx, isFilter * evtWeight );
	 hmg->GetHistoTH2F(idhist+173)->Fill(x1_lab, x1_cms, isFilter * evtWeight );
	 hmg->GetHistoTH2F(idhist+174)->Fill(x2_lab, x2_cms, isFilter * evtWeight );

         // here is longitudinal component 
	 double btau_nu_long  = alf1_approx * btau_h.pz();
	 double btau2_nu_long = alf2_approx * btau2_h.pz();
	 
	 // here monitor correlations
	 hmg->GetHistoTH2F(idhist+163)->Fill(btau_nu.pz(),  btau_nu_long,  isFilter * evtWeight );
	 hmg->GetHistoTH2F(idhist+164)->Fill(btau2_nu.pz(),  btau2_nu_long, isFilter * evtWeight );

	 if( BENCHMARK ){
	   std::cout<< "NeusA1Rho: --- btau_nu_pz,  btau2_nu_pz       = " << btau_nu.pz()  << "  " << btau2_nu.pz() << std::endl;
	   std::cout<< "NeusA1Rho: --- btau_nu_long,  btau2_nu_long       = " << btau_nu_long  << "  " << btau2_nu_long << std::endl;
	 }

         // here is energy component 
	 double btau_nu_E   = 1/(2 * btau_h.e())*(mtau*mtau -  btau_h.e()* btau_h.e() + 2*btau_nu_long*btau_h.pz() + btau_h.pz()* btau_h.pz() );
	 double btau2_nu_E  = 1/(2 * btau2_h.e())*(mtau*mtau -  btau2_h.e()* btau2_h.e() + 2*btau2_nu_long*btau2_h.pz() + btau2_h.pz()* btau2_h.pz() );

	 if( btau_nu_E < 0 || btau2_nu_E < 0 )  isFilter=0;
	 // here  monitor correlations
	 hmg->GetHistoTH2F(idhist+165)->Fill(btau_nu.e(),  btau_nu_E,  isFilter * evtWeight );
	 hmg->GetHistoTH2F(idhist+166)->Fill(btau2_nu.e(), btau2_nu_E, isFilter * evtWeight );

	 if( BENCHMARK ){
	   std::cout<< "NeusA1Rho: --- btau_nu.e,  btau2_nu.e       = " << btau_nu.e()  << "  " << btau2_nu.e() << std::endl;
	   std::cout<< "NeusA1Rho: --- btau_nu_E,  btau2_nu_E       = " << btau_nu_E  << "  " << btau2_nu_E << std::endl;
	 }

         // here is transverse component
	 double btau_nu_pT  = sqrt( btau_nu.px()*btau_nu.px() + btau_nu.py()*btau_nu.py() );
	 double btau2_nu_pT = sqrt( btau2_nu.px()*btau2_nu.px() + btau2_nu.py()*btau2_nu.py() );

	 double btau_nu_trans = 0;
	 if(  btau_nu_E*btau_nu_E > btau_nu_long * btau_nu_long )
	   btau_nu_trans = sqrt( btau_nu_E*btau_nu_E - btau_nu_long * btau_nu_long);
	 double btau2_nu_trans = 0;
	 if(  btau2_nu_E*btau2_nu_E > btau2_nu_long * btau2_nu_long )
	   btau2_nu_trans = sqrt( btau2_nu_E*btau2_nu_E - btau2_nu_long * btau2_nu_long);

	 if(  btau_nu_E*btau_nu_E <  btau_nu_long * btau_nu_long ||  btau2_nu_E*btau2_nu_E <  btau2_nu_long * btau2_nu_long )   isFilter=0;

	 // here monitor correlations
	 hmg->GetHistoTH2F(idhist+167)->Fill(btau_nu_pT,  btau_nu_trans,  isFilter * evtWeight );
	 hmg->GetHistoTH2F(idhist+168)->Fill(btau2_nu_pT, btau2_nu_trans, isFilter * evtWeight );

	 if( BENCHMARK ){
	   std::cout<< "NeusA1Rho: --- btau_nu_pT,  btau2_nu_pT       = " << btau_nu_pT  << "  " << btau2_nu_pT << std::endl;
	   std::cout<< "NeusA1Rho: --- btau_nu_trans,  btau2_nu_trans       = " << btau_nu_trans  << "  " << btau2_nu_trans << std::endl;
	 }


	 // fill to the NN: btau_nu_trans, btau2_nu_trans, btau_nu_E, btau2_nu_E, btau_nu_long, btau2_nu_long

       }
    }
}

//----------------------------------------------
void AnalTauSpinnerCP::testA1A1( int idhist, HepMC::GenEvent *hepmc, float evtWeight )
//----------------------------------------------
{

  HistoManager  *hmg = HistoManager::getInstance();
 
  BuildTauSpinnerBranch  *evtmg = BuildTauSpinnerBranch::getInstance();


  // prepare branch for TauSpinner weight
  SimpleParticle sp_X, sp_tau, sp_tau2; // SimpleParticle consist of 4 momentum and pdgId.
  vector<SimpleParticle> sp_tau_daughters, sp_tau2_daughters;

   // search for tau decay in the event
    int status =  evtmg->buildSimpleBranch(hepmc, sp_X, sp_tau, sp_tau2, sp_tau_daughters, sp_tau2_daughters);

    if(status == 2) {

      float accFlag = 0;
      hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );

      hmg->GetHistoTH1F(idhist+1)->Fill( evtWeight, 1.0 );

      vector<Particle> tau_pi,  tau2_pi ;
      vector<Particle> tau_rho, tau2_rho ;
      vector<Particle> tau_other_pi, tau2_other_pi ;
      vector< vector<Particle> > tau_rho_daughters, tau2_rho_daughters ;

      double etmiss_px = 0;
      double etmiss_py = 0;

      // Create list of tau daughters and reconstruct a1 resonances
      for(unsigned int i=0; i<sp_tau_daughters.size(); i++) {
	if(abs(sp_tau_daughters[i].pdgid()) == 16){
	  etmiss_px += sp_tau_daughters[i].px();
	  etmiss_py += sp_tau_daughters[i].py();
	}
	if( abs(sp_tau_daughters[i].pdgid()) == 211 ){
	  Particle pp(sp_tau_daughters[i].px(),
		      sp_tau_daughters[i].py(),
		      sp_tau_daughters[i].pz(),
		      sp_tau_daughters[i].e(),
		      sp_tau_daughters[i].pdgid() );
	  tau_pi.push_back(pp);
	  for(unsigned int ii=0; ii<sp_tau_daughters.size(); ii++) {
	    if( abs(sp_tau_daughters[ii].pdgid()) == 211 ){
	      if( sp_tau_daughters[i].pdgid() * sp_tau_daughters[ii].pdgid()< 0 && ii > i ){
		vector<Particle> temp;
		Particle p1(sp_tau_daughters[i].px() ,
			    sp_tau_daughters[i].py(),
			    sp_tau_daughters[i].pz(),
			    sp_tau_daughters[i].e(),
			    sp_tau_daughters[i].pdgid());
		temp.push_back(p1);
		Particle p2(sp_tau_daughters[ii].px() ,
			    sp_tau_daughters[ii].py(),
			    sp_tau_daughters[ii].pz(),
			    sp_tau_daughters[ii].e(),
			    sp_tau_daughters[ii].pdgid());
		temp.push_back(p2);
		Particle pp(sp_tau_daughters[i].px()+sp_tau_daughters[ii].px() ,
			    sp_tau_daughters[i].py()+sp_tau_daughters[ii].py(),
			    sp_tau_daughters[i].pz()+sp_tau_daughters[ii].pz(),
			    sp_tau_daughters[i].e() +sp_tau_daughters[ii].e(),
			    sp_tau_daughters[i].pdgid()+sp_tau_daughters[i].pdgid() );
		tau_rho.push_back(pp);
		tau_rho_daughters.push_back(temp);
	      } else if ( ii != i ){
		Particle p(sp_tau_daughters[ii].px() ,
			   sp_tau_daughters[ii].py(),
			   sp_tau_daughters[ii].pz(),
			   sp_tau_daughters[ii].e(),
			   sp_tau_daughters[ii].pdgid());
		tau_other_pi.push_back(p);
	      }
	    }
	  }
	}
      }
       if(DEBUG){
	std::cout << "tau_rho.size = " << tau_rho.size() << std::endl;
	if( tau_rho.size() > 0) {
	  std::cout << "tau_rho_daughters[0].size =" << tau_rho_daughters[0].size() << std::endl;
	  tau_rho_daughters[0][0].print();
	  tau_rho_daughters[0][1].print();
	}
	if( tau_rho.size() > 1) {
	  std::cout << "tau_rho_daughters[1].size =" << tau_rho_daughters[1].size() << std::endl;
	  tau_rho_daughters[1][0].print();
	  tau_rho_daughters[1][1].print();
	}
	for(unsigned int i=0; i<tau_other_pi.size(); i++) {
	  std::cout << "tau_other_pi.size =" << tau_other_pi.size() << std::endl;
	  tau_other_pi[i].print();
	}
      }


      // Create list of tau2 daughters and reconstruct a1 resonances
      for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) {
	if(abs(sp_tau2_daughters[i].pdgid()) == 16){
	  etmiss_px += sp_tau2_daughters[i].px();
	  etmiss_py += sp_tau2_daughters[i].py();
	}
	if( abs(sp_tau2_daughters[i].pdgid()) == 211 ){
	  Particle pp(sp_tau2_daughters[i].px(),
		      sp_tau2_daughters[i].py(),
		      sp_tau2_daughters[i].pz(),
		      sp_tau2_daughters[i].e(),
		      sp_tau2_daughters[i].pdgid() );
	  tau2_pi.push_back(pp);
	  for(unsigned int ii=0; ii<sp_tau2_daughters.size(); ii++) {
	    if( abs(sp_tau2_daughters[ii].pdgid()) == 211 ){ 
	      if( sp_tau2_daughters[i].pdgid() * sp_tau2_daughters[ii].pdgid()< 0  && ii > i ){
		vector<Particle> temp;
		Particle p1(sp_tau2_daughters[i].px() ,
			    sp_tau2_daughters[i].py(),
			    sp_tau2_daughters[i].pz(),
			    sp_tau2_daughters[i].e(),
			    sp_tau2_daughters[i].pdgid());
		temp.push_back(p1);
		Particle p2(sp_tau2_daughters[ii].px() ,
			    sp_tau2_daughters[ii].py(),
			    sp_tau2_daughters[ii].pz(),
			    sp_tau2_daughters[ii].e(),
			    sp_tau2_daughters[ii].pdgid());
		temp.push_back(p2);
		Particle pp(sp_tau2_daughters[i].px()+sp_tau2_daughters[ii].px() ,
			    sp_tau2_daughters[i].py()+sp_tau2_daughters[ii].py(),
			    sp_tau2_daughters[i].pz()+sp_tau2_daughters[ii].pz(),
			    sp_tau2_daughters[i].e() +sp_tau2_daughters[ii].e(),
			    sp_tau2_daughters[i].pdgid()+sp_tau2_daughters[i].pdgid() );
		tau2_rho.push_back(pp);
		tau2_rho_daughters.push_back(temp);
	      } else if (ii != i) {
		Particle p(sp_tau2_daughters[ii].px() ,
			   sp_tau2_daughters[ii].py(),
			   sp_tau2_daughters[ii].pz(),
			   sp_tau2_daughters[ii].e(),
			   sp_tau2_daughters[ii].pdgid());
		tau2_other_pi.push_back(p);
	      }
	    }
	  }
	}
      }
      if(DEBUG){
	std::cout << "tau2_rho.size = " << tau2_rho.size() << std::endl;
	if( tau2_rho.size() > 0) {
	  std::cout << "tau2_rho_daughters[0].size =" << tau2_rho_daughters[0].size() << std::endl;
	  tau2_rho_daughters[0][0].print();
	  tau2_rho_daughters[0][1].print();
	}
	if( tau2_rho.size() > 1) {
	  std::cout << "tau2_rho_daughters[1].size =" << tau2_rho_daughters[1].size() << std::endl;
	  tau2_rho_daughters[1][0].print();
	  tau2_rho_daughters[1][1].print();
	}
	for(unsigned int i=0; i<tau2_other_pi.size(); i++) {
	  std::cout << "tau2_other_pi.size =" << tau2_other_pi.size() << std::endl;
	  tau2_other_pi[i].print();
	}
      }

      if( tau_pi.size() == 3 &&  tau2_pi.size() == 3 ){
 
	float accFlag = 1;
	hmg->GetHistoTH1F(idhist)->Fill( accFlag, 1.0 );


	if( outNNlab ){
	  printf("TUPLE  %lf  \n", evtWeight);
	  for(unsigned int i=0; i<sp_tau_daughters.size(); i++) 
	    printf(" %lf  %lf  %lf  %lf %d\n", 
		   sp_tau_daughters[i].px(), sp_tau_daughters[i].py(), sp_tau_daughters[i].pz(), sp_tau_daughters[i].e(), sp_tau_daughters[i].pdgid());
	  for(unsigned int i=0; i<sp_tau2_daughters.size(); i++) 
	    printf(" %lf  %lf  %lf  %lf %d\n", 
		   sp_tau2_daughters[i].px(), sp_tau2_daughters[i].py(), sp_tau2_daughters[i].pz(), sp_tau2_daughters[i].e(), sp_tau2_daughters[i].pdgid());
	  //	  return;
	}

		
	// Build the a1 resonances
	Particle tau_a1 ( tau_pi[0].px()+tau_pi[1].px()+tau_pi[2].px(), 
			  tau_pi[0].py()+tau_pi[1].py()+tau_pi[2].py(),
			  tau_pi[0].pz()+tau_pi[1].pz()+tau_pi[2].pz(),
			  tau_pi[0].e() +tau_pi[1].e() +tau_pi[2].e(), 0.0);
	
	Particle tau2_a1 ( tau2_pi[0].px()+tau2_pi[1].px()+tau2_pi[2].px(), 
			   tau2_pi[0].py()+tau2_pi[1].py()+tau2_pi[2].py(),
			   tau2_pi[0].pz()+tau2_pi[1].pz()+tau2_pi[2].pz(),
			   tau2_pi[0].e() +tau2_pi[1].e() +tau2_pi[2].e(), 0.0);
	
	// Build the a1-a1 pair system
	Particle a1a1 (  tau_a1.px()+tau2_a1.px(),
			 tau_a1.py()+tau2_a1.py(),
			 tau_a1.pz()+tau2_a1.pz(),
			 tau_a1.e() +tau2_a1.e(), 0.0);
	if(BENCHMARK){
	  std::cout<< "--- tau_a1 --" << std::endl;
	  tau_a1.print();
	  std::cout<< "--- tau2_a1 --" << std::endl;
	  tau2_a1.print();
	  std::cout<< "--- tau_a1 tau2_a1 system--" << std::endl;
	  a1a1.print();
	}

	int isFilter =1;
	double tau_a1_pTlab = sqrt( tau_a1.px()*tau_a1.px()+ tau_a1.py()*tau_a1.py() );
	double tau2_a1_pTlab = sqrt( tau2_a1.px()*tau2_a1.px()+ tau2_a1.py()*tau2_a1.py() );
	if(  tau_a1_pTlab < 20.0 ||  tau2_a1_pTlab < 20.0 ) isFilter =0;
	for(unsigned int i=0; i<tau_pi.size(); i++) {
	  double pT = sqrt( tau_pi[i].px()* tau_pi[i].px()+ tau_pi[i].py()* tau_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        
	for(unsigned int i=0; i<tau2_pi.size(); i++) {
	  double pT = sqrt( tau2_pi[i].px()* tau2_pi[i].px()+ tau2_pi[i].py()* tau2_pi[i].py());
	  if( pT < 1.0 )  isFilter =0;
	}        

	if(BENCHMARK)
	  std::cout<< "--- filter = " <<  isFilter << std::endl;
	if(BENCHMARK)
	  std::cout<< "--- loping over possible configuration of the rho-rho pairs-- "<< std::endl;

	// calculate angles between normal to rho planes
	double  costheta[16], theta[16];
	int index = -1;
        if( tau_rho.size() == 2 &&  tau2_rho.size() == 2 ){

	  for(int i = 0; i<2; i++){
	    for(int ii = 0; ii<2; ii++){

	      if(BENCHMARK){
		std::cout << " " << std::endl;
		std::cout << " --- NEXT rho-rho pair --" << std::endl;
		std::cout << " " << std::endl;
	      }

              Particle rho_1pi(
			       tau_rho_daughters[i][0].px(),
			       tau_rho_daughters[i][0].py(),
			       tau_rho_daughters[i][0].pz(),
			       tau_rho_daughters[i][0].e(),0);
              Particle rho_2pi(
			       tau_rho_daughters[i][1].px(),
			       tau_rho_daughters[i][1].py(),
			       tau_rho_daughters[i][1].pz(),
			       tau_rho_daughters[i][1].e(),0);
	      Particle rho(
			       tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			       tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			       tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			       tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
              Particle other_pi(
			       tau_other_pi[i].px(),
			       tau_other_pi[i].py(),
			       tau_other_pi[i].pz(),
			       tau_other_pi[i].e(),0);
              Particle rho2_1pi(
				tau2_rho_daughters[ii][0].px(),
				tau2_rho_daughters[ii][0].py(),
				tau2_rho_daughters[ii][0].pz(),
				tau2_rho_daughters[ii][0].e(),0);
              Particle rho2_2pi(
				tau2_rho_daughters[ii][1].px(),
				tau2_rho_daughters[ii][1].py(),
				tau2_rho_daughters[ii][1].pz(),
				tau2_rho_daughters[ii][1].e(),0);
	      Particle rho2(
			       tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			       tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			       tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			       tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
              Particle other2_pi(
			       tau2_other_pi[ii].px(),
			       tau2_other_pi[ii].py(),
			       tau2_other_pi[ii].pz(),
			       tau2_other_pi[ii].e(),0);
	      Particle rho_rho (
			       tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px() +  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			       tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py() +  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			       tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz() +  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			       tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e()  +  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	      Particle a1_rho (
			       tau_a1.px() +  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			       tau_a1.py() +  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			       tau_a1.pz() +  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			       tau_a1.e()  +  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	      Particle rho_a1 (
			       tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px() +  tau2_a1.px(),
			       tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py() +  tau2_a1.py(),
			       tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz() +  tau2_a1.pz(),
			       tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e()  +  tau2_a1.e(), 0);
	      Particle a1_a1 (
			       tau_a1.px() +  tau2_a1.px(),
			       tau_a1.py() +  tau2_a1.py(),
			       tau_a1.pz() +  tau2_a1.pz(),
			       tau_a1.e()  +  tau2_a1.e(), 0);

	      if(BENCHMARK){
		std::cout << "-- still in the LAB frame  " << std::endl;
		std::cout<< "--- tau_rho components --" << std::endl;
		rho_1pi.print();
		rho_2pi.print();
		std::cout<< "--- tau2_rho components --" << std::endl;
		rho2_1pi.print();
		rho2_2pi.print();
	      }

	      // acoplanarity between rho planes
	      rho_1pi.boostToRestFrame(rho_rho);
	      rho_2pi.boostToRestFrame(rho_rho);
	      rho2_1pi.boostToRestFrame(rho_rho);
	      rho2_2pi.boostToRestFrame(rho_rho);

	      if(BENCHMARK){
		std::cout << "-- after boost to rho-rho frame  " << std::endl;
		std::cout<< "--- tau_rho components --" << std::endl;
		rho_1pi.print();
		rho_2pi.print();
		std::cout<< "--- tau2_rho components --" << std::endl;
		rho2_1pi.print();
		rho2_2pi.print();
	      }

	      //normal to the plane spanned by rho-> pi+ pi-  
	      double n_1[3];
	      P_norm_cross_product(rho_1pi,rho_2pi,n_1);
	      //normal to the plane spanned by rho2-> pi+ pi-  
	      double n_2[3];
	      P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);
	      // calculate the acoplanarity angle
	      index++;
	      costheta[index] = dot_product(n_1,n_2);
              theta[index] = P_acoplanarAngle(rho_1pi, rho_2pi, rho2_1pi, rho2_2pi);

	      if(BENCHMARK){
		std::cout<< "--- normal to the tau_rho plane --" << std::endl;
		std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
		std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
		std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
		std::cout<< "--- cos( acoplanarity angle) --" << std::endl;
		std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
		std::cout<< "--- acoplanarity angle --" << std::endl;
		std::cout<< "Theta = " <<  theta[index] << std::endl;
	      }

	      double theta4Pi = theta[index];
	      double y1 = (rho_1pi.e()-rho_2pi.e())/(rho_1pi.e()+rho_2pi.e());
	      double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+121)->Fill( theta[index], isFilter * evtWeight );
	      hmg->GetHistoTH1F(idhist+122)->Fill( rho.recalculated_mass(), isFilter * evtWeight );
	      hmg->GetHistoTH1F(idhist+123)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );

	      hmg->GetHistoTH1F(idhist+124)->Fill( theta4Pi, isFilter * evtWeight );
	      hmg->GetHistoTH2F(idhist+124)->Fill(y1, y2, isFilter * evtWeight );

	    }
	  }

	  if(BENCHMARK)
	    std::cout<< "--- looping over possible configuration of the a1-rho and rho_a1 pairs-- "<< std::endl;

	  for(int i = 0; i<2; i++){
	    for(int ii = 0; ii<2; ii++){

	      if(BENCHMARK){
		std::cout << " " << std::endl;
		std::cout << " --- NEXT a1-rho pair --" << std::endl;
		std::cout << " " << std::endl;
	      }

              Particle rho_1pi(
			       tau_rho_daughters[i][0].px(),
			       tau_rho_daughters[i][0].py(),
			       tau_rho_daughters[i][0].pz(),
			       tau_rho_daughters[i][0].e(),0);
              Particle rho_2pi(
			       tau_rho_daughters[i][1].px(),
			       tau_rho_daughters[i][1].py(),
			       tau_rho_daughters[i][1].pz(),
			       tau_rho_daughters[i][1].e(),0);
	      Particle rho(
			       tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			       tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			       tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			       tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
              Particle other_pi(
			       tau_other_pi[i].px(),
			       tau_other_pi[i].py(),
			       tau_other_pi[i].pz(),
			       tau_other_pi[i].e(),0);
              Particle rho2_1pi(
				tau2_rho_daughters[ii][0].px(),
				tau2_rho_daughters[ii][0].py(),
				tau2_rho_daughters[ii][0].pz(),
				tau2_rho_daughters[ii][0].e(),0);
              Particle rho2_2pi(
				tau2_rho_daughters[ii][1].px(),
				tau2_rho_daughters[ii][1].py(),
				tau2_rho_daughters[ii][1].pz(),
				tau2_rho_daughters[ii][1].e(),0);
	      Particle rho2(
			       tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			       tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			       tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			       tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
              Particle other2_pi(
			       tau2_other_pi[ii].px(),
			       tau2_other_pi[ii].py(),
			       tau2_other_pi[ii].pz(),
			       tau2_other_pi[ii].e(),0);
	      Particle rho_rho (
			       tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px() +  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			       tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py() +  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			       tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz() +  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			       tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e()  +  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	      Particle a1_rho (
			       tau_a1.px() +  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			       tau_a1.py() +  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			       tau_a1.pz() +  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			       tau_a1.e()  +  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	      Particle rho_a1 (
			       tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px() +  tau2_a1.px(),
			       tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py() +  tau2_a1.py(),
			       tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz() +  tau2_a1.pz(),
			       tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e()  +  tau2_a1.e(), 0);
	      Particle a1_a1 (
			       tau_a1.px() +  tau2_a1.px(),
			       tau_a1.py() +  tau2_a1.py(),
			       tau_a1.pz() +  tau2_a1.pz(),
			       tau_a1.e()  +  tau2_a1.e(), 0);

	      if(BENCHMARK){
		std::cout << "-- still in the LAB frame  " << std::endl;
		std::cout<< "--- tau_a1 components --" << std::endl;
		rho.print();
		other_pi.print();
		std::cout<< "--- tau2_rho components --" << std::endl;
		rho2_1pi.print();
		rho2_2pi.print();
	      }

	      // acoplanarity between a1-rho planes
	      rho.boostToRestFrame(a1_rho);
	      other_pi.boostToRestFrame(a1_rho);
	      rho2_1pi.boostToRestFrame(a1_rho);
	      rho2_2pi.boostToRestFrame(a1_rho);

	      if(BENCHMARK){
		std::cout << "-- after boost to a1-rho frame  " << std::endl;
		std::cout<< "--- tau_a1 components --" << std::endl;
		rho.print();
		other_pi.print();
		std::cout<< "--- tau2_rho components --" << std::endl;
		rho2_1pi.print();
		rho2_2pi.print();
	      }

	      //normal to the plane spanned by rho-> pi+ pi-  
	      double n_1[3];
	      P_norm_cross_product(rho,other_pi,n_1);
	      //normal to the plane spanned by rho2-> pi+ pi-  
	      double n_2[3];
	      P_norm_cross_product(rho2_1pi,rho2_2pi,n_2);
	      // calculate the acoplanarity angle
	      index++;
	      costheta[index] = dot_product(n_1,n_2);
              theta[index] = P_acoplanarAngle(rho, other_pi, rho2_1pi, rho2_2pi);

	      if(BENCHMARK){
		std::cout<< "--- normal to the tau_a1 plane --" << std::endl;
		std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
		std::cout<< "--- normal to the tau2_rho plane --" << std::endl;
		std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
		std::cout<< "--- cos( acoplanarity angle ) --" << std::endl;
		std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
		std::cout<< "--- acoplanarity angle --" << std::endl;
		std::cout<< "Theta = " <<  theta[index] << std::endl;
	      }

	      hmg->GetHistoTH1F(idhist+131)->Fill( theta[index], isFilter * evtWeight );
	      hmg->GetHistoTH1F(idhist+132)->Fill( tau_a1.recalculated_mass(), isFilter * evtWeight );
	      hmg->GetHistoTH1F(idhist+133)->Fill( rho2.recalculated_mass(), isFilter * evtWeight );

	      double y1 = (rho.e()-other_pi.e())/(rho.e()+other_pi.e());
	      double y2 = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	      double theta4Pi = theta[index];
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+134)->Fill( theta4Pi, isFilter * evtWeight );
	      hmg->GetHistoTH2F(idhist+134)->Fill(y1, y2, isFilter * evtWeight );

	      y1 = rho.e()/(rho.e()+other_pi.e());
	      double x1 = (pow(tau_a1.recalculated_mass(),2)- pow(other_pi.recalculated_mass(),2)+pow(rho.recalculated_mass(),2))  
	   	          /( 2 * pow(tau_a1.recalculated_mass(),2));
              if( y1 > x1 ) y1 = -y1;
	      theta4Pi = theta[index];
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+135)->Fill( theta4Pi, isFilter * evtWeight );
	      hmg->GetHistoTH2F(idhist+135)->Fill(y1, y2, isFilter * evtWeight );

	      y1 = other_pi.e()/(rho.e()+other_pi.e());
	      x1 = (pow(tau_a1.recalculated_mass(),2)+ pow(other_pi.recalculated_mass(),2)- pow(rho.recalculated_mass(),2))  
	   	          /( 2 * pow(tau_a1.recalculated_mass(),2));
              if( y1 > x1 ) y1 = -y1;
	      theta4Pi = theta[index];
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+136)->Fill( theta4Pi, isFilter * evtWeight );
	      hmg->GetHistoTH2F(idhist+136)->Fill(y1, y2, isFilter * evtWeight );

	      y1 = rho.e()/(rho.e()+other_pi.e());
	      x1 = (1.260*1.260 - 0.140*0.140 + 0.770*0.770)  /( 2 * 1.260*1.260);
              if( y1 > x1 ) y1 = -y1;
	      theta4Pi = theta[index];
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+137)->Fill( theta4Pi, isFilter * evtWeight );
	      hmg->GetHistoTH2F(idhist+137)->Fill(y1, y2, isFilter * evtWeight );




	      if(BENCHMARK){
		std::cout << " " << std::endl;
		std::cout << " --- NEXT rho a1 pair --" << std::endl;
		std::cout << " " << std::endl;
		
		std::cout << "-- still in the LAB frame  " << std::endl;
		std::cout<< "--- tau_rho components --" << std::endl;
		rho_1pi.print();
		rho_2pi.print();
		std::cout<< "--- tau2_a1 components --" << std::endl;
		rho2.print();
		other2_pi.print();
	      }

	      // acoplanarity between rho-a1 planes
	      rho_1pi.boostToRestFrame(rho_a1);
	      rho_2pi.boostToRestFrame(rho_a1);
	      rho2.boostToRestFrame(rho_a1);
	      other2_pi.boostToRestFrame(rho_a1);

	      if(BENCHMARK){
		std::cout << "-- after boost to rho-a1 frame  " << std::endl;
		std::cout<< "--- tau_rho components --" << std::endl;
		rho_1pi.print();
		rho_2pi.print();
		std::cout<< "--- tau2_a1 components --" << std::endl;
		rho2.print();
		other2_pi.print();
	      }

	      //normal to the plane spanned by rho-> pi+ pi-  
	      //double n_1[3];
	      P_norm_cross_product(rho_1pi,rho_2pi,n_1);
	      //normal to the plane spanned by rho2-> pi+ pi-  
	      //double n_2[3];
	      P_norm_cross_product(rho2,other2_pi,n_2);	
	      // calculate the acoplanarity angle
	      index++;
	      costheta[index] = dot_product(n_1,n_2);
              theta[index] = P_acoplanarAngle(rho_1pi, rho_2pi, rho2, other2_pi);

	      if(BENCHMARK){
		std::cout<< "--- normal to the tau_rho plane --" << std::endl;
		std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
		std::cout<< "--- normal to the tau2_a1 plane --" << std::endl;
		std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
		std::cout<< "--- acoplanarity angle --" << std::endl;
		std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
		std::cout<< "--- acoplanarity angle --" << std::endl;
		std::cout<< "Theta = " <<  theta[index] << std::endl;
	      }

	      hmg->GetHistoTH1F(idhist+141)->Fill( theta[index], isFilter *  evtWeight );
	      hmg->GetHistoTH1F(idhist+142)->Fill( rho.recalculated_mass(), isFilter * evtWeight );
	      hmg->GetHistoTH1F(idhist+143)->Fill( tau2_a1.recalculated_mass(), isFilter * evtWeight );

	      theta4Pi = theta[index];
	      y1 = (rho_1pi.e()-rho_2pi.e())/(rho_1pi.e()+rho_2pi.e());
	      y2 = (rho2.e()-other2_pi.e())/(rho2.e()+other2_pi.e());
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+144)->Fill( theta4Pi, isFilter *  evtWeight );
	      hmg->GetHistoTH2F(idhist+144)->Fill(y1, y2, isFilter * evtWeight );

	      y2 = rho2.e()/(rho2.e()+other2_pi.e());
	      double x2 = (pow(tau2_a1.recalculated_mass(),2)- pow(other2_pi.recalculated_mass(),2)+pow(rho2.recalculated_mass(),2))  
	   	          /( 2 * pow(tau2_a1.recalculated_mass(),2));
              if( y2 > x2 ) y2 = -y2;
	      theta4Pi = theta[index];
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+145)->Fill( theta4Pi, isFilter *  evtWeight );
	      hmg->GetHistoTH2F(idhist+145)->Fill(y1, y2, isFilter * evtWeight );

	      y2 = other2_pi.e()/(rho2.e()+other2_pi.e());
	      x2 = (pow(tau2_a1.recalculated_mass(),2)+ pow(other2_pi.recalculated_mass(),2) - pow(rho2.recalculated_mass(),2))  
	   	          /( 2 * pow(tau2_a1.recalculated_mass(),2));
              if( y2 > x2 ) y2 = -y2;
	      theta4Pi = theta[index];
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+146)->Fill( theta4Pi, isFilter *  evtWeight );
	      hmg->GetHistoTH2F(idhist+146)->Fill(y1, y2, isFilter * evtWeight );

	      y2 = rho2.e()/(rho2.e()+other2_pi.e());
	      x2 = (1.260*1.260 - 0.140*0.140 + 0.770*0.770)  /( 2 * 1.260*1.260);
              if( y2 > x2 ) y2 = -y2;
	      theta4Pi = theta[index];
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+147)->Fill( theta4Pi, isFilter *  evtWeight );
	      hmg->GetHistoTH2F(idhist+147)->Fill(y1, y2, isFilter * evtWeight );


	    }
	  }

	  if(BENCHMARK){
	    std::cout<< "--- loping over possible configuration of the a1-a1 pairs-- "<< std::endl;
	  }

	  for(int i = 0; i<2; i++){
	    for(int ii = 0; ii<2; ii++){

	      if(BENCHMARK){
		std::cout << " " << std::endl;
		std::cout << " --- NEXT a1-a1 pair --" << std::endl;
		std::cout << " " << std::endl;
	      }

              Particle rho_1pi(
			       tau_rho_daughters[i][0].px(),
			       tau_rho_daughters[i][0].py(),
			       tau_rho_daughters[i][0].pz(),
			       tau_rho_daughters[i][0].e(),0);
              Particle rho_2pi(
			       tau_rho_daughters[i][1].px(),
			       tau_rho_daughters[i][1].py(),
			       tau_rho_daughters[i][1].pz(),
			       tau_rho_daughters[i][1].e(),0);
	      Particle rho(
			       tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px(),
			       tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py(),
			       tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz(),
			       tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e(), 0);
              Particle other_pi(
			       tau_other_pi[i].px(),
			       tau_other_pi[i].py(),
			       tau_other_pi[i].pz(),
			       tau_other_pi[i].e(),0);
              Particle rho2_1pi(
				tau2_rho_daughters[ii][0].px(),
				tau2_rho_daughters[ii][0].py(),
				tau2_rho_daughters[ii][0].pz(),
				tau2_rho_daughters[ii][0].e(),0);
              Particle rho2_2pi(
				tau2_rho_daughters[ii][1].px(),
				tau2_rho_daughters[ii][1].py(),
				tau2_rho_daughters[ii][1].pz(),
				tau2_rho_daughters[ii][1].e(),0);
	      Particle rho2(
			       tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			       tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			       tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			       tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
              Particle other2_pi(
			       tau2_other_pi[ii].px(),
			       tau2_other_pi[ii].py(),
			       tau2_other_pi[ii].pz(),
			       tau2_other_pi[ii].e(),0);
	      Particle rho_rho (
			       tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px() +  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			       tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py() +  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			       tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz() +  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			       tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e()  +  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	      Particle a1_rho (
			       tau_a1.px() +  tau2_rho_daughters[ii][0].px()+tau2_rho_daughters[ii][1].px(),
			       tau_a1.py() +  tau2_rho_daughters[ii][0].py()+tau2_rho_daughters[ii][1].py(),
			       tau_a1.pz() +  tau2_rho_daughters[ii][0].pz()+tau2_rho_daughters[ii][1].pz(),
			       tau_a1.e()  +  tau2_rho_daughters[ii][0].e() +tau2_rho_daughters[ii][1].e(), 0);
	      Particle rho_a1 (
			       tau_rho_daughters[i][0].px()+tau_rho_daughters[i][1].px() +  tau2_a1.px(),
			       tau_rho_daughters[i][0].py()+tau_rho_daughters[i][1].py() +  tau2_a1.py(),
			       tau_rho_daughters[i][0].pz()+tau_rho_daughters[i][1].pz() +  tau2_a1.pz(),
			       tau_rho_daughters[i][0].e() +tau_rho_daughters[i][1].e()  +  tau2_a1.e(), 0);
	      Particle a1_a1 ( 
			       tau_a1.px() +  tau2_a1.px(),
			       tau_a1.py() +  tau2_a1.py(),
			       tau_a1.pz() +  tau2_a1.pz(),
			       tau_a1.e()  +  tau2_a1.e(), 0);

	      if(BENCHMARK){
		std::cout << "-- still in the LAB frame  " << std::endl;
		std::cout<< "--- tau_a1 components --" << std::endl;
		rho.print();
		other_pi.print();
		std::cout<< "--- tau2_a1 components --" << std::endl;
		rho2.print();
		other2_pi.print();
	      }

	      // acoplanarity between a1-rho planes
	      rho.boostToRestFrame(a1_a1);
	      other_pi.boostToRestFrame(a1_a1);
	      rho2.boostToRestFrame(a1_a1);
	      other2_pi.boostToRestFrame(a1_a1);

	      // calculate also y1, y2 for rho and rho2
	      rho_1pi.boostToRestFrame(rho_rho);
	      rho_2pi.boostToRestFrame(rho_rho);
	      rho2_1pi.boostToRestFrame(rho_rho);
	      rho2_2pi.boostToRestFrame(rho_rho);
	      double y1rho = (rho_1pi.e()-rho_2pi.e())/(rho_1pi.e()+rho_2pi.e());
	      double y2rho = (rho2_1pi.e()-rho2_2pi.e())/(rho2_1pi.e()+rho2_2pi.e());
	      int isFilter2 = 0;
	      if( abs( y1rho ) < 0.3 && abs( y2rho ) < 0.3 ) isFilter2 = 1;

	      if(BENCHMARK){
		std::cout << "-- after boost to a1-a1 frame  " << std::endl;
		std::cout<< "--- tau_a1 components --" << std::endl;
		rho.print();
		other_pi.print();
		std::cout<< "--- tau2_a1 components --" << std::endl;
		rho2.print();
		other2_pi.print();
	      }

	      //normal to the plane spanned by a1-a1 decays  
	      double n_1[3];
	      P_norm_cross_product(rho,other_pi,n_1);
	      //normal to the plane spanned by rho2-> pi+ pi-  
	      double n_2[3];
	      P_norm_cross_product(rho2,other2_pi,n_2);	
	      // calculate the acoplanarity angle
	      index++;
	      costheta[index] = dot_product(n_1,n_2);
              theta[index] = P_acoplanarAngle(rho, other_pi, rho2, other2_pi);

	      if(BENCHMARK){
		std::cout<< "--- normal to the tau_a1 plane --" << std::endl;
		std::cout<< "n []= " << n_1[0] << " " <<  n_1[1] << " " <<  n_1[2] << std::endl;
		std::cout<< "--- normal to the tau2_a1 plane --" << std::endl;
		std::cout<< "n []= " << n_2[0] << " " <<  n_2[1] << " " <<  n_2[2] << std::endl;
		std::cout<< "--- cos(acoplanarity angle) --" << std::endl;
		std::cout<< "cosTheta = " <<  costheta[index] << std::endl;
		std::cout<< "--- acoplanarity angle --" << std::endl;
		std::cout<< "Theta = " <<  theta[index] << std::endl;
	      }

	      hmg->GetHistoTH1F(idhist+151)->Fill( theta[index], isFilter *  evtWeight );
	      hmg->GetHistoTH1F(idhist+152)->Fill( tau_a1.recalculated_mass(), isFilter * evtWeight );
	      hmg->GetHistoTH1F(idhist+153)->Fill( tau2_a1.recalculated_mass(), isFilter * evtWeight );

	      double theta4Pi = theta[index];
	      double y1 = (rho.e()-other_pi.e())/(rho.e()+other_pi.e());
	      double y2 = (rho2.e()-other2_pi.e())/(rho2.e()+other2_pi.e());
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+154)->Fill( theta4Pi, isFilter * evtWeight );
	      hmg->GetHistoTH2F(idhist+154)->Fill(y1, y2, isFilter * evtWeight );

	      y1 = rho.e()/(rho.e()+other_pi.e());
	      double x1 = (pow(tau_a1.recalculated_mass(),2)- pow(other_pi.recalculated_mass(),2)+pow(rho.recalculated_mass(),2))  
	   	          /( 2 * pow(tau_a1.recalculated_mass(),2));
              if( y1 > x1 ) y1 = -y1;
	      y2 = rho2.e()/(rho2.e()+other2_pi.e());
	      double x2 = (pow(tau2_a1.recalculated_mass(),2)- pow(other2_pi.recalculated_mass(),2)+pow(rho2.recalculated_mass(),2))  
	   	          /( 2 * pow(tau2_a1.recalculated_mass(),2));
              if( y2 > x2 ) y2 = -y2;
	      theta4Pi = theta[index];
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+155)->Fill( theta4Pi, isFilter * evtWeight );
	      hmg->GetHistoTH2F(idhist+155)->Fill(y1, y2, isFilter * evtWeight );

	      if(isFilter2)
		hmg->GetHistoTH1F(idhist+158)->Fill( theta4Pi, isFilter * evtWeight );
	      else
		hmg->GetHistoTH1F(idhist+159)->Fill( theta4Pi, isFilter * evtWeight );


	      y1 = other_pi.e()/(rho.e()+other_pi.e());
	      x1 = (pow(tau_a1.recalculated_mass(),2)+ pow(other_pi.recalculated_mass(),2)- pow(rho.recalculated_mass(),2))  
	   	          /( 2 * pow(tau_a1.recalculated_mass(),2));
              if( y1 > x1 ) y1 = -y1;
	      y2 = other2_pi.e()/(rho2.e()+other2_pi.e());
	      x2 = (pow(tau2_a1.recalculated_mass(),2)+ pow(other2_pi.recalculated_mass(),2) - pow(rho2.recalculated_mass(),2))  
	   	          /( 2 * pow(tau2_a1.recalculated_mass(),2));
              if( y2 > x2 ) y2 = -y2;
	      theta4Pi = theta[index];
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+156)->Fill( theta4Pi, isFilter * evtWeight );
	      hmg->GetHistoTH2F(idhist+156)->Fill(y1, y2, isFilter * evtWeight );

	      y1 = rho.e()/(rho.e()+other_pi.e());
	      x1 = (1.260*1.260 - 0.140*0.140 + 0.770*0.770)  /( 2 * 1.260*1.260);
              if( y1 > x1 ) y1 = -y1;
	      y2 = rho2.e()/(rho2.e()+other2_pi.e());
	      x2 = (1.260*1.260 - 0.140*0.140 + 0.770*0.770)  /( 2 * 1.260*1.260);
              if( y2 > x2 ) y2 = -y2;
	      theta4Pi = theta[index];
	      if( y1*y2 < 0) theta4Pi = theta4Pi + 2 * kPI;

	      hmg->GetHistoTH1F(idhist+157)->Fill( theta4Pi, isFilter * evtWeight );
	      hmg->GetHistoTH2F(idhist+157)->Fill(y1, y2, isFilter * evtWeight );

	    }
	  }
	}
	
	if( a1a1Frame ){
	  // Boost pions and a1 into  a1-a1 center of mass frame
	  tau_a1.boostToRestFrame(a1a1);
	  tau2_a1.boostToRestFrame(a1a1);
	  //----
	  for(unsigned int i=0; i<tau_pi.size(); i++) {
	    tau_pi[i].boostToRestFrame(a1a1);
	    if(DEBUG) std::cout << "tau_pi pdgid = " << tau_pi[i].pdgid() << std::endl;
	  }        
	  for(unsigned int i=0; i<tau2_pi.size(); i++) {
	    tau2_pi[i].boostToRestFrame(a1a1);
	    if(DEBUG) std::cout << "tau2_pi pdgid = " << tau2_pi[i].pdgid() << std::endl;
	  } 
	  //----
	  for(unsigned int i=0; i<tau_rho.size(); i++) {
	    tau_rho[i].boostToRestFrame(a1a1);
	    if(DEBUG) std::cout << "tau_rho mass = " <<  tau_rho[i].recalculated_mass() << std::endl;

	  }        
	  for(unsigned int i=0; i<tau2_rho.size(); i++) {
	    tau2_rho[i].boostToRestFrame(a1a1);
	    if(DEBUG) std::cout << "tau2_rho mass = " <<  tau2_rho[i].recalculated_mass() << std::endl;
	  } 

	  if( a1a1Frame_zRotate ){
	    // Rotate the system so a1 are along the z-axis
	    double phi   = tau_a1.getAnglePhi();
            tau_a1.rotateXY( -phi);
	    for(unsigned int i=0; i<tau_pi.size(); i++) {
	      tau_pi[i].rotateXY( -phi);
	    }        
	    for(unsigned int i=0; i<tau_rho.size(); i++) {
	      tau_rho[i].rotateXY( -phi);
	    }        
	    for(unsigned int i=0; i<tau2_pi.size(); i++) {
	      tau2_pi[i].rotateXY( -phi);
	    } 
	    for(unsigned int i=0; i<tau2_rho.size(); i++) {
	      tau2_rho[i].rotateXY( -phi);
	    } 

	    double theta = tau_a1.getAngleTheta();
	    for(unsigned int i=0; i<tau_pi.size(); i++) {
	      tau_pi[i].rotateXZ( kPI - theta);
	    }        
	    for(unsigned int i=0; i<tau_rho.size(); i++) {
	      tau_rho[i].rotateXZ( kPI - theta);
	    }        
	    for(unsigned int i=0; i<tau2_pi.size(); i++) {
	      tau2_pi[i].rotateXZ( kPI - theta);
	    } 
	    for(unsigned int i=0; i<tau2_rho.size(); i++) {
	      tau2_rho[i].rotateXZ( kPI - theta);
	    } 
	  }

	}
	// monitoring histograms
	// how visible is rho resonance   (not weighted)
        hmg->GetHistoTH1F(idhist+103)->Fill(tau_rho[0].recalculated_mass(), 1.0 );
        hmg->GetHistoTH1F(idhist+103)->Fill(tau_rho[1].recalculated_mass(), 1.0 );        	
        hmg->GetHistoTH1F(idhist+104)->Fill(tau2_rho[0].recalculated_mass(), 1.0 );
        hmg->GetHistoTH1F(idhist+104)->Fill(tau2_rho[1].recalculated_mass(), 1.0 );
        hmg->GetHistoTH1F(idhist+105)->Fill(tau_a1.recalculated_mass(), 1.0 );
        hmg->GetHistoTH1F(idhist+106)->Fill(tau2_a1.recalculated_mass(), 1.0 );
	for(int index =0; index<4; index++)
	  hmg->GetHistoTH1F(idhist+107)->Fill(costheta[index], 1.0 );
	//adding filter information
        hmg->GetHistoTH1F(idhist+108)->Fill(tau_a1_pTlab,  isFilter * 1.0);
        hmg->GetHistoTH1F(idhist+108)->Fill(tau2_a1_pTlab, isFilter * 1.0);

	// how visible is rho resonance   (weighted)
        hmg->GetHistoTH1F(idhist+203)->Fill(tau_rho[0].recalculated_mass(), evtWeight );
        hmg->GetHistoTH1F(idhist+203)->Fill(tau_rho[1].recalculated_mass(), evtWeight );        	
        hmg->GetHistoTH1F(idhist+204)->Fill(tau2_rho[0].recalculated_mass(), evtWeight );
        hmg->GetHistoTH1F(idhist+204)->Fill(tau2_rho[1].recalculated_mass(), evtWeight  );
        hmg->GetHistoTH1F(idhist+205)->Fill(tau_a1.recalculated_mass(), evtWeight );
        hmg->GetHistoTH1F(idhist+206)->Fill(tau2_a1.recalculated_mass(), evtWeight );
	for(int index =0; index<16; index++)
	  hmg->GetHistoTH1F(idhist+207)->Fill(costheta[index], evtWeight );
	//adding filter information
        hmg->GetHistoTH1F(idhist+208)->Fill(tau_a1_pTlab,  isFilter * evtWeight);
        hmg->GetHistoTH1F(idhist+208)->Fill(tau2_a1_pTlab, isFilter * evtWeight);


	//printout data for training
	if( outNN ){
	  printf("TUPLE  %lf  %lf %lf %lf %lf   %lf %lf %lf %lf   %lf %lf %lf %lf   %lf %lf %lf %lf   %lf %lf %lf %lf   %lf %lf %lf %lf   %lf %lf   %lf %lf %lf %lf   %lf %lf %lf %lf   %lf %lf %lf %lf   %lf %lf %lf %lf   %lf   %lf   %lf   %lf     %lf   %lf      %lf   %lf   %lf   %lf   %lf   %lf   %lf   %lf   %lf   %lf   %lf   %lf   %lf   %lf   %lf   %lf   %4d \n", 
		 evtWeight, 
		 tau_pi[0].px(),  tau_pi[0].py(),  tau_pi[0].pz(),  tau_pi[0].e(), 
		 tau_pi[1].px(),  tau_pi[1].py(),  tau_pi[1].pz(),  tau_pi[1].e(), 
		 tau_pi[2].px(),  tau_pi[2].py(),  tau_pi[2].pz(),  tau_pi[2].e(), 
		 tau2_pi[0].px(), tau2_pi[0].py(), tau2_pi[0].pz(), tau2_pi[0].e(), 
		 tau2_pi[1].px(), tau2_pi[1].py(), tau2_pi[1].pz(), tau2_pi[1].e(), 
		 tau2_pi[2].px(), tau2_pi[2].py(), tau2_pi[2].pz(), tau2_pi[2].e(), 
		 etmiss_px, etmiss_py,
		 tau_rho[0].px(),  tau_rho[0].py(),  tau_rho[0].pz(),  tau_rho[0].e(), 
		 tau_rho[1].px(),  tau_rho[1].py(),  tau_rho[1].pz(),  tau_rho[1].e(), 
		 tau2_rho[0].px(), tau2_rho[0].py(), tau2_rho[0].pz(), tau2_rho[0].e(), 
		 tau2_rho[1].px(), tau2_rho[1].py(), tau2_rho[1].pz(), tau2_rho[1].e(),
                 tau_rho[0].recalculated_mass(),   tau_rho[1].recalculated_mass(),  
                 tau2_rho[0].recalculated_mass(), tau2_rho[1].recalculated_mass(),
                 tau_a1.recalculated_mass(), tau2_a1.recalculated_mass(),
                 costheta[0], costheta[1], costheta[2], costheta[3], costheta[4], costheta[5], costheta[6], costheta[7], costheta[8], costheta[9], costheta[10], 
                 costheta[11], costheta[12], costheta[13],costheta[14],costheta[15],
                 isFilter);
	}
	}
    }// end of analysing a1-a1       


}

//----------------------------------------------------
// Define  histograms for analysis
//----------------------------------------------------
//
void AnalTauSpinnerCP::CreateHistos( int idhist)  {

  
  HistoManager *hmg  = HistoManager::getInstance();

  Text_t namehist[500];

  //monitor cut flow
  if(  idhist == m_idHist+1000000 ){
    sprintf(namehist,"hist%.2d",idhist);  
    hmg->addTH1F(namehist,    "  cut flow counter (events) ", 50  ,   0.0  ,  50.0); 
    sprintf(namehist,"hist%.2d",idhist+1);  
    hmg->addTH1F(namehist,    " WT weight ", 50  ,   0.0  ,  10.0); 
 
 }
  for (int i1 = 0; i1 < 8; i1++){
    for (int i = 0; i < 8; i++){
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000);  
      hmg->addTH1F(namehist,    " cut flow counter (events) ", 50  ,   0.0  ,  50.0); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +1);  
      hmg->addTH1F(namehist,    " WT weight ", 50  ,   0.0  ,  10.0); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +2);  
      hmg->addTH1F(namehist,    " polarisation ", 5  ,   -2.5  ,  2.5); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +3);  
      hmg->addTH1F(namehist,    " WT cpmix ", 18  ,  0.0  ,  1.8); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +4);  
      hmg->addTH1F(namehist,    " WT cpmix ", 18  ,  0.0  ,  1.8); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +5);  
      hmg->addTH1F(namehist,    " WT cpmix ", 18  ,  0.0  ,  1.8); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +6);  
      hmg->addTH1F(namehist,    " WT cpmix ", 18  ,  0.0  ,  1.8); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +7);  
      hmg->addTH1F(namehist,    " WT cpmix ", 18  ,  0.0  ,  1.8); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +8);  
      hmg->addTH1F(namehist,    " WT cpmix ", 18  ,  0.0  ,  1.8); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +9);  
      hmg->addTH1F(namehist,    " WT cpmix ", 18  ,  0.0  ,  1.8); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +11);  
      hmg->addTH1F(namehist,    " E pi0 ", 50  ,  0.0  ,  100.0); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +12);  
      hmg->addTH1F(namehist,    " E pi+- ", 50  ,  0.0  ,  100.0);
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +13);  
      hmg->addTH1F(namehist,    " Mtautau ", 50  ,  50.0  ,  150.0);
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +14);  
      hmg->addTH1F(namehist,    " Mtautau ", 50  ,  50.0  ,  150.0);
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +15);  
      hmg->addTH1F(namehist,    " WT cpmix ", 50  ,  -1.0  ,  1.0); 
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +16);  
      hmg->addTH1F(namehist,    " Mtautau ", 50  ,  50.0  ,  150.0);
      sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 +17);  
      hmg->addTH1F(namehist,    " Mtautau ", 50  ,  50.0  ,  150.0);
 
      for (int i2 = 1; i2 < 4; i2++){
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +1);  
	hmg->addTH1F(namehist,    " acoplanarity ", 50  ,  0  ,  6.1415); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +2);  
	hmg->addTH1F(namehist,    " acoplanarity ", 50  ,  0  ,  6.1415); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +3);  
	hmg->addTH1F(namehist,    " rho invariant mass ", 50  ,  0  ,  1.5); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +4);  
	hmg->addTH1F(namehist,    " rho invariant mass ", 50  ,  0  ,  1.5); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +5);  
	hmg->addTH1F(namehist,    " a1 invariant mass ", 50  ,  0  ,  2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +6);  
	hmg->addTH1F(namehist,    " a1 invariant mass ", 50  ,  0  ,  2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +7);  
	hmg->addTH1F(namehist,    " cos(acoplanarity angles) ", 50  ,  -1.0  ,  1.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +8);  
	hmg->addTH1F(namehist,    " filtered pT ", 50  , 0.0  ,  150.0); 

	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +24);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50, -1.0, 1.0); 

	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +21);  
	hmg->addTH1F(namehist,    " acoplanarity ", 50  , 0.0  ,  6.1415); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +22);  
	hmg->addTH1F(namehist,    " tau_rho invariant mass ", 50  , 0.0  ,  1.5); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +23);  
	hmg->addTH1F(namehist,    " tau2_rho invariant mass ", 50  , 0.0  , 1.5); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +24);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +25);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +26);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +27);  
	hmg->addTH1F(namehist,    " y1 ", 50  , -2.0  ,  2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +28);  
	hmg->addTH1F(namehist,    " y2 ", 50  , -2.0  ,  2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +29);  
	hmg->addTH1F(namehist,    " y2 ", 50  , -2.0  ,  2.0); 


	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +34);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50, -1.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +35);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50, -1.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +36);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50, -1.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +37);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50, -1.0, 1.0); 

	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +31);  
	hmg->addTH1F(namehist,    " acoplanarity ", 50  , 0.0  ,  6.1415); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +32);  
	hmg->addTH1F(namehist,    " tau_a1 invariant mass ", 50  , 0.0  ,  2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +33);  
	hmg->addTH1F(namehist,    " tau2_rho invariant mass ", 50  , 0.0  , 1.5); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +34);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +35);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +36);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 

	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +37);  
	hmg->addTH1F(namehist,    " y1_v1 ", 50  , -2.0  ,  2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +38);  
	hmg->addTH1F(namehist,    " y1_v2 ", 50  , -2.0  ,  2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +39);  
	hmg->addTH1F(namehist,    " y2 ", 50  , -2.0  ,  2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +40);  
	hmg->addTH1F(namehist,    " y1_v1*y2 ", 50  , -2.0  ,  2.0); 


	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +44);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50,  -1.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +45);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50,  -1.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +46);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50,  -1.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +47);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50,  -1.0, 1.0); 

	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +41);  
	hmg->addTH1F(namehist,    " acoplanarity ", 50  , 0.0  ,  6.1415); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +42);  
	hmg->addTH1F(namehist,    " tau_rho invariant mass ", 50  , 0.0  ,  1.5); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +43);  
	hmg->addTH1F(namehist,    " tau2_a1 invariant mass ", 50  , 0.0  , 2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +44);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +45);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +46);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +47);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 


	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +54);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50,  -1.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +55);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50,  -1.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +56);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50,  -1.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +57);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , -1.0, 1.0, 50,  -1.0, 1.0);

	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +51);  
	hmg->addTH1F(namehist,    " acoplanarity ", 50  , 0.0  ,  6.1415); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +52);  
	hmg->addTH1F(namehist,    " tau_a1 invariant mass ", 50  , 0.0  ,  2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +53);  
	hmg->addTH1F(namehist,    " tau2_a1 invariant mass ", 50  , 0.0  , 2.0); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +54);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +55);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +56);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +57);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +58);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +59);  
	hmg->addTH1F(namehist,    " signed acoplanarity ", 50  , 0.0  ,  12.2830);


	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +61);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , 0.0, 1.0, 50,  0.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +62);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , 0.0, 1.0, 50,  0.0, 1.0);
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +71);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , 0.0, 1.0, 50,  0.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +72);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , 0.0, 1.0, 50,  0.0, 1.0);
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +73);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , 0.0, 1.0, 50,  0.0, 1.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +74);  
	hmg->addTH2F(namehist,    " y1 vs y2 ", 50  , 0.0, 1.0, 50,  0.0, 1.0);
	
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +63);  
	hmg->addTH2F(namehist,    " p_nu_long vs  p_nu_long ", 50  , -100.0, 100.0, 50,  -100.0, 100.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +64);  
	hmg->addTH2F(namehist,    " p_nu_long vs  p_nu_long ", 50  , -100.0, 100.0, 50,  -100.0, 100.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +65);  
	hmg->addTH2F(namehist,    " p_nu_E vs  p_nu_E ", 50  , 0.0, 100.0, 50,  0.0, 100.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +66);  
	hmg->addTH2F(namehist,    " p_nu_E vs  p_nu_E ", 50  , 0.0, 100.0, 50,  0.0, 100.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +67);  
	hmg->addTH2F(namehist,    " p_nu_trans vs  p_nu_PT ", 50  , 0.0, 4.0, 50,  0.0, 4.0); 
	sprintf(namehist,"histomap%.2d",idhist+ i1*10000+ i*1000 + i2*100 +68);  
	hmg->addTH2F(namehist,    " p_nu_trans vs  p_nu_PT ", 50  , 0.0, 4.0, 50,  0.0, 4.0); 

	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +81);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  1.57); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +82);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  1.57); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +83);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  1.57); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +84);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  6.28); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +85);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  6.28); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +86);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  6.28); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +87);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  6.28); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +88);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  6.28); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +89);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  6.28); 

	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +91);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  , 1.57); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +92);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  6.28); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +93);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  6.28); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +94);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +95);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +96);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +97);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +98);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  12.2830); 
	sprintf(namehist,"hist%.2d",idhist+ i1*10000+ i*1000 + i2*100 +99);  
	hmg->addTH1F(namehist,    "  ", 50  , 0.0  ,  12.2830); 

	
      }
    }
  }


}

//----------------------------------------------------
// calculates vector product of 3-vectors:  result = (v1 x v2), normalising it to unity
// function returns normalisation factor for optional use.
// useful for calculation of normal vector to the plane spaned on (v1, v2)
//----------------------------------------------------
//
double AnalTauSpinnerCP::normalised_cross_product(double * v1, double * v2, double * result){
  result[0] = v1[1]*v2[2] - v1[2]*v2[1];
  result[1] = v1[2]*v2[0] - v1[0]*v2[2];
  result[2] = v1[0]*v2[1] - v1[1]*v2[0];

  double normalisation = sqrt(result[0]*result[0]
			      +result[1]*result[1]
			      +result[2]*result[2]);

  for(int i=0; i<3; i++)
    result[i]=result[i]/normalisation;

  return normalisation;
}

//----------------------------------------------------
// calculates vector product of 3-vectors:  result = (v1 x v2), normalising it to unity
// function returns normalisation factor for optional use.
// useful for calculation of normal vector to the plane spaned on (v1, v2)
//----------------------------------------------------
//
double AnalTauSpinnerCP::P_norm_cross_product(Particle v1, Particle v2, double * result){
  result[0] = v1.py()*v2.pz() - v1.pz()*v2.py();
  result[1] = v1.pz()*v2.px() - v1.px()*v2.pz();
  result[2] = v1.px()*v2.py() - v1.py()*v2.px();

  double normalisation = sqrt(result[0]*result[0]
			      +result[1]*result[1]
			      +result[2]*result[2]);

  for(int i=0; i<3; i++)
    result[i]=result[i]/normalisation;

  return normalisation;
}

//----------------------------------------------------
// scalar product of 3-vectors
//----------------------------------------------------
//
double AnalTauSpinnerCP::dot_product(double *v1, double *v2){
  return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
}


//----------------------------------------------------
// length of 3-vector v
//----------------------------------------------------
//
double AnalTauSpinnerCP::magnitude(double *v){
  return sqrt(dot_product(v,v));
}


//----------------------------------------------------
// calculates acoplanarity angle between oriented half-planes
//----------------------------------------------------
//
double AnalTauSpinnerCP::P_acoplanarAngle(Particle p1, Particle p2, Particle p3, Particle p4){

  double kPI = 3.14159265359;

  //normal to the plane spanned by p1, p2  
  double n_1[3];
  P_norm_cross_product(p1,p2,n_1);
  double p1_3v[3]={p1.px(),p1.py(),p1.pz()};

  //normal to the plane spanned by p3, p4  
  double n_2[3];
  P_norm_cross_product(p3,p4,n_2);
	
  // calculate the acoplanarity angle
  double theta = acos(dot_product(n_1,n_2));
	
  // extend definition of acoplanarity theta to lie between 0 and 2 PI
  // that is to angle between oriented half-planes
  double pi_minus_n_plus = dot_product(p1_3v,n_2)/magnitude(p1_3v);    
  if(pi_minus_n_plus>0)
    theta=2*kPI-theta;

  return theta;
}

//------------------------------------------------------------------------------------
void AnalTauSpinnerCP::calcCosThetaPhiMustraal(Particle tau_plus, Particle tau_minus, Particle p3,
					       double& costheta1, double& costheta2, double& phi1, double& phi2, double& wt1, double& wt2){


  // P_QQ = sum of tau+ and tau- in lab frame
  Particle P_QQ( tau_plus.px()+tau_minus.px(), tau_plus.py()+tau_minus.py(), tau_plus.pz()+tau_minus.pz(), tau_plus.e()+tau_minus.e(), 0 );
  
  Particle b1, b2;
  double CMSENE = 8000;
  Particle P_QQJ( tau_plus.px()+tau_minus.px()+p3.px(), tau_plus.py()+tau_minus.py()+p3.py(), 
		  tau_plus.pz()+tau_minus.pz()+p3.pz(), tau_plus.e()+tau_minus.e()+p3.e(), 0 );
  double SS     = P_QQJ.recalculated_mass()*P_QQJ.recalculated_mass();
  double x1x2   = SS/CMSENE/CMSENE;
  double x1Mx2  = P_QQJ.pz()/CMSENE*2;      
  double x1 = (  x1Mx2 + sqrt(x1Mx2*x1Mx2 + 4*x1x2) )/2;
  double x2 = ( -x1Mx2 + sqrt(x1Mx2*x1Mx2 + 4*x1x2) )/2;

  if(DEBUG)
    std::cout << "calcCosThetaPhiMustraal: " << SS << "  " << x1 << "  " << x2 << std::endl;

  if( x1 > x2 ){
    b1.setPx(0.); b1.setPy(0.); b1.setPz( x1*CMSENE/2.0); b1.setE( x1*CMSENE/2.0 );
    b2.setPx(0.); b2.setPy(0.); b2.setPz(-x2*CMSENE/2.0); b2.setE( x2*CMSENE/2.0 );
  } else {
    b2.setPx(0.); b2.setPy(0.); b2.setPz( x1*CMSENE/2.0); b2.setE( x1*CMSENE/2.0 );
    b1.setPx(0.); b1.setPy(0.); b1.setPz(-x2*CMSENE/2.0); b1.setE( x2*CMSENE/2.0 );
  }

  tau_minus.boostToRestFrame(P_QQ);
  tau_plus. boostToRestFrame(P_QQ);

  b1.     boostToRestFrame(P_QQ);
  b2.     boostToRestFrame(P_QQ);

  // ex_lab_cms
  Particle ex_lab(1.0,0.0,0.0,0.0,0.0);
  ex_lab.     boostToRestFrame(P_QQ);
  
  // here calculate Cosine of hard scattering  
  //-------------------------------------------------------------
  costheta1 = (tau_minus.px()*b1.px()     +tau_minus.py()*b1.py()       +tau_minus.pz()*b1.pz()       ) /
    sqrt(tau_minus.px()*tau_minus.px()+tau_minus.py()*tau_minus.py()+tau_minus.pz()*tau_minus.pz()) /
    sqrt(b1.px()       *b1.px()       +b1.py()       *b1.py()       +b1.pz()       *b1.pz()       );
  
  costheta2 = (tau_plus.px()*b2.px()     +tau_plus.py()*b2.py()      +tau_plus.pz()*b2.pz()      ) /
    sqrt(tau_plus.px()*tau_plus.px()+tau_plus.py()*tau_plus.py()+tau_plus.pz()*tau_plus.pz()) /
    sqrt(b2.px()      *b2.px()      +b2.py()      *b2.py()      +b2.pz()      *b2.pz()      );

     
  // here calculate phiPrim  style A
  // here b1 is after boost to rest frame
  //-------------------------------------------------------------
  double p3p1_x = p3.py()*b1.pz() -  p3.pz()*b1.py();
  double p3p1_y = p3.pz()*b1.px() -  p3.px()*b1.pz();
  double p3p1_z = p3.px()*b1.py() -  p3.py()*b1.px();
  
  double p3tauP_x = p3.py()*tau_plus.pz() -  p3.pz()*tau_plus.py();
  double p3tauP_y = p3.pz()*tau_plus.px() -  p3.px()*tau_plus.pz();
  double p3tauP_z = p3.px()*tau_plus.py() -  p3.py()*tau_plus.px();
  
  double pr_x = p3.py()* p3tauP_z -  p3.pz()* p3tauP_y;
  double pr_y = p3.pz()* p3tauP_x -  p3.px()* p3tauP_z;
  double pr_z = p3.px()* p3tauP_y -  p3.py()* p3tauP_x;
  
  
  const float pi   =   3.1415927;
  // here calculate phiPrim born style   for tau_plus, z-axis is along b2 and x-axis is in b2 ex_lab plane
  //-------------------------------------------------------------
  
  // y-versor   ez \times ex  that is minus  ex_lab  \times b2
  double ey_x = - (ex_lab.py()*b2.pz() -  ex_lab.pz()*b2.py());
  double ey_y = - (ex_lab.pz()*b2.px() -  ex_lab.px()*b2.pz());
  double ey_z = - (ex_lab.px()*b2.py() -  ex_lab.py()*b2.px());
  double xnorm =sqrt(ey_x*ey_x+ey_y*ey_y+ey_z*ey_z);
  ey_x=ey_x/xnorm;
  ey_y=ey_y/xnorm;
  ey_z=ey_z/xnorm;
  // x-versor   ey \times ez  that is ey \times b2
  double ex_x =  (ey_y*b2.pz() -  ey_z*b2.py());
  double ex_y =  (ey_z*b2.px() -  ey_x*b2.pz());
  double ex_z =  (ey_x*b2.py() -  ey_y*b2.px());
  xnorm =sqrt(ex_x*ex_x+ex_y*ex_y+ex_z*ex_z);
  ex_x=ex_x/xnorm;
  ex_y=ex_y/xnorm;
  ex_z=ex_z/xnorm;
  double cosP=ex_x*tau_plus.px()+ex_y*tau_plus.py()+ex_z*tau_plus.pz();
  double sinP=ey_x*tau_plus.px()+ey_y*tau_plus.py()+ey_z*tau_plus.pz();
  xnorm=sqrt(cosP*cosP+sinP*sinP);
  cosP=cosP/xnorm;
  sinP=sinP/xnorm;
  // now cosP sinP can be used for phiPrim of (tauplus b2 born) wt2
  
  phi1 = acos(cosP);
  // we need range 0 to 2pi     
  if(pr_x*p3p1_x+pr_y*p3p1_y+pr_z*p3p1_z<0) phi1=2*pi-phi1;
  
  
  // here calculate phiPrim born style  for tau_minus, z-axis is along b1 and x-axis is in b2 ex_lab plane
  //-------------------------------------------------------------
  
  // y-versor   ez \times ex  that is minus  ex_lab  \times b2
  ey_x = - (ex_lab.py()*b1.pz() -  ex_lab.pz()*b1.py());
  ey_y = - (ex_lab.pz()*b1.px() -  ex_lab.px()*b1.pz());
  ey_z = - (ex_lab.px()*b1.py() -  ex_lab.py()*b1.px());
  xnorm =sqrt(ey_x*ey_x+ey_y*ey_y+ey_z*ey_z);
  ey_x=ey_x/xnorm;
  ey_y=ey_y/xnorm;
  ey_z=ey_z/xnorm;
  // x-versor   ey \times ez  that is ey \times b1
  ex_x =  (ey_y*b1.pz() -  ey_z*b1.py());
  ex_y =  (ey_z*b1.px() -  ey_x*b1.pz());
  ex_z =  (ey_x*b1.py() -  ey_y*b1.px());
  xnorm =sqrt(ex_x*ex_x+ex_y*ex_y+ex_z*ex_z);
  ex_x=ex_x/xnorm;
  ex_y=ex_y/xnorm;
  ex_z=ex_z/xnorm;
  
  double cosM=ex_x*tau_minus.px()+ex_y*tau_minus.py()+ex_z*tau_minus.pz();
  double sinM=ey_x*tau_minus.px()+ey_y*tau_minus.py()+ey_z*tau_minus.pz();
  xnorm=sqrt(cosM*cosM+sinM*sinM);
  cosM=cosM/xnorm;
  sinM=sinM/xnorm;
  // now cosM sinM can be used for phiPrim of (tauminus b1 born) wt1
  // this give range from 0 to pi
  
  phi2 = acos(cosM);
  // we need range 0 to 2pi     
  if(pr_x*p3p1_x+pr_y*p3p1_y+pr_z*p3p1_z<0) phi2=2*pi-phi2;
  
  
  // here calculate weights 
  //-------------------------------------------------------------
  double BB1=1.0+costheta1*costheta1;
  double BB2=1.0+costheta2*costheta2;
  wt1 = b1.e()*b1.e()*BB1/( b1.e()*b1.e()*BB1+b2.e()*b2.e()*BB2 );
  wt2 = b2.e()*b2.e()*BB2/( b1.e()*b1.e()*BB1+b2.e()*b2.e()*BB2 );

  
}
