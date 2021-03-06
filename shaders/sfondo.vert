#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

uniform mat4 world;

out vec2 texCoord;

void main()
{
    gl_Position = world * vec4(aPos.x, aPos.y, -0.1, 1.0);
    texCoord = aTex;
}