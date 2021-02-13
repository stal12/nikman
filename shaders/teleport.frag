#version 330 core
out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D teleportTexture;

void main()
{
    FragColor = texture(teleportTexture, texCoord);
    if (FragColor.a < 0.01)
        discard;
} 