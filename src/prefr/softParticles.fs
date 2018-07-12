#version 400

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

  float fragmentDepth = gl_FragCoord.z / gl_FragCoord.w / zFar;

  float linealScreenDepth = LinearizeDepth( screenDepth ) / zFar;

  if( fragmentDepth > linealScreenDepth )
  	discard;

  float weight = clamp((( linealScreenDepth - fragmentDepth ) / size ), 0.0, 1.0 );

  vec2 p = -1.0 + 2.0 * uvCoord;
  float l = sqrt( dot( p,p ));
  l = 1.0 - clamp( l, 0.0, 1.0 );

  float margin = 1.0 - threshold;
  float alpha = float(l <= margin) + (float(l > margin) * (1.0 -((l - margin) / (1.0 - margin))));
  alpha = 1.0 - alpha;
  alpha = alpha * weight;

  //outputColor = vec4( vec3( linealDepth ), 1.0 );
  //outputColor = vec4( vec3( fragmentDepth ), 1.0 );

  outputColor = vec4( color.rgb, alpha );
}