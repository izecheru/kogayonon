#version 460 core

struct PointLight {
    vec4 translation;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 color;
    vec4 params;// x = constant, y = linear, z = quadratic, w = enabled
};

struct DirectionalLight
{
  vec4 direction;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};

struct SpotLight
{
  vec4 translation;
  vec4 direction;
  float cutOff;
  float pad[3];
};

layout(std140, binding = 3) uniform LightCounts {
    int u_NumPointLights;
    int u_NumDirectionalLights;
    int u_NumSpotLights;
    int pad;
};

// SSBOs for light arrays
layout(std430, binding = 0) buffer PointLights {
    PointLight pointLights[];
};

layout(std430, binding = 1) buffer DirectionalLights{
    DirectionalLight directionalLights[];
};

layout(std430, binding = 2) buffer SpotLights{
    SpotLight spotLights[];
};

in vec2 TexCoord;
in vec4 ShadowCoord;
in vec3 Normal;
in vec3 FragPos;

layout(binding = 1) uniform sampler2D u_Texture;
layout(binding = 2) uniform sampler2D u_DepthMap;

out vec4 FragColor;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    float shadow = 0.0f;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(projCoords.z<=1.0f)
    {
	  projCoords = projCoords * 0.5 + 0.5;
	  float closestDepth = texture(u_DepthMap, projCoords.xy).r; 
	  float currentDepth = projCoords.z;
	  float bias = 0.005;
	  shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	}
    return shadow;
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(vec3(light.translation) - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    float distance_ = length(vec3(light.translation) - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance_ + light.params.z * (distance_ * distance_));

    vec3 ambient  = vec3(0.1) * vec3(light.color);
    vec3 diffuse  = diff * vec3(light.color);
    vec3 specular = spec * vec3(light.color);
    float shadow = ShadowCalculation(ShadowCoord);
    return (ambient + (1.0 - shadow)*(diffuse + specular)) * attenuation;
}

void main()
{
  vec3 norm = normalize(Normal);
  vec3 result = vec3(0.0);
  for (int i = 0; i < u_NumPointLights; ++i)
  {
    // if it is not visible just skip this light
    if(pointLights[i].params.w==0.0f) continue;

    vec3 viewDir = normalize(pointLights[i].translation.xyz - FragPos);
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
  }

  vec3 objectColor = texture(u_Texture, TexCoord).rgb;
  vec3 litColor = result * objectColor;
  FragColor = vec4(litColor, 1.0);
}
