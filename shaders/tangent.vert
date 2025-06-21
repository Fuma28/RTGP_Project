#version 410 core

// number of lights in the scene
#define MAX_NR_LIGHTS 5

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform int nLights;  //actual number of lights in the scene

uniform vec3 pointLightPosition[MAX_NR_LIGHTS];
uniform vec3 viewPosition;

out vec2 UV;
out vec3 tLightDir[MAX_NR_LIGHTS];
out vec3 tViewDir;

void main(){
  vec4 fragPos = modelMatrix * vec4( aPosition, 1.0 );  //apply model transformations -> fragment position in world space

  vec3 T = normalize(normalMatrix * aTangent);
  vec3 B = normalize(normalMatrix * aBitangent);
  vec3 N = normalize(normalMatrix * aNormal);
  mat3 TBN = transpose(mat3(T, B, N));
   
  //for all the lights in the scene
  for(int i=0; i<nLights; i++){
    tLightDir[i] = normalize(TBN * pointLightPosition[i] - TBN * fragPos.xyz);    //calculate light direction in tangent space
  }

  tViewDir = normalize(TBN * viewPosition - TBN * fragPos.xyz);   //calculate view direction in tangent space
  UV = aUV;
  gl_Position = projectionMatrix * viewMatrix * fragPos;
}
