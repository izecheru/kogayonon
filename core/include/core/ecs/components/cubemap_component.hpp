#pragma once

namespace resources
{
struct Texture;
}

namespace core
{

struct CubemapComponent
{
    resources::Texture* cubemapTexture;
    bool isUsed{ false };
};

} // namespace core