//
// Created by gaeqs on 9/06/22.
//

#ifndef SYNCOPA_STATICMODEL_H
#define SYNCOPA_STATICMODEL_H

#include <plab/reto/CameraModel.h>
#include <glm/vec4.hpp>

class StaticModel : public plab::CameraModel
{

  float _particlePreSize;
  float _particlePostSize;
  glm::vec4 _particlePreColor;
  glm::vec4 _particlePostColor;
  bool _particlePreVisibility;
  bool _particlePostVisibility;

public:

  StaticModel( const std::shared_ptr< plab::ICamera >& camera ,
               float particlePreSize , float particlePostSize ,
               const glm::vec4& particlePreColor ,
               const glm::vec4& particlePostColor ,
               bool particlePreVisibility ,
               bool particlePostVisibility );

  float getParticlePreSize( ) const;

  void setParticlePreSize( float particlePreSize );

  float getParticlePostSize( ) const;

  void setParticlePostSize( float particlePostSize );

  const glm::vec4& getParticlePreColor( ) const;

  void setParticlePreColor( const glm::vec4& particlePreColor );

  const glm::vec4& getParticlePostColor( ) const;

  void setParticlePostColor( const glm::vec4& particlePostcolor );

  bool isParticlePreVisibility( ) const;

  void setParticlePreVisibility( bool particlePreVisibility );

  bool isParticlePostVisibility( ) const;

  void setParticlePostVisibility( bool particlePostVisibility );

  void uploadDrawUniforms( plab::UniformCache& cache ) const override;

};


#endif //SYNCOPA_STATICMODEL_H
