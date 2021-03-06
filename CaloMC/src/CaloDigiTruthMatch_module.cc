//
// An EDProducer Module to match CaloHits to MC info
//
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art_root_io/TFileService.h"
#include "art_root_io/TFileDirectory.h"
#include "fhiclcpp/types/Atom.h"

#include "MCDataProducts/inc/CaloEDepMC.hh"
#include "MCDataProducts/inc/CaloDigiMC.hh"
#include "MCDataProducts/inc/CaloShowerStep.hh"
#include "MCDataProducts/inc/CaloShowerSim.hh"
#include "MCDataProducts/inc/CaloMCTruthAssns.hh"
#include "MCDataProducts/inc/PrimaryParticle.hh"
#include "MCDataProducts/inc/SimParticle.hh"
#include "MCDataProducts/inc/MCRelationship.hh"
#include "RecoDataProducts/inc/CaloHit.hh"
#include "Mu2eUtilities/inc/CaloPulseShape.hh"

#include "TH2F.h"
#include "TFile.h"

#include <iostream>
#include <string>
#include <cmath>
#include <map>
#include <vector>


namespace mu2e {

  class CaloDigiTruthMatch : public art::EDProducer 
  {
      public:
         struct Config 
         {
             using Name    = fhicl::Name;
             using Comment = fhicl::Comment;
             fhicl::Atom<art::InputTag>  caloShowerSimCollection  { Name("caloShowerSimCollection"),  Comment("Name of caloShowerSim Collection") };
             fhicl::Atom<art::InputTag>  caloHitCollection        { Name("caloHitCollection"),        Comment("Name of CaloHit collection") };
             fhicl::Atom<art::InputTag>  primaryParticle          { Name("primaryParticle"),	      Comment("PrimaryParticle producer")};
             fhicl::Atom<double>         MeVToADC                 { Name("MeVToADC"),                 Comment("MeV to ADC conversion factor") }; 
             fhicl::Atom<double>         digiSampling             { Name("digiSampling"),             Comment("Digitization time sampling") }; 
             fhicl::Atom<double>         minAmplitude             { Name("minAmplitude"),             Comment("Minimum amplitude of waveform to define hit length") }; 
             fhicl::Atom<bool>           fillDetailedMC           { Name("fillDetailedMC"),           Comment("Fill SimParticle - SimShower Assn map")};
             fhicl::Atom<int>            diagLevel                { Name("diagLevel"),                Comment("Diag Level"),0 };
         };


        explicit CaloDigiTruthMatch(const art::EDProducer::Table<Config>& config) :
           EDProducer{config},
           caloShowerSimToken_ {consumes<CaloShowerSimCollection> (config().caloShowerSimCollection())},
           CaloHitToken_       {consumes<CaloHitCollection>(config().caloHitCollection())},
           ppToken_            {consumes<PrimaryParticle>(config().primaryParticle())},
           MeVToADC_           (config().MeVToADC()),
           digiSampling_       (config().digiSampling()),
           minAmplitude_       (config().minAmplitude()),
           fillDetailedMC_     (config().fillDetailedMC()),
           diagLevel_          (config().diagLevel())
        {
            produces<CaloDigiMCCollection>();    
            produces<CaloDigiMCTruthAssn>();    
            if (fillDetailedMC_) produces<CaloShowerMCTruthAssn>();    
        }

        void beginJob() override;
        void produce(art::Event& e) override;
        void beginRun(art::Run& aRun) override;


      private:
         using SimParticlePtr = art::Ptr<SimParticle>;

         void makeTruthMatch (art::Event&, CaloDigiMCCollection&, CaloDigiMCTruthAssn&, CaloShowerMCTruthAssn&, const PrimaryParticle&);
         void diag           (const CaloShowerSim*, const CaloHit* );


