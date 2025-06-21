#version 410 core

// number of lights in the scene
#define MAX_NR_LIGHTS 5

out vec4 colorFrag;

in vec2 UV;
in vec3 tLightDir[MAX_NR_LIGHTS];
in vec3 tViewDir;

uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float shininess;
uniform int repeat;
uniform int nLights;  //actual number of lights in the scene

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D heightMap;

// takes as input the UV coordinate of the fragment and the view direction, outputs the new UV coordinate dispaced according to the height map
vec2 OcclusionParallaxMapping(vec2 UV, vec3 viewDir);

void main(){

    vec2 repeated_UV = mod(UV * repeat, 1.0);
    vec3 color = Ka * ambientColor;
    vec3 V = normalize(tViewDir);
    vec2 parallaxUV = OcclusionParallaxMapping(repeated_UV, V);
    vec3 N = texture(normalMap, parallaxUV).rgb;
    N = normalize(N * 2.0 - 1.0);       //transform from range [0,1] into [-1,1]
    vec3 surface = texture(diffuseMap, parallaxUV).rgb;

    //for all the lights in the scene
    for(int i=0; i< nLights; i++){

        vec3 L = normalize(tLightDir[i]);
        float lambertian = max(dot(L,N), 0.0);

        // if the lambert coefficient is positive, then I can calculate the specular component
        if(lambertian > 0.0)
        { 
            vec3 H = normalize(L + V);
            float specAngle = max(dot(H, N), 0.0);
            float specular = pow(specAngle, shininess);
            color += vec3( Kd * lambertian * surface + Ks * specular * specularColor);
        }
    }
    colorFrag = vec4(color, 1.0);
}

vec2 OcclusionParallaxMapping(vec2 UV, vec3 viewDir)
{ 
    const float minLayers = 8.0;
    const float maxLayers = 64.0;
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));   //we take more samples when looking from an angle beacuse there is more displacement
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy * 0.2;     //0.2 is a value to scale height for better results, found empirically with some tests
    vec2 deltaTexCoords = P / numLayers;
    vec2  currentTexCoords = UV;
    float currentDepthMapValue = 1.0 - texture(heightMap, currentTexCoords).r;  // we want "depth" value because depth is easier to fake, so (1 - height)

    // search for the crossing point
    while(currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = 1.0 - texture(heightMap, currentTexCoords).r;  
        currentLayerDepth += layerDepth;  
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = 1.0 - texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight); //interpolation

    return finalTexCoords;  
}  