#version 330 core
out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D playerTexture;

void main()
{
    FragColor = texture(playerTexture, texCoord);
    if (FragColor.a < 0.01)
        discard;
} 