         const art::ProductToken<CaloShowerSimCollection>  caloShowerSimToken_;
         const art::ProductToken<CaloHitCollection> CaloHitToken_;
         const art::ProductToken<PrimaryParticle>          ppToken_;
         double               deltaTimeMinus_;
         double               MeVToADC_;
         double               digiSampling_;
         double               minAmplitude_;
         bool                 fillDetailedMC_;
         std::vector<double>  wf_;
         int                  diagLevel_;

         //some diagnostic histograms
         TH1F*  hTime_;
         TH1F*  hTime2_;
         TH2F*  hTime2d_;
         TH2F*  hEner2d_;
         TH2F*  hEnerTime_;
         TH2F*  hdEdT_;
         TH1F*  hChi2_;
   };







  //--------------------------------------------------------------------
  void CaloDigiTruthMatch::beginJob()
  {
      if ( diagLevel_ > 2)
      {
	  art::ServiceHandle<art::TFileService> tfs;
	  hTime_     = tfs->make<TH1F>("hTime",    "delta Time",   2000, -20., 180);
	  hTime2_    = tfs->make<TH1F>("hTime2",   "delta Time",   2000, -20., 180);
	  hTime2d_   = tfs->make<TH2F>("hTime2d",  "Reco vs Gen time",  200,500,1700, 200,500,1700);
	  hEner2d_   = tfs->make<TH2F>("hEner2d",  "Reco vs gen Ener",  200,0,40,  200,0.,40);
	  hEnerTime_ = tfs->make<TH2F>("hTimeEner","delta Time vs Ener",500,0,100, 300,-50.,250);
	  hdEdT_     = tfs->make<TH2F>("hdEdt",    "delta Time vs delta Ener",100,-10,70, 170,-10.,160);
	  hChi2_     = tfs->make<TH1F>("hChi2",    "chi2 large dE",     50, 0., 10);
      }
  }

  //-----------------------------------------------------------------------------
  void CaloDigiTruthMatch::beginRun(art::Run& aRun)
  {
      CaloPulseShape cps(digiSampling_);
      cps.buildShapes();
      
      wf_ = cps.digitizedPulse(0);
      for (auto& v : wf_) v *= MeVToADC_;
  }


  //--------------------------------------------------------------------
  void CaloDigiTruthMatch::produce(art::Event& event)
  {
      auto pph = event.getValidHandle<PrimaryParticle>(ppToken_);
      auto const& primaryParticles = *pph;
      
      std::unique_ptr<CaloDigiMCCollection>  caloDigiMCs(new CaloDigiMCCollection);
      std::unique_ptr<CaloDigiMCTruthAssn>   caloDigiMCTruth(new CaloDigiMCTruthAssn);
      std::unique_ptr<CaloShowerMCTruthAssn> caloHitMCTruth(new CaloShowerMCTruthAssn);

      makeTruthMatch(event, *caloDigiMCs, *caloDigiMCTruth, *caloHitMCTruth, primaryParticles);

      event.put(std::move(caloDigiMCTruth));
      event.put(std::move(caloDigiMCs));
      if (fillDetailedMC_) event.put(std::move(caloHitMCTruth));
  } 

  
  
