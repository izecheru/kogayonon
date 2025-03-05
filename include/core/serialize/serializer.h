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
    bool isEmptyIn()
    {
      if(!m_in) return true;  // Ensure stream is valid
      m_in.seekg(0, std::ios::end); // Move to end to check file size
      std::streampos size = m_in.tellg();
      m_in.seekg(0, std::ios::beg); // Reset back to beginning
      return size <= 0;
    }


    bool openFile(const std::string& path, FileMode mode)
    {
      bool result = false;
      switch(mode)
      {
        case FileMode::READ:
          {
            m_in.open(path, std::ios::binary | std::ios::in);
            if(m_in.is_open() && m_in.good()) result = true;
            break;
          }
        case FileMode::WRITE:
          {
            m_out.open(path, std::ios::binary | std::ios::out);
            if(m_out.is_open() && m_out.good()) result = true;
            break;
          }
      }
      return result;
    }

    bool closeFile()
    {
      if(m_in.is_open())
      {
        m_in.close();
        return true;
      }
      if(m_out.is_open())
      {
        m_out.close();
        return true;
      }
    }

    template<typename V>
    bool serializeRaw(const V* data, size_t size)
    {
      m_out.write(reinterpret_cast<const char*>(data), size);
      return m_out.good();
    }

    template<typename V>
    bool deserializeRaw(V* data, size_t size)
    {
      m_in.read(reinterpret_cast<char*>(data), size);
      return m_in.good();
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

    inline bool inGood()
    {
      return m_in.good();
    }

    inline bool outGood()
    {
      return m_out.good();
    }
  private:
    std::ifstream m_in{};
    std::ofstream m_out{};
  };
}
