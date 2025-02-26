#pragma once
#include <fstream>
#include "core/singleton/singleton.h"
#include "core/model_loader/mesh.h"

namespace kogayonon
{
  class MeshSerializer :public Singleton<MeshSerializer>
  {
  public:
    bool serialize(std::ofstream& out, Mesh& mesh);
    bool deserialize(std::ifstream& in, Mesh& mesh);
  };
}
