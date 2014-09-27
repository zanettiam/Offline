// Simple approximations available for DIO spectrum.
//
// $Id: PionCaptureSpectrum.cc,v 1.1 2014/02/05 21:32:26 knoepfel Exp $
// $Author: knoepfel $
// $Date: 2014/02/05 21:32:26 $
//

// Mu2e includes
#include "ConditionsService/inc/GlobalConstantsHandle.hh"
#include "ConditionsService/inc/PhysicsParams.hh"
#include "ConditionsService/inc/ParticleDataTable.hh"
#include "Mu2eUtilities/inc/PionCaptureSpectrum.hh"
#include "Mu2eUtilities/inc/RandomUnitSphere.hh"

// Framework includes
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "cetlib/pow.h"

// CLHEP includes
#include "CLHEP/Random/RandFlat.h"

// C++ includes
#include <iostream>

using namespace std;

using cet::pow;
using cet::square;

namespace mu2e {

  double PionCaptureSpectrum::getWeight( const double E ) const {

    double weight(0.);
    if      ( _spectrum == Flat )       weight = getFlat( E );
    else if ( _spectrum == Bistirlich ) weight = getBistirlichSpectrum( E );

    return weight;

  }

  double PionCaptureSpectrum::get2DWeight( const double x, const double y, const double E ) const {
    
    double weight(0.);
    if      ( _spectrum2D == Flat2D          ) weight = getFlat( E, x, y );
    else if ( _spectrum2D == KrollWadaJoseph ) weight = getKrollWadaJosephSpectrum( E, x, y );

    return weight;

  }

  //======================================================
  // Flat spectrum to be used with weighting
  //======================================================
  double PionCaptureSpectrum::getFlat( const double E, const double x, const double y ) {
    return 1.;
  }

  //=======================================================
  // Analytic fit to the photon energy spectrum for Mg
  //  - J.A.Bistirlich, et al, Phys. Rev. C 5, 1867 (1972)
  //  - Fit results given in I. Sarra, mu2e-docdb-665-v2
  //=======================================================

  double PionCaptureSpectrum::getBistirlichSpectrum( const double e ) {
    static const double emax  = 138.2;
    static const double alpha =   2.691;
    static const double gamma =   1.051;
    static const double tau   =   8.043;
    static const double c0    =   2.741;
    static const double c1    =  -0.005;
    
    if ( e > emax ) return 0.;
    
    return std::pow(emax-e,alpha) * std::exp(-(emax-gamma*e)/tau) * (c0 + c1*e) / 10000;
  }

  //=======================================================
  // Analytic expression for electron/positron energies via 
  //  - Kroll and Wada, Phys. Rev. 98, 1355 (1955)
  //  - Joseph, Il Nuovo Cimento 16, 997 (1960)
  //=======================================================

  double PionCaptureSpectrum::get2DMax( const double E ) const {
    static const GlobalConstantsHandle<ParticleDataTable> pdt;
    static const HepPDT::ParticleData& e_data  = pdt->particle(PDGCode::e_minus).ref();

    return getKrollWadaJosephSpectrum( E, 2*e_data.mass().value(), 0. );
  }

