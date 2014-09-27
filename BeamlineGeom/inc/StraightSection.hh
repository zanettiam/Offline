#ifndef BeamlineGeom_StraightSection_hh
#define BeamlineGeom_StraightSection_hh

//
// Class to represent the transport solenoid
//
#include <memory>

#include "BeamlineGeom/inc/TSSection.hh"
#include "GeomPrimitives/inc/Tube.hh"

#include "CLHEP/Vector/Rotation.h"
#include "CLHEP/Vector/ThreeVector.h"

namespace mu2e {

  class StraightSection : public TSSection {

  friend class BeamlineMaker;

  public:

    StraightSection() :
      _rIn(0.), _rOut(0.), _halfZ(0.) 
    {}

    StraightSection(double rIn, double rOut, double halfZ, 
                    CLHEP::Hep3Vector origin, CLHEP::HepRotation rotation = CLHEP::HepRotation()) :
      _rIn(rIn), _rOut(rOut), _halfZ(halfZ)
    {
      _origin=origin; // _origin is derived data member; cannot be in initialization list
      _rotation=rotation;
    }

    StraightSection( const Tube& tube )  :
      _rIn  ( tube.innerRadius() ),
      _rOut ( tube.outerRadius() ),
      _halfZ( tube.zHalfLength() ) 
    {
      _origin   = tube.originInMu2e();
      _rotation = tube.rotation();
    }

    ~StraightSection(){}

    void set(double rIn, double rOut, double halfZ, 
             CLHEP::Hep3Vector origin, CLHEP::HepRotation rotation=CLHEP::HepRotation() ) {
      _rIn=rIn;
      _rOut=rOut;
      _halfZ=halfZ;
      _origin=origin;
      _rotation=rotation;
    }

    double rIn() const { return _rIn; }
    double rOut() const { return _rOut; }
    double getHalfLength() const { return _halfZ; }

  private:

    double _rIn;
    double _rOut;
    double _halfZ;

    // no copying (because it would do the wrong thing):
    StraightSection( StraightSection const & );
    void  operator = ( StraightSection const & );
};

}
#endif /* BeamlineGeom_StraightSection_hh */