#version 460 core

struct PointLight {
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 params;
    //float constant;
    //float linear;
    //float quadratic;
    //bool enabled;
};

layout(std140, binding = 0) uniform LightCounts {
    int u_NumPointLights;
    int u_NumDirectionalLights;
    int pad[2];
};

// SSBOs for light arrays
layout(std430, binding = 1) buffer PointLights {
    PointLight pointLights[];
};

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

layout(binding = 1) uniform sampler2D u_Texture;

layout(location = 0) out vec4 FragColor;


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(vec3(light.position) - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    float distance_ = length(vec3(light.position) - fragPos);
    float attenuation = 1.0 / (light.params.x+ light.params.y* distance_ + light.params.z* (distance_ * distance_));

    vec3 ambient  = vec3(light.ambient);
    vec3 diffuse  = diff * vec3(light.diffuse);
    vec3 specular = spec * vec3(light.specular);

    return (ambient + diffuse + specular) * attenuation;
}

void main()
{
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(vec3(0.0,0.0,-3.0) - FragPos);
  vec3 result = vec3(0.0);
  for (int i = 0; i < u_NumPointLights; ++i)
  {
    // if it is not visible just skip this light
    if(pointLights[i].params.w==0.0f) continue;

    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
  }
  vec3 objectColor = texture(u_Texture, TexCoord).rgb;
  vec3 litColor = result * objectColor;
  if(u_NumPointLights>=1)
  {
    FragColor = vec4(litColor, 1.0);
  }
  else
  {
    FragColor = vec4(objectColor, 1.0);
  }
}
