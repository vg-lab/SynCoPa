//
// Created by gaeqs on 14/06/22.
//

#include <GL/glew.h>

#include "DynamicModel.h"
#include <plab/core/UniformCache.h>

#include <cmath>

DynamicModel::DynamicModel( const std::shared_ptr< plab::ICamera >& camera ,
                            float particlePreSize , float particlePostSize ,
                            const glm::vec4& particlePreColor ,
                            const glm::vec4& particlePostColor ,
                            bool particlePreVisibility ,
                            bool particlePostVisibility ,
                            float timestamp ,
                            float maxTime ,
                            float pulseDuration )
  : StaticModel( camera , particlePreSize , particlePostSize ,
                 particlePreColor , particlePostColor , particlePreVisibility ,
                 particlePostVisibility )
  , _timestamp( timestamp )
  , _maxTime( maxTime )
  , _pulseDuration( pulseDuration )
{ }

float DynamicModel::getTimestamp( ) const
{
  return _timestamp;
}

void DynamicModel::setTimestamp( float timestamp )
{
  _timestamp = _maxTime == 0.0f ? 0.0f :
    fmodf( timestamp , _maxTime + _pulseDuration );
}

void DynamicModel::addTime( float time )
{
  _timestamp = _maxTime == 0.0f ? 0.0f :
    fmodf( _timestamp + time , _maxTime + _pulseDuration );
}

float DynamicModel::getMaxTime( ) const
{
  return _maxTime;
}

void DynamicModel::setMaxTime( float maxTime )
{
  _maxTime = maxTime;
  _timestamp = maxTime == 0.0f ? 0.0f :
    fmodf( _timestamp , _maxTime + _pulseDuration );
}

float DynamicModel::getPulseDuration( ) const
{
  return _pulseDuration;
}

void DynamicModel::setPulseDuration( float pulseDuration )
{
  _pulseDuration = pulseDuration;
  _timestamp = _maxTime == 0.0f ? 0.0f :
               fmodf( _timestamp , _maxTime + _pulseDuration );
}


void DynamicModel::uploadDrawUniforms( plab::UniformCache& cache ) const
{
  StaticModel::uploadDrawUniforms( cache );
  glUniform1f( cache.getLocation( "timestamp" ) , _timestamp );
  glUniform1f( cache.getLocation( "pulseDuration" ) , _pulseDuration );
}
