#version 410 core

// number of lights in the scene
#define MAX_NR_LIGHTS 5

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform int nLights;  //actual number of lights in the scene

uniform vec3 pointLightPosition[MAX_NR_LIGHTS];
uniform vec3 viewPosition; 

out vec2 UV;
out vec3 normal;
out vec3 lightDir[MAX_NR_LIGHTS];
out vec3 viewDir;

void main(){
  vec4 fragPos = modelMatrix * vec4( aPosition, 1.0 );  //apply model transformations -> fragment position in world space
  normal = normalize( normalMatrix * aNormal );         //transform normal in world space
  
  //for all the lights in the scene
  for(int i=0; i<nLights; i++){
    lightDir[i] = normalize(pointLightPosition[i] - fragPos.xyz);   //calculate light direction in world space
  } 
  
  viewDir = normalize(viewPosition - fragPos.xyz);    //calculate view direction in world space
  UV = aUV;
  gl_Position = projectionMatrix * viewMatrix * fragPos;  //apply project-view trasformation
}
