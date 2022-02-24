/*
 * @file  renderToTexture.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_PREFR_RENDERTOTEXTURE_CPP_
#define SRC_PREFR_RENDERTOTEXTURE_CPP_
/*
 * Copyright (c) 2014-2018 GMRV/URJC.
 *
 * Authors: Sergio Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of PReFr <https://gmrv.gitlab.com/nsviz/prefr>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

// Part of this code is made following this guide:
// https://learnopengl.com/Advanced-OpenGL/Framebuffers

#include <GL/glew.h>

#ifdef Darwin
  #define __gl_h_
  #define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <GL/freeglut.h>
#else
  #include <GL/gl.h>
  #include <GL/freeglut.h>
#endif

#include <cmath>

#include <nlgeometry/nlgeometry.h>
#include <nlgenerator/nlgenerator.h>
#include <nlrender/nlrender.h>
#include <reto/reto.h>
#include <nsol/nsol.h>
#include <prefr/prefr.h>
#include <reto/reto.h>
#include <Eigen/Dense>



// GLUT Functions
void renderFunc( void );
void resizeFunc( int width, int height );
void idleFunc( void );
void mouseFunc( int button, int state, int x, int y );
void mouseMoveFunc( int xCoord, int yCoord );
void initContext( int argc, char** argv );
void initOGL( void );


reto::OrbitalCameraController* camera;

bool rotation = false;
bool translation = false;

bool paintSoma = true;
bool paintNeurites = true;

int mxCoord;
int myCoord;

unsigned int screenWidth;
unsigned int screenHeight;

reto::ShaderProgram shader;
reto::ShaderProgram screenShader;

nlgeometry::Meshes meshes;
nlrender::Renderer* renderer;
std::vector< Eigen::Matrix4f > models;

//reto::Texture2D* cubeTexture;
//reto::Texture2D* floorTexture;

unsigned int framebuffer;
unsigned int textureColorbuffer;
unsigned int textureDepthbuffer;

//unsigned int cubeVAO;
//unsigned int cubeVBO;
//
//unsigned int planeVAO;
//unsigned int planeVBO;

unsigned int quadVAO;
unsigned int quadVBO;

/*
//std::string objectsVertex = "\
//\
//#version 330 core\n\
//layout (location = 0) in vec3 aPos;\
//layout (location = 1) in vec2 aTexCoords;\
//\
//out vec2 TexCoords;\
//\
//uniform mat4 model;\
//uniform mat4 view;\
//uniform mat4 projection;\
//\
//void main()\
//{\
//    TexCoords = aTexCoords;\
//    gl_Position = projection * view * model * vec4(aPos, 1.0);\
//}";


//std::string objectsFragment = "\
//\
//#version 330 core\n\
//out vec4 FragColor;\
//\
//in vec2 TexCoords;\
//\
//uniform sampler2D texture1;\
//\
//void main()\
//{    \
//    FragColor = texture(texture1, TexCoords);\
//}";
*/
/*
std::string screenVertex = "\
#version 330 core\n\
layout (location = 0) in vec2 aPos;\
layout (location = 1) in vec2 aTexCoords;\
\
out vec2 TexCoords;\
\
void main() \
{\
    TexCoords = aTexCoords;\
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\
}";
*/
/*
std::string screenFragment = "\
#version 330 core\n\
out vec4 FragColor;\
\
in vec2 TexCoords;\
\
uniform sampler2D screenTexture;\
\
void main()\
{\
    vec3 col = texture(screenTexture, TexCoords).rgb;\
    FragColor = vec4(col, 1.0);\
}";
*/

std::string vertex = "\
#version 400\n\
\
in vec2 inPosition;\
out vec2 uv;\
\
void main( void )\n\
{\n\
  uv = (inPosition + vec2( 1.0, 1.0 )) * 0.5;\
  gl_Position = vec4( inPosition,  0.1, 1.0 );\
}\n";

