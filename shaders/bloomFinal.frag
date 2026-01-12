#version 410 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;

void main() {             
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    
    // Additive blending
    vec3 result = hdrColor + bloomColor; 
  
    // Tone mapping
    vec3 mapped = vec3(1.0) - exp(-result * exposure);
    
    // --- TRY DISABLING THIS LINE IF IT LOOKS TOO WHITE ---
    // mapped = pow(mapped, vec3(1.0 / 2.2)); 
  
    FragColor = vec4(mapped, 1.0);
}