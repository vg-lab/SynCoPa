//
// Created by gaeqs on 22/3/22.
//

#ifndef SYNCOPA_SHADERPROGRAM_H
#define SYNCOPA_SHADERPROGRAM_H


#include "reto/reto.h"
#include "prefr/prefr.h"

class ShaderProgram : public prefr::IGLRenderProgram, public reto::ShaderProgram
{

public:

  ShaderProgram( );

  virtual ~ShaderProgram( )= default;

  void prefrActivateGLProgram( ) override;

  unsigned int prefrGLProgramID( ) override;

};


#endif //SYNCOPA_SHADERPROGRAM_H