std::string fragment = "\
#version 400\n\
\
in vec2 uv;\
out vec3 oColor;\
\
uniform sampler2D renderedTexture;\
\
float near = 0.1;\
float far = 100.;\
\
float LinearizeDepth(float depth)\
{\
  float z = depth * 2.0 - 1.0;\
  return (2.0 * near * far) / (far + near - z * (far - near));\
}\
\
void main( void )\
{\
  //float depthValue = LinearizeDepth( texture( depthTexture, uv ).r ) / far;\n\
  //oColor = vec3( depthValue );\n\
  oColor = texture( renderedTexture, uv ).xyz;\
\n}\n";


glm::vec3 floatPtrToVec3( float* floatPos )
{
  return glm::vec3( floatPos[ 0 ],
                    floatPos[ 1 ],
                    floatPos[ 2 ]);
}

glm::mat4x4 floatPtrToMat4( float* floatPos )
{
  return glm::mat4x4( floatPos[ 0 ], floatPos[ 1 ],
                      floatPos[ 2 ], floatPos[ 3 ],
                      floatPos[ 4 ], floatPos[ 5 ],
                      floatPos[ 6 ], floatPos[ 7 ],
                      floatPos[ 8 ], floatPos[ 9 ],
                      floatPos[ 10 ], floatPos[ 11 ],
                      floatPos[ 12 ], floatPos[ 13 ],
                      floatPos[ 14 ], floatPos[ 15 ]);
}

void initContext( int argc, char** argv )
{
  glutInit( &argc,argv );
  glutInitContextVersion( 4, 0 );
//  glutInitContextFlags( GLUT_FORWARD_COMPATIBLE );
//  glutInitContextProfile( GLUT_CORE_PROFILE );

  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
  glutInitWindowSize( screenWidth, screenHeight );
  glutInitWindowPosition( 0, 0 );
  glutCreateWindow( "GLUT example" );

  glewExperimental = GL_TRUE;
  GLenum err = glewInit( );
  if ( GLEW_OK != err ) {
    std::cout << "Error: " << glewGetErrorString( err ) << std::endl;
    exit ( -1 );
  }
  const GLubyte *oglVersion = glGetString( GL_VERSION );
  std::cout << "This system supports OpenGL Version: "
            << oglVersion << std::endl;

  glutReshapeFunc( resizeFunc );
  glutDisplayFunc( renderFunc );
  glutIdleFunc( idleFunc );
  glutMouseFunc( mouseFunc );
  glutMotionFunc( mouseMoveFunc );
}

void initOGL( void )
{

  glGenTextures( 1, &textureColorbuffer );
  glBindTexture( GL_TEXTURE_2D, textureColorbuffer );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0,
                GL_RGB, GL_UNSIGNED_BYTE, 0 );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glActiveTexture( GL_TEXTURE0 );

  glGenTextures( 1, &textureDepthbuffer );
  glBindTexture( GL_TEXTURE_2D, textureDepthbuffer );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0 );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );


  glGenFramebuffers( 1, &framebuffer );
  glBindFramebuffer( GL_FRAMEBUFFER, framebuffer );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                          textureColorbuffer, 0 );
//  unsigned int rbo;
//  glGenRenderbuffers( 1, &rbo );
//  glBindRenderbuffer( GL_RENDERBUFFER, rbo );
//  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
//                         screenWidth, screenHeight);
//
//  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
//                             GL_RENDERBUFFER, rbo);

//  glBindFramebuffer( GL_FRAMEBUFFER, framebuffer );
//  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
//                          textureColorbuffer, 0 );


  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                          textureDepthbuffer, 0 );

//  GLenum buffers [ 2 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
//
//  glDrawBuffers( 2, buffers );

  if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
      std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  screenShader.loadFromText( vertex, fragment );
//  screenShader.loadFromText( vertex, fragment);
  screenShader.compileAndLink( );
  screenShader.autocatching( );

   static const float quadBufferData[ ] =
   {
     -1.0f, 1.0f,
     -1.0f, -1.0f,
     1.0f,  -1.0f,
     1.0f,  1.0f,
   };

   static const unsigned int quadIndices[ ] =
   {
     0, 1, 2,
     0, 2, 3,
   };

   glGenVertexArrays( 1, &quadVAO);
   glBindVertexArray( quadVAO );

   unsigned int vbo[2];
   glGenBuffers( 2, vbo );
   glBindBuffer( GL_ARRAY_BUFFER, vbo[0]);
   glBufferData( GL_ARRAY_BUFFER, sizeof( quadBufferData ), quadBufferData,
                 GL_STATIC_DRAW );
   glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );
   glEnableVertexAttribArray( 0 );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo[1] );
   glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( quadIndices ), quadIndices,
                 GL_STATIC_DRAW );

}


