#ifndef RecoDataProducts_StrawHitPosition_hh
#define RecoDataProducts_StrawHitPosition_hh
//
// Class to describe derived information from a StrawHit, in particular pos().
//
// $Id: StrawHitPosition.hh,v 1.4 2013/04/04 01:08:01 brownd Exp $
// $Author: brownd $
// $Date: 2013/04/04 01:08:01 $
//
// Original author David Brown
//
// Mu2e includes
#include "RecoDataProducts/inc/StrawHit.hh"
#include "RecoDataProducts/inc/StereoHit.hh"
#include "RecoDataProducts/inc/StrawHitFlag.hh"
#include "ConditionsService/inc/TrackerCalibrationStructs.hh"
// clhep includes
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Units/PhysicalConstants.h"
namespace mu2e {

  class StrawHitPosition {
    public:
#ifndef __GCCXML__
// construct from a straw hit.  Calibration and other information must be provided from outside
      StrawHitPosition(StrawHit const& hit, Straw const& straw, SHInfo const& shinfo, StrawHitFlag const& flag=_nullflag );
// construct from another hit, optionally with additional flag bits (these will be ORed with the existing bits
      StrawHitPosition(StrawHitPosition const& pos,StrawHitFlag const& orflag=_nullflag);
// construct from a stereo hit
      StrawHitPosition(StereoHit const& sthit,size_t index);
// null constructor for root
#endif /* __GCCXML__ */
      StrawHitPosition();
      virtual ~StrawHitPosition();
      StrawHitPosition& operator =(StrawHitPosition const& other);
#ifndef __GCCXML__
      enum edir {phi=0,rho=1};
// accessors
      CLHEP::Hep3Vector const& pos() const { return _pos; }
      float wireDist() const { return _wdist; }
      float posRes(edir dir) const { return dir==phi ? _pres : _rres; }
      float chisq() const { return _chisq; }
      StrawHitFlag const& flag() const { return _flag; }
#endif /* __GCCXML__ */
    private:
      CLHEP::Hep3Vector _pos; // pos() of the hit
      float _wdist; // distance along the wire
      float _pres; // pos resolution along phi
      float _rres; // pos resolution perpendicular to the Z axis
      float _chisq; // self-consistency chisquared
      StrawHitFlag _flag; // bit flags for this hit
      static StrawHitFlag _nullflag; // null flag
      static double _invsqrt12; // 1/sqrt(12)
  };
}
#endif


