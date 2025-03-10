#pragma once

#include <fstream>
#include <vector>
#include <filesystem>

namespace kogayonon
{
  enum class FileMode
  {
    READ = 1,
    WRITE = 0
  };

  template<typename T>
  class Serializer
  {
  public:
    template<typename S>
    bool openFile(const std::string& path, S& file_stream, FileMode mode)
    {
      if(mode == FileMode::READ)
      {
        file_stream.open(path, std::ios::binary | std::ios::in);
        if(file_stream.is_open() && file_stream.good()) return true;
      }
      else if(mode == FileMode::WRITE)
      {
        file_stream.open(path, std::ios::binary | std::ios::out);
        if(file_stream.is_open() && file_stream.good()) return true;
      }
      return false;
    }

    template<typename S>
    bool closeFile(S&& file_stream)
    {
      if(file_stream.is_open())
      {
        file_stream.close();
        return true;
      }
      return false;
    }

    template<typename V>
    bool serializeRaw(const V* data, size_t size, std::ofstream& out)
    {
      out.write(reinterpret_cast<const char*>(data), size);
      return isGood(out);
    }

    template<typename V>
    bool deserializeRaw(V* data, size_t size, std::ifstream& in)
    {
      in.read(reinterpret_cast<char*>(data), size);
      return isGood(in);
    }

    template<typename V>
    bool serializeVar(V&& var, std::ofstream& out)
    {
      out.write(reinterpret_cast<const char*>(&var), sizeof(V));
      return isGood(out);
    }

    template<typename V>
    bool deserializeVar(V&& var, std::ifstream& in)
    {
      in.read(reinterpret_cast<char*>(&var), sizeof(V));
      return isGood(in);
    }

    /// <summary>
    /// Serialize a single type T object
    /// </summary>
    /// <param name="obj">-> Reference to object</param>
    /// <returns></returns>
    virtual bool serialize(T& obj, std::ofstream& out) = 0;
    virtual bool deserialize(T& obj, std::ifstream& in) = 0;

    /// <summary>
    /// Serialize a vector of type T objects
    /// </summary>
    /// <param name="vec">-> Reference to vector of objects</param>
    /// <returns></returns>
    virtual bool serialize(std::vector<T>& vec, std::ofstream& out) = 0;
    virtual bool deserialize(std::vector<T>& vec, std::ifstream& in) = 0;

    template<typename S>
    inline bool isGood(S& file_stream)
    {
      return file_stream.good();
    }
  };
}
