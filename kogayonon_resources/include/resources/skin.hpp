#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

namespace kogayonon_resources
{

struct JointTransform
{
    glm::quat rotation;
    glm::vec3 translation;
    float scale{ 1.0f };

    auto getMatrix() const -> glm::mat4
    {
        return glm::translate( glm::mat4{ 1.0f }, translation ) * glm::mat4_cast( rotation ) *
               glm::scale( glm::mat4{ 1.0f }, glm::vec3{ scale } );
    }
};

struct Joint
{
    Joint* parent{ nullptr };
    std::vector<Joint*> children;

    uint32_t id{ 0 };
    JointTransform localBindPose;
    glm::mat4 worldMatrix;
    glm::mat4 inverseBindPose;
    glm::mat4 offsetMatrix;
};

struct Skeleton
{
    std::vector<Joint> joints;
};
} // namespace kogayonon_resources
