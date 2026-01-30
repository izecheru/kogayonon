#pragma once
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace kogayonon_utilities
{
class YamlSerializer
{
public:
  explicit YamlSerializer( const std::string& path );
  ~YamlSerializer();

  auto beginMap() -> YamlSerializer&
  {
    m_yamlEmitter << YAML::BeginMap;
    return *this;
  }

  auto endMap() -> YamlSerializer&
  {
    m_yamlEmitter << YAML::EndMap;
    return *this;
  }

  auto beginSeq() -> YamlSerializer&
  {
    m_yamlEmitter << YAML::BeginSeq;
    return *this;
  }

  auto endSeq() -> YamlSerializer&
  {
    m_yamlEmitter << YAML::EndSeq;
    return *this;
  }

  template <typename T>
  auto addKeyValuePair( const std::string& key, const T& v ) -> YamlSerializer&
  {
    m_yamlEmitter << YAML::Key << key << YAML::Value << v;
    return *this;
  }

  auto addKey( const std::string& key ) -> YamlSerializer&
  {
    m_yamlEmitter << key << YAML::Value;
    return *this;
  }

  template <typename T>
  auto addValue( const T& value ) -> YamlSerializer&
  {
    m_yamlEmitter << value;
    return *this;
  }

private:
  std::fstream m_fileStream;
  YAML::Emitter m_yamlEmitter;
};
} // namespace kogayonon_utilities
