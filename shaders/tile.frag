#version 330 core
out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D tileTexture;

void main()
{
    FragColor = texture(tileTexture, texCoord);
    if (FragColor.a < 0.5)
        discard;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/2.2));
} 