  // first, sort the CaloShowers/ CaloHits into vector for each crystalId
  // next, for each crystal, sort the corresponding vectors by time;
  // finally, perform the association with the following rules:
  //   MCtime must be inside the window [recoTime-deltaTimeMinus, recoTime+deltaTimePlus] to be associated to RecoHit
  //   MCtime must not already be in the window of the next hit, in which case it is associate to this one
  //
  //--------------------------------------------------------------------
  void CaloDigiTruthMatch::makeTruthMatch(art::Event& event, CaloDigiMCCollection& caloDigiMCs,
                                         CaloDigiMCTruthAssn& caloDigiTruthMatch, CaloShowerMCTruthAssn& caloShowerTruthMatch, 
                                         const PrimaryParticle& primaryParticle)
  {
        
      art::ProductID digiMCProductID(event.getProductID<CaloDigiMCCollection>());
      const art::EDProductGetter* digiMCProductGetter = event.productGetter(digiMCProductID);

      const auto CaloHitHandle = event.getValidHandle(CaloHitToken_);
      const auto& CaloHits(*CaloHitHandle);
      const auto caloShowerSimHandle = event.getValidHandle(caloShowerSimToken_);
      const auto& caloShowerSims(*caloShowerSimHandle);
     
      
      std::map<int, std::vector<const CaloHit*>> caloHitMap;
      std::map<int,std::vector<const CaloShowerSim*>>   caloShowerSimsMap;
      for (auto const& CaloHit: CaloHits) caloHitMap[CaloHit.crystalID()].push_back(&CaloHit);
      for (auto const& caloShowerSim: caloShowerSims)   caloShowerSimsMap[caloShowerSim.crystalID()].push_back(&caloShowerSim);
      
      int wfBinMax = std::distance(wf_.begin(),std::max_element(wf_.begin(),wf_.end())); 
      
      int nMatched(0);
      double totalEnergyMatched(0);
      
      
      for (auto &kv : caloHitMap)
      {
          int crystalId = kv.first;

          std::vector<const CaloHit*> &caloHits = kv.second;          
          std::sort(caloHits.begin(),caloHits.end(), [](auto const a, auto const b){return a->time() < b->time();});
          
          std::vector<const CaloShowerSim*>& caloSims = caloShowerSimsMap[crystalId];
          std::sort(caloSims.begin(),caloSims.end(), [](auto const a, auto const b){return a->time() < b->time();});
          
          if (diagLevel_ > 2) 
             for (const auto& shower : caloSims) 
                 std::cout<<"[CaloDigiTruthMatch] Sim shower  id/energy/time="<<shower->crystalID()<<" / "<<shower->energyDep()<<" / "<<shower->time()<<std::endl;
          

          auto showerIt    = caloSims.begin();
          auto showerItEnd = caloSims.end();
          auto hitIt       = caloHits.begin();
          auto hitItEnd    = caloHits.end();

          while (hitIt != hitItEnd)
          {            
             std::vector<CaloEDepMC> edeps; 

             auto hitNextIt = std::next(hitIt);
             bool hitIsMatched(false);
             
             // make art Ptr to CaloHit collection
             const CaloHit* hit = *hitIt;
             size_t idxHit(0); 
             while (idxHit < CaloHits.size()) {if (&CaloHits[idxHit]==hit) break; ++idxHit;}
             auto hitPtr = art::Ptr<CaloHit>(CaloHitHandle,idxHit);               
             
             //calculate the length of the pulse
             unsigned nbin(wfBinMax);
             while (nbin < wf_.size()) {if (wf_[nbin]*hit->energyDep()<minAmplitude_) break; ++nbin;} 
             double deltaTimePlus(nbin*digiSampling_);


             if (diagLevel_ > 2) std::cout<<"[CaloDigiTruthMatch]  inspect hit id/time/energy/length "<<hit->crystalID()<<" / "<<hit->time()<<" / "<<hit->energyDep()<<" "<<nbin*digiSampling_<<std::endl;
                                      
             //forward until we reach the recoHit time;
             while (showerIt != showerItEnd && ( (*showerIt)->time() < (*hitIt)->time() - digiSampling_) ) ++showerIt; 
                          
             //loop as long as the shower time is witthin the recoTime window
             while (showerIt != showerItEnd && ( (*showerIt)->time() < (*hitIt)->time() + deltaTimePlus) )
             {
                 //check if we're already inside the next hit time window
                 if (hitNextIt != hitItEnd && ( (*showerIt)->time() > (*hitNextIt)->time() - digiSampling_) ) break;
                 
                 //calculate the ratio between the hit energy and MC hit energy
                 //double dt  = std::max(0.0,(*hitIt)->time() - (*showerIt)->time());
                 //int    idx = int(dt/digiSampling_)+wfBinMax; 
                 //double AmplitudeAtNextMax = (idx < int(wf_.size())) ? (*hitIt)->energyDep()*wf_[idx] : 0;
                 //if (dt > 3*digiSampling_ && AmplitudeAtNextMax/(*showerIt)->energyDep()>0.5) {++showerIt; continue;}
                 
                 hitIsMatched = true;
                                  
                 size_t idxShower(0); 
                 while (idxShower < caloShowerSims.size()) {if (&caloShowerSims[idxShower]==*showerIt) break; ++idxShower;}
                 auto  ShowerSimPtr = art::Ptr<CaloShowerSim>(caloShowerSimHandle,idxShower);
                 
                 const CaloShowerSim* showerSim(*showerIt);                 
                 const auto& sim    = showerSim->sim();
                 float edep         = showerSim->energyDep();
                 float edepG4       = showerSim->energyDepG4();
                 float time         = showerSim->time();
                 float pIn          = showerSim->momentumIn();
                 
                 //add shower to CaloEDepMC list
                 auto it = edeps.begin();
                 while (it != edeps.end()) {if (it->sim() == sim) break;  ++it;}

                 if (it!= edeps.end())
                 { 
                     it->addEDep(edep);
                     it->addEDepG4(edepG4);
                     it->addTime(time);
                     it->addMom(pIn);
                 }
                 else 
                 {                                     
                     MCRelationship mcrel;                     
                     for (const auto& spp : primaryParticle.primarySimParticles())
                     {
                         MCRelationship mcr(spp,sim);
                         if (mcr > mcrel) mcrel = mcr;
                     }
                     edeps.emplace_back(CaloEDepMC(sim,edep,edepG4,time,pIn,mcrel));
                 }
                 
                 // add shower to the detailed truthMatch
                 if (fillDetailedMC_) caloShowerTruthMatch.addSingle(hitPtr, showerSim->sim(), ShowerSimPtr);
                                 
                                                  
                 if (diagLevel_ > 1) diag(showerSim,hit);
                 if (diagLevel_ > 2) std::cout<<"[CaloDigiTruthMatch]  matched shower id/time/energyDep()= "<<showerSim->crystalID()
                                              <<" / "<<showerSim->time()<<" / "<<showerSim->energyDep()<<std::endl;
                 ++showerIt;                
             }
             
             //sort CaloEDepMC by decreasing energy
             std::sort(edeps.begin(),edeps.end(),[](const auto& a, const auto& b){return a.energyDep() > b.energyDep();});
             caloDigiMCs.emplace_back(CaloDigiMC(std::move(edeps)));

             art::Ptr<CaloDigiMC> digiMCPtr = art::Ptr<CaloDigiMC>(digiMCProductID, caloDigiMCs.size()-1, digiMCProductGetter);             
             caloDigiTruthMatch.addSingle(hitPtr,digiMCPtr);
                          
             if (hitIsMatched) {totalEnergyMatched += (*hitIt)->energyDep();++nMatched;}
             ++hitIt;
          }      
      }      
       
      if (diagLevel_ > 0) std::cout<<"[CaloDigiTruthMatch]  total particles / energy matched = "<<nMatched<<" / "<<totalEnergyMatched<<std::endl;
  } 
  

  void CaloDigiTruthMatch::diag(const CaloShowerSim* shower, const CaloHit* hit)
  {
      hTime_->Fill(shower->time()-hit->time());
      hEnerTime_->Fill(shower->energyDep(),shower->time()-hit->time());
      hTime2d_->Fill(shower->time(),hit->time());
      hEner2d_->Fill(shower->energyDep(),hit->energyDep());
      hdEdT_->Fill(hit->energyDep()-shower->energyDep(),shower->time()-hit->time());

      if (shower->energyDep() > 5) hTime2_->Fill(shower->time()-hit->time()); 

      double deltaE = std::abs(shower->energyDep()-hit->energyDep());
      if (deltaE > 5 && shower->energyDep() > 5)
      hChi2_->Fill(hit->recoCaloDigis().at(0)->chi2()/hit->recoCaloDigis().at(0)->ndf());		   
  }


}

using mu2e::CaloDigiTruthMatch;
DEFINE_ART_MODULE(CaloDigiTruthMatch);



