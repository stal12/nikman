#version 330 core
out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D ghostTexture;

void main()
{
    FragColor = texture(ghostTexture, texCoord);
    if (FragColor.a < 0.5)
        discard;
} 