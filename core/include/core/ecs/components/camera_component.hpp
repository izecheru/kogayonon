#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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
     * @brief
     */
    glm::vec3 direction{ 0.0f, 0.0f, -1.0f };
    float fov{ 45.0f };
    glm::ivec2 extent;
    float nearView{ 0.01f };
    float farView{ 3000.0f };

    float yaw{ -90.0f };
    float pitch{ 0.0f };

    // If we modify the props, update the projection and the view
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
        if ( props.changed )
        {
            if ( props.pitch > 89.0f )
                props.pitch = 89.0f;

            if ( props.pitch < -89.0f )
                props.pitch = -89.0f;

            props.direction.x = cos( glm::radians( props.yaw ) ) * cos( glm::radians( props.pitch ) );
            props.direction.y = sin( glm::radians( props.pitch ) );
            props.direction.z = sin( glm::radians( props.yaw ) ) * cos( glm::radians( props.pitch ) );

            props.direction = glm::normalize( props.direction );

            props.center = props.eye + props.direction;

            ubo.view = glm::lookAt( props.eye, props.center, props.up );
            ubo.projection = glm::perspective(
                glm::radians( props.fov ), props.extent.x / (float)props.extent.y, props.nearView, props.farView );
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