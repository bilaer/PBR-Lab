#version 330 core

in vec3 WorldPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 TangentWS;
in vec3 BitangentWS;

out vec4 FragColor;

uniform vec3 camPos;

// Env lookup
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdflut;

// If use single value or texture
uniform bool useAlbedoMap;
uniform bool useRoughnessMap;
uniform bool useMetalnessMap;
uniform bool useNormalMap;
uniform bool useAOMap;
uniform bool useRoughnessMetalMap;
uniform bool useEmissiveMap;

// Material fallback values (used when corresponding texture is absent)
uniform vec3 baseColor;
uniform float roughness;
uniform float metalness;
uniform float ao;
uniform vec3 emissive;

// Whether use tangent from vertex
uniform bool  useVertexTangent;
uniform float normalScale; // = glTF normalTexture.scale）

// Material Texture
uniform sampler2D albedoMap;
uniform sampler2D roughnessMap;
uniform sampler2D metalnessMap;
uniform sampler2D normalMap;
uniform sampler2D aoMap;
uniform sampler2D roughnessMetalMap;
uniform sampler2D emissiveMap;

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 7.0;

// Christian Schüler - "Followup: Normal Mapping Without Precomputed Tangents", 2013
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // Get the edge vectors of the triangle
    vec3 dp1 = dFdx(p);  // Compute the derivative of the position along the x-axis
    vec3 dp2 = dFdy(p);  // Compute the derivative of the position along the y-axis
    vec2 duv1 = dFdx(uv);  // Compute the derivative of the texture coordinates along the x-axis
    vec2 duv2 = dFdy(uv);  // Compute the derivative of the texture coordinates along the y-axis
 
    // Solve the linear system using cross products
    vec3 dp2perp = cross(dp2, N);  // Get a vector perpendicular to dp2 and the normal
    vec3 dp1perp = cross(N, dp1);  // Get a vector perpendicular to dp1 and the normal
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;  // Calculate the tangent
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;  // Calculate the bitangent
 
    // Construct a scale-invariant frame (so it doesn't depend on the model's scale)
    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));  // Normalize the tangent and bitangent
    return mat3(T * invmax, B * invmax, N);  // Return the final TBN matrix
}

// Use the TBN matrix to retrieve the normal
vec3 getNormalFromMap()
{
    vec3 N_ws = normalize(Normal);

    vec3 n_ts = texture(normalMap, TexCoords).xyz * 2.0 - 1.0; // Sample tangent-space normal and remap from [0,1] to [-1,1]
    n_ts.xy *= normalScale; 

    if (useVertexTangent) {
        // Use TBN built from per-vertex tangent/bitangent
        vec3 T = normalize(TangentWS);
        vec3 B = normalize(BitangentWS);
        mat3 TBN = mat3(T, B, N_ws);
        return normalize(TBN * n_ts);
    } else {
        // Use TBN from screen-space derivatives (robust for procedurally generated geometry)
        mat3 TBN = cotangent_frame(N_ws, WorldPos, TexCoords);
        return normalize(TBN * n_ts);
    }
}

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
    // Albedo
    vec3 baseColorFinal = useAlbedoMap ? pow(texture(albedoMap, TexCoords).rgb, vec3(2.2)) : baseColor;

    // Roughness and metalic
    float roughnessFinal;
    float metalnessFinal;
    if(useRoughnessMetalMap) {
        roughnessFinal = texture(roughnessMetalMap, TexCoords).g;
        metalnessFinal = texture(roughnessMetalMap, TexCoords).b;
    } else {
        roughnessFinal = useRoughnessMap ? texture(roughnessMap, TexCoords).r : roughness;
        metalnessFinal = useMetalnessMap ? texture(metalnessMap, TexCoords).r : metalness;
    }
    
    // AO
    float aoFinal = useAOMap ? texture(aoMap, TexCoords).r : ao;

    // Emissive
    vec3 emissiveFinal = useEmissiveMap ? texture(emissiveMap, TexCoords).rgb : emissive;

    // Transform normal from tangent space to world space
    vec3 N = useNormalMap ? getNormalFromMap() : normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = normalize(reflect(-V, N));
    float NdotV = max(dot(N, V), 0.0);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow) 
    vec3 F0 = mix(vec3(0.04), baseColorFinal, metalnessFinal);
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughnessFinal);

    // Energy conservation
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metalnessFinal;

    // IBL - diffuse  
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * baseColorFinal;

    // IBL - Specular
    vec3 prefilter = textureLod(prefilterMap, R, roughnessFinal * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdflut, vec2(NdotV, roughnessFinal)).rg;
    vec3 specular =  prefilter * (FresnelSchlick(NdotV, F0) * brdf.x + brdf.y);

    // Combine diffuse and specular values
    vec3 ambient = (kD * diffuse + specular) * aoFinal + emissiveFinal;

    // HDR tonemapping
    vec3 color = ambient / (ambient + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}