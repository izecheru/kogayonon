#pragma once
#include <benchmark/benchmark.h>
#include <filesystem>
#include <rapidjson/istreamwrapper.h>
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/registry.hpp"
#include "utilities/yaml_serializer/yaml_serializer.hpp"

// provide overloads

glm::vec3 getVec3( const Value& value )
{
  if ( !value.IsArray() || value.Size() != 3 )
    throw std::runtime_error( "Expected array of 3 for glm::vec3" );

  return glm::vec3{ value[0].GetFloat(), value[1].GetFloat(), value[1].GetFloat() };
}

namespace kogayonon_benchmark
{

// define functions here
static void BM_CreateEntities( benchmark::State& state )
{
  for ( auto _ : state )
  {
    kogayonon_core::Registry registry;

    for ( std::size_t i = 0; i < state.range( 0 ); ++i )
    {
      auto ent = registry.createEntity();
      benchmark::DoNotOptimize( ent );
    }
  }
}

static void BM_JsonDeserialization( benchmark::State& state )
{
  for ( auto _ : state )
  {
    kogayonon_core::Registry registry;
    // create the file
    auto p = std::filesystem::absolute( "." );
    p /= "benchmark.json";
    std::ifstream ifs( p );
    IStreamWrapper isw( ifs );

    Document doc{};
    doc.ParseStream( isw );

    std::vector<kogayonon_core::Entity> entities;
    // pass the args in here to allocate enough memory
    entities.reserve( state.range( 0 ) );

    auto entityArray = doc["entities"].GetArray();
    for ( auto i = 0u; i < entityArray.Size(); i++ )
    {
      kogayonon_core::Entity ent{ &registry, registry.createEntity() };
      ent.addComponent<kogayonon_core::TransformComponent>( kogayonon_core::TransformComponent{
        .translation = std::move( getVec3( entityArray[i]["transform"]["translation"] ) ),
        .rotation = std::move( getVec3( entityArray[i]["transform"]["rotation"] ) ),
        .scale = std::move( getVec3( entityArray[i]["transform"]["scale"] ) ) } );
      entities.emplace_back( ent );
    }
  }
}

static void BM_Serialization( benchmark::State& state )
{
  for ( auto _ : state )
  {
    kogayonon_core::Registry registry;
    // create the file
    auto p = std::filesystem::absolute( "." );
    p /= "benchmark.yaml";

    // yaml serialzier handles file creation and filestream closing
    auto serializer = std::make_unique<kogayonon_utilities::YamlSerializer>( p.string() );

    std::vector<kogayonon_core::Entity> entities;
    // pass the args in here to allocate enough memory
    entities.reserve( state.range( 0 ) );

    for ( std::size_t i = 0; i < state.range( 0 ); ++i )
    {
      kogayonon_core::Entity entity{ &registry, registry.createEntity() };

      entity.addComponent<kogayonon_core::TransformComponent>( kogayonon_core::TransformComponent{
        .translation = { 1.0f, 2.0f, 3.0f }, .rotation = { 1.0f, 2.0f, 3.0f }, .scale = { 1.0f, 2.0f, 3.0f } } );

      entities.emplace_back( entity );
    }

    // now serialize
    for ( auto entity : entities )
    {
      // clang-format off
        serializer->
            beginMap()
                .addKeyValuePair("entityId",static_cast<int>(entity.getEntityId()))
                .addKeyValuePair("transformComponent",entity.getComponent<kogayonon_core::TransformComponent>())
                .addKeyValuePair("identifierComponent",entity.getComponent<kogayonon_core::IdentifierComponent>())
            .endMap();
        // clang-format on 
      }
    }
  }

static void BM_JsonSerialization( benchmark::State& state )
{
  for ( auto _ : state )
  {
    kogayonon_core::Registry registry;
    // create the file
    auto p = std::filesystem::absolute( "." );
    p /= "benchmark.json";

    // have a constant number of entries in the file for the deserialization
    if(std::filesystem::exists(p))
        std::filesystem::remove(p);

    // yaml serialzier handles file creation and filestream closing
    auto serializer = std::make_unique<kogayonon_utilities::JsonSerializer>( p.string() );

    std::vector<kogayonon_core::Entity> entities;
    // pass the args in here to allocate enough memory
    entities.reserve( state.range( 0 ) );

    for ( std::size_t i = 0; i < state.range( 0 ); ++i )
    {
      kogayonon_core::Entity entity{ &registry, registry.createEntity() };

      entity.addComponent<kogayonon_core::TransformComponent>( kogayonon_core::TransformComponent{
        .translation = { 1.0f, 2.0f, 3.0f }, .rotation = { 1.0f, 2.0f, 3.0f }, .scale = { 1.0f, 2.0f, 3.0f } } );

      entities.emplace_back( entity );
    }

    // now serialize
    serializer->startDocument().startArray( "entities" );
    for ( auto entity : entities )
    {
      auto& transform = entity.getComponent<kogayonon_core::TransformComponent>();
      auto& rotation = transform.rotation;
      auto& scale = transform.scale;
      auto& translation = transform.translation;
      // clang-format off
        serializer->
            startObject()
            .addKeyValuePair("id",static_cast<int>(entity.getEntityId()))
                .startObject("transform")
                    .startArray("rotation")
                        .addValue(rotation.x)
                        .addValue(rotation.x)
                        .addValue(rotation.x)
                    .endArray()
                    .startArray("scale")
                        .addValue(scale.x)
                        .addValue(scale.x)
                        .addValue(scale.x)
                    .endArray()
                    .startArray("translation")
                        .addValue(translation.x)
                        .addValue(translation.x)
                        .addValue(translation.x)
                    .endArray()
                .endObject()
            .endObject();

      // clang-format on 
    }
    serializer->endArray().endDocument();
}
}

} // namespace kogayonon_benchmark