
#include "core/serialize/mesh_serializer.h"
#include "core/asset_manager/model_loader/mesh.h"
#include "core/logger.h"

namespace kogayonon
{
  bool MeshSerializer::serialize(Mesh& mesh)
  {
    assert(m_out.good());

    // vertices serialization
    const std::vector<Vertex>& vertices = mesh.getVertices();
    size_t vert_size = vertices.size();
    if (vert_size > 0)
    {
      m_out.write(reinterpret_cast<const char*>(&vert_size), sizeof(vert_size));
      m_out.write(reinterpret_cast<const char*>(vertices.data()), vert_size * sizeof(Vertex));
    }
    else
    {
      Logger::logError("Attempting to serialize mesh with 0 vertices");
    }

    // indices serialization
    const std::vector<uint32_t>& indices = mesh.getIndices();
    size_t ind_size = indices.size();
    if (ind_size > 0)
    {
      m_out.write(reinterpret_cast<const char*>(&ind_size), sizeof(ind_size));
      m_out.write(reinterpret_cast<const char*>(indices.data()), ind_size * sizeof(uint32_t));
    }
    else
    {
      Logger::logError("Attempting to serialize mesh with 0 indices");
    }

    // textures serialization
    const std::vector<Texture>& textures = mesh.getTextures();
    size_t tex_size = textures.size();
    m_out.write(reinterpret_cast<const char*>(&tex_size), sizeof(tex_size));

    for (const Texture& tex : textures)
    {
      Logger::logInfo(tex.type);
      m_out.write(reinterpret_cast<const char*>(&tex.id), sizeof(tex.id));
      size_t type_size = tex.type.size() + 1;
      m_out.write(reinterpret_cast<const char*>(&type_size), sizeof(type_size));
      m_out.write(tex.type.c_str(), type_size);

      size_t path_size = tex.path.size() + 1;
      m_out.write(reinterpret_cast<const char*>(&path_size), sizeof(path_size));
      m_out.write(tex.path.c_str(), path_size);

      m_out.write(reinterpret_cast<const char*>(&tex.width), sizeof(tex.width));
      m_out.write(reinterpret_cast<const char*>(&tex.height), sizeof(tex.height));
      m_out.write(reinterpret_cast<const char*>(&tex.num_components), sizeof(tex.num_components));
      m_out.write(reinterpret_cast<const char*>(&tex.gamma), sizeof(tex.gamma));

      size_t data_size = tex.width * tex.height * tex.num_components;

      if (data_size > 0 && !tex.data.empty())
      {
        m_out.write(reinterpret_cast<const char*>(tex.data.data()), data_size);
      }
      else
      {
        size_t zero_data_size = 0;
        m_out.write(reinterpret_cast<const char*>(&zero_data_size), sizeof(zero_data_size));  // Writing 0 means no data
      }
    }

    return m_out.good();
  }

  bool MeshSerializer::deserialize(Mesh& mesh)
  {
    assert(m_in.good());

    // Deserialize vertices
    size_t vertex_count = 0;
    m_in.read(reinterpret_cast<char*>(&vertex_count), sizeof(vertex_count));
    std::vector<Vertex> vertices(vertex_count);
    m_in.read(reinterpret_cast<char*>(vertices.data()), vertex_count * sizeof(Vertex));

    // Deserialize indices
    size_t index_count = 0;
    m_in.read(reinterpret_cast<char*>(&index_count), sizeof(index_count));
    std::vector<uint32_t> indices(index_count);
    m_in.read(reinterpret_cast<char*>(indices.data()), index_count * sizeof(uint32_t));

    // Deserialize textures
    size_t texture_count = 0;
    m_in.read(reinterpret_cast<char*>(&texture_count), sizeof(texture_count));

    std::vector<Texture> textures;
    if (texture_count > 0)
    {
      textures.resize(texture_count);
      for (size_t i = 0; i < texture_count; i++)
      {
        Texture tex;

        m_in.read(reinterpret_cast<char*>(&tex.id), sizeof(tex.id));

        size_t typeLength;
        m_in.read(reinterpret_cast<char*>(&typeLength), sizeof(typeLength));
        tex.type.resize(typeLength);
        m_in.read(tex.type.data(), typeLength);

        size_t path_size = 0;
        m_in.read(reinterpret_cast<char*>(&path_size), sizeof(path_size));
        tex.path.resize(path_size);
        m_in.read(tex.path.data(), path_size);

        m_in.read(reinterpret_cast<char*>(&tex.width), sizeof(tex.width));
        m_in.read(reinterpret_cast<char*>(&tex.height), sizeof(tex.height));
        m_in.read(reinterpret_cast<char*>(&tex.num_components), sizeof(tex.num_components));
        m_in.read(reinterpret_cast<char*>(&tex.gamma), sizeof(tex.gamma));

        // Load texture data
        size_t data_size = tex.width * tex.height * tex.num_components;
        if (data_size > 0)
        {
          tex.data.resize(data_size);
          m_in.read(reinterpret_cast<char*>(tex.data.data()), data_size);
        }
        else
        {
          tex.data.clear();
        }

        textures[i] = tex;
      }
    }

    mesh = Mesh(vertices, indices, textures);
    return m_in.good();
  }

  bool MeshSerializer::serialize(std::vector<Mesh>& meshes)
  {
    for (Mesh& m : meshes)
    {
      if (!serialize(m))
      {
        Logger::logError("Could not serialize mesh");
        return false;
      }
    }
    return m_out.good();
  }

  bool MeshSerializer::deserialize(std::vector<Mesh>& meshes)
  {
    for (Mesh& m : meshes)
    {
      if (!deserialize(m))
      {
        Logger::logError("Could not deserialize mesh");
        return false;
      }
    }
    return m_in.good();
  }
}
