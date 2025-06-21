#version 410 core

layout (location = 0) in vec3 aPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

void main()
{    
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( aPosition, 1.0 );

}