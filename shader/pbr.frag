#version 330 core

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 camPos;

// Env lookup
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdflut;

// Material
uniform vec3 baseColor;
uniform float roughness;
uniform float metalness;
uniform float ao;

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;

// Fresnel-Schlick approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

void main()
{
    vec3 N = normalize(Normal);  // Use geometry normal
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = normalize(reflect(-V, N));
    float NdotV = max(dot(N, V), 0.0);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow) 
    vec3 F0 = mix(vec3(0.04), baseColor, metalness);
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metalness;

    // IBL - diffuse  
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * baseColor;

    // IBL - Specular
    vec3 prefilter = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdflut, vec2(NdotV, roughness)).rg;
    vec3 specular =  prefilter * (FresnelSchlick(NdotV, F0) * brdf.x + brdf.y);

    // Combine    
    vec3 ambient = (kD * diffuse + specular) * ao;

    // HDR tonemapping
    vec3 color = ambient / (ambient + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}


