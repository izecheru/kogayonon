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
    // mode = 1 -> ifstream
    // mode = 0 -> ofstream

    bool openFile(const std::string& path, FileMode mode)
    {
      bool result = false;
      switch (mode)
      {
        case FileMode::READ:
          {
            m_in.open(path, std::ios::binary | std::ios::in);
            if (m_in.is_open() && m_in.good()) result = true;
            break;
          }
        case FileMode::WRITE:
          {
            m_out.open(path, std::ios::binary | std::ios::out);
            if (m_out.is_open() && m_out.good()) result = true;
            break;
          }
      }
      return result;
    }

    bool closeFile()
    {
      if (m_in.is_open())
      {
        m_in.close();
        return true;
      }
      if (m_out.is_open())
      {
        m_out.close();
        return true;
      }
    }

    template<typename V>
    bool serializeVar(V&& var)
    {
      m_out.write(reinterpret_cast<const char*>(&var), sizeof(V));
      return m_out.good();
    }


    template<typename V>
    bool deserializeVar(V&& var)
    {
      m_in.read(reinterpret_cast<char*>(&var), sizeof(V));
      return m_in.good();
    }

    virtual bool serialize(T& obj) = 0;
    virtual bool deserialize(T& obj) = 0;

    virtual bool serialize(std::vector<T>& vec) = 0;
    virtual bool deserialize(std::vector<T>& vec) = 0;

  protected:
    std::ifstream m_in{};
    std::ofstream m_out{};
  };
}
