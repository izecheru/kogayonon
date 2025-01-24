#pragma once
#include <core/logger.h>
#include <core/renderer/mesh.h>
#include <map>
#include <optional>
#include <shader/shader.h>
#include <core/renderer/camera.h>

class Renderer
{
public:
  Renderer() = default;
  ~Renderer() = default;

  void render(const char* mesh_name);

  void pushShader(const char* vertex_shader, const char* fragment_shader, const char* shader_name);
  void pushMesh(const char* mesh_name, const Mesh& mesh);

  Shader getShader(const char* shader_name);
  GLint getShaderId(const char* shader_name);

  void bindShader(const char* shader_name);
  void unbindShader(const char* shader_name);

private:
  std::map<const char*, Shader> m_shaders_array;
  std::map<const char*, Mesh> m_mesh_array;
};

