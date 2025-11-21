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

// ubo for the light count
layout(std140, binding = 3) uniform LightCounts {
    int u_NumPointLights;
    int u_NumDirectionalLights;
    int u_NumSpotLights;
    int pad;
};

in vec2 TexCoord;
in vec4 ShadowCoord;
in vec3 Normal;
in vec3 FragPos;

layout(binding = 3) uniform sampler2D u_Texture;
layout(binding = 4) uniform sampler2D u_ShadowMap;

out vec4 FragColor;

float gAmbient = 0.8;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 shadowCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	shadowCoords = shadowCoords * 0.5 + 0.5;
	float closestDepth = texture(u_ShadowMap, shadowCoords.xy).r; 
	float currentDepth = shadowCoords.z;
	float bias = 0.0005;
    int sampleRadius = 1;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    float shadow = 0.0f;

    // create a box around our pixel and filter the shadow
    // meaning we just make kind of a gradient around it for a better
    // transition from shadow to light
	for(int y = -sampleRadius; y <= sampleRadius; y++)
	{
	  for(int x = -sampleRadius; x <= sampleRadius; x++)
	  {
          vec2 offset = vec2(x, y) * texelSize;
		  float pcfDepth = texture(u_ShadowMap, shadowCoords.xy + offset ).x; 
          if(currentDepth > pcfDepth + bias)
            shadow += 1.0f;
	  }    
	}

    float shadowFactor = shadow/float(pow(sampleRadius,2));
    return shadowFactor;
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    float shadow = 0.0f;
    vec3 lightDir = normalize(vec3(light.translation) - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    float distance_ = length(vec3(light.translation) - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance_ + light.params.z * (distance_ * distance_));

    if(u_NumDirectionalLights>=1)
    {
      shadow = ShadowCalculation(ShadowCoord);
    }

    vec3 ambient  = gAmbient * vec3(light.color);
    vec3 diffuse  = diff * vec3(light.color) * (1.0 - shadow);
    vec3 specular = spec * vec3(light.color) * (1.0 - shadow);
    return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 objectColor)
{
    vec3 lightDir = normalize(-light.direction.xyz);

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    // lighting components
    float shadow = ShadowCalculation(ShadowCoord);
    vec3 ambient  = gAmbient * light.ambient.xyz * objectColor;
    vec3 diffuse  = light.diffuse.xyz * diff * objectColor;
    vec3 specular = light.specular.xyz * spec; // no specular map, you could scale this if needed

    diffuse *= 1.0-shadow;
    specular *= 1.0-shadow;
    return ambient + diffuse + specular;
}


void main()
{
  vec3 norm = normalize(Normal);
  vec3 result = vec3(0.0);
  vec3 objectColor = texture(u_Texture, TexCoord).rgb;
  for (int i = 0; i < u_NumPointLights; ++i)
  {
    // if it is not visible just skip this light
    if(pointLights[i].params.w==0.0f) continue;

    vec3 viewDir = normalize(pointLights[i].translation.xyz - FragPos);
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
  }

  for (int i = 0; i < u_NumDirectionalLights; ++i)
  {
      vec3 viewDir = normalize(-FragPos);
      result += CalcDirLight(directionalLights[i], norm, viewDir, objectColor);
  }

	vec3 litColor = result * objectColor;
	FragColor = vec4(litColor, 1.0);
}
