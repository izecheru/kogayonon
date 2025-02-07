#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

void main()
{
    vec4 diffuseColor = texture(texture_diffuse1, TexCoords);
    vec3 normalColor = texture(texture_normal1, TexCoords).rgb;

    // Remap normal from [0,1] to [-1,1]
    normalColor = normalColor * 2.0 - 1.0;

    // Enhance visibility: Mix diffuse with normal map influence
    vec3 colorOutput = diffuseColor.rgb * 0.8 + normalColor * 0.2; 

    FragColor = vec4(colorOutput, diffuseColor.a);
}
