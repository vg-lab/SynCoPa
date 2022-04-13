//
// Created by gaeqs on 22/3/22.
//

#include "ShaderProgram.h"

void ShaderProgram::prefrActivateGLProgram( )
{ use( );}

unsigned int ShaderProgram::prefrGLProgramID( )
{ return program( ); }

ShaderProgram::ShaderProgram( )
  : prefr::IGLRenderProgram( )
  , reto::ShaderProgram( )
{
  _viewProjectionMatrixAlias = std::string( "modelViewProjM" );
  _viewMatrixUpComponentAlias = std::string( "cameraUp" );
  _viewMatrixRightComponentAlias = std::string( "cameraRight" );
}
