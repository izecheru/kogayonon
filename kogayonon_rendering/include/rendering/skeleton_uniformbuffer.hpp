#pragma once
#include <cinttypes>
#include "rendering/uniformbuffer.hpp"

namespace kogayonon_rendering
{
class SkeletonsUniformbuffer : public Uniformbuffer
{
public:
  SkeletonsUniformbuffer() = default;
  ~SkeletonsUniformbuffer() = default;

  void initialize( uint32_t bindingIndex ) override;
  void destroy() override;
  void update();
  void bind() override;
  void unbind() override;

  void incrementSkeletonCount();
  auto getSkeletonCount() const -> uint32_t;

private:
  uint32_t m_ubo;
  uint32_t m_bindingIndex;

  uint32_t m_count;
};
} // namespace kogayonon_rendering