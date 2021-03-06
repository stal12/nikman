#version 330 core
out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D ghostTexture;

void main()
{
    FragColor = texture(ghostTexture, texCoord);
    if (FragColor.a < 0.01)
        discard;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/2.2));
} 