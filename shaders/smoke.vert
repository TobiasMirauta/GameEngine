#version 410 core

layout (location = 0) in vec4 aPosLife; 
uniform mat4 view;
uniform mat4 projection;

out float particleLife;

void main() {
    
    particleLife = aPosLife.w; 
    
    vec4 viewPos = view * vec4(aPosLife.xyz, 1.0);
    gl_Position = projection * viewPos;
    

    float dist = length(viewPos.xyz);
    gl_PointSize = (1200.0 * (1.5 - particleLife)) / dist; 
}