#pragma once
#include <string>

namespace kogayonon_core
{
struct NameComponent
{
    NameComponent(std::string n) : name(n)
    {
    }

    ~NameComponent() = default;

    std::string name;
};
} // namespace kogayonon_core