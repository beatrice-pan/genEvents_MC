/**
 *  @brief Example of using input file in  HepMC format
 *
 *  Usage ./hepmcfile-testCP.exe <input_file> <anal_mode> <events_count>
 *
 */
#include <iostream>

// HepMC IO_GenEvent header
#include "HepMC/IO_GenEvent.h"

// HepMC include file
#include "HepMC/GenEvent.h"

//PHOTOS header files
//#include "Photos/PhotosHepMCEvent.h"


//TAUOLA header files
#include "Tauola/Tauola.h"
#include "Tauola/TauolaHepMCEvent.h"

#include <TRandom.h>

// Particle class for boosts, angles calculation and rotations
#include "TauSpinner/Particle.h"

// Histomanager
#include <TROOT.h>
#include "../libHistoManager/HistoManager.h"

//Configuration for TauSpinner and Tauola
#include "../libTauSpinnerManager/TauSpinnerConfig.h"

//Configuration for Photos
//#include "../libPhotosManager/PhotosConfig.h"

// Analysis classes
#include "../libAnalAlgo/AnalTauSpinnerCP.h"

using std::cout;
using std::endl;
using namespace HepMC;
using namespace Tauolapp;

const bool  DEBUG = 0;

//replacement of default (not best quality) random number generator                                                                                                         
      
//----------------------------------------------------------------------------
int main( int argc, char **argv ) {

    if( argc < 3 ) {
      //        cout<<"Usage: "<<argv[0]<<" <input_file> <anal_mode> <events_skip>  <events_count>"<<endl;
        cout<<"Usage: "<<argv[0]<<" <input_file> <anal_mode>  <evt_max>"<<endl;
        return -1;
    }

    char *input_filename = argv[1];
    // Open I/O files  (in our example events are taken from "events.dat")
    HepMC::IO_GenEvent input_file(input_filename,std::ios::in);

    int anal_mode = atoi( argv[2] ); 
    //    int evt_skip  = atoi( argv[3] );
    int evt_max   = atoi( argv[3] );



    //create histomanager instance
    HistoManager *hmg    = HistoManager::getInstance();
    // tables for histograms created
    hmg->CreateHistTables();

    //create TauSpinner and Tauola configuration  instance
    TauSpinnerConfig   *tauspinnermg  = TauSpinnerConfig::getInstance();
    tauspinnermg->Initialize("", 0, anal_mode);  // idhist

    if (anal_mode == 0 ) {
      for(int i=1;i<=30;i++) Tauola::setTauBr(i,0);
      Tauola::setTauBr(4,1.0);
    }

    //create analysis instance
    AnalTauSpinnerCP   *analTauSpinnerCP    = AnalTauSpinnerCP::getInstance();
    analTauSpinnerCP->Initialize("",  10000000);  // idhist
    

    bool status = true;
    int iEvent = 0;
    int iEventSkip = 0;
    int iEventRead = 0;

    if(DEBUG)
      printf("evt_max = %10d\n",evt_max);

    while(true){

      if(DEBUG) 
	printf("Analysed: read next event \n");

      HepMC::GenEvent  *hepmc = new HepMC::GenEvent();
      // Read event from input file
      status = input_file.fill_next_event(hepmc);
      iEventRead++;

      // std::cout << " before redecay " << std::endl;
      // hepmc->print();      
      /*
      if (anal_mode == 0 ) {
	// Run TAUOLA on the event
	TauolaHepMCEvent * t_event = new TauolaHepMCEvent(hepmc);
	// We may want to undecay previously decayed taus.
	t_event->undecayTaus();
	t_event->decayTaus();
	delete t_event;
      
	//      std::cout << "after redecay " << std::endl;
	// hepmc->print();
      }
      */

      /*
      std::cout << " =====  Before Photos ======" << std::endl;
      hepmc->print();

      // Run PHOTOS on the event
      PhotosHepMCEvent evt(hepmc);
      evt.process();

      std::cout << " ===== After Photos  =======" << std::endl;
      hepmc->print();
      */
 

      if(DEBUG)
      printf("Analysed: %10d events, Skipped:  %10d events, Read: %10d events \n", iEvent, iEventSkip, iEventRead);

      if(DEBUG) 
	printf("Analysed: status=%d \n",status);

      if(iEvent%1000 == 0  && DEBUG) 
	printf("Read: %10d events \n", iEventRead);

    
      // Finish if there is nothing more to read from the file
      if( status==0 ) break;

      /*
      iEventSkip++;
      if( iEventSkip < evt_skip ) continue;
      iEventSkip = evt_skip;

      if( iEvent + evt_skip > evt_max ) break;
      */      

      if( iEvent > evt_max ) break;

      if(DEBUG)
	printf("Analysed: process next \n"); 
     
      iEvent++;

      // Analyse event (in hepmc format)
      analTauSpinnerCP->Make( hepmc, anal_mode);

      if(DEBUG)
	printf("Analysed: %10d events \n", iEvent);

      if(iEvent%1000 == 0) 
	printf("Analysed: %10d events \n", iEvent);

    }

    //final statistics + output rootfiles 
    printf("Analysed in total: %10d events \n", iEvent);
    //// ************************************************
    
    analTauSpinnerCP->Finalize();    
    
    //// ************************************************
    char outname[100]="";
    strcat (outname,argv[1]);
    strcat (outname,"_mode_");
    strcat (outname,argv[2]);
    strcat (outname,".root");
    TFile outFile(outname, "recreate");
    outFile.cd();
    hmg->StoreHistos();
    //// ************************************************
    
    return 0;
}
