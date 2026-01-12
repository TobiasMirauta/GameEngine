#version 410 core

in vec3 fNormal;
in vec4 fPosEye; // Position in View Space
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

// --- DIRECTIONAL LIGHT (Sun/Moon) ---
uniform vec3 lightDir; // Direction in View Space
uniform vec3 lightColor;

// --- POINT LIGHTS (Street Lamps) ---
struct PointLight {
    vec3 position; // Position in View Space
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 16 
uniform PointLight pointLights[NR_POINT_LIGHTS]; 

// --- MATERIAL & TEXTURES ---
uniform int isObjectEmission; 
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// Global Lighting Settings
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

// 1. CALCULATE SUN/MOON (Directional)
void computeDirectionalLight()
{		
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(fNormal);	
    vec3 lightDirN = normalize(lightDir);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
    // Ambient
    ambient = ambientStrength * lightColor;
	
    // Diffuse
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
    // Specular
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}

// 2. CALCULATE ONE LAMP (Point Light)
vec3 computePointLight(PointLight light)
{
    vec3 normalEye = normalize(fNormal);
    vec3 lightDir = normalize(light.position - fPosEye.xyz);
    vec3 viewDir = normalize(-fPosEye.xyz);

    // Diffuse
    float diff = max(dot(normalEye, lightDir), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normalEye);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    // Attenuation (Fading over distance)
    float distance = length(light.position - fPosEye.xyz);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);    

    // Combine
    vec3 ambient  = (ambientStrength * 0.1) * light.color; // Weak ambient for lamps
    vec3 diffuse  = diff * light.color;
    vec3 specular = spec * light.color;
    
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

float computeShadow()
{
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    if (normalizedCoords.z > 1.0f) return 0.0f;
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    float currentDepth = normalizedCoords.z;
    float bias = 0.0001f; 
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

void main() 
{
    if (isObjectEmission == 1) {
        fColor = texture(diffuseTexture, fTexCoords);
        return; 
    }

    vec3 texDiffuse = texture(diffuseTexture, fTexCoords).rgb;
    vec3 texSpecular = texture(specularTexture, fTexCoords).rgb;

    // A. Sun/Moon Lighting
    computeDirectionalLight();
    float shadow = computeShadow();
    
    vec3 dirLightResult = (ambient + (1.0 - shadow) * diffuse) * texDiffuse + 
                          ((1.0 - shadow) * specular) * texSpecular;

    // B. Point Lights (Lamps) Lighting
    vec3 pointLightsResult = vec3(0.0);
    
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        pointLightsResult += computePointLight(pointLights[i]) * texDiffuse;
    }

    // Combine Everything
    vec3 finalColor = dirLightResult + pointLightsResult;

    // Fog (Optional: Adds atmosphere)
    // float fogFactor = 1.0 - exp(-length(fPosEye) * 0.002);
    // finalColor = mix(finalColor, vec3(0.5, 0.5, 0.5), fogFactor);

    fColor = vec4(min(finalColor, 1.0), 1.0);
}