void renderFunc( void )
{



  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
//  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


  glEnable( GL_DEPTH_TEST ); // enable depth testing (is disabled for rendering screen-space quad)

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  // make sure we clear the framebuffer's content
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//  glViewport( 0, 0, screenWidth, screenHeight );

  Eigen::Matrix4f view( camera->camera()->viewMatrix( ));
  renderer->viewMatrix( ) = view;
  Eigen::Matrix4f proj( camera->camera()->projectionMatrix( ));
  renderer->projectionMatrix( ) = proj;
  renderer->render( meshes, models, Eigen::Vector3f( 0.3f, 0.3f, 0.8f ));
  glFlush( );

//  size_t size = screenWidth * screenHeight * 1;
//  float* data  = (float*) malloc( size * sizeof( float ));
//  glBindTexture(GL_TEXTURE_2D, textureDepthbuffer );
//  glGetTexImage( GL_TEXTURE_2D, 0, GL_DEPTH, GL_FLOAT, data );

//  for( unsigned int i = 0; i < size; ++i )
//  {
//    std::cout << " " << data[ i ];
//  }

//  std::cout << std::endl;
//  std::cout << std::endl;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable( GL_DEPTH_TEST );

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

//  glViewport( 0, 0, screenWidth, screenHeight );

  screenShader.use( );
  GLuint texID = glGetUniformLocation( screenShader.program( ),
                                       "renderedTexture" );

//  GLuint depthtexID = glGetUniformLocation( screenShader.program( ),
//                                            "depthTexture" );

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureColorbuffer );
  glUniform1i( texID, 0);

//  glActiveTexture( GL_TEXTURE0 );
//  glBindTexture( GL_TEXTURE_2D, textureDepthbuffer );
//  glUniform1i( depthtexID, 0 );
  glBindVertexArray( quadVAO );

//  glBindTexture(GL_TEXTURE_2D, textureColorbuffer );
//  glBindTexture(GL_TEXTURE_2D, textureDepthbuffer );

  screenShader.use();
  glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );


  glutSwapBuffers( );

}

void resizeFunc( int width, int height )
{
  screenWidth = width;
  screenHeight = height;

  glBindTexture( GL_TEXTURE_2D, textureColorbuffer );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0,
                GL_RGB, GL_UNSIGNED_BYTE, 0 );

  glBindTexture( GL_TEXTURE_2D, textureDepthbuffer );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0 );


  camera->windowSize(width, height);
  glViewport( 0, 0, width, height );
}

void idleFunc( void )
{

  static float angle = 0.0f;
  angle = ( angle > 2.0f * float( M_PI )) ? 0 : angle + 0.01f;
  glutPostRedisplay( );

}

