#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec2 fTexCoords;
out vec3 fNormal;
out vec3 fPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() 
{
    vec4 viewPos = view * model * vec4(vPosition, 1.0f);
    fPosition = viewPos.xyz;

    fNormal = mat3(view * model) * vNormal;

    fTexCoords = vTexCoords;
    gl_Position = projection * viewPos;
}