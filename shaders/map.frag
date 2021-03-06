#version 330 core
out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D mapTexture;

uniform float x_s;
uniform float x_e;
uniform float y_s;
uniform float y_e;

void main()
{
    float x = x_s + (x_e - x_s) * mod(texCoord.x, 1.f);
    float y = y_s + (y_e - y_s) * mod(texCoord.y, 1.f);
    FragColor = texture(mapTexture, vec2(x, y));
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/2.2));
} 