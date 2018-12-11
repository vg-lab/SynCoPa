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

const static std::string prefrVertexShader = R"(#version 330
#extension GL_ARB_separate_shader_objects: enable
uniform mat4 modelViewProjM;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 particlePosition;
layout(location = 2) in vec4 particleColor;
out vec4 color;
out vec2 uvCoord;
out float size;
void main()
{
  gl_Position = modelViewProjM
        * vec4(
        (vertexPosition.x * particlePosition.a * cameraRight)
        + (vertexPosition.y * particlePosition.a * cameraUp)
        + particlePosition.rgb, 1.0);
  color = particleColor;
  uvCoord = vertexPosition.rg + vec2(0.5, 0.5);
  size = particlePosition.a;
})";

const static std::string prefrFragmentShader = R"(#version 330

in vec4 color; 
in vec2 uvCoord;

out vec4 outputColor;

uniform float threshold;
uniform sampler2D depthMap;
uniform vec2 invResolution;

uniform float zNear;
uniform float zFar;

float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0;
  return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
  vec2 p = -1.0 + 2.0 * uvCoord;
  float l = sqrt(dot(p,p));
  l = 1.0 - clamp(l, 0.0, 1.0);
  vec2 coord = gl_FragCoord.xy * invResolution;\
  float backGroundDepth = -texture( depthMap, coord ).r;
  float depth = gl_FragCoord.z / gl_FragCoord.w;\
  float fade = clamp(( backGroundDepth - depth ) * 0.2f, 0.0, 1.0 );
  float margin = 1.0 - threshold;
  float alpha = float(l <= margin) + (float(l > margin) * (1.0 -((l - margin) / (1.0 - margin))));
  alpha = 1.0 - alpha;
  outputColor = vec4(color.rgb, alpha );
  outputColor = vec4(vec3( LinearizeDepth( texture( depthMap, coord ).r ) / zFar ), 1.0 );
  outputColor = vec4(vec3( 0 ), 1 );
})";

const static std::string prefrSoftParticles = R"(#version 400

in vec4 color;
in vec2 uvCoord;
in float size;

out vec4 outputColor;

uniform float threshold;
uniform vec2 invResolution;

uniform float zNear;
uniform float zFar;

uniform sampler2D depthMap;

float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0;
  return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
  vec2 screenCoord = gl_FragCoord.xy * invResolution;
  
  float screenDepth = texture( depthMap, screenCoord ).r;
  
  float fragmentDepth = gl_FragCoord.z / gl_FragCoord.w;
  
  float linealScreenDepth = LinearizeDepth( screenDepth );
  
  if( fragmentDepth > linealScreenDepth )
    discard;
  
  float weight = clamp((( linealScreenDepth - fragmentDepth ) / size ), 0.0, 1.0 );
  
  vec2 p = -1.0 + 2.0 * uvCoord;
  float l = sqrt( dot( p,p ));
  l = 1.0 - clamp( l, 0.0, 1.0 );
  
  float margin = 1.0 - threshold;
  float alpha = 
    float(l <= margin) + (float(l > margin) * (1.0 -((l - margin) / (1.0 - margin))));
  alpha = 1.0 - alpha;
  alpha = alpha * weight;
  
  //outputColor = vec4( vec3( linealDepth ), 1.0 );
  //outputColor = vec4( vec3( fragmentDepth ), 1.0 );
  
  outputColor = vec4( color.rgb, alpha * color.a );
})";

}

#endif /* PREFRSHADERS_H_ */
