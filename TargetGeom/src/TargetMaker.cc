//
// Construct and return an Target.
//
//
// $Id: TargetMaker.cc,v 1.15 2013/05/31 18:07:18 gandr Exp $
// $Author: gandr $
// $Date: 2013/05/31 18:07:18 $
//
// Original author Peter Shanahan
//

#include <iostream>
#include <iomanip>
#include <cmath>


// Framework includes
#include "messagefacility/MessageLogger/MessageLogger.h"

// Mu2e includes
#include "TargetGeom/inc/TargetMaker.hh"
#include "CLHEP/Vector/Rotation.h"
#include "CLHEP/Vector/RotationY.h"
#include "CLHEP/Vector/RotationZ.h"
#include "TargetGeom/inc/Target.hh"
#include "ConfigTools/inc/SimpleConfig.hh"

using namespace std;

namespace mu2e {

  TargetMaker::TargetMaker(const CLHEP::Hep3Vector& detSysOrigin, SimpleConfig const& c)
    : _detSysOrigin(detSysOrigin)
    , _rIn(0.)
  {
    int verbosity(c.getInt("stoppingTarget.verbosity",0));

    // positions are in detector coordinates (mm).
    _z0InMu2e            = c.getDouble("stoppingTarget.z0InMu2e");
    _deltaZ        = c.getDouble("stoppingTarget.deltaZ");

    // all outer radii must be specified.
    c.getVectorDouble("stoppingTarget.radii", _rOut);

    // Downstream code counts on this so test it here.
    if ( _rOut.size() < 1 ){
      throw cet::exception("GEOM")
        << "Specified a stopping target with no foils!\n";
    }

    // halfThicknesses can be repeated from last element specified
    c.getVectorDouble("stoppingTarget.halfThicknesses",_halfThicknesses);
    unsigned int size=_halfThicknesses.size();
    for (unsigned int ii=size; ii<_rOut.size(); ++ii)
      _halfThicknesses.push_back(_halfThicknesses[size-1]);

    // x variations can be repeated from last element specified
    c.getVectorDouble("stoppingTarget.xVars",_xVars);
    size=_xVars.size();
    for (unsigned int ii=size; ii<_rOut.size(); ++ii)
      _xVars.push_back(_xVars[size-1]);

    // y variations can be repeated from last element specified
    c.getVectorDouble("stoppingTarget.yVars",_yVars);
    size=_yVars.size();
    for (unsigned int ii=size; ii<_rOut.size(); ++ii)
      _yVars.push_back(_yVars[size-1]);

    // z variations can be repeated from last element specified
    c.getVectorDouble("stoppingTarget.zVars",_zVars);
    size=_zVars.size();
    for (unsigned int ii=size; ii<_rOut.size(); ++ii)
      _zVars.push_back(_zVars[size-1]);

    // x cosines can be repeated from last element specified
    c.getVectorDouble("stoppingTarget.xCos",_xCos);
    size=_xCos.size();
    for (unsigned int ii=size; ii<_rOut.size(); ++ii)
      _xCos.push_back(_xCos[size-1]);

    // y cosines can be repeated from last element specified
    c.getVectorDouble("stoppingTarget.yCos",_yCos);
    size=_yCos.size();
    for (unsigned int ii=size; ii<_rOut.size(); ++ii)
      _yCos.push_back(_yCos[size-1]);

    // materials can be repeated from last element specified
    c.getVectorString("stoppingTarget.materials",_materials);
    size=_materials.size();
    for (unsigned int ii=size; ii<_rOut.size(); ++ii)
      _materials.push_back(_materials[size-1]);

    // material of the target enclosing volume
    _fillMaterial=c.getString("stoppingTarget.fillMaterial");

    // debugging print...
    if ( verbosity > 0 ) PrintConfig();

    // Do the real work.
    BuildIt( );
  }


