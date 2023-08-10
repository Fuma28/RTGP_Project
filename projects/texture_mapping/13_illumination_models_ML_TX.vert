#version 410 core

// number of lights in the scene
#define NR_LIGHTS 1

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;
// UV coordinates
layout (location = 2) in vec2 UV;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;  
// the numbers used for the location in the layout qualifier are the positions of the vertex attribute
// as defined in the Mesh class

// vectors of lights positions (passed from the application)
uniform vec3 lights[NR_LIGHTS];

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;

// normals transformation matrix (= transpose of the inverse of the model-view matrix)
uniform mat3 normalMatrix;

// array of light incidence directions (in view coordinate)
out vec3 lightDirs[NR_LIGHTS];

// the transformed normal (in view coordinate) is set as an output variable, to be "passed" to the fragment shader
// this means that the normal values in each vertex will be interpolated on each fragment created during rasterization between two vertices
out vec3 vNormal;

out VS_OUT {
    vec3 vViewPosition;
    vec2 interp_UV;
    mat3 TBN;
} vs_out;  


void main(){

  vec3 T = normalize(vec3(modelMatrix * vec4(aTangent,   0.0)));
  vec3 B = normalize(vec3(modelMatrix * vec4(aBitangent, 0.0)));
  vec3 N = normalize(vec3(modelMatrix * vec4(normal,    0.0)));
  vs_out.TBN = mat3(T, B, N);
  
  // vertex position in ModelView coordinate (see the last line for the application of projection)
  // when I need to use coordinates in camera coordinates, I need to split the application of model and view transformations from the projection transformations
  vec4 mvPosition = viewMatrix * modelMatrix * vec4( position, 1.0 );

  // view direction, negated to have vector from the vertex to the camera
  vs_out.vViewPosition = -mvPosition.xyz;

  // transformations are applied to the normal
  vNormal = normalize( normalMatrix * normal );

  // light incidence directions for all the lights (in view coordinate)
  for (int i=0;i<NR_LIGHTS;i++)
  {
    vec4 lightPos = viewMatrix  * vec4(lights[i], 1.0);;
    lightDirs[i] = lightPos.xyz - mvPosition.xyz;
  }

  // I assign the values to a variable with "out" qualifier so to use the per-fragment interpolated values in the Fragment shader
  vs_out.interp_UV = UV;

  // we apply the projection transformation
  gl_Position = projectionMatrix * mvPosition;

}
