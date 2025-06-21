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

uniform vec3 pointLightPosition[MAX_NR_LIGHTS];
uniform vec3 viewPosition;
uniform int repeat;
uniform int nLights; //actual number of lights in the scene

out vec2 UV;
out vec3 lightDir[MAX_NR_LIGHTS];
out vec3 tViewDir;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;


void main(){

  vec4 fragPos = modelMatrix * vec4( aPosition, 1.0 );  //apply model transformations -> fragment position in world space
  
  normal = normalize(normalMatrix * aNormal);       //transform normal in world space
  tangent = normalize(normalMatrix * aTangent);     //transform tangent in world space
  bitangent = normalize(normalMatrix * aBitangent); //transform bitangent in world space
  
  //for all the lights in the scene
  for(int i=0; i<nLights; i++){
    lightDir[i] = normalize(pointLightPosition[i] - fragPos.xyz);   //calculate light direction in world space
  } 
  tViewDir = normalize(viewPosition - fragPos.xyz);   //calculate view direction in world space
  UV = aUV;
  gl_Position = projectionMatrix * viewMatrix * fragPos;  //apply project-view trasformation

}

/*vec3 computeNormal (vec2 UV){

    float currentHeight = texture(heightMap, UV).r;                 //we sample height at [x,y]
    float x1_Height = texture(heightMap, UV + vec2(offset, 0.0)).r; //we sample height at [x+1,y]
    float y1_Height = texture(heightMap, UV + vec2(0.0, offset)).r; //we sample height at [x,y+1] 

    float dx = currentHeight - x1_Height;
    float dy = currentHeight - y1_Height;
    
    vec3 normal = normalize(vec3(dx, dy, offset));  //We should calculate tangent and bitangent and do cross product, but because 1 value in each vector is zero this is the easier and faster way

    return normal;
}*/
