#include <TApplication.h>
#include <TEvePad.h>
#include <TObject.h>
#include <TSystem.h>
#include "TEveEventDisplay/src/TEveMu2e_base_classes/TEveMu2eMCInterface.h"
#include "TEveEventDisplay/src/TEveMu2e_base_classes/TEveMu2eMCTraj.h"
#include "MCDataProducts/inc/MCTrajectoryPoint.hh"
#include "TEveEventDisplay/src/dict_classes/GeomUtils.h"
using namespace mu2e;
namespace mu2e{

  template <typename T, typename U> void DataLists(T data, bool Redraw, bool show2D, bool accumulate, string title, TEveElementList **List3D, TEveElementList **List2D = 0, U projection = 0){	
      if(data == 0 && Redraw){
        if (*List3D != 0){
          (*List3D)->DestroyElements();
        }
        if(show2D){
          if (*List2D != 0){
            (*List2D)->DestroyElements();
          }
          projection->fXYMgr->ImportElements(*List2D, projection->fDetXYScene); 
          projection->fRZMgr->ImportElements(*List2D, projection->fDetRZScene);
        }
        gEve->AddElement(*List3D);
        gEve->Redraw3D(kTRUE); 
       } 
      if(data!=0){
        if (*List3D== 0) {
          *List3D = new TEveElementList((title + "3D").c_str());
          if(!accumulate){(*List3D)->DestroyElements();} if(!accumulate){(*List3D)->DestroyElements();}     
        }
        else {
          (*List3D)->DestroyElements();  
        }
        if (*List2D== 0) {
          *List2D = new TEveElementList((title + "2D").c_str());
          (*List2D)->IncDenyDestroy();     
        }
        else {
          if (!accumulate){(*List2D)->DestroyElements();}    
        }
    }
  }

  template <typename L> std::vector<double> Energies(L data, int *energylevels[]){
    std::vector<double> energies = {0, 0};
    double Max_Energy = 0;
    double Min_Energy = 1000;
    for(unsigned int i=0; i < data->size();i++){
      if (((*data)[i]).energyDep() > Max_Energy){Max_Energy = ((*data)[i]).energyDep();}
      if (((*data)[i]).energyDep()< Min_Energy){Min_Energy = ((*data)[i]).energyDep();}
    }
    double interval = (Max_Energy - Min_Energy)/(9);

    for(size_t i=0; i<data->size();i++){
      for(int n=0; n<9;n++){
        if(((*data)[i]).energyDep() >= Min_Energy + n * interval && ((*data)[i]).energyDep() <=Min_Energy + (n+1)*interval){
        (*energylevels)[i] = n;}
      }
    }
    energies.at(0) = Min_Energy;
    energies.at(1) = Max_Energy;
    return energies;
  }

  void TEveMu2eMCInterface::AddMCTrajectory(bool firstloop, const MCTrajectoryCollection *trajcol, TEveMu2e2DProjection *tracker2Dproj, bool Redraw, bool show2D, bool accumulate){
	DataLists<const MCTrajectoryCollection*, TEveMu2e2DProjection*>(trajcol, Redraw, show2D, accumulate, "MC Trajectory", &fTrackList3D, &fTrackList2D, tracker2Dproj);
    if(trajcol!=0){
      TEveElementList *HitList3D = new TEveElementList("MCtraj3D");
      TEveElementList *HitList2D = new TEveElementList("MCtraj2D");
      std::map<art::Ptr<mu2e::SimParticle>,mu2e::MCTrajectory>::const_iterator trajectoryIter;
      for(trajectoryIter=trajcol->begin(); trajectoryIter!=trajcol->end(); trajectoryIter++)
      {
        const std::vector<MCTrajectoryPoint> &points = trajectoryIter->second.points();
        string pdgId= to_string(trajectoryIter->first->pdgId());
        CLHEP::Hep3Vector StartHitPos(points[0].x(), points[0].y(), points[0].z());
        CLHEP::Hep3Vector EndHitPos(points[points.size()-1].x(), points[points.size()-1].y(), points[points.size()-1].z());
        TEveMu2eMCTraj *teve_hit3D = new TEveMu2eMCTraj();
        string energy = to_string(points[0].kineticEnergy());
        teve_hit3D->DrawLine("MCTraj PDG " + pdgId + "Energy = " + energy  + ", ",  StartHitPos, EndHitPos, HitList3D);
        
        fTrackList3D->AddElement(HitList3D);
        if(show2D){
          GeomHandle<DetectorSystem> det;
          StartHitPos = det->toMu2e(StartHitPos);
          EndHitPos = det->toMu2e(EndHitPos);
          TEveMu2eMCTraj *teve_hit2D = new TEveMu2eMCTraj();
          teve_hit2D->DrawLine("MCTraj PDG " + pdgId + "Energy = " + energy + ", ", StartHitPos, EndHitPos, HitList2D);
          fTrackList2D->AddElement(HitList2D); 
        }
      }
      if(show2D){
        tracker2Dproj->fXYMgr->ImportElements(fTrackList2D, tracker2Dproj->fDetXYScene); 
        tracker2Dproj->fRZMgr->ImportElements(fTrackList2D, tracker2Dproj->fDetRZScene);
      }
      gEve->AddElement(fTrackList3D);
      gEve->Redraw3D(kTRUE);
      }
    }
 
}
