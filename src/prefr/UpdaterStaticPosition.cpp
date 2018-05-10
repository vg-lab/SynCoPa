/*
 * @file  UpdaterStaticPosition.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "UpdaterStaticPosition.h"

#include "SourceMultiPosition.h"

namespace synvis
{
  using namespace prefr;

  UpdaterStaticPosition::UpdaterStaticPosition( void )
  : prefr::Updater( )
  { }

  UpdaterStaticPosition::~UpdaterStaticPosition( void )
  {

  }

  void UpdaterStaticPosition::updateParticle( prefr::tparticle current,
                                              float /*deltaTime*/ )
    {

      unsigned int id = current.id( );
      SourceMultiPosition* source =
          dynamic_cast< SourceMultiPosition* >( _updateConfig->source( id ));

      Model* model = _updateConfig->model( id );

      assert( model );
      assert( source );


      if( _updateConfig->emitted( id ) && !current.alive( ))
      {
        current.set_life( 1 );

        current.set_alive( true );

        current.set_position( eigenToGLM( source->position( id )));
        current.set_velocity( glm::vec3( 0, 1, 0 ) );

        current.set_color( model->color.GetFirstValue( ));
        current.set_size( model->size.GetFirstValue( ));
        current.set_velocityModule( 0 );
        current.set_acceleration( glm::vec3( 0, 0, 0 ));

        _updateConfig->setEmitted( id, false );
      }


//      if( current.alive( ))
//      {
//        if( current.life( ) <= 0.0f )
//        {
//          current.set_life( 0.0f );
//          current.set_alive( false );
//
//          _updateConfig->setDead( id, true );
//        }
//      }

    }

}


