#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

out vec2 texCoord;

uniform mat4 projection;
uniform mat4 world;

void main()
{
    gl_Position = projection * world * vec4(aPos.x, aPos.y, -6.0, 1.0);
    texCoord = aTex;
}