/*
 * @file	PathFinder.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef PATHFINDER_H_
#define PATHFINDER_H_

#include "types.h"

#include <nsol/nsol.h>

namespace synvis
{
  class PathFinder
  {
  public:

    PathFinder( nsol::DataSet* dataset = nullptr );
    ~PathFinder( void );

    void dataset( nsol::DataSet* dataset_ );

    mat4 getTransform( unsigned int gid ) const;

    std::set< unsigned int > connectedTo( unsigned int gid ) const;

    std::vector< vec3 > getAllPathsPoints( unsigned int gid,
                                           const std::set< unsigned int >& gidsPost,
                                           float pointSize,
                                           TNeuronConnection type = PRESYNAPTIC ) const;

    std::vector< nsol::NeuronMorphologySectionPtr >
      findPathToSoma( const nsol::MorphologySynapsePtr synapse,
                      TNeuronConnection type = PRESYNAPTIC ) const;

  protected:

    nsol::DataSet* _dataset;

  };
}



#endif /* PATHFINDER_H_ */
