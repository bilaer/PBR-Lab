#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 camPos;

// Environment maps
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdflut;

// Material maps
uniform sampler2D albedoMap;
uniform sampler2D roughnessMap;
uniform sampler2D metalnessMap;
uniform sampler2D normalMap;

// Material fallback values
uniform vec3 baseColor;
uniform float roughness;
uniform float metalness;
uniform float ao;

uniform bool useAlbedoMap;
uniform bool useRoughnessMap;
uniform bool useMetalnessMap;
uniform bool useNormalMap;
uniform bool useAOMap;

const float PI = 3.14159265359;

// Fresnel-Schlick approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 N = normalize(Normal); // TODO: use TBN if using normal map
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);
    float NdotV = max(dot(N, V), 0.0);

    // Material lookup
    vec3 albedoFinal = useAlbedoMap ? texture(albedoMap, TexCoords).rgb : baseColor;
    float roughnessFinal = useRoughnessMap ? texture(roughnessMap, TexCoords).r : roughness;
    float metalnessFinal = useMetalnessMap ? texture(metalnessMap, TexCoords).r : metalness;

    vec3 F0 = mix(vec3(0.04), albedoFinal, metalnessFinal);

    // IBL - diffuse
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedoFinal;

    // IBL - specular
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughnessFinal * 5.0).rgb;
    vec2 brdf = texture(brdflut, vec2(NdotV, roughnessFinal)).rg;
    vec3 specular = prefilteredColor * (FresnelSchlick(NdotV, F0) * brdf.x + brdf.y);

    vec3 ambient = (diffuse + specular) * ao;

    FragColor = vec4(ambient, 1.0);
}
