#version 410 core

// number of lights in the scene
#define NR_LIGHTS 1

const float PI = 3.14159265359;

// output shader variable
out vec4 colorFrag;

// array with lights incidence directions (calculated in vertex shader, interpolated by rasterization)
in vec3 lightDirs[NR_LIGHTS];

// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 vNormal;
// vector from fragment to camera (in view coordinate)
in vec3 vViewPosition;

// interpolated texture coordinates
in vec2 interp_UV;

in VS_OUT {
    vec3 vViewPosition;
    vec2 interp_UV;
    mat3 TBN;
} fs_in;  

// texture repetitions
uniform float repeat;

// texture sampler
uniform sampler2D tex;

// normal map sampler
uniform sampler2D normal0;

// ambient and specular components (passed from the application)
uniform vec3 ambientColor;
uniform vec3 specularColor;

// weight of the components
// in this case, we can pass separate values from the main application even if Ka+Kd+Ks>1. In more "realistic" situations, I have to set this sum = 1, or at least Kd+Ks = 1, by passing Kd as uniform, and then setting Ks = 1.0-Kd
uniform float Ka;
uniform float Kd;
uniform float Ks;

// shininess coefficients (passed from the application)
uniform float shininess;

////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine vec4 ill_model();

// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform ill_model Illumination_Model_ML_TX;

////////////////////////////////////////////////////////////////////

//////////////////////////////////////////
subroutine(ill_model)
vec4 BlinnPhong_ML_TX() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // we repeat the UVs and we sample the texture
    vec2 repeated_UV = mod(fs_in.interp_UV*repeat, 1.0);
    vec4 surfaceColor = texture(tex, repeated_UV);

    // ambient component can be calculated at the beginning
    vec4 color = vec4(Ka*ambientColor,1.0);

    // normalization of the per-fragment normal
    vec3 N = normalize(vNormal);

    //for all the lights in the scene
    for(int i = 0; i < NR_LIGHTS; i++)
    {
        // normalization of the per-fragment light incidence direction
        vec3 L = normalize(lightDirs[i]);

        // Lambert coefficient
        float lambertian = max(dot(L,N), 0.0);

        // if the lambert coefficient is positive, then I can calculate the specular component
        if(lambertian > 0.0)
        {
            // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
            vec3 V = normalize( fs_in.vViewPosition );

            // in the Blinn-Phong model we do not use the reflection vector, but the half vector
            vec3 H = normalize(L + V);

            // we use H to calculate the specular component
            float specAngle = max(dot(H, N), 0.0);
            // shininess application to the specular component
            float specular = pow(specAngle, shininess);

            // We add diffusive (= color sampled from texture) and specular components to the final color
            // N.B. ): in this implementation, the sum of the components can be different than 1
            color += Kd * lambertian * surfaceColor + vec4(Ks * specular * specularColor,1.0);
        }
    }
    return color;
}
//////////////////////////////////////////

// a subroutine for the Blinn-Phong model with normal mapping for multiple lights and texturing
subroutine(ill_model)
vec4 BlinnPhong_ML_TX_with_normal_map() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // we repeat the UVs and we sample the texture
    vec2 repeated_UV = mod(fs_in.interp_UV*repeat, 1.0);
    vec4 surfaceColor = texture(tex, repeated_UV);

    // ambient component can be calculated at the beginning
    vec4 color = vec4(Ka*ambientColor,1.0);

    // normalization of the per-fragment normal using normal map texture
    vec3 N = normalize(texture((normal0), fs_in.interp_UV).xyz * 2.0f - 1.0f);

    //for all the lights in the scene
    for(int i = 0; i < NR_LIGHTS; i++)
    {
        // normalization of the per-fragment light incidence direction
        vec3 L = normalize(lightDirs[i]);

        // Lambert coefficient
        float lambertian = max(dot(N,L), 0.0);

        // if the lambert coefficient is positive, then I can calculate the specular component
        if(lambertian > 0.0)
        {
            // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
            vec3 V = normalize( fs_in.vViewPosition );

            // in the Blinn-Phong model we do not use the reflection vector, but the half vector
            vec3 H = normalize(L + V);

            // we use H to calculate the specular component
            float specAngle = max(dot(H, normalize(vNormal)), 0.0);
            // shininess application to the specular component
            float specular = pow(specAngle, shininess);

            // We add diffusive (= color sampled from texture) and specular components to the final color
            // N.B. ): in this implementation, the sum of the components can be different than 1
            color += Kd * lambertian * surfaceColor + vec4(Ks * specular * specularColor,1.0);
        }
    }
    return color;
}
//////////////////////////////////////////

// a subroutine for the Blinn-Phong model with normal mapping for multiple lights and texturing
subroutine(ill_model)
vec4 BlinnPhong_ML_TX_with_normal_map_and_tangent() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // we repeat the UVs and we sample the texture
    vec2 repeated_UV = mod(fs_in.interp_UV*repeat, 1.0);
    vec4 surfaceColor = texture(tex, repeated_UV);

    // ambient component can be calculated at the beginning
    vec4 color = vec4(Ka*ambientColor,1.0);

    // normalization of the per-fragment normal
    vec3 N = normalize(texture((normal0), fs_in.interp_UV).rgb * 2.0f - 1.0f);
    //we normalize using TBN matrix
    N = normalize(fs_in.TBN * N);

    //for all the lights in the scene
    for(int i = 0; i < NR_LIGHTS; i++)
    {
        // normalization of the per-fragment light incidence direction
        vec3 L = normalize(lightDirs[i]);

        // Lambert coefficient
        float lambertian = max(dot(N,L), 0.0);

        // if the lambert coefficient is positive, then I can calculate the specular component
        if(lambertian > 0.0)
        {
            // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
            vec3 V = normalize( fs_in.vViewPosition );

            // in the Blinn-Phong model we do not use the reflection vector, but the half vector
            vec3 H = normalize(L + V);

            // we use H to calculate the specular component
            float specAngle = max(dot(H, normalize(vNormal)), 0.0);
            // shininess application to the specular component
            float specular = pow(specAngle, shininess);

            // We add diffusive (= color sampled from texture) and specular components to the final color
            // N.B. ): in this implementation, the sum of the components can be different than 1
            color += Kd * lambertian * surfaceColor + vec4(Ks * specular * specularColor,1.0);
        }
    }
    return color;
}
//////////////////////////////////////////

// main
void main(void)
{
    // we call the pointer function Illumination_Model_ML_TX():
    // the subroutine selected in the main application will be called and executed
    colorFrag = Illumination_Model_ML_TX();
}
