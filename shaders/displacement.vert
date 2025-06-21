#version 410 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat3 normalMatrix;
uniform mat4 modelMatrix;

out vec2 UVs;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;

// using tessellation vertex shader just apsses through UV, normal and position
void main()
{    
    UVs = aUV;
    normal = aNormal;
    tangent = aTangent;
    bitangent = aBitangent;
    gl_Position = vec4( aPosition, 1.0 );

}