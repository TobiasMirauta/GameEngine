#version 410 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D flareTexture;
uniform float brightness; // Controlled by moon distance

void main() {
    vec4 texColor = texture(flareTexture, TexCoords);
    
    FragColor = vec4(texColor.rgb, texColor.a * brightness);
}