#include "core/asset_manager/tinygltf_loader.hpp"
#include "utilities/utils/utils.hpp"

core::TinyGltfLoader::TinyGltfLoader( std::string_view modelPath )
    : m_modelPath{ modelPath }
{

    tg3_parse_options_init( &m_opts );
    tg3_error_stack_init( &m_errors );

    tg3_error_code err = tg3_parse_file( &m_model, &m_errors, m_modelPath.c_str(), m_modelPath.size(), &m_opts );

    if ( err != TG3_OK )
    {
        for ( auto i = 0u; i < m_errors.count; i++ )
        {
            const char* message = m_errors.entries[i].message;
            KERROR( "[{}] {}", i, message ? message : "null" );
        }

        tg3_model_free( &m_model );
        tg3_error_stack_free( &m_errors );
    }

    // just to stop execution if there is an error with the
    // model parsing part of the loading, this is ran in a separate thread so yea
    assert( err == TG3_OK );
}

core::TinyGltfLoader::~TinyGltfLoader()
{
    tg3_model_free( &m_model );
    tg3_error_stack_free( &m_errors );
}

auto core::TinyGltfLoader::processVertexData( resources::Mesh* pMesh ) -> void
{
    auto& submeshes = pMesh->getSubmeshes();
    auto& vertices = pMesh->getVertices();
    auto& indices = pMesh->getIndices();

    for ( auto i = 0u; i < m_model.meshes_count; ++i )
    {
        const tg3_mesh* mesh = &m_model.meshes[i];

        for ( auto j = 0u; j < mesh->primitives_count; ++j )
        {
            const tg3_primitive* primitive = &mesh->primitives[j];

            std::vector<uint32_t> localIndices;
            std::vector<resources::Vertex> localVertices;
            std::vector<resources::Submesh> localSubmeshes;

            for ( auto k = 0u; k < primitive->attributes_count; ++k )
            {
                const tg3_str_int_pair* attr = &primitive->attributes[k];
                if ( strcmp( attr->key.data, "POSITION" ) == 0 )
                {
                    const tg3_accessor* accessor = &m_model.accessors[attr->value];
                    KASSERT( accessor->type == TG3_TYPE_VEC3 && accessor->component_type == TG3_COMPONENT_TYPE_FLOAT );
                    writeAttribute( &resources::Vertex::translation, attr, localVertices );
                }

                if ( strcmp( attr->key.data, "NORMAL" ) == 0 )
                {
                    const tg3_accessor* accessor = &m_model.accessors[attr->value];
                    KASSERT( accessor->type == TG3_TYPE_VEC3 && accessor->component_type == TG3_COMPONENT_TYPE_FLOAT );
                    writeAttribute( &resources::Vertex::normal, attr, localVertices );
                }

                if ( strcmp( attr->key.data, "TEXCOORD_0" ) == 0 )
                {
                    const tg3_accessor* accessor = &m_model.accessors[attr->value];
                    KASSERT( accessor->type == TG3_TYPE_VEC2 && accessor->component_type == TG3_COMPONENT_TYPE_FLOAT );
                    writeAttribute( &resources::Vertex::uv, attr, localVertices );
                }
            }

            resources::Material mat{};
            if ( primitive->material >= 0 && primitive->material < m_model.materials_count )
            {
                const tg3_material* material = &m_model.materials[primitive->material];

                mat.diffuseTextureIndex =
                    imageIndexFromTexture( material->pbr_metallic_roughness.base_color_texture.index );
                mat.normalTextureIndex = imageIndexFromTexture( material->normal_texture.index );
                mat.emissiveTextureIndex = imageIndexFromTexture( material->emissive_texture.index );
            }

            if ( primitive->indices != -1 )
            {
                const tg3_accessor* accessor = &m_model.accessors[primitive->indices];
                const tg3_buffer_view* bufferView = &m_model.buffer_views[accessor->buffer_view];
                const tg3_buffer* buffer = &m_model.buffers[bufferView->buffer];

                if ( localIndices.size() == 0 )
                {
                    localIndices.resize( accessor->count );
                }

                if ( accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT )
                {
                    const uint32_t* buffData = reinterpret_cast<const uint32_t*>(
                        buffer->data.data + bufferView->byte_offset + accessor->byte_offset );
                    memcpy( localIndices.data(), buffData, accessor->count * sizeof( uint32_t ) );
                }
                else if ( accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT )
                {
                    const uint16_t* buffData = reinterpret_cast<const uint16_t*>(
                        buffer->data.data + bufferView->byte_offset + accessor->byte_offset );
                    for ( uint64_t idx = 0; idx < accessor->count; ++idx )
                    {
                        localIndices[idx] = static_cast<uint32_t>( buffData[idx] );
                    }
                }
            }

            uint32_t vertOffset = vertices.size();
            uint32_t indexOffset = indices.size();

            localSubmeshes.emplace_back( resources::Submesh{ .vertexOffset = vertOffset,
                                                             .indexOffset = indexOffset,
                                                             .indexCount = static_cast<uint32_t>( localIndices.size() ),
                                                             .submeshMaterial = mat } );

            vertices.insert( vertices.end(), localVertices.begin(), localVertices.end() );
            indices.insert( indices.end(), localIndices.begin(), localIndices.end() );
            submeshes.insert( submeshes.end(), localSubmeshes.begin(), localSubmeshes.end() );
        }
    }
}

