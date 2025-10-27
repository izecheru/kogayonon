#version 460 core

// Inputs
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

// Textures
layout(binding = 1) uniform sampler2D u_Texture;

// Outputs
layout(location = 0) out vec4 FragColor;

void main()
{
    float ambient = 0.10f;
    float specularStrength = 0.5f;
    vec3 norm = normalize(Normal);
    vec3 lightPos = vec3(1.0,1.0,1.0);
    vec3 lightDir = normalize(lightPos-FragPos);
    vec3 lightColor = vec3(1.0,0.0,0.4);
    float diff = max(dot(norm,lightDir),0.0);
    vec3 diffuse = diff * lightColor;
    vec3 objectColor = texture(u_Texture, TexCoord).rgb;
    vec3 viewDir = normalize(vec3(0.0f,0.0f,-1.0f) - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}