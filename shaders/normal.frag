#version 410 core

// number of lights in the scene
#define MAX_NR_LIGHTS 5

out vec4 colorFrag;

uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float shininess;
uniform int repeat;
uniform int nLights;  //actual number of lights in the scene

in vec2 UV;
in vec3 tLightDir[MAX_NR_LIGHTS];
in vec3 tViewDir;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

void main(){

    vec2 repeated_UV = mod(UV * repeat, 1.0);
    vec3 color = Ka * ambientColor;
    vec3 N = texture(normalMap, repeated_UV).rgb;
    N = normalize(N * 2.0 - 1.0);       //transform from range [0,1] into [-1,1]
    vec3 surface = texture(diffuseMap, repeated_UV).rgb;

    //for all the lights in the scene
    for(int i=0; i<nLights; i++){

        vec3 L = normalize(tLightDir[i]);
        float lambertian = max(dot(L,N), 0.0);
        
         // if the lambert coefficient is positive, then I can calculate the specular component
        if(lambertian > 0.0)
        { 
            vec3 V = normalize(tViewDir);
            vec3 H = normalize(L + V);
            float specAngle = max(dot(H, N), 0.0);
            float specular = pow(specAngle, shininess);
            color += vec3( Kd * lambertian * surface + Ks * specular * specularColor);
        }
    } 
    colorFrag = vec4(color, 1.0);
}