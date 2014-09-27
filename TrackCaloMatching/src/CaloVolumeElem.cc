//
//
//
// $Id: CaloVolumeElem.cc,v 1.2 2013/03/05 20:33:26 aluca Exp $
// $Author: aluca $
// $Date: 2013/03/05 20:33:26 $
//
// Original author G. Pezzullo & G. Tassielli
//
#include "BaBar/BaBar.hh"
#include "BaBar/Constants.hh"

#include "CLHEP/Geometry/Transformation.h"
#include "DetectorModel/DetIntersection.hh"
#include "DetectorModel/DetSurface.hh"
#include "DetectorModel/DetVolSideIntersection.hh"
#include "DetectorModel/DetVolumeType.hh"
#include "DetectorModel/Intersection.hh"

#include "TrackCaloMatching/inc/CaloVolumeElem.hh"
#include "TrackCaloMatching/inc/CaloVolumeType.hh"
#include "TrackCaloMatching/inc/CaloSurface.hh"

#include <iostream>
#include <assert.h>


namespace mu2e{

void CaloVolumeElem::createCache(){
        CaloVolumeType* volType = (CaloVolumeType*) detectorType();

                size_t side=0, nSides = volType->sides()->size();
                for( side = 0; side<nSides; side++ )
                {

                        HepTransformation tran( transf() );
                        CaloSurface* thisSurface = dynamic_cast<CaloSurface *>( (*volType->sides())[side] );
                        //sides()->push_back(  dynamic_cast<DetSurface*>(thisSurface->copy() ) );
                        AddSurface(dynamic_cast<DetSurface*>(thisSurface->copyOf() ) );

                        assert( sides()->size() == side+1 );

                        tran *= *( thisSurface->transform() );

                        thisSurface->printAll(std::cout);
			
			//gianipez debugging
			thisSurface->print(std::cout);
			//--

			
                        //*( (*sides())[side]->transform() ) = tran;
                        *( (*_sides)[side]->transform() ) = tran;
                }
}




}