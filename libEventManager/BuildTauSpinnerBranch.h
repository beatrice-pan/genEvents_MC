//! @file   BuildTauSpinnerBranch.h
//! @author Elzbieta Richter-Was <elzbieta.richter-was@cern.ch>
//! @date   created June 2015
#ifndef BUILDTAUSPINNERBRANCH_H
#define BUILDTAUSPINNERBRANCH_H

#include <TROOT.h>
#include "TLorentzVector.h"
#include "TVector.h"
#include <TRandom.h>

// HepMC include file
#include "HepMC/GenEvent.h"
#include "HepMC/GenParticle.h"
#include "HepMC/IO_GenEvent.h"

// TauSpinner include file
#include "TauSpinner/SimpleParticle.h"
#include <vector>


using namespace std;
using namespace TauSpinner;

//! @class BuildTauSpinnerBranch:
class BuildTauSpinnerBranch{

 private:
  
 public:
  //! Constructor
  BuildTauSpinnerBranch();
  //! Destructor
  ~BuildTauSpinnerBranch();
  
 private:
  static BuildTauSpinnerBranch* gBuildTauSpinnerBranch;
  
 public:
  //! Creates instance of the class   
  static BuildTauSpinnerBranch* getInstance();
  //! Executes subalgorithm:   
  void Initialize(char *filename, int idhist);
  void Finalize();
  //! Executes subalgorithms  
  int getZDecayDaughters(HepMC::GenEvent *evt, SimpleParticle &X, vector<SimpleParticle> &daughters);
  
  int buildSimpleBranch(HepMC::GenEvent *evt, SimpleParticle &X, SimpleParticle &tau, SimpleParticle &tau2, 
		        vector<SimpleParticle> &tau_daughters, vector<SimpleParticle> &tau2_daughters);
  int readSimpleBranch(HepMC::IO_GenEvent &input_file, SimpleParticle &X, SimpleParticle &tau, SimpleParticle &tau2, 
		        vector<SimpleParticle> &tau_daughters, vector<SimpleParticle> &tau2_daughters);
  int readVBFBranch(HepMC::IO_GenEvent &input_file, SimpleParticle &p1, SimpleParticle &p2, SimpleParticle &X, 
                        SimpleParticle &p3, SimpleParticle &p4, SimpleParticle &tau, SimpleParticle &tau2, 
		        vector<SimpleParticle> &tau_daughters, vector<SimpleParticle> &tau2_daughters);
  int buildVBFBranch(HepMC::GenEvent *evt, SimpleParticle &p1, SimpleParticle &p2, SimpleParticle &X, 
                        SimpleParticle &p3, SimpleParticle &p4, SimpleParticle &tau, SimpleParticle &tau2, 
		        vector<SimpleParticle> &tau_daughters, vector<SimpleParticle> &tau2_daughters);
  int readHqBranch(HepMC::IO_GenEvent &input_file, SimpleParticle &p1, SimpleParticle &p2, SimpleParticle &X, 
                        SimpleParticle &p3, SimpleParticle &tau, SimpleParticle &tau2, 
		        vector<SimpleParticle> &tau_daughters, vector<SimpleParticle> &tau2_daughters);
  int buildHqBranch(HepMC::GenEvent *evt, SimpleParticle &p1, SimpleParticle &p2, SimpleParticle &X, 
                        SimpleParticle &p3, SimpleParticle &tau, SimpleParticle &tau2, 
		        vector<SimpleParticle> &tau_daughters, vector<SimpleParticle> &tau2_daughters);
  int readBornBranch(HepMC::IO_GenEvent &input_file, SimpleParticle &p1, SimpleParticle &p2, SimpleParticle &X, 
                        SimpleParticle &tau, SimpleParticle &tau2, 
		        vector<SimpleParticle> &tau_daughters, vector<SimpleParticle> &tau2_daughters);
  int buildBornBranch(HepMC::GenEvent *evt, SimpleParticle &p1, SimpleParticle &p2, SimpleParticle &X, 
                        SimpleParticle &tau, SimpleParticle &tau2, 
		        vector<SimpleParticle> &tau_daughters, vector<SimpleParticle> &tau2_daughters);
  //! helpers  
  SimpleParticle GenParticle_to_SimpleParticle(HepMC::GenParticle *p); 

