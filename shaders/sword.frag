#version 330 core
out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D swordTexture;

void main()
{
    FragColor = texture(swordTexture, texCoord);
    if (FragColor.a < 0.5)
        discard;
} 