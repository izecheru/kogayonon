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
in vec3 ViewPos;
flat in uint Selected;

layout(binding = 3) uniform sampler2D u_Texture;
layout(binding = 4) uniform sampler2D u_ShadowMap;

out vec4 FragColor;

float gAmbient = 0.3;

float FogAlphaCalculation(float distance_, float near, float far)
{
  float fogMax = 1.0 * far;
  float fogMin = 0.7 * far;

  if(distance_ >= fogMax) return 1.0;
  if(distance_ <= fogMin) return 0.0;

  return 1.0 - (fogMax - distance_) / (fogMax - fogMin);
}

float FogCalculation(float distance_, float near, float far)
{
  float fogMax = 1.0 * far;
  float fogMin = 0.5 * far;

  if(distance_ >= fogMax) return 1.0;
  if(distance_ <= fogMin) return 0.0;

  return 1.0 - (fogMax - distance_) / (fogMax - fogMin);
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
  vec3 shadowCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	shadowCoords = shadowCoords * 0.5 + 0.5;
	float closestDepth = texture(u_ShadowMap, shadowCoords.xy).r; 
	float currentDepth = shadowCoords.z;
  int sampleRadius = 1;
  float bias = 0.00005;
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
        shadow += 0.8f;
	  }    
	}

  float shadowFactor = shadow / 16.0;
  return clamp(shadowFactor, 0.0, 1.0);
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(vec3(light.translation) - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    float distance_ = length(vec3(light.translation) - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance_ + light.params.z * (distance_ * distance_));

    vec3 ambient  = gAmbient * vec3(light.color);
    vec3 diffuse  = diff * vec3(light.color) * shadow;
    vec3 specular = spec * vec3(light.color) * shadow;
    return (ambient + diffuse + specular) * attenuation;
}


vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(-light.direction.xyz);

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 50.0);

    // lighting components
    vec3 ambient  = gAmbient * vec3(1.0) ;
    vec3 diffuse  = light.diffuse.xyz * diff ;
    vec3 specular = light.specular.xyz * spec; // no specular map, you could scale this if needed

    diffuse *= shadow;
    specular *= shadow;
    return ambient + diffuse + specular;
}

void main()
{
  // discard if the entity is selected, it goes through a different rendering process
  // if(Selected==1){discard;}
  vec3 result = vec3(0.0);
  vec3 objectColor = texture(u_Texture, TexCoord).rgb;
  float shadow = 1 - ShadowCalculation(ShadowCoord);
  for (int i = 0; i < u_NumPointLights; ++i)
  {
    // if it is not visible just skip this light
    if(pointLights[i].params.w==0.0f) continue;

    vec3 viewDir = normalize(pointLights[i].translation.xyz - FragPos);
    result += CalcPointLight(pointLights[i], Normal, FragPos, viewDir, shadow);
  }

  for (int i = 0; i < u_NumDirectionalLights; ++i)
  {
      vec3 viewDir = normalize(-directionalLights[i].direction.xyz);
      result += CalcDirLight(directionalLights[i], Normal, viewDir, shadow);
  }
  // calculate fog here
  float d = distance(ViewPos,FragPos);
  float fog = FogCalculation(d, 0.1,100.0);

	vec3 litColor = result * objectColor;
  vec4 baseColor = vec4(litColor, 1.0);

  vec3 fogColor = vec3(0.3);  // light sky color
  vec3 finalRGB = mix(baseColor.rgb, fogColor, fog);
  float fadeAlpha = smoothstep(100.0, 70.0, d);
  FragColor = vec4(finalRGB, fadeAlpha);
}
