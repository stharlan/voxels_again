#version 330

layout (location = 0) in vec3 vert;
layout (location = 1) in vec4 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texc;
layout (location = 4) in vec3 translation;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec4 vertexColor;
out vec2 texCoord;
out vec3 vertexNormal;

void main()
{
    gl_Position = projection * view * model * vec4(vert + translation,1.0f);
    vertexColor = color;
    texCoord = texc;
    vertexNormal = normal;
}
