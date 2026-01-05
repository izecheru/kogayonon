#pragma once

namespace kogayonon_core
{
struct DirectionalLightComponent
{
  int directionalLightIndex{ 0 };
  float nearPlane{ 0.1f };
  float farPlane{ 300.0f };
  float orthoSize{ 70.0f };
  float positionFactor{ 20.0f };
};
} // namespace kogayonon_core