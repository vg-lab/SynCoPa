/*
 * @file	PrefrShaders.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef PREFRSHADERS_H_
#define PREFRSHADERS_H_

namespace prefr
{

const static std::string prefrVertexShader = "#version 330\n\
#extension GL_ARB_separate_shader_objects: enable\n\
uniform mat4 modelViewProjM;\n\
uniform vec3 cameraUp;\n\
uniform vec3 cameraRight;\n\
layout(location = 0) in vec3 vertexPosition;\n\
layout(location = 1) in vec4 particlePosition;\n\
layout(location = 2) in vec4 particleColor;\n\
out vec4 color;\n\
out vec2 uvCoord;\n\
out float size;\n\
void main()\n\
{\n\
  gl_Position = modelViewProjM\n\
        * vec4(\n\
        (vertexPosition.x * particlePosition.a * cameraRight)\n\
        + (vertexPosition.y * particlePosition.a * cameraUp)\n\
        + particlePosition.rgb, 1.0);\n\
  color = particleColor;\n\
  uvCoord = vertexPosition.rg + vec2(0.5, 0.5);\n\
  size = particlePosition.a;\
}";

const static std::string prefrFragmentShader = "#version 330\n\
\
in vec4 color; \n\
in vec2 uvCoord;\n\
\
out vec4 outputColor;\n\
\
uniform float threshold;\n\
uniform sampler2D depthMap;\
uniform vec2 invResolution;\
\
uniform float zNear;\
uniform float zFar;\
\
float LinearizeDepth(float depth)\
{\
  float z = depth * 2.0 - 1.0;\
  return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));\
}\
\
void main()\n\
{\n \
  vec2 p = -1.0 + 2.0 * uvCoord;\n\
  float l = sqrt(dot(p,p));\n \
  l = 1.0 - clamp(l, 0.0, 1.0);\n\
  vec2 coord = gl_FragCoord.xy * invResolution;\
  float backGroundDepth = -texture( depthMap, coord ).r;\
  float depth = gl_FragCoord.z / gl_FragCoord.w;\
  float fade = clamp(( backGroundDepth - depth ) * 0.2f, 0.0, 1.0 );\
  float margin = 1.0 - threshold;\n\
  float alpha = float(l <= margin) + (float(l > margin) * (1.0 -((l - margin) / (1.0 - margin))));\n\
  alpha = 1.0 - alpha;\n\
  outputColor = vec4(color.rgb, alpha );\n\
  outputColor = vec4(vec3( LinearizeDepth( texture( depthMap, coord ).r ) / zFar ), 1.0 );\n\
  outputColor = vec4(vec3( 0 ), 1 );\n\
}";

const static std::string prefrSoftParticles = "#version 400\n\
\n\
in vec4 color;\n\
in vec2 uvCoord;\n\
in float size;\n\
\n\
out vec4 outputColor;\n\
\n\
uniform float threshold;\n\
uniform vec2 invResolution;\n\
\n\
uniform float zNear;\n\
uniform float zFar;\n\
\n\
uniform sampler2D depthMap;\n\
\n\
float LinearizeDepth(float depth)\n\
{\n\
  float z = depth * 2.0 - 1.0;\n\
  return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));\n\
}\n\
\n\
void main()\n\
{\n\
  vec2 screenCoord = gl_FragCoord.xy * invResolution;\n\
  \n\
  float screenDepth = texture( depthMap, screenCoord ).r;\n\
  \n\
  float fragmentDepth = gl_FragCoord.z / gl_FragCoord.w;\n\
  \n\
  float linealScreenDepth = LinearizeDepth( screenDepth );\n\
  \n\
  if( fragmentDepth > linealScreenDepth )\n\
    discard;\n\
  \n\
  float weight = clamp((( linealScreenDepth - fragmentDepth ) / size ), 0.0, 1.0 );\n\
  \n\
  vec2 p = -1.0 + 2.0 * uvCoord;\n\
  float l = sqrt( dot( p,p ));\n\
  l = 1.0 - clamp( l, 0.0, 1.0 );\n\
  \n\
  float margin = 1.0 - threshold;\n\
  float alpha = float(l <= margin) + (float(l > margin) * (1.0 -((l - margin) / (1.0 - margin))));\n\
  alpha = 1.0 - alpha;\n\
  alpha = alpha * weight;\n\
  \n\
  //outputColor = vec4( vec3( linealDepth ), 1.0 );\n\
  //outputColor = vec4( vec3( fragmentDepth ), 1.0 );\n\
  \n\
  outputColor = vec4( color.rgb, alpha );\n\
}";

}

#endif /* PREFRSHADERS_H_ */
