#version 410 core

#define offset 0.0005   // =1/heighmap_size
#define scale_factor 100
#define MAX_NR_LIGHTS 5 // number of lights in the scene

out vec4 colorFrag;

uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float shininess;
uniform int repeat;
uniform int nLights;

in vec2 UV;
in vec3 tLightDir[MAX_NR_LIGHTS];
in vec3 tViewDir;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;

uniform sampler2D diffuseMap;
uniform sampler2D heightMap;

void main(){

    vec2 repeated_UV = mod(UV * repeat, 1.0);
    vec3 color = Ka * ambientColor;

    //perturbed normal computation
    float currentHeight = texture(heightMap, repeated_UV).r;                 //we sample height at [x,y]
    float x1_Height = texture(heightMap, repeated_UV + vec2(offset, 0.0)).r; //we sample height at [x+1,y]
    float y1_Height = texture(heightMap, repeated_UV + vec2(0.0, offset)).r; //we sample height at [x,y+1] 

    float Bv = y1_Height - currentHeight;
    float Bu = x1_Height - currentHeight;

    vec3 N = normalize(cross((bitangent + Bv * scale_factor * normal), (tangent + Bu * scale_factor* normal)));  // N' = B'x T' = (B + Bu*N) x (T + Bv*N)
    
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
