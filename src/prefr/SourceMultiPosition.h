/*
 * @file  SourceMultiPosition.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_PREFR_SOURCEMULTIPOSITION_H_
#define SRC_PREFR_SOURCEMULTIPOSITION_H_

#include "../types.h"
#include <prefr/prefr.h>

namespace syncopa
{
  class SourceMultiPosition : public prefr::Source
  {
  public:

    SourceMultiPosition( void );
    ~SourceMultiPosition( void );

    void addPositions( const prefr::ParticleSet& indices,
                       const std::vector< vec3 >& positions_ );

    void removeElements( const prefr::ParticleSet& indices );

    vec3 position( unsigned int idx );

    void clear( void );

  protected:

    std::unordered_map< unsigned int, vec3 > _positions;
  };


}


#endif /* SRC_PREFR_SOURCEMULTIPOSITION_H_ */
