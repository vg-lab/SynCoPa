//
// Created by gaeqs on 9/06/22.
//

#ifndef SYNCOPA_STATICPARTICLEMODEL_H
#define SYNCOPA_STATICPARTICLEMODEL_H

#include "../types.h"

#include <plab/reto/CameraModel.h>
#include <glm/vec4.hpp>


class StaticGradientModel : public plab::CameraModel
{

  float _particlePreSize;
  float _particlePostSize;
  tColorVec _gradient;
  bool _particlePreVisibility;
  bool _particlePostVisibility;

public:

  StaticGradientModel( const std::shared_ptr< plab::ICamera >& camera ,
                       float particlePreSize , float particlePostSize ,
                       const tColorVec& gradient ,
                       bool particlePreVisibility ,
                       bool particlePostVisibility );

  float getParticlePreSize( ) const;

  void setParticlePreSize( float particlePreSize );

  float getParticlePostSize( ) const;

  void setParticlePostSize( float particlePostSize );

  bool isParticlePreVisibility( ) const;

  void setParticlePreVisibility( bool particlePreVisibility );

  bool isParticlePostVisibility( ) const;

  void setParticlePostVisibility( bool particlePostVisibility );

  const tColorVec& getGradient( ) const;

  void setGradient( const tColorVec& gradient );

  void uploadDrawUniforms( plab::UniformCache& cache ) const override;

};


#endif //SYNCOPA_STATICPARTICLEMODEL_H
