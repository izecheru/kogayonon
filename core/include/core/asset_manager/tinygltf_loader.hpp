#pragma once
#include "tiny_gltf_v3.h"

namespace core
{
class TinyGltfLoader
{
public:
  TinyGltfLoader() = default;
  ~TinyGltfLoader() = default;

  auto loadModel( std::string_view path ) -> void;

private:
  tg3_parse_options m_opts;
  tg3_error_stack m_errors;
  tg3_model m_model;
};
} // namespace core