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

  MobilePolylineSource::MobilePolylineSource( float emissionRate ,
                                              const vec3& position_ )
    : prefr::Source( emissionRate , eigenToGLM( position_ ))
//  , _initialNode( nullptr )
    , _functionType( TSF_UNDEFINED )
    , _currentSegment( 0 )
    , _currentDistance( 0.0f )
    , _lastDisplacement( 0.0f )
    , _velocityModule( 0.0f )
    , _delay( 0.0f )
    , _currentTime( 0.0f )
    , _invDisplacementIndex( 0.0f )
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

  void
  MobilePolylineSource::path( const utils::PolylineInterpolation& interpolator )
  {
    if ( interpolator.empty( ))
    {
      std::cout << "Error: Assigning empty path interpolator." << std::endl;
      return;
    }

    _interpolator = interpolator;

//    std::cout << "Source positions " << this << " ";
//    for( auto node : _interpolator.positions( ))
//      std::cout << node << " ";

    std::cout << std::endl;

    _currentDistance = 0.0f;
    _position = eigenToGLM( _interpolator.firstPosition( ));
  }

  void MobilePolylineSource::path(
    const utils::EventPolylineInterpolation& interpolator )
  {
    if ( interpolator.empty( ))
    {
      std::cout << "Error: Assigning empty path interpolator." << std::endl;
      return;
    }


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

  void MobilePolylineSource::functionType( TSourceFunction type )
  {
    _functionType = type;
  }

  TSourceFunction MobilePolylineSource::functionType( void ) const
  {
    return _functionType;
  }

  void MobilePolylineSource::addEventNode( float distance , cnode_ptr section )
  {
    _interpolator.addEventNode( distance , section->section( )->id( ));
  }

//  void MobilePolylineSource::initialNode( cnode_ptr section_ )
//  {
//    _initialNode = section_;
//  }
//
//  cnode_ptr MobilePolylineSource::initialNode( void ) const
//  {
//    return _initialNode;
//  }

  void MobilePolylineSource::delay( float delay_ )
  {
    _delay = delay_;
  }

  void MobilePolylineSource::_checkEmissionEnd( )
  {
    if ( _maxEmissionCycles > 0 )
      _continueEmission = !( _currentCycle >= _maxEmissionCycles );
    else
      _continueEmission = true;

//    _continueEmission = _continueEmission && _totalTime >= _delay;

  }

  void MobilePolylineSource::_checkFinished( )
  {
    if ( _currentDistance >= _interpolator.totalDistance( ))
    {
//      std::cout << "Reached end source " << _gid
//                << " " << _lastFrameAliveParticles
//                << " " << std::boolalpha << _continueEmission
//                << std::endl;

      if ( _continueEmission )
      {
        _finished = false;
        _currentDistance = 0;
        _position = eigenToGLM( _interpolator.firstPosition( ));
        //        _totalTime = 0;

      }
      else if ( _lastFrameAliveParticles == 0 )
      {
        _finished = true;

        if ( _autoDeactivateWhenFinished )
        {
          _active = false;
          if ( _killParticlesIfInactive )
          {
            _updateConfig->setDead( _particles.indices( ) , true );
            _updateConfig->setEmitted( _particles.indices( ) , false );
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

    _updatePosition( deltaTime );


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

  void MobilePolylineSource::_updatePosition( float deltaTime )
  {
    if ( !_continueEmission || _finished )
    {
      _lastDisplacement = 0.0f;
      _invDisplacementIndex = 1.0f;
      return;
    }

    _currentTime += deltaTime;
//    if( _currentTime < _delay )
//      return;

    float displacement = _velocityModule * deltaTime;

//    float prevDistance = _currentDistance;
    _currentDistance += displacement;
    _lastDisplacement = displacement;

    auto events = _interpolator.eventsAt( _currentDistance , displacement );
    for ( auto event: events )
    {
      if ( std::get< 2 >( event ) == utils::TEvent_section )
      {
        finishedSection( std::get< 1 >( event ));
//        std::cout << "Reached section " << std::get< 1 >( event ) << std::endl;
      }
      else
      {
        reachedSynapse( std::get< 1 >( event ));
//        std::cout << "Reached synapse " << std::get< 1 >( event )
//                  << " at dist " << std::get< 0 >( event )
//                  << std::endl;
      }
    }

    if ( _currentDistance >= _interpolator.totalDistance( ))
    {
      ++_currentCycle;
      _currentDistance = _interpolator.totalDistance( );
//      std::cout << "Finished source " << _gid << std::endl;
    }

    _lastPosition = glmToEigen( _position );

    _currentSegment = _interpolator.segmentFromDistance( _currentDistance );

    _position = eigenToGLM( _interpolator.pointAtDistance( _currentDistance ,
                                                           _currentSegment ));

    _invDisplacementIndex = _lastDisplacement / _currentFrameEmittedParticles;
//    std::cout << "- source " << _gid
//              << " dst " << _currentDistance
//              << " -> " << _interpolator.totalDistance( )
//              << " dsp " << displacement
//              << " -> " << _position
//              << std::endl;

//    std::cout << " " << _currentDistance << " " << glmToEigen( _position );
  }

  void MobilePolylineSource::sample( prefr::SampledValues* values )
  {
    Source::sample( values );

    auto index = _emittedIndices.find( values->index );
    if ( index == _emittedIndices.end( )) return;

    float distance = index->second * _invDisplacementIndex;
    distance += ( _currentDistance - _lastDisplacement );

    values->position = eigenToGLM( _interpolator.pointAtDistance( distance ));

  }


}
