#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.01, 0.01, 0.01, 0.7);
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/2.2));
} 