auto core::TinyGltfLoader::parseTextureData( resources::Mesh* pMesh )
    -> std::unordered_map<uint32_t, std::map<TextureType, std::string>>
{
    std::unordered_map<uint32_t, std::map<TextureType, std::string>> submeshMaterials;

    auto getEnginePath = []( const std::string& texturePathFromLoader ) -> std::string {
        if ( texturePathFromLoader.empty() )
            return { "" };

        std::filesystem::path p{ texturePathFromLoader };
        auto path = std::filesystem::current_path() / "engine_resources" / "textures" / p.filename();
        return path.string();
    };

    std::vector<resources::Submesh>& submeshes = pMesh->getSubmeshes();
    for ( uint32_t i = 0u; i < submeshes.size(); i++ )
    {
        auto& submesh = submeshes.at( i );

        std::map<TextureType, std::string> texturePaths;
        std::string diffusePath = "";
        std::string emissivePath = "";
        std::string normalPath = "";

        if ( submesh.submeshMaterial.diffuseTextureIndex != -1 )
        {
            const tg3_str* uri = &m_model.images[submesh.submeshMaterial.diffuseTextureIndex].uri;
            diffusePath = std::string{ uri->data, uri->len };
        }

        if ( submesh.submeshMaterial.normalTextureIndex != -1 )
        {
            const tg3_str* uri = &m_model.images[submesh.submeshMaterial.normalTextureIndex].uri;
            normalPath = std::string{ uri->data, uri->len };
        }

        if ( submesh.submeshMaterial.emissiveTextureIndex != -1 )
        {
            const tg3_str* uri = &m_model.images[submesh.submeshMaterial.emissiveTextureIndex].uri;
            emissivePath = std::string{ uri->data, uri->len };
        }

        KINFO( "Material {}, {}, {}", normalPath, emissivePath, diffusePath );

        diffusePath = getEnginePath( diffusePath );
        normalPath = getEnginePath( normalPath );
        emissivePath = getEnginePath( emissivePath );

        texturePaths.emplace( TextureType::Diffuse, diffusePath );
        texturePaths.emplace( TextureType::Emissive, emissivePath );
        texturePaths.emplace( TextureType::Normal, normalPath );

        submeshMaterials.emplace( i, texturePaths );
    }

    return submeshMaterials;
}

auto core::TinyGltfLoader::imageIndexFromTexture( int textureIndex ) const -> int
{
    if ( textureIndex < 0 || textureIndex >= m_model.textures_count )
    {
        return -1;
    }

    int src = m_model.textures[textureIndex].source;

    if ( src < 0 || src >= m_model.images_count )
    {
        return -1;
    }

    return src;
};
