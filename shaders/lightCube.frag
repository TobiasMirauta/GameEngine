#version 410 core

in vec2 fTexCoords;
in vec3 fNormal;
in vec3 fPosition;

out vec4 fColor; // <--- ONLY ONE OUTPUT

uniform sampler2D diffuseTexture;
uniform float alphaMultiplier;
uniform float time;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main() 
{
    vec4 texColor = texture(diffuseTexture, fTexCoords);
    vec3 baseColor = texColor.rgb;

    // Twinkle Animation
    float speed = 3.0;
    float offset = random(floor(fPosition.xy * 0.1)); 
    float twinkle = 0.5 + 0.5 * sin(time * speed + offset * 10.0);
    twinkle = clamp(twinkle, 0.5, 1.0); 

    vec3 finalColor;

    // A. FILAMENT
    if (alphaMultiplier > 15.0) {
        finalColor = vec3(1.0, 1.0, 1.0) * alphaMultiplier * twinkle;
    } 
    // B. GLOW
    else {
        vec3 viewDir = normalize(-fPosition);
        vec3 normal = normalize(fNormal);
        float facing = max(dot(viewDir, normal), 0.0);
        vec3 coreColor = mix(baseColor, vec3(1.0), pow(facing, 4.0)); 
        float glowShape = pow(facing, 0.5); 
        finalColor = coreColor * glowShape * alphaMultiplier * twinkle;
    }

    fColor = vec4(finalColor, 1.0);
}