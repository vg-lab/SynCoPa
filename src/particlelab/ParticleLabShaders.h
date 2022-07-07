//
// Created by gaeqs on 9/06/22.
//


#ifndef SYNCOPA_PARTICLELABSHADERS_H
#define SYNCOPA_PARTICLELABSHADERS_H

#include <string>

const static std::string STATIC_VERTEX_SHADER = R"(#version 430
uniform mat4 viewProjectionMatrix;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

uniform float particlePreSize;
uniform float particlePostSize;
uniform vec4 particlePreColor;
uniform vec4 particlePostColor;
uniform float particlePreVisibility;
uniform float particlePostVisibility;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 position;
layout(location = 2) in float isPostsynaptic;
layout(location = 3) in float value;

flat out vec4 color;
out vec2 uvCoord;
out float size;

void main()
{
    float pSize = isPostsynaptic * particlePostSize * particlePostVisibility
    + (1 - isPostsynaptic) * particlePreSize * particlePreVisibility;
    color = isPostsynaptic * particlePostColor + (1 - isPostsynaptic) * particlePreColor;

    gl_Position = viewProjectionMatrix
    * vec4(
    (vertex.x * pSize * cameraRight)
    + (vertex.y * pSize * cameraUp)
    + position, 1.0f) - vec4(0.0f, 0.0f, 0.1f, 0.0f);

    uvCoord = vertex.rg + vec2(0.5f, 0.5f);
    size = pSize;
})";

const static std::string STATIC_GRADIENT_VERTEX_SHADER = R"(#version 430
uniform mat4 viewProjectionMatrix;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

uniform float particlePreSize;
uniform float particlePostSize;

uniform int gradientSize;
uniform float gradientTimes[256];
uniform vec4 gradientColors[256];

uniform float particlePreVisibility;
uniform float particlePostVisibility;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 position;
layout(location = 2) in float isPostsynaptic;
layout(location = 3) in float value;

flat out vec4 color;
out vec2 uvCoord;
out float size;

vec4 gradient (float t) {
    if (gradientSize == 0) return vec4(1.0f, 0.0f, 1.0f, 1.0f);
    int first = gradientSize - 1;
    for (int i = 0; i < gradientSize; i++) {
        if (gradientTimes[i] > t) {
            first = i - 1;
            break;
        }
    }

    if(first == -1) return gradientColors[0];
    if (first == gradientSize - 1) return gradientColors[first];

    float start = gradientTimes[first];
    float end = gradientTimes[first + 1];
    float normalizedT = (t - start) / (end - start);

    return mix(gradientColors[first], gradientColors[first + 1], normalizedT);
}

void main()
{
    float pSize = isPostsynaptic * particlePostSize * particlePostVisibility
    + (1 - isPostsynaptic) * particlePreSize * particlePreVisibility;
    color = gradient(value);

    gl_Position = viewProjectionMatrix
    * vec4(
    (vertex.x * pSize * cameraRight)
    + (vertex.y * pSize * cameraUp)
    + position, 1.0f) - vec4(0.0f, 0.0f, 0.1f, 0.0f);

    uvCoord = vertex.rg + vec2(0.5f, 0.5f);
    size = pSize;
})";


const static std::string DYNAMIC_VERTEX_SHADER = R"(#version 430
uniform mat4 viewProjectionMatrix;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

uniform float particlePreSize;
uniform float particlePostSize;
uniform vec4 particlePreColor;
uniform vec4 particlePostColor;
uniform float particlePreVisibility;
uniform float particlePostVisibility;

uniform float timestamp;
uniform float pulseDuration;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 position;
layout(location = 2) in float isPostsynaptic;
layout(location = 3) in float particleTimestamp;

flat out vec4 color;
out vec2 uvCoord;
out float size;

void main()
{

    float pulseActive = float(timestamp > particleTimestamp &&
    timestamp <= particleTimestamp + pulseDuration);

    float pulseAlpha = 1 - (timestamp - particleTimestamp) / pulseDuration;

    float pSize = pulseActive *
    (isPostsynaptic * particlePostSize * particlePostVisibility
    + (1 - isPostsynaptic) * particlePreSize * particlePreVisibility);

    color = isPostsynaptic * particlePostColor + (1 - isPostsynaptic) * particlePreColor;
    color.a *= pulseActive * pulseAlpha;

    gl_Position = viewProjectionMatrix
    * vec4(
    (vertex.x * pSize * cameraRight)
    + (vertex.y * pSize * cameraUp)
    + position, 1.0f) - vec4(0.0f, 0.0f, 0.1f, 0.0f);

    uvCoord = vertex.rg + vec2(0.5f, 0.5f);
    size = pSize;
})";

const static std::string PARTICLE_FRAGMENT_SHADER = R"(#version 430

flat in vec4 color;
in vec2 uvCoord;
in float size;

out vec4 outputColor;

void main()
{
  vec2 p = -1.0 + 2.0 * uvCoord;
  float l = sqrt( dot( p,p ));
  l = 1.0 - clamp( l, 0.0, 1.0 );

  float margin = 0.55;
  float alpha =
    float(l <= margin) + (float(l > margin) * (1.0 -((l - margin) / (1.0 - margin))));
  alpha = 1.0 - alpha;

  //outputColor = vec4( vec3( linealDepth ), 1.0 );
  //outputColor = vec4( vec3( fragmentDepth ), 1.0 );

  outputColor = vec4( color.rgb,  alpha * color.a );
})";


#endif //SYNCOPA_PARTICLELABSHADERS_H
