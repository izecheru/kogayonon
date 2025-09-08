#include "core/ecs/entity.h"

namespace kogayonon_core {
Entity::Entity( Registry& registry ) : Entity( registry, "Entity" ) {}

Entity::Entity( Registry& registry, const std::string& name )
    : m_registry( registry ), m_entity( registry.createEntity() ), m_name( name )
{}
} // namespace kogayonon_core