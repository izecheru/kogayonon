#pragma once
#include <fstream>
#include "core/singleton/singleton.h"
#include "core/asset_manager/model_loader/mesh.h"
#include "serializer.h"

namespace kogayonon
{
  class MeshSerializer :public Singleton<MeshSerializer>, public Serializer<Mesh>
  {
  public:
    bool serialize(Mesh& mesh) override;
    bool deserialize(Mesh& mesh) override;
    bool serialize(std::vector<Mesh>& mesh) override;
    bool deserialize(std::vector<Mesh>& mesh) override;
  };
}
