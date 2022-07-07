//
// Created by gaeqs on 14/06/22.
//

#ifndef SYNCOPA_DYNAMICMODEL_H
#define SYNCOPA_DYNAMICMODEL_H


#include "StaticModel.h"

class DynamicModel : public StaticModel
{

  float _timestamp;
  float _maxTime;

  float _pulseDuration;

public:

  DynamicModel( const std::shared_ptr< plab::ICamera >& camera ,
                float particlePreSize , float particlePostSize ,
                const glm::vec4& particlePreColor ,
                const glm::vec4& particlePostColor ,
                bool particlePreVisibility , bool particlePostVisibility ,
                float timestamp , float maxTime, float pulseDuration );

  float getTimestamp( ) const;

  void setTimestamp( float timestamp );

  void addTime( float time );

  float getMaxTime( ) const;

  void setMaxTime( float maxTime );

  float getPulseDuration( ) const;

  void setPulseDuration( float pulseDuration );

  void uploadDrawUniforms( plab::UniformCache& cache ) const override;

};


#endif //SYNCOPA_DYNAMICMODEL_H
