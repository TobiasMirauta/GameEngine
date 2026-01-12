#version 410 core

out vec4 fColor;
in float particleLife;

void main() {

    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);

    if(dist > 0.5) discard;

    float alpha = 1.0 - (dist * 2.0);
    
    alpha *= particleLife;
    
    vec3 smokeColor = vec3(0.2, 0.2, 0.2);

    fColor = vec4(smokeColor, alpha * 0.5); 
}