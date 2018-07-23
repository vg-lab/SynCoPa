/*
 * @file  MobileSource.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "MobilePolylineSource.h"

namespace syncopa
{
  unsigned int MobilePolylineSource::_counter = 0;

  MobilePolylineSource::MobilePolylineSource( float emissionRate,
                                              const vec3& position_ )
  : prefr::Source( emissionRate, eigenToGLM( position_ ))
  , _initialNode( nullptr )
  , _currentDistance( 0.0f )
  , _velocityModule( 0.0f )
  , _delay( 0.0f )
  , _currentTime( 0.0f )
  {
    _gid = _counter;
    ++_counter;
  }

  void MobilePolylineSource::path( const tPosVec& nodes )
  {
    _interpolator.clear( );
    _interpolator.insert( nodes );

//    for( auto node : nodes )
//      std::cout << " "<< node << std::endl;

    _currentDistance = 0.0f;
    _position = eigenToGLM( _interpolator.firstPosition( ));
  }

  void MobilePolylineSource::path( const utils::PolylineInterpolation& interpolator )
  {
    _interpolator = interpolator;

    _currentDistance = 0.0f;
    _position = eigenToGLM( _interpolator.firstPosition( ));
  }

  void MobilePolylineSource::path( const utils::EventPolylineInterpolation& interpolator )
  {
    _interpolator = interpolator;

    _currentDistance = 0.0f;
    _position = eigenToGLM( _interpolator.firstPosition( ));

  }

  void MobilePolylineSource::velocityModule( float module )
  {
    _velocityModule = module;
  }

  float MobilePolylineSource::currentTime( void ) const
  {
    return _currentTime;
  }

  unsigned int MobilePolylineSource::gid( void ) const
  {
    return _gid;
  }

  void MobilePolylineSource::addEventNode( float distance, cnode_ptr section )
  {
    _interpolator.addEventNode( distance, section->section( )->id( ));
  }

  void MobilePolylineSource::initialNode( cnode_ptr section_ )
  {
    _initialNode = section_;
  }

  cnode_ptr MobilePolylineSource::initialNode( void ) const
  {
    return _initialNode;
  }

  void MobilePolylineSource::delay( float delay_ )
  {
    _delay = delay_;
  }

  void MobilePolylineSource::_checkEmissionEnd( )
  {
    if( _maxEmissionCycles > 0 )
      _continueEmission = !( _currentCycle >= _maxEmissionCycles );
    else
      _continueEmission = true;

//    _continueEmission = _continueEmission && _totalTime >= _delay;

  }

  void MobilePolylineSource::_checkFinished( )
  {
    if( _currentDistance >= _interpolator.totalDistance( ))
    {
      if( _continueEmission  )
      {
          _finished = false;
          _currentDistance = 0;
          _position = eigenToGLM( _interpolator.firstPosition( ));
  //        _totalTime = 0;

      }
      else if( _lastFrameAliveParticles == 0 )
      {
        _finished = true;

        if( _autoDeactivateWhenFinished )
        {
          _active = false;
          if( _killParticlesIfInactive )
          {
            _updateConfig->setDead( _particles.indices( ), true );
            _updateConfig->setEmitted( _particles.indices( ), false );
          }
        }

        finishedPath( _gid );
      }
    }
  }

  void MobilePolylineSource::restart( )
  {
    Source::restart( );

    _currentDistance = 0;
    _currentTime = 0;
    _finished = false;
    _continueEmission = true;
    _active = true;
  }

  void MobilePolylineSource::prepareFrame( const float& deltaTime )
  {


    Source::prepareFrame( deltaTime );

    updatePosition( deltaTime );


  }

//  void MobilePolylineSource::closeFrame( )
//  {
//    Source::closeFrame( );
//
////    {
////      if(( _maxEmissionCycles == 0 || _currentCycle < _maxEmissionCycles ))
////      {
////
////      }
////    }
////      else
////      {
////        _finished = true;
////      }
////
////      if( _killParticlesIfInactive )
////      {
////        _updateConfig->setDead( _particles.indices( ), true );
////      }
////    }
//
//  }

  void MobilePolylineSource::updatePosition( float deltaTime )
  {
//    if( !_continueEmission || _finished )
//      return;

    _currentTime += deltaTime;
//    if( _currentTime < _delay )
//      return;

    float displacement = _velocityModule * deltaTime;

//    float prevDistance = _currentDistance;
    _currentDistance += displacement;

    auto events = _interpolator.eventsAt( _currentDistance, displacement );
    for( auto event : events )
    {
      finishedSection( event.second );
    }

    if( _currentDistance >= _interpolator.totalDistance( ))
    {
      ++_currentCycle;
      _currentDistance = _interpolator.totalDistance( );
//      std::cout << "Finished." << std::endl;
    }

    _position = eigenToGLM( _interpolator.pointAtDistance( _currentDistance ));


//    std::cout << "- source " << _gid
//              << " dst " << _currentDistance
//              << " -> " << _interpolator.totalDistance( )
//              << " dsp " << displacement
//              << " -> " << _position
//              << std::endl;

//    std::cout << " " << _currentDistance << " " << glmToEigen( _position );
  }


}
