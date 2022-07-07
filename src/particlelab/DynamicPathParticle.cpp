//
// Created by gaeqs on 14/06/22.
//

#include <GL/glew.h>

#include "DynamicPathParticle.h"

void DynamicPathParticle::enableVAOAttributes( )
{
  glEnableVertexAttribArray( 1 );
  glVertexAttribPointer( 1 , 3 , GL_FLOAT , GL_FALSE ,
                         sizeof( DynamicPathParticle ) ,
                         ( void* ) 0 );
  glVertexAttribDivisor( 1 , 1 );

  glEnableVertexAttribArray( 2 );
  glVertexAttribPointer( 2 , 1 , GL_FLOAT , GL_FALSE ,
                         sizeof( DynamicPathParticle ) ,
                         ( void* ) ( sizeof( float ) * 3 ));
  glVertexAttribDivisor( 2 , 1 );

  glEnableVertexAttribArray( 3 );
  glVertexAttribPointer( 3 , 1 , GL_FLOAT , GL_FALSE ,
                         sizeof( DynamicPathParticle ) ,
                         ( void* ) ( sizeof( float ) * 4 ));
  glVertexAttribDivisor( 3 , 1 );
}