void mouseFunc( int button, int state, int xCoord, int yCoord )
{
  if( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
  {
    rotation = true;
    mxCoord = xCoord;
    myCoord = yCoord;
  }

  if( button == GLUT_LEFT_BUTTON && state == GLUT_UP )
  {
    rotation=false;
  }

  if( button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN )
  {
    translation = true;
    mxCoord = xCoord;
    myCoord = yCoord;
  }

  if( button == GLUT_MIDDLE_BUTTON && state == GLUT_UP )
  {
    translation = false;
  }

  if( button == 3 && state == GLUT_DOWN )
  {
    camera->radius( camera->radius( ) / 1.1f );
  }

  if( button == 4 && state == GLUT_DOWN )
  {
    camera->radius( camera->radius( ) * 1.1f );
  }
}

void mouseMoveFunc( int xCoord, int yCoord )
{
  if( rotation )
  {
    camera->rotate( Eigen::Vector3f( - ( mxCoord - xCoord ) * 0.01,
                          - ( myCoord - yCoord ) * 0.01, 0.0f));
    mxCoord = xCoord;
    myCoord = yCoord;
  }
  if( translation )
  {
    mxCoord = xCoord;
    myCoord = yCoord;
  }
}


void usage( void )
{
  std::cout << "PReFr OpenGL-Glut example. " << std::endl
            << "Usage: " << std::endl
            << " prefrOGL [ Max_Particles ][ Num_Clusters ]" << std::endl
            << " where Max_Particles is the maximum number of the particles used on the example"
            << " and Num_Clusters is the total number of particle clusters. In this example "
            << " the number of total particles will be split uniformly over the clusters, as "
            << " for example 1000 particles and 20 clusters will lead to 50 particles per cluster."
            << " Note: by default example will use 1000 particles and a single cluster."
            << std::endl;
}

int main( int argc, char** argv )
{
  screenWidth = 500;
  screenHeight = 500;

  initContext( argc, argv );
  initOGL( );

  camera = new reto::OrbitalCameraController( );
  renderer = new nlrender::Renderer( );

  renderer->lod( ) = 0.3f;
  renderer->maximumDistance( ) = 1000.0f;
  nlgeometry::MeshPtr mesh;
  nlgeometry::AttribsFormat format( 3 );
  format[0] = nlgeometry::TAttribType::POSITION;
  format[1] = nlgeometry::TAttribType::CENTER;
  format[2] = nlgeometry::TAttribType::TANGENT;

  std::string fileName( argv[ 1 ] );

  nsol::MorphologyPtr morphology = nullptr;
  nsol::SwcReader swcr;
  morphology = swcr.readMorphology( fileName );
  nsol::Simplifier::Instance( )->simplify(
    dynamic_cast< nsol::NeuronMorphologyPtr >( morphology ),
    nsol::Simplifier::DIST_NODES_RADIUS );

  nlgeometry::AxisAlignedBoundingBox aabb;

  if ( morphology )
  {
    mesh = nlgenerator::MeshGenerator::generateMesh( morphology );
    std::cout << "Loaded morphology with: "
              << mesh->vertices( ).size( ) << " vertices, "
              << mesh->triangles( ).size( ) << " triangles and "
              << mesh->quads( ).size( ) << " quads" << std::endl;

    mesh->uploadGPU( format, nlgeometry::Facet::PATCHES );
    mesh->computeBoundingBox( );
    mesh->clearCPUData( );
    meshes.push_back( mesh );
    models.push_back( Eigen::Matrix4f::Identity( ));

    nlgeometry::AxisAlignedBoundingBox meshAABB = mesh->aaBoundingBox( );

    if ( meshAABB.minimum( ).x( ) < aabb.minimum( ).x( ))
      aabb.minimum( ).x( ) = meshAABB.minimum( ).x();
    if ( meshAABB.minimum( ).y( ) < aabb.minimum( ).y( ))
      aabb.minimum( ).y( ) = meshAABB.minimum( ).y();
    if ( meshAABB.minimum( ).z( ) < aabb.minimum( ).z( ))
      aabb.minimum( ).z( ) = meshAABB.minimum( ).z();
    if ( meshAABB.maximum( ).x( ) > aabb.maximum( ).x( ))
      aabb.maximum( ).x( ) = meshAABB.maximum( ).x();
    if ( meshAABB.maximum( ).y( ) > aabb.maximum( ).y( ))
      aabb.maximum( ).y( ) = meshAABB.maximum( ).y();
    if ( meshAABB.maximum( ).z( ) > aabb.maximum( ).z( ))
      aabb.maximum( ).z( ) = meshAABB.maximum( ).z();
  }

  camera->position( aabb.center( ));
  camera->radius( aabb.radius( ) / sin( camera->camera()->fieldOfView()));
  // camera->pivot( Eigen::Vector3f::Zero( ));
  // camera->radius( 1000.0f );

  Eigen::Matrix4f projection( camera->camera()->projectionMatrix( ));
  renderer->projectionMatrix( ) = projection;
  Eigen::Matrix4f view( camera->camera()->viewMatrix( ));
  renderer->viewMatrix( ) = view;



  glutMainLoop( );

  return 0;
}

#endif /* SRC_PREFR_RENDERTOTEXTURE_CPP_ */
