//
// Created by gaeqs on 9/06/22.
//

#include <GL/glew.h>

#include "StaticGradientModel.h"

#include <plab/core/UniformCache.h>
#include <QDebug>

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
  constexpr int MAX_COLORS = 256;

  CameraModel::uploadDrawUniforms( cache );

  glUniform1f( cache.getLocation( "particlePreSize" ) , _particlePreSize );
  glUniform1f( cache.getLocation( "particlePostSize" ) , _particlePostSize );

  int maxSize = std::min( MAX_COLORS , static_cast<int>(_gradient.size( )));
  glUniform1i( cache.getLocation( "gradientSize" ) , maxSize );

  float timeStamps[MAX_COLORS];
  glm::vec4 colors[MAX_COLORS];

  for ( int i = 0; i < maxSize; i++ )
  {
    auto& item = _gradient.at( i );
    timeStamps[ i ] = item.first;
    colors[ i ] = item.second;
  }

  glUniform1fv( cache.getLocation( "gradientTimes" ) , maxSize , timeStamps );
  glUniform4fv( cache.getLocation( "gradientColors" ) , maxSize ,
                ( float* ) colors );

  glUniform1f( cache.getLocation( "particlePreVisibility" ) ,
               _particlePreVisibility ? 1.0f : 0.0f );
  glUniform1f( cache.getLocation( "particlePostVisibility" ) ,
               _particlePostVisibility ? 1.0f : 0.0f );
}