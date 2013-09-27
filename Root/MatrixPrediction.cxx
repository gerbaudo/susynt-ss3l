#include "SusyntHlfv/MatrixPrediction.h"

#include "SusyntHlfv/WeightComponents.h"
#include "SusyntHlfv/EventFlags.h"
//#include "SusyntHlfv/DileptonVariables.h"

#include "DileptonMatrixMethod/Systematic.h"
#include "SusyNtuple/SusyDefs.h"

#include <iomanip>
#include <sstream>      // std::ostringstream

using namespace std;
using namespace Susy;
namespace sf = susy::fake;
using hlfv::MatrixPrediction;
using hlfv::Selector;
using hlfv::WeightComponents;
using hlfv::EventFlags;
using hlfv::DileptonVariables;
using sf::Parametrization;


//----------------------------------------------------------
MatrixPrediction::MatrixPrediction() :
   Selector(),
    m_matrix(0),
    m_use2dparametrization(false),
    m_allconfigured(false),
    m_dbg(false)
{
}
//----------------------------------------------------------
void MatrixPrediction::Begin(TTree* /*tree*/)
{
  if(m_dbg) cout << "MatrixPrediction::Begin" << endl;
  Selector::Begin(0);
  m_allconfigured = initMatrixTool();
}
//----------------------------------------------------------
Bool_t MatrixPrediction::Process(Long64_t entry)
{
#warning todo rename smm hmp


    m_counter.nextEvent();
    m_counterEmu.nextEvent();
    m_counterMue.nextEvent();
    m_printer.countAndPrint(cout);
    GetEntry(entry);
    m_chainEntry++; // SusyNtAna counter
    clearObjects();
    WeightComponents weightComponents;
    assignStaticWeightComponents(nt, *m_mcWeighter, weightComponents);
    m_counter.pass(weightComponents.product());
    bool removeLepsFromIso(false);
    selectObjects(NtSys_NOM, removeLepsFromIso, TauID_medium); // always select with nominal? (to compute event flags)
    EventFlags eventFlags = computeEventFlags();
    incrementEventCounters(eventFlags, weightComponents);
    if(eventFlags.passAllEventCriteria()) {
        const JetVector&    j = m_signalJets2Lep;
        const JetVector&   bj = m_baseJets; // why are we using basejets and not m_signalJets2Lep?
        const LeptonVector& l = m_baseLeptons;
        const Met*          m = m_met;
        if(eventHasTwoLeptons(l) && eventIsEmu(l)) { // several vars cannot be computed if we don't have 2 lep
            // SsPassFlags ssf(SusySelection::passSrSs(WH_SRSS1, ncl, t, j, m, allowQflip));
            // if(!ssf.passCommonCriteria()) return false;
            // if(m_writeTuple && ssf.lepPt) {
            // const Susy::Lepton *l0=baseLeps[0], *l1=baseLeps[1];
            double gev=1.0;
            unsigned int run(nt.evt()->run), event(nt.evt()->event);
            uint nVtx = nt.evt()->nVtx;
            bool isMC = nt.evt()->isMC;
            const Lepton &l0 = *m_signalLeptons[0];
            const Lepton &l1 = *m_signalLeptons[1];
            float metRel = getMetRel(m, l, j);
            bool l0IsSig(SusyNtTools::isSignalLepton(&l0, m_baseElectrons, m_baseMuons, nVtx, isMC));
            bool l1IsSig(SusyNtTools::isSignalLepton(&l1, m_baseElectrons, m_baseMuons, nVtx, isMC));
            string regionName="emu";
            sf::Systematic::Value sys = sf::Systematic::SYS_NOM;
            size_t iRegion = m_matrix->getIndexRegion(regionName); 
            sf::Lepton fl0(l0IsSig, l0.isEle(), l0.Pt()*gev, l0.Eta());
            sf::Lepton fl1(l1IsSig, l1.isEle(), l1.Pt()*gev, l1.Eta());
            double weight = m_matrix->getTotalFake(fl0, fl1, iRegion, metRel*gev, sys);
            m_tupleMaker.fill(weight, run, event, l0, l1, *m_met);
                // const JetVector clJets(SusySelection::filterClJets(m_signalJets2Lep));
                // m_tupleMaker.fill(weight, run, event, *l0, *l1, *m, lowPtLep, m_signalJets2Lep);
        }
    } // if(emu)
    return true;
}
//----------------------------------------------------------
void MatrixPrediction::Terminate()
{
    Selector::Terminate();
    if(m_dbg) cout << "MatrixPrediction::Terminate" << endl;
    delete m_matrix;
}
//----------------------------------------------------------
float MatrixPrediction::getFakeWeight(const LeptonVector &baseLeps,
                                      std::string &regionName,
                                      float metRel,
                                      susy::fake::Systematic::Value)
{
/*
  if(baseLeps.size() != 2) return 0.0;
  uint nVtx = nt.evt()->nVtx;
  bool isMC = nt.evt()->isMC;
  float gev2mev(1000.);
  //m_matrix->setDileptonType(baseLeps[0]->isEle(), baseLeps[1]->isEle());
  const Susy::Lepton *l0=baseLeps[0], *l1=baseLeps[1];
  bool l0IsSig(SusyNtTools::isSignalLepton(l0, m_baseElectrons, m_baseMuons, nVtx, isMC));
  bool l1IsSig(SusyNtTools::isSignalLepton(l1, m_baseElectrons, m_baseMuons, nVtx, isMC));
  return m_matrix->getTotalFake(l0IsSig, l0->isEle(), l0->Pt(), l0->Eta(),
                                l1IsSig, l1->isEle(), l1->Pt(), l1->Eta(),
                                region, metRel, sys);
*/
    return 0.0;
}
//----------------------------------------------------------
float MatrixPrediction::getRFWeight(const LeptonVector &baseLeps,
                                    std::string &regionName,
                                    float metRel,
                                    susy::fake::Systematic::Value)
{
/*
  if(baseLeps.size() != 2) return 0.0;
  uint nVtx = nt.evt()->nVtx;
  bool isMC = nt.evt()->isMC;
  return m_matrix->getRF( isSignalLepton(baseLeps[0],m_baseElectrons, m_baseMuons,nVtx,isMC),
                          baseLeps[0]->isEle(),
                          baseLeps[0]->Pt(),
                          baseLeps[0]->Eta(),
                          isSignalLepton(baseLeps[1],m_baseElectrons, m_baseMuons,nVtx,isMC),
                          baseLeps[1]->isEle(),
                          baseLeps[1]->Pt(),
                          baseLeps[1]->Eta(),
                          regionName,
                          metRel,
                          sys);
*/
    return 0.0;
}
//----------------------------------------------------------
MatrixPrediction& MatrixPrediction::setMatrixFilename(const std::string filename)
{
//  if(!fileExists(filename))
    if(true)
    cout<<"MatrixPrediction::setMatrixFilename: invalid file '"<<filename<<"'"<<endl
        <<"\t"<<"something will go wrong"<<endl;
  m_matrixFilename = filename;
  return *this;
}
//----------------------------------------------------------
bool MatrixPrediction::initMatrixTool()
{
    m_matrix = new sf::DileptonMatrixMethod();
    sf::Parametrization::Value p = (m_use2dparametrization ? sf::Parametrization::PT_ETA : sf::Parametrization::PT);
    std::vector<std::string> regions;
    regions.push_back("emu");
    return m_matrix->configure(m_matrixFilename, regions, p, p, p, p);
}
//----------------------------------------------------------
std::string MatrixPrediction::dilepDetails(const Susy::Event &event,
                                           const DiLepEvtType &ll,
                                           const LeptonVector &ls)
{
  bool ee(ll==ET_ee), mm(ll==ET_mm);
  const Lepton *l0(ls.size()>0 ? ls[0] : NULL), *l1(ls.size()>1 ? ls[1] : NULL);
  float l0pt(l0 ? l0->Pt() : 0.0), l0eta(l0 ? l1->Eta() : 0.0);
  float l1pt(l1 ? l1->Pt() : 0.0), l1eta(l1 ? l1->Eta() : 0.0);
  std::ostringstream oss;
  oss<<"run "<<event.run
     <<" evt "<<event.event
     <<" "<<(ee?"ee":(mm?"mm":"em"))
     <<" l0: pt="<<l0pt<<" eta="<<l0eta
     <<" l1: pt="<<l1pt<<" eta="<<l1eta;
  return oss.str();
}
//----------------------------------------------------------
std::string MatrixPrediction::eventDetails(bool passSrSs, const Susy::Event &event,
                                           const DiLepEvtType &ll,
                                           const LeptonVector &ls)
{
  std::ostringstream oss;
  oss<<"MatrixPrediction passSrSs("<<(passSrSs?"true":"false")<<")"
     <<" "<<dilepDetails(event, ll, ls)
//     <<" weight="<<m_weightComponents.fake
      ;
  return oss.str();
}
//----------------------------------------------------------