#version 330 core
out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D mapTexture;

void main()
{
    FragColor = texture(mapTexture, texCoord);
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/2.2));
} 