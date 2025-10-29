#pragma once

namespace kogayonon_rendering
{
class Uniformbuffer
{
public:
  Uniformbuffer() = default;
  virtual ~Uniformbuffer() = default;

  virtual void bind() = 0;
  virtual void unbind() = 0;

  virtual void initialize( uint32_t bindingIndex ) = 0;
  virtual void destroy() = 0;
};
} // namespace kogayonon_rendering