//
// Free function to create and place a new G4ExtrudedSolid inside a logical volume.
//
// $Id: nestExtrudedSolid.cc,v 1.5 2013/06/28 19:51:12 wieschie Exp $
// $Author: wieschie $
// $Date: 2013/06/28 19:51:12 $
//
// Original author Rob Kutschke
//

#include <string>

// Mu2e includes
#include "Mu2eG4/inc/nestExtrudedSolid.hh"
#include "Mu2eG4/inc/finishNesting.hh"

// G4 includes
#include "G4ExtrudedSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4TwoVector.hh"
#include "G4ThreeVector.hh"

using namespace std;

namespace mu2e {

  //
  // Create and place a G4ExtrudedSolid inside a logical volume.
  //
  VolumeInfo nestExtrudedSolid( std::string const& name,
                                double hz,
                                std::vector<double> &x,
                                std::vector<double> &y,
                                G4Material* material,
                                G4RotationMatrix const* rot,
                                const G4ThreeVector& offset,
                                G4LogicalVolume* parent,
                                int copyNo,
                                bool const isVisible,
                                G4Colour const color,
                                bool const forceSolid,
                                bool const forceAuxEdgeVisible,
                                bool const placePV,
                                bool const doSurfaceCheck
                                ){

    VolumeInfo info;

    info.name    = name;

    if( x.size()!=y.size() || x.size()==0 ) return info;

    vector<G4TwoVector> polygon;
    for( size_t i=0; i<x.size(); ++i ) polygon.push_back(G4TwoVector(x[i],y[i]));

    info.solid   = new G4ExtrudedSolid( name, polygon, hz,
                                        G4TwoVector(0.0,0.0), 1.0,
                                        G4TwoVector(0.0,0.0), 1.0 );

    finishNesting(info,
                  material,
                  rot,
                  offset,
                  parent,
                  copyNo,
                  isVisible,
                  color,
                  forceSolid,
                  forceAuxEdgeVisible,
                  placePV,
                  doSurfaceCheck
                  );

    return info;

  }

  //=======================================================================================

  VolumeInfo nestExtrudedSolid( std::string const& name,
                                double hz,
                                std::vector<double> &x,
                                std::vector<double> &y,
                                G4Material* material,
                                G4RotationMatrix const* rot,
                                G4ThreeVector const& offset,
                                VolumeInfo const & parent,
                                int copyNo,
                                bool const isVisible,
                                G4Colour const color,
                                bool const forceSolid,
                                bool const forceAuxEdgeVisible,
                                bool const placePV,
                                bool const doSurfaceCheck
                                ){

    VolumeInfo info(name,offset,parent.centerInWorld);

    if( x.size()!=y.size() || x.size()==0 ) return info;

    vector<G4TwoVector> polygon;
    for( size_t i=0; i<x.size(); ++i ) polygon.push_back(G4TwoVector(x[i],y[i]));

    info.solid   = new G4ExtrudedSolid( name, polygon, hz,
                                        G4TwoVector(0.0,0.0), 1.0,
                                        G4TwoVector(0.0,0.0), 1.0 );
    finishNesting(info,
                  material,
                  rot,
                  offset,
                  parent.logical,
                  copyNo,
                  isVisible,
                  color,
                  forceSolid,
                  forceAuxEdgeVisible,
                  placePV,
                  doSurfaceCheck
                  );

    return info;

  }  

  //=======================================================================================

  VolumeInfo nestExtrudedSolid(std::string const& name,
                               const std::vector<G4TwoVector>& polygon,
                               const std::vector<G4ExtrudedSolid::ZSection>& zsections,
                               G4Material* material,
                               G4RotationMatrix const* rot,
                               G4ThreeVector const& offset,
                               VolumeInfo const & parent,
                               int copyNo,
                               bool const isVisible,
                               G4Colour const color,
                               bool const forceSolid,
                               bool const forceAuxEdgeVisible,
                               bool const placePV,
                               bool const doSurfaceCheck
                               ){
    VolumeInfo info(name,offset,parent.centerInWorld);
    info.solid   = new G4ExtrudedSolid( name, polygon, zsections);
    finishNesting(info,
                  material,
                  rot,
                  offset,
                  parent.logical,
                  copyNo,
                  isVisible,
                  color,
                  forceSolid,
                  forceAuxEdgeVisible,
                  placePV,
                  doSurfaceCheck
                  );
    return info;

  }


}