  double PionCaptureSpectrum::getKrollWadaJosephSpectrum(const double ePhoton,
                                                         const double x,
                                                         const double y ) const {

    static const GlobalConstantsHandle<ParticleDataTable> pdt;
    static const HepPDT::ParticleData& e_data  = pdt->particle(PDGCode::e_minus).ref();
    static const HepPDT::ParticleData& pi_data = pdt->particle(PDGCode::pi_minus).ref();

    static const double m = e_data.mass().value();
    static const double E = ePhoton;

    // Assume pion is not bound to stopped nucleus and there is no
    // nuclear recoil
    static const double M = GlobalConstantsHandle<PhysicsParams>()->getAtomicMass() + pi_data.mass().value() - ePhoton;
    
    // Set pdf to zero if x is out of bounds
    if (  x > ePhoton || x < 2*m ) return 0.;

    // Set pdf to zero if x or y are out of bounds
    const double eta = sqrt( 1 - pow(2*m/x,2) );
    if ( abs( y ) > eta ) return 0.;

    // Set parameters
    static const double rV = 0.57;
    static const double muS = 0.064;
    const double rT = 1+rV*rV/3 - 2*muS*pow(x/E,2);
    const double rL = 0.142*pow(1+muS,2)*(1/(1-0.466*pow(x/E,2))+1.88*(rV*rV/6 + muS*(1-pow(x/E,2))));
    
    const double kRatio2 = ( pow(2*E*M + E*E,2) - 2*x*x*(2*M*M + 2*E*M + E*E ) + pow( x, 4 ) )/pow(2*E*M + E*E,2) ;
    const double kRatio  = sqrt( kRatio2 );

    const double prefactor = ( pow( E+M,2 ) + M*M - x*x )/( pow( E+M,2 ) + M*M );
    
    const double trans     = rT*( ( 1+y*y )/x + 4*m*m/pow(x,3) );
    const double longit    = rL*(1-y*y)*8*pow(E+M,2)*x/pow( 2*E*M + E*E + x*x, 2 );
    
    const double prob      = kRatio*prefactor*( trans + longit );
    
    return prob;

  }

  std::pair<CLHEP::HepLorentzVector,CLHEP::HepLorentzVector> PionCaptureSpectrum::getElecPosiVectors( const double ePhoton, 
                                                                                                      const double x,
                                                                                                      const double y ) {

    // Get electron/positron energies from x, y values (see Mu2eUtilities/src/PionCaptureSpectrum.cc for details)
    const double eElectron = 0.5*( ePhoton - y*std::sqrt( cet::diff_of_squares( ePhoton, x ) ) ); 
    const double ePositron = 0.5*( ePhoton + y*std::sqrt( cet::diff_of_squares( ePhoton, x ) ) ); 
        
    // Get electron/positron momentum magnitudes
    static const double m = GlobalConstantsHandle<ParticleDataTable>()->particle(PDGCode::e_minus).ref().mass().value();

    const double pElectron = std::sqrt( cet::diff_of_squares( eElectron, m ) );
    const double pPositron = std::sqrt( cet::diff_of_squares( ePositron, m ) );
        
    // Produce electron momentum
    static RandomUnitSphere randomUnitSphere;
    const CLHEP::Hep3Vector p3electron = randomUnitSphere.fire( pElectron );

    // Get positron momentum
    CLHEP::Hep3Vector p3positron( p3electron );
    p3positron.setMag( pPositron );
        
    // - theta (opening angle wrt electron) is constrained by virtual mass formula
    // - phi is allowed to vary between 0 and 2pi
        
    const double cosTheta = 1/(2*pElectron*pPositron)*( cet::square(ePhoton) - cet::sum_of_squares( x, pElectron, pPositron) );

    static CLHEP::RandFlat randFlat ( art::ServiceHandle<art::RandomNumberGenerator>()->getEngine() );
    const double phi = 2*M_PI*randFlat.fire();
        
    // - find a vector that is not collinear with the electron direction
    const CLHEP::Hep3Vector n1 = (std::abs(p3electron.x()) < std::abs(p3electron.y())) ?
      ((std::abs(p3electron.x()) < std::abs(p3electron.z())) ? CLHEP::Hep3Vector(1,0,0) : CLHEP::Hep3Vector(0,0,1)) :
      ((std::abs(p3electron.x()) < std::abs(p3electron.y())) ? CLHEP::Hep3Vector(1,0,0) : CLHEP::Hep3Vector(0,1,0));
        
    // - construct a vector perpendicular to the electron momentum
    const CLHEP::Hep3Vector perp = p3electron.cross(n1);
        
    p3positron.rotate(perp      , std::acos(cosTheta) );
    p3positron.rotate(p3electron, phi                 );

    return std::make_pair( CLHEP::HepLorentzVector( p3electron, eElectron ),
                           CLHEP::HepLorentzVector( p3positron, ePositron ) );

  }

}