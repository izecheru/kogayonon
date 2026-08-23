#include "core/ecs/components/text_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include <gtest/gtest.h>

namespace core
{
TEST( CoreEntt, DeleteEntityTest )
{
  auto scene = std::make_shared<core::Scene>( "test" );
  auto entity = scene->addEntity();
  EXPECT_EQ( scene->getEntityCount(), 1 );
  scene->removeEntity( entity );
  EXPECT_EQ( scene->getEntityCount(), 0 );
}

TEST( CoreEntt, AddComponentTest )
{
  auto scene = std::make_shared<core::Scene>( "test" );
  auto entity = scene->addEntity();
  core::Entity ent{ scene->getRegistry(), entity };
  ent.addComponent<core::TextComponent>( core::TextComponent{ .text = "test", .pFont = nullptr } );
  EXPECT_EQ( ent.hasComponent<core::TextComponent>(), true );
}

TEST( CoreEntt, RemoveComponentTest )
{
  auto scene = std::make_shared<core::Scene>( "test" );
  auto entity = scene->addEntity();
  core::Entity ent{ scene->getRegistry(), entity };
  ent.addComponent<core::TextComponent>( core::TextComponent{ .text = "test", .pFont = nullptr } );
  EXPECT_EQ( ent.hasComponent<core::TextComponent>(), true );
  ent.removeComponent<core::TextComponent>();
  EXPECT_EQ( ent.hasComponent<core::TextComponent>(), false );
}

} // namespace core
