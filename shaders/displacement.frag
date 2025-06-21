#version 410 core

// number of lights in the scene
#define MAX_NR_LIGHTS 5

out vec4 colorFrag;

uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float shininess;
uniform int nLights;  //actual number of lights in the scene
uniform vec3 pointLightPosition[MAX_NR_LIGHTS];
uniform vec3 viewPosition;

in vec2 UVs;
in vec4 fragPos;
in vec3 normal_out;

uniform sampler2D diffuseMap;

void main()
{   
    vec3 color = Ka * ambientColor;
    vec3 N = normalize(normal_out);
    vec3 surface = texture(diffuseMap, UVs).rgb;

    //for all the lights in the scene
    for(int i=0; i<nLights; i++){

        vec3 L = normalize(pointLightPosition[i] - fragPos.xyz);
        float lambertian = max(dot(L,N), 0.0);

        // if the lambert coefficient is positive, then I can calculate the specular component
        if(lambertian > 0.0)
        { 
            vec3 V = normalize(viewPosition - fragPos.xyz);
            vec3 H = normalize(L + V);
            float specAngle = max(dot(H, N), 0.0);
            float specular = pow(specAngle, shininess);
            color += vec3( Kd * lambertian * surface + Ks * specular * specularColor);
        }
    }
    colorFrag = vec4(color, 1.0);
}