#pragma once

namespace resources
{
/**
 * @brief This basically holds an index into the bindless texture buffer, buffer[normalTextureIndex] is the normal
 * texture for that specific submesh/ mesh
 */
struct Material
{
  // TODO(kogayonon) extend this to support more type of textures, but i dont really know if i'll support PBR
  int normalTextureIndex{ -1 };
  int specularTextureIndex{ -1 };
  int diffuseTextureIndex{ -1 };
};
} // namespace resources