#version 410 core

in vec3 textureCoordinates;
out vec4 fColor;

uniform samplerCube skybox;

void main()
{
    fColor = texture(skybox, textureCoordinates);
}