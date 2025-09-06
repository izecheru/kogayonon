#pragma once
#include <string>
#include <vector>

namespace kogayonon_resources {
struct Texture
{
    unsigned int id = 0;
    std::string type;
    std::string path;
    int width = 0;
    int height = 0;
    int num_components = 0;
    std::vector<unsigned char> data;
    bool gamma = true;

    Texture() = default;

    explicit Texture(const std::string& t, const std::string& p, int w, int h, int n, const std::vector<unsigned char>& d, bool g)
        : type(t), path(p), width(w), height(h), num_components(n), data(d), gamma(g)
    {}

    inline std::string getPath() const
    {
        return path;
    }
};

} // namespace kogayonon_resources