#pragma once
#include <fstream>
#include <mutex>

#include "core/singleton/singleton.h"
#include "core/asset_manager/loader/mesh.h"
#include "core/asset_manager/loader/model.h"
#include "serializer.h"
namespace kogayonon
{
  class MeshSerializer :public Singleton<MeshSerializer>, public Serializer<Mesh>
  {
  public:
    bool serialize(Mesh& mesh, std::ofstream& out) override;
    bool serialize(std::vector<Mesh>& mesh, std::ofstream& out) final;

    bool deserialize(Mesh& mesh, std::ifstream& in) override;
    bool deserialize(std::vector<Mesh>& mesh, std::ifstream& in) final;

    void serializeMeshes(const std::string& bin_path, Model& model, std::ofstream& out);
    void deserializeMeshes(const std::string& bin_path, Model& model, std::ifstream& in);

    inline std::mutex& getMutex()
    {
      return m_file_mutex;
    }

  private:
    std::mutex m_file_mutex;
  };
}
