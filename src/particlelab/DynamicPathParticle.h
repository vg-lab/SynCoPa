//
// Created by gaeqs on 14/06/22.
//

#ifndef SYNCOPA_DYNAMICPATHPARTICLE_H
#define SYNCOPA_DYNAMICPATHPARTICLE_H


#include <glm/vec3.hpp>

struct DynamicPathParticle
{

  glm::vec3 position;
  float isPostsynaptic;
  float timestamp;

  static void enableVAOAttributes( );

};


#endif //SYNCOPA_DYNAMICPATHPARTICLE_H
