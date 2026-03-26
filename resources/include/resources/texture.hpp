#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace resources
{
class Texture
{
public:
  Texture() = default;

  explicit Texture( const std::string& p, const std::string& name );
  explicit Texture( const std::string& p, int w, int h, int n );
  explicit Texture( uint32_t id, const std::string& p, const std::string& name, int w, int h, int n );

  std::string getPath() const;
  std::string getName() const;
  int getWidth() const;
  int getHeight() const;
  bool getLoaded() const;

  void setPath( const std::string& path );
  void setWidth( int width );
  void setHeight( int height );
  void setLoaded( bool value );

  auto getImage() -> VkImage&;
  auto getView() -> VkImageView&;
  auto getMemory() -> VkDeviceMemory&;

private:
  VkImage m_image{};
  VkImageView m_imageView{};
  VkDeviceMemory m_imageMemory{};

  std::string m_path;
  std::string m_name;
  int m_width{ 0 };
  int m_height{ 0 };
  int m_numComponents{ 0 };
  bool m_loaded{ false };
};
} // namespace resources