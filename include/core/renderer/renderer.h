#pragma once
#include "core/renderer/mesh.h"
#include <map>
#include "shader/shader.h"

class Renderer
{
public:
  Renderer();
  ~Renderer() = default;

  void render(const char* mesh_name);

  void pushShader(const char* vertex_shader, const char* fragment_shader, const char* shader_name);
  void pushMesh(const char* mesh_name, const Mesh& mesh);

  Shader getShader(const char* shader_name);
  GLint getShaderId(const char* shader_name);

  void bindShader(const char* shader_name);
  void unbindShader(const char* shader_name);

  bool getPolyMode();
  void togglePolyMode();

private:
  bool is_poly;
  std::map<const char*, Shader> shaders;
  std::map<const char*, Mesh> meshes;
};

