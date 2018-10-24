/*
 * @file  UpdaterMappedValue.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "UpdaterMappedValue.h"

#include "SourceMultiPosition.h"

namespace syncopa
{
  using namespace prefr;

  UpdaterMappedValue::UpdaterMappedValue( void )
  : prefr::Updater( )
  { }

  UpdaterMappedValue::~UpdaterMappedValue( void )
  {

  }

  void UpdaterMappedValue::updateParticle( prefr::tparticle current,
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
        current.set_alive( true );

        current.set_position( eigenToGLM( source->position( id )));
        current.set_velocity( glm::vec3( 0, 1, 0 ) );

        current.set_velocityModule( 0 );
        current.set_acceleration( glm::vec3( 0, 0, 0 ));

        _updateConfig->setEmitted( id, false );
      }

//      float refLife = 1.0f -
//          glm::clamp( current.life( ) * ( model->_lifeNormalization ),
//                      0.0f, 1.0f );

      current.set_color( model->color.GetValue( current.life( ) ));
      current.set_size( model->size.GetValue( current.life( ) ));

    }

}


