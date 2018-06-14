/*
 * @file  SourceMultiPosition.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#include "SourceMultiPosition.h"

namespace syncopa
{
  SourceMultiPosition::SourceMultiPosition( void )
  : Source( -1, glm::vec3( 0, 0, 0))
  { }

  SourceMultiPosition::~SourceMultiPosition( void )
  { }

  void SourceMultiPosition::addPositions( const prefr::ParticleSet& indices,
                                          const std::vector< vec3 >& positions_ )
  {
    assert( indices.size( ) == positions_.size( ));

    auto posIt = positions_.begin( );
    for( auto idx : indices )
    {
      if( _positions.find( idx ) == _positions.end( ))
      {
//        std::cout << idx << " " << posIt->x( )
//                         << " " << posIt->y( )
//                         << " " << posIt->z( )
//                         << std::endl;

        _positions.insert( std::make_pair( idx,  *posIt ));
      }

      ++posIt;
    }

//    _particles.addIndices( indices );
//    _updateConfig->setSource( this, indices );

  }

  void SourceMultiPosition::removeElements( const prefr::ParticleSet& indices )
  {
    std::vector< vec4 > resultPositions;
    resultPositions.reserve( _positions.size( ));
    prefr::ParticleSet resultIndices;

    for( auto idx : indices )
    {
      _positions.erase( idx );
    }

    _updateConfig->removeSourceIndices( this, indices );
  }

  vec3 SourceMultiPosition::position( unsigned int idx )
  {
    auto pos = _positions.find( idx );
    assert( pos != _positions.end( ));

    return pos->second;
  }

  void SourceMultiPosition::clear( void )
  {
    _positions.clear( );
  }
}
