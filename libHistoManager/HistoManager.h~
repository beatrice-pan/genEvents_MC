//! @file   HistoManager.h
//! @author Elzbieta Richter-Was <elzbieta.richter-was@cern.ch>
//! @date   created December 2004
#ifndef HISTOMANAGER_H
#define HISTOMANAGER_H

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TProfile.h>
#include <TClonesArray.h>

#include <string>
#include <vector>
#include <iostream>

class HistDefTH1F {
 public: 
  std::string m_name;
  std::string m_title;

  int   m_nbins;
  float m_xlow;
  float m_xhigh; 

  HistDefTH1F(){}   
  HistDefTH1F(const char *name, const char *title, int bins, float xlow, float xhigh);


};

class HistDefTH2F {
 public: 
  std::string m_name;
  std::string m_title;

  int   m_n1bins;
  float m_x1low;
  float m_x1high;
  int   m_n2bins;
  float m_x2low;
  float m_x2high;
  
  HistDefTH2F(){} 
  HistDefTH2F(const char *name, const char *title, int bins1, float xlow1, float xhigh1,int bins2, float xlow2, float xhigh2);



};

class HistDefTH3F {
 public: 
  std::string m_name;
  std::string m_title;

  int   m_n1bins;
  float m_x1low;
  float m_x1high;
  int   m_n2bins;
  float m_x2low;
  float m_x2high;
  int   m_n3bins;
  float m_x3low;
  float m_x3high;

  HistDefTH3F(){}  
  HistDefTH3F(const char *name, const char *title, int bins1, float xlow1, float xhigh1,int bins2, float xlow2, float xhigh2,
                                                   int bins3, float xlow3, float xhigh3);


};

class HistDefTProf {
 public: 
  std::string m_name;
  std::string m_title;

  int   m_nbins;
  float m_x1low;
  float m_x1high;
  float m_x2low;
  float m_x2high;

  HistDefTProf(){}
  HistDefTProf(const char *name, const char *title, int bins, float xlow1, float xhigh1,float xlow2 = 0., float xhigh2 = 1.);
 

};


//! @class HistoManager:
//! Provides intialisation/bookeeping/storage for all histograms 
class HistoManager: public TObject {
   public : 
   //! Constructor
   HistoManager();
   //! Destructor     
   ~HistoManager();

   private:
   static HistoManager* gHstMan;
   public:
   //! Creates instance of the class   
   static HistoManager* getInstance();


  void CreateHistTables();              // creates all the histograms in the TClonesArray
  void StoreHistos();                   // saves all the histograms in the TClonesArray 

  
  /** Helper methods to fill the histograms */
  TH1F     *GetHistoTH1F(int x);
  TH2F     *GetHistoTH2F(int x);
  TH3F     *GetHistoTH3F(int x);
  TProfile *GetHistoTProf(int x);

  /** Histo adders*/
  void addTH1F(const  char *name, const char *title, int bins,  float xlow, float xhigh);
  void addTH1F(const  char *name, const char *title, int bins, const double *xbins );
  void addTH2F(const  char *name, const char *title, int bins1, float xlow1, float xhigh1,int bins2, float xlow2, float xhigh2);
  void addTProf(const char *name, const char *title, int bins,  float xlow1, float xhigh1,float xlow2 = 0., float xhigh2 = 1.);
  void addTH2F(const  char *name, const char *title, int binsx, const double *xbins , int binsy, const double *ybins );
  void addTH3F(const  char *name, const char *title, int bins1, float xlow1, float xhigh1,int bins2, float xlow2, float xhigh2,
                                                     int bins3, float xlow3, float xhigh3 );
  void addTH3F(const  char *name, const char *title, int binsx, const double *xbins , int binsy, const double *ybins,
                                                     int binsz, const double *zbins );

 private:

  /** Histograms and profiles*/
  std::vector <HistDefTH1F>  m_histoTH1F;
  std::vector <HistDefTH2F>  m_histoTH2F;
  std::vector <HistDefTProf> m_histoTProf;
  std::vector <HistDefTH2F>  m_histoTH3F;

  TClonesArray      *m_TH1FArray;   //!< list of histograms TH1F;
  TClonesArray      *m_TH2FArray;   //!< list of histograms TH2F;
  TClonesArray      *m_TProfArray;  //!< list of histograms TProf;
  TClonesArray      *m_TH3FArray;   //!< list of histograms TH3F;

  int th1fSize;
  int th2fSize;
  int th3fSize;
  int tprofSize;

   ClassDef(HistoManager,1) // End of CLASS  HistoManager


};

#endif
