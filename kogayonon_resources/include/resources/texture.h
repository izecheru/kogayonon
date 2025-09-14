#pragma once
#include <string>
#include <vector>

namespace kogayonon_resources
{
class Texture
{
  public:
    Texture() = default;

    explicit Texture(const std::string& p, int w, int h, int n);
    explicit Texture(unsigned int id, const std::string& p, int w, int h, int n);
    std::string getPath() const;
    int getWidth() const;
    int getHeight() const;
    unsigned int getTextureId() const;

    void setPath(const std::string& path);
    void setWidth(int width);
    void setHeight(int height);
    void setTextureId(unsigned int id);

  private:
    unsigned int m_id = 0;
    std::string m_path;
    int m_width = 0;
    int m_height = 0;
    int m_numComponents = 0;
};
} // namespace kogayonon_resources