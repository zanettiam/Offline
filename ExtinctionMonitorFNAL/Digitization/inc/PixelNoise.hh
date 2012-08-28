// Add random noise hits with the specified probability.
//
// Andrei Gaponenko, 2012

#ifndef ExtinctionMonitorFNAL_Digitization_PixelNoise_hh
#define ExtinctionMonitorFNAL_Digitization_PixelNoise_hh

#include "RecoDataProducts/inc/ExtMonFNALRawHitCollection.hh"
#include "CLHEP/Random/RandPoissonQ.h"
#include "CLHEP/Random/RandFlat.h"

#include "art/Framework/Services/Optional/RandomNumberGenerator.h"

namespace mu2e {

  class PixelNoise {
  public:

    PixelNoise(art::RandomNumberGenerator::base_engine_t& rng,
               double noisePerPixelPerBC,
               int clockMin,
               int clockMax);

    // Adds noise hits to the collection.  Noise hits can be
    // identified by the lack of associated truth.  No need to have a
    // truth collection ptr arg.
    void add(ExtMonFNALRawHitCollection *hits);

  private:
    CLHEP::RandPoissonQ poisson_;
    CLHEP::RandFlat flat_;
    double noisePerPixelPerBC_;
    int clockMin_;
    int clockMax_;
  };

} // namespace mu2e

#endif/*ExtinctionMonitorFNAL_Digitization_PixelNoise_hh*/
