//
// Created by gaeqs on 9/06/22.
//

#ifndef SYNCOPA_SYNAPSEPARTICLE_H
#define SYNCOPA_SYNAPSEPARTICLE_H

#include <glm/vec3.hpp>

struct SynapseParticle
{

  glm::vec3 position;
  float isPostsynaptic;
  float value;

  static void enableVAOAttributes( );

};


#endif //SYNCOPA_SYNAPSEPARTICLE_H
