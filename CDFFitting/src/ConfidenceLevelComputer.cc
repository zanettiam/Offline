//-------------------------------------------------------
// ConfidenceLevelComputer: A class to compute confidence
// Levels.  Author:  Joe Boudreau
//-------------------------------------------------------

#include "CDFFitting/inc/ConfidenceLevelComputer.hh"

// Static variable definitions

const unsigned int ConfidenceLevelComputer::ITMAX=100;
const double       ConfidenceLevelComputer::EPS=3.0E-7;
const double       ConfidenceLevelComputer::FPMIN=1.0e-30;