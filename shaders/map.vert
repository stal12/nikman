#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

out vec2 texCoord;

uniform mat4 projection;
uniform mat4 world;
uniform float h;
uniform float w;

void main()
{
    gl_Position = projection * world * vec4(aPos.x, aPos.y, -8.0, 1.0);
    texCoord = (aTex - vec2(0.5, 0.5)) * vec2(w, h) + vec2(0.5, 0.5);
}