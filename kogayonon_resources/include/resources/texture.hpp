#pragma once
#include <string>
#include <vector>

namespace kogayonon_resources
{
class Texture
{
public:
  Texture() = default;

  explicit Texture( const std::string& p, int w, int h, int n );
  explicit Texture( uint32_t id, const std::string& p, const std::string& name, int w, int h, int n );
  std::string getPath() const;
  std::string getName() const;
  int getWidth() const;
  int getHeight() const;
  uint32_t getTextureId() const;

  void setPath( const std::string& path );
  void setWidth( int width );
  void setHeight( int height );
  void setTextureId( unsigned int id );

private:
  uint32_t m_id = 0;
  std::string m_path;
  std::string m_name;
  int m_width = 0;
  int m_height = 0;
  int m_numComponents = 0;
};
} // namespace kogayonon_resources