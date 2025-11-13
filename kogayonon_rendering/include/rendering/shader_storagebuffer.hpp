#pragma once
#include <cinttypes>

namespace kogayonon_rendering
{
class IShaderStorageBuffer
{
public:
  IShaderStorageBuffer() = default;
  virtual ~IShaderStorageBuffer() = default;

  virtual void bind() = 0;
  virtual void bind( uint32_t index ) = 0;
  virtual void unbind() = 0;

  virtual void initialize() = 0;
  virtual void destroy() = 0;
  virtual void destroy( uint32_t index ) = 0;

  virtual void update() = 0;
  virtual void update( uint32_t index ) = 0;

private:
};
} // namespace kogayonon_rendering