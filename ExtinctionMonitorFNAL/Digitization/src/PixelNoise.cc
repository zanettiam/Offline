// Add random noise hits with the specified probability.
//
// Andrei Gaponenko, 2012


#include "ExtinctionMonitorFNAL/Digitization/inc/PixelNoise.hh"

#include <iostream>
#include <sstream>
#include <cmath>
#include <map>

#include "cetlib/exception.h"

#include "DataProducts/inc/ExtMonFNALPixelId.hh"
#include "DataProducts/inc/ExtMonFNALPixelDenseId.hh"
#include "ExtinctionMonitorFNAL/Geometry/inc/ExtMonFNALPixelIdConverter.hh"

#include "RecoDataProducts/inc/ExtMonFNALRawHit.hh"

#include "ExtinctionMonitorFNAL/Geometry/inc/ExtMonFNAL.hh"
#include "GeometryService/inc/GeomHandle.hh"

//#define AGDEBUG(stuff) do { std::cerr<<"AG: "<<__FILE__<<", line "<<__LINE__<<", func "<<__func__<<": "<<stuff<<std::endl; } while(0)
#define AGDEBUG(stuff)

namespace mu2e {

  namespace {

    typedef std::multimap<ExtMonFNALPixelDenseId,ExtMonFNALRawHit> NoiseMap;

    bool isHigh(const ExtMonFNALRawHit& hit, int clock) {
      return (hit.clock() <= clock) && (clock <= hit.clock() + int(hit.tot()));
    }

    void removeConflictingNoise(NoiseMap *nm,
                                const ExtMonFNALRawHit& signalHit,
                                const ExtMonFNALPixelIdConverter& conv)
    {
      std::vector<NoiseMap::iterator> toBeDeleted;

      typedef std::pair<NoiseMap::iterator, NoiseMap::iterator> Range;
      Range r = nm->equal_range(conv.densePixelNumber(signalHit.pixelId()));

      for(NoiseMap::iterator i = r.first; i != r.second; ++i) {
        if(isHigh(signalHit, i->second.clock())) {
          toBeDeleted.push_back(i);
          AGDEBUG("Found conflicting noise hit: "<<i->second<<" for signal hit "<<signalHit);
        }
      }

      for(unsigned i=0; i<toBeDeleted.size(); ++i) {
        nm->erase(toBeDeleted[i]);
      }

    }

  } // namespace{}


  //================================================================
  PixelNoise::PixelNoise(art::RandomNumberGenerator::base_engine_t& rng,
                         double noisePerPixelPerBC,
                         int clockMin,
                         int clockMax)
    : poisson_(rng)
    , flat_(rng)
    , noisePerPixelPerBC_(noisePerPixelPerBC)
    , clockMin_(clockMin)
    , clockMax_(clockMax)
  {

    AGDEBUG("clockMin = "<<clockMin_<<", clockMax = "<<clockMax_<<", noise = "<<noisePerPixelPerBC_);

    if(clockMax_ < clockMin_) {
      throw cet::exception("CONFIG")<<" PixelNoise: clockMin < clockMax is not allowed\n";
    }
  }


  //================================================================
  void PixelNoise::add(ExtMonFNALRawHitCollection *hits) {
    GeomHandle<ExtMonFNAL::ExtMon> extmon;
    ExtMonFNALPixelIdConverter conv(extmon->up().nplanes() + extmon->dn().nplanes(),
                                    extmon->sensor(),
                                    extmon->chip());

    AGDEBUG("Input hits size = "<<hits->size());
    AGDEBUG("clockMin = "<<clockMin_<<", clockMax = "<<clockMax_<<", noise = "<<noisePerPixelPerBC_<<", input hits size = "<<hits->size());

    const int numBCs = 1 + clockMax_ - clockMin_;

    const double meanNoise = conv.totalNumberOfPixels() * numBCs * noisePerPixelPerBC_;

    int numNoise = poisson_.fire(meanNoise);

    AGDEBUG("Adding "<<numNoise<<" noise hits, mean noise = "<<meanNoise<<std::endl);

    // Generate the noise hits
    NoiseMap noise;
    for(int i=0; i<numNoise; ++i) {
      ExtMonFNALPixelDenseId pix(flat_.fireInt(conv.totalNumberOfPixels()));
      unsigned int clock = flat_.fireInt(numBCs);
      unsigned int tot = 0; // noise hits are short
      noise.insert(std::make_pair(pix, ExtMonFNALRawHit(conv.pixelId(pix), clock, tot)));
    }

    // Retain only those noise hits that do not overlap with an existing hit
    for(ExtMonFNALRawHitCollection::const_iterator i=hits->begin(); i!=hits->end(); ++i) {
      removeConflictingNoise(&noise, *i, conv);
    }

    // Add surviving noise hits to the output
    for(NoiseMap::const_iterator i=noise.begin(); i!=noise.end(); ++i) {
      hits->push_back(i->second);
    }

    AGDEBUG("Final hits size = "<<hits->size());
  }

} // namespace mu2e
