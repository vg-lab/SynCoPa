//
// Created by gaeqs on 9/06/22.
//

#include <GL/glew.h>

#include "StaticModel.h"

#include <plab/core/UniformCache.h>

StaticModel::StaticModel( const std::shared_ptr< plab::ICamera >& camera ,
                          float particlePreSize , float particlePostSize ,
                          const glm::vec4& particlePreColor ,
                          const glm::vec4& particlePostColor ,
                          bool particlePreVisibility ,
                          bool particlePostVisibility )
  : CameraModel( camera )
  , _particlePreSize( particlePreSize )
  , _particlePostSize( particlePostSize )
  , _particlePreColor( particlePreColor )
  , _particlePostColor( particlePostColor )
  , _particlePreVisibility( particlePreVisibility )
  , _particlePostVisibility( particlePostVisibility )
{

}

float StaticModel::getParticlePreSize( ) const
{
  return _particlePreSize;
}

void StaticModel::setParticlePreSize( float particlePreSize )
{
  _particlePreSize = particlePreSize;
}

float StaticModel::getParticlePostSize( ) const
{
  return _particlePostSize;
}

void StaticModel::setParticlePostSize( float particlePostSize )
{
  _particlePostSize = particlePostSize;
}

const glm::vec4& StaticModel::getParticlePreColor( ) const
{
  return _particlePreColor;
}

void StaticModel::setParticlePreColor( const glm::vec4& particlePreColor )
{
  _particlePreColor = particlePreColor;
}

const glm::vec4& StaticModel::getParticlePostColor( ) const
{
  return _particlePostColor;
}

void StaticModel::setParticlePostColor( const glm::vec4& particlePostColor )
{
  _particlePostColor = particlePostColor;
}


bool StaticModel::isParticlePreVisibility( ) const
{
  return _particlePreVisibility;
}

void StaticModel::setParticlePreVisibility( bool particlePreVisibility )
{
  _particlePreVisibility = particlePreVisibility;
}

bool StaticModel::isParticlePostVisibility( ) const
{
  return _particlePostVisibility;
}

void StaticModel::setParticlePostVisibility( bool particlePostVisibility )
{
  _particlePostVisibility = particlePostVisibility;
}

void StaticModel::uploadDrawUniforms( plab::UniformCache& cache ) const
{
  CameraModel::uploadDrawUniforms( cache );

  glUniform1f( cache.getLocation( "particlePreSize" ) , _particlePreSize );
  glUniform1f( cache.getLocation( "particlePostSize" ) , _particlePostSize );

  glUniform4f(
    cache.getLocation( "particlePreColor" ) ,
    _particlePreColor.r ,
    _particlePreColor.g ,
    _particlePreColor.b ,
    _particlePreColor.a
  );

  glUniform4f(
    cache.getLocation( "particlePostColor" ) ,
    _particlePostColor.r ,
    _particlePostColor.g ,
    _particlePostColor.b ,
    _particlePostColor.a
  );

  glUniform1f( cache.getLocation( "particlePreVisibility" ) ,
               _particlePreVisibility ? 1.0f : 0.0f );
  glUniform1f( cache.getLocation( "particlePostVisibility" ) ,
               _particlePostVisibility ? 1.0f : 0.0f );

}