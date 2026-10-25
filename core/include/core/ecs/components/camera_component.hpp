#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "tracy/Tracy.hpp"

namespace core
{
struct CameraProps
{
    /**
     * @brief Camera (eye) position
     */
    glm::vec3 eye{ 0.0f, 2.0f, 10.0f };

    /**
     * @brief Where camera is looking
     */
    glm::vec3 center{ 0.0f, 0.0f, 0.0f };

    /**
     * @brief Which way is up, +Y is up
     */
    glm::vec3 up{ 0.0f, 1.0f, 0.0f };

    /**
     * @brief The difference from eye -> center,
     * this is built after calculating yaw and pitch
     */
    glm::vec3 direction{ 0.0f, 0.0f, -1.0f };

    float fov{ 45.0f };
    glm::ivec2 extent{ 20, 20 };

    float nearView{ 0.01f };
    float farView{ 3000.0f };

    /**
     * @brief Those are built using the relative mouse movement accumulated
     * since last frame
     */
    float yaw{ -90.0f };
    float pitch{ 0.0f };

    glm::vec3 forward{};
    glm::vec3 right{};

    /**
     * @brief If this is true, matrices are rebuilt before sending the data to the shader
     */
    bool changed{ false };
};

struct CameraUbo
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 translation;
};

struct PerspectiveCameraComponent
{
    CameraUbo ubo;
    CameraProps props;
    bool isUsed{ false };

    inline void updateUbo()
    {
        ZoneScopedN( "Camera::updateUbo" );
        if ( props.changed )
        {
            props.pitch = std::clamp( props.pitch, -89.0f, 89.0f );

            props.direction.x = cos( glm::radians( props.yaw ) ) * cos( glm::radians( props.pitch ) );
            props.direction.y = sin( glm::radians( props.pitch ) );
            props.direction.z = sin( glm::radians( props.yaw ) ) * cos( glm::radians( props.pitch ) );

            props.direction = glm::normalize( props.direction );
            props.right = glm::normalize( glm::cross( props.up, props.direction ) );
            props.forward = glm::normalize( glm::vec3{ props.direction.x, 0.0f, props.direction.z } );

            props.center = props.eye + props.direction;

            ubo.view = glm::lookAt( props.eye, props.center, props.up );
            ubo.projection =
                glm::perspective( glm::radians( props.fov ),
                                  static_cast<float>( props.extent.x ) / static_cast<float>( props.extent.y ),
                                  props.nearView,
                                  props.farView );
            ubo.projection[1][1] *= -1;
            // this is for reflection from the cubemap
            ubo.translation = props.eye;
            props.changed = false;
        }
    }
};

struct OrthoCameraProps
{
    glm::vec2 horizontal;
    glm::vec2 vertical;
    glm::vec3 position;
    float zoom{ 1.0f };
    float aspect{ 1.0f };
    float nearView{ 0.1f };
    float farView{ 3000.0f };
};

struct OrthoUbo
{
    glm::mat4 view;
    glm::mat4 projection;
};

// This should be the engine default camera
struct OrthoCameraComponent
{
    OrthoUbo ubo;
    OrthoCameraProps props;

    inline void updateUbo()
    {
        ubo.view = glm::translate( glm::mat4( 1.0f ), -props.position );

        ubo.projection = glm::ortho(
            props.horizontal.x, props.horizontal.y, props.vertical.x, props.vertical.y, props.nearView, props.farView );
    }
};
} // namespace core