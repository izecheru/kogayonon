#pragma once

namespace kogayonon_rendering
{
class ShaderStoragebuffer
{
public:
  ShaderStoragebuffer() = default;
  virtual ~ShaderStoragebuffer() = default;

  virtual void bind() = 0;
  virtual void unbind() = 0;

  virtual void initialize( uint32_t bindingIndex ) = 0;
  virtual void destroy() = 0;
  virtual void update() = 0;

private:
};
} // namespace kogayonon_rendering