  //! Algorithm for monitoring
  
  //! Helpers
/* Get decay channel  (sequence of daughters matter)
   Returns:
   -1 - unrecognized (unsupported) channel
    0 - no daughters
    1 - electron (tau -> nu_tau nue e [+gamma])
    2 - muon     (tau -> nu_tau nu_mu mu [+gamma])
    3 - pion     (tau -> nu_tau pi)
    4 - rho      (tau -> nu_tau pi0 pi) */
  int get_decay_channel(vector<SimpleParticle> &tau_daughters);

  /** Get direct daughters of HepMC::GenParticle 'x' */
  vector<SimpleParticle> *getDirectDaughters(HepMC::GenParticle *x);

  /** Get daughters of HepMC::GenParticle
      Recursively searches for final-state daughters of 'x' */
  vector<SimpleParticle> *getDaughters(HepMC::GenParticle *x);

  /**  Calculates cosThetaStar, as introduced for asymmetry definition
    in the ActaPhysPolon  */
  double get_costheta(SimpleParticle &pp1, SimpleParticle &pp2);

  /**
  Search for qqH production branch, defined by vertex where X of pdg_id = 25 is outgoing and two incoming 
  particles are found (pdgid not checked) */
  bool findProdBranchHqq(HepMC::GenEvent *evt, HepMC::GenParticle *&beam1, HepMC::GenParticle *&beam2);

  /**
  Search for qH production branch, defined by vertex where X of pdg_id = 25 is outgoing and two incoming 
  particles are found (pdgid not checked) */
  bool findProdBranchHq(HepMC::GenEvent *evt, HepMC::GenParticle *&beam1, HepMC::GenParticle *&beam2);

  /**
  Search for Born production branch, defined by vertex where X of pdg_id = 25, 23 is outgoing and two incoming 
  particles are found (pdgid not checked) */
  bool findProdBranchBorn(HepMC::GenEvent *evt, HepMC::GenParticle *&beam1, HepMC::GenParticle *&beam2);

  /**
  Search for Hqq decay branch, defined by vertex where X of pdg_id = 25 and two outgoing particles
  are found (pdgid not checked)
 */
  bool findDecayBranchHqq(HepMC::GenEvent *evt, HepMC::GenParticle *mother, HepMC::GenParticle *&X, HepMC::GenParticle *&jet1, HepMC::GenParticle *&jet2); 

  /**
  Search for Hq decay branch, defined by vertex where X of pdg_id = 25 and one outgoing particle
  are found (pdgid not checked)
 */
  bool findDecayBranchHq(HepMC::GenParticle *mother, HepMC::GenParticle *&X, HepMC::GenParticle *&jet1); 

  /**
  Search for Born decay branch, defined by vertex where X of pdg_id = 25, 23 is found
 */
  bool findDecayBranchBorn(HepMC::GenParticle *mother, HepMC::GenParticle *&X); 

  /**
  Search for tau pair from particle X, defined by vertex where tau is particle of pdg_id = 15,
  and tau2 is particle from the same decay vertex (pdgid not checked) */
  bool findDecayBranchXtautau(HepMC::GenEvent *evt, HepMC::GenParticle *X, HepMC::GenParticle *&tau, HepMC::GenParticle *&tau2);

  //! Histograms instantation 
  void CreateHistos( int idhist);
  
  //generic id for histograms
  int m_idHist;

  //generic random engine
  TRandom random;



  
  //  ClassDef(BuildTauSpinnerBranch,1) // End of CLASS  BuildTauSpinnerBranch
    };

#endif

