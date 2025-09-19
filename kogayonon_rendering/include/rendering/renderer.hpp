#pragma once
#include <memory>
#include <vector>
#define MAX_QUADS 10000
#define MAX_VERT_COUNT 4 * MAX_QUADS
#define MAX_IND_COUNT MAX_QUADS * 6

namespace kogayonon_resources
{
class Model;
struct Vertex;
} // namespace kogayonon_resources

namespace kogayonon_rendering
{
class Renderer
{
public:
  Renderer();
  ~Renderer();

  void drawLine();

  void begin();
  void end();

  void initialise();
  bool getPoly();
  void togglePoly();

private:
  unsigned int m_vao, m_vbo, m_ebo;
  bool m_bPolyMode = false;
};
} // namespace kogayonon_rendering