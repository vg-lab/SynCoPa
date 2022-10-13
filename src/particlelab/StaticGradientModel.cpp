//
// Created by gaeqs on 9/06/22.
//

#include <GL/glew.h>

#include "StaticGradientModel.h"

#include <plab/core/UniformCache.h>
#include <QDebug>

glm::vec4 getGradientColor( const tColorVec& gradient , float t )
{
  if ( gradient.empty( )) return { 1.0f , 0.0f , 1.0f , 1.0f };
  int size = static_cast<int>(gradient.size( ));
  int first = size - 1;
  for ( int i = 0; i < size; i++ )
  {
    if ( gradient[ i ].first > t )
    {
      first = i - 1;
      break;
    }
  }

  if ( first == -1 ) return gradient[ 0 ].second;
  if ( first == size - 1 ) return gradient[ first ].second;

  float start = gradient[ first ].first;
  float end = gradient[ first + 1 ].first;
  float normalizedT = ( t - start ) / ( end - start );

  return glm::mix( gradient[ first ].second , gradient[ first + 1 ].second ,
              normalizedT );
}

StaticGradientModel::StaticGradientModel(
  const std::shared_ptr< plab::ICamera >& camera ,
  float particlePreSize , float particlePostSize ,
  const tColorVec& gradient ,
  bool particlePreVisibility ,
  bool particlePostVisibility )
  : CameraModel( camera )
  , _particlePreSize( particlePreSize )
  , _particlePostSize( particlePostSize )
  , _gradient( gradient )
  , _particlePreVisibility( particlePreVisibility )
  , _particlePostVisibility( particlePostVisibility )
{
  for ( int i = 0; i < MAX_COLORS; i++ )
  {
    float time = static_cast<float>(i) / 255.0f;
    _colors[ i ] = getGradientColor( _gradient , time );
  }
}

float StaticGradientModel::getParticlePreSize( ) const
{
  return _particlePreSize;
}

void StaticGradientModel::setParticlePreSize( float particlePreSize )
{
  _particlePreSize = particlePreSize;
}

float StaticGradientModel::getParticlePostSize( ) const
{
  return _particlePostSize;
}

void StaticGradientModel::setParticlePostSize( float particlePostSize )
{
  _particlePostSize = particlePostSize;
}

const tColorVec& StaticGradientModel::getGradient( ) const
{
  return _gradient;
}

void StaticGradientModel::setGradient( const tColorVec& gradient )
{
  _gradient = gradient;

  for ( int i = 0; i < MAX_COLORS; i++ )
  {
    float time = static_cast<float>(i) / 255.0f;
    _colors[ i ] = getGradientColor( _gradient , time );
  }
}

bool StaticGradientModel::isParticlePreVisibility( ) const
{
  return _particlePreVisibility;
}

void StaticGradientModel::setParticlePreVisibility( bool particlePreVisibility )
{
  _particlePreVisibility = particlePreVisibility;
}

bool StaticGradientModel::isParticlePostVisibility( ) const
{
  return _particlePostVisibility;
}

void
StaticGradientModel::setParticlePostVisibility( bool particlePostVisibility )
{
  _particlePostVisibility = particlePostVisibility;
}

void StaticGradientModel::uploadDrawUniforms( plab::UniformCache& cache ) const
{


  CameraModel::uploadDrawUniforms( cache );

  glUniform1f( cache.getLocation( "particlePreSize" ) , _particlePreSize );
  glUniform1f( cache.getLocation( "particlePostSize" ) , _particlePostSize );

  glUniform4fv( cache.getLocation( "gradientColors" ) , MAX_COLORS ,
                ( float* ) _colors );

  glUniform1f( cache.getLocation( "particlePreVisibility" ) ,
               _particlePreVisibility ? 1.0f : 0.0f );
  glUniform1f( cache.getLocation( "particlePostVisibility" ) ,
               _particlePostVisibility ? 1.0f : 0.0f );
}