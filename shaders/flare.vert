#version 410 core
layout (location = 0) in vec2 aPos; // 2D Screen Coordinates
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform vec2 position; // Center of the specific flare
uniform float scale;   // Size of the flare

void main() {
    TexCoords = aTexCoords;
    

    vec2 finalPos = aPos * scale + position;
    gl_Position = vec4(finalPos, 0.0, 1.0);
}