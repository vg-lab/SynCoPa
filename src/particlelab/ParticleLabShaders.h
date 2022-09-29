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

layout (location = 0) out vec4 accumulation;
layout (location = 1) out float reveal;

void main()
{
    vec2 p = -1.0 + 2.0 * uvCoord;
    float l = sqrt(dot(p, p));
    l = 1.0 - clamp(l, 0.0, 1.0);
    l *= color.a;

    vec4 c = vec4(color.rgb, l);

    float weight = clamp(pow(min(1.0, c.a * 10.0) + 0.01, 3.0) * 1e8
    * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

    accumulation = vec4 (c.rgb * c.a, c.a) * weight;
    reveal = c.a;
})";

const static std::string PARTICLE_ACC_FRAGMENT_SHADER = R"(#version 430
flat in vec4 color;
in vec2 uvCoord;

out vec4 fragColor;

void main()
{
    vec2 p = -1.0 + 2.0 * uvCoord;
    float l = sqrt(dot(p, p));
    l = 1.0 - clamp(l, 0.0, 1.0);
    l *= color.a;

    fragColor = vec4(color.rgb, l);
}
)";

const static std::string SHADER_SCREEN_VERTEX = R"(#version 330

// shader inputs
layout (location = 0) in vec3 position;

void main()
{
	gl_Position = vec4(position, 1.0f);
}
)";

const static std::string SHADER_SCREEN_FRAGMENT = R"(#version 330

// shader outputs
layout (location = 0) out vec4 frag;

// color accumulation buffer
uniform sampler2D accumulation;

// revealage threshold buffer
uniform sampler2D reveal;

// opaque color buffer
uniform sampler2D opaque;

// epsilon number
const float EPSILON = 0.00001f;

// caluclate floating point numbers equality accurately
bool isApproximatelyEqual(float a, float b)
{
    return abs(a - b) <= max(a, b) * EPSILON;
}

// get the max value between three values
float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

void main()
{
    // fragment coordination
    ivec2 coords = ivec2(gl_FragCoord.xy);

    // fragment revealage
    float revealage = texelFetch(reveal, coords, 0).r;

    // save the blending and color texture fetch cost if there is not a transparent fragment
    vec4 accumulationColor = vec4(0.0f);
    if (isApproximatelyEqual(revealage, 1.0f)) {
      revealage = 1.0f;
      accumulationColor = vec4(0.0f);
    } else {
      // fragment color
      accumulationColor = texelFetch(accumulation, coords, 0);

      // suppress overflow
      if (isinf(max3(abs(accumulationColor.rgb)))) {
        accumulationColor.rgb = vec3(accumulationColor.a);
      }
    }

    // prevent floating point precision bug
    vec3 average_color = accumulationColor.rgb / max(accumulationColor.a, EPSILON);

    // blend pixels
    vec3 opaque_color = texelFetch(opaque, coords, 0).rgb;
    frag = vec4(mix(average_color, opaque_color, revealage), 1.0f);
}
)";


#endif //SYNCOPA_PARTICLELABSHADERS_H
