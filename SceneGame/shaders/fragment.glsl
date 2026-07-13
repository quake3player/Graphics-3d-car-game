#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;
uniform vec3 objectColor;
uniform bool useTexture;
uniform sampler2D texture1;
uniform sampler2D shadowMap;
uniform float metallic;
uniform float ambientStrength;
uniform bool enableShadows;

uniform sampler2DArray spotShadowMaps;
uniform mat4 spotLightSpaceMatrices[8];

// Directional sun light
uniform vec3 sunDirection;
uniform vec3 sunColor;

struct Spotlight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};

#define MAX_SPOTLIGHTS 8
uniform int numSpotlights;
uniform Spotlight spotlights[MAX_SPOTLIGHTS];

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

#define MAX_POINT_LIGHTS 16
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

// Simple shadow calculation with small bias
float calcShadow(vec4 fragPosLS) {
    // Perspective divide (for orthographic this is trivial)
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords = projCoords * 0.5 + 0.5; // to [0,1]
    
    // Out of shadow map range = no shadow
    if (projCoords.z > 1.0) return 0.0;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) return 0.0;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.005;
    
    // PCF: sample 3x3 area for soft edges
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}

float calcSpotShadow(vec4 fragPosLS, int index) {
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords = projCoords * 0.5 + 0.5; // to [0,1]
    
    if (projCoords.z > 1.0) return 0.0;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) return 0.0;
    
    float closestDepth = texture(spotShadowMaps, vec3(projCoords.xy, float(index))).r;
    float currentDepth = projCoords.z;
    // Perspective depth is highly non-linear; near the far plane, 0.005 NDC is > 4.0 meters of world space!
    // We must use a dramatically smaller bias for spot FBOs to prevent swallowing geometric shadows.
    float bias = 0.0001;
    
    // PCF: sample 3x3 area for soft edges
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(spotShadowMaps, 0).xy;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(spotShadowMaps, vec3(projCoords.xy + vec2(x, y) * texelSize, float(index))).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}

void main()
{
    vec3 baseColor = objectColor;
    if(useTexture) {
        baseColor = texture(texture1, TexCoord).rgb;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Ambient (controlled by daylight)
    vec3 ambient = ambientStrength * baseColor;
    vec3 result = ambient;

    // Shadow factor (0 = fully lit, 1 = fully shadowed)
    float shadow = enableShadows ? calcShadow(FragPosLightSpace) : 0.0;

    // --- Sun contribution (affected by shadow) ---
    {
        vec3 lightDir = normalize(-sunDirection);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * sunColor * baseColor * 0.6;

        vec3 reflectDir = reflect(-lightDir, norm);
        float specPhase = metallic > 0.5 ? 64.0 : 16.0;
        float specStrength = metallic > 0.5 ? 0.6 : 0.1;
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), specPhase);
        vec3 specular = specStrength * spec * sunColor;

        result += (1.0 - shadow) * (diffuse + specular);
    }
    
    // --- Spot lights (not shadowed for simplicity) ---
    for(int i = 0; i < numSpotlights; i++) {
        vec3 lightDir = normalize(spotlights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * spotlights[i].color * baseColor;
        
        vec3 reflectDir = reflect(-lightDir, norm);
        float specPhase = metallic > 0.5 ? 64.0 : 8.0; 
        float specStrength = metallic > 0.5 ? 0.8 : 0.0;
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), specPhase);
        vec3 specular = specStrength * spec * spotlights[i].color;
        
        float theta = dot(lightDir, normalize(-spotlights[i].direction));
        float epsilon = spotlights[i].cutOff - spotlights[i].outerCutOff;
        float intensity = clamp((theta - spotlights[i].outerCutOff) / epsilon, 0.0, 1.0);
        
        float distance = length(spotlights[i].position - FragPos);
        float attenuation = 1.0 / (spotlights[i].constant + spotlights[i].linear * distance + spotlights[i].quadratic * (distance * distance));
        
        diffuse *= intensity * attenuation;
        specular *= intensity * attenuation;
        
        float spotShadow = 0.0;
        if (enableShadows) {
            vec4 fragPosLS = spotLightSpaceMatrices[i] * vec4(FragPos, 1.0);
            spotShadow = calcSpotShadow(fragPosLS, i);
        }
        
        result += (1.0 - spotShadow) * (diffuse + specular);
    }
    
    // --- Point lights ---
    for(int i = 0; i < numPointLights; i++) {
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * pointLights[i].color * baseColor;
        
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
        vec3 specular = 0.15 * spec * pointLights[i].color;
        
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + pointLights[i].quadratic * (distance * distance));
        
        diffuse *= attenuation;
        specular *= attenuation;
        result += diffuse + specular;
    }
    
    FragColor = vec4(result, 1.0);
}