  void TargetMaker::BuildIt(){

    // Build the Target Geometry.  This means MU2E internal geometry, not
    // Root, G4, or any other scheme.

    _targ = unique_ptr<Target>(new Target());

    // create the TargetFoils

    // compute the z-detector position of each foil, starting with nominal
    // position and including any specified variations.
    const int n0 = _rOut.size()/2;
    const double offset = ( _rOut.size()%2 == 1) ?
      _z0InMu2e : _z0InMu2e + _deltaZ/2.;

    for ( vector<double>::size_type i=0;
          i<_rOut.size(); ++i){

      // z position of the center of the foil.
      const double z = offset + (int(i)-n0)*_deltaZ + _zVars[i];

      // Only x and y directional cosines are specified in config.  Z
      // is derived locally

      double zCos=1-_xCos[i]*_xCos[i]-_yCos[i]*_yCos[i];
      if (zCos<0.) {
        throw cet::exception("RANGE") <<"Target Foil "<<i
                                      <<" ZCos^2 is negative, ="<< zCos<<"\n";
      } else {
        zCos=sqrt(zCos);
      }

      _targ->_foils.push_back( TargetFoil( i,
                                           CLHEP::Hep3Vector(_xVars[i] + _detSysOrigin.x(),
                                                             _yVars[i] + _detSysOrigin.y(),
                                                             z),
                                           CLHEP::Hep3Vector(_xCos[i],_yCos[i],zCos),
                                           _rOut[i],
                                           _rIn,
                                           _halfThicknesses[i],
                                           _materials[i],
                                           _detSysOrigin
                                           )
                               );
    }// foil i


    // calculate the parameters of the enclosing cylinder
    //find the radius - maximum of foil radius + offset from axis
    double radius=-1;
    for (unsigned int ifoil=0; ifoil<_targ->_foils.size(); ifoil++)
      {
        double rtest=_targ->_foils[ifoil].rOut() + _targ->_foils[ifoil].centerInDetectorSystem().perp();
        radius=max(radius,rtest);
      }
    // beef it up by a mm
    radius+=1;

    // give it to the Target
    _targ->_radius=radius;

    // set the length to accomodate generous tilts to the first and last foils
    double zmin=_targ->_foils[0].centerInMu2e().z()-5;
    double zmax=_targ->_foils[_targ->_foils.size()-1].centerInMu2e().z()+5;

    _targ->_zLen=zmax-zmin;
    _targ->_centerInMu2e = CLHEP::Hep3Vector(_detSysOrigin.x(), _detSysOrigin.y(), (zmax+zmin)/2.);

    _targ->_fillMaterial=_fillMaterial;

  }//::BuildIt

  void TargetMaker::PrintConfig()
  {
    // printout the TargetMaker's understanding of what it needs to build.
    //  for debugging...

    std::cout<<"\n TargetMaker Input Configuration -----------------"<<std::endl;
    std::cout<<"\n Target System:"<<std::endl;
    std::cout
      <<"Stopping target Z0 in Mu2e ="<<_z0InMu2e
      <<", nominal spacing="<<_deltaZ
      <<", enclosing material="<<_fillMaterial
      <<std::endl;

    std::cout<<"\n Foils:"<<std::endl;
    std::cout
      <<"Total Foils="<<_rOut.size()<<std::endl;
    std::cout <<"Outer Radii="<<std::endl;
    for (unsigned int itf=0; itf<_rOut.size(); itf++)
      cout <<" "<<_rOut[itf];
    cout <<std::endl;
    std::cout <<"1/2 Thicknesses="<<std::endl;
    for (unsigned int itf=0; itf<_rOut.size(); itf++)
      cout <<" "<<_halfThicknesses[itf];
    cout <<std::endl;
    std::cout <<"Center Variations from Nominal"<<std::endl;
    cout <<"x:";
    for (unsigned int itf=0; itf<_rOut.size(); itf++)
      cout <<" "<<_yVars[itf];
    cout <<std::endl;
    cout <<"y:";
    for (unsigned int itf=0; itf<_rOut.size(); itf++)
      cout <<" "<<_zVars[itf];
    cout <<std::endl;
    cout <<"z:";
    for (unsigned int itf=0; itf<_rOut.size(); itf++)
      cout <<" "<<_xVars[itf];
    cout <<std::endl;
    std::cout <<"Normal Directional Cosine"<<std::endl;
    cout <<"x:";
    for (unsigned int itf=0; itf<_rOut.size(); itf++)
      cout <<" "<<_xCos[itf];
    cout <<std::endl;
    cout <<"y:";
    for (unsigned int itf=0; itf<_rOut.size(); itf++)
      cout <<" "<<_yCos[itf];
    cout <<std::endl;
    std::cout <<"Material="<<std::endl;
    for (unsigned int itf=0; itf<_rOut.size(); itf++)
      cout <<" "<<_materials[itf];
    cout <<std::endl;
  }

} // namespace mu2e
