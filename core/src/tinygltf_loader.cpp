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

    auto visitNode = [this, pMesh]( this auto&& self, int nodeIndex, const glm::mat4& parentTransform ) {
        if ( nodeIndex < 0 || nodeIndex > m_model.nodes_count )
        {
            return;
        }

        auto& submeshes = pMesh->getSubmeshes();
        auto& vertices = pMesh->getVertices();
        auto& indices = pMesh->getIndices();

        const tg3_node* node = &m_model.nodes[nodeIndex];

        glm::mat4 localTransform{};

        if ( node->has_matrix )
        {
            localTransform = glm::make_mat4( node->matrix );
        }
        else
        {

            glm::vec3 translation{ node->translation[0], node->translation[1], node->translation[2] };
            glm::vec3 rotation{ node->rotation[0], node->rotation[1], node->rotation[2] };
            glm::vec3 scale{ node->scale[0], node->scale[1], node->scale[2] };
            glm::quat q{ static_cast<float>( node->rotation[3] ),
                         static_cast<float>( node->rotation[0] ),
                         static_cast<float>( node->rotation[1] ),
                         static_cast<float>( node->rotation[2] ) };
            localTransform = glm::translate( glm::mat4( 1.0f ), translation );
            localTransform = localTransform * glm::mat4_cast( q );
            localTransform = glm::scale( localTransform, scale );
        }

        glm::mat4 local = parentTransform * localTransform;

        if ( node->mesh != -1 )
        {
            const tg3_mesh* mesh = &m_model.meshes[node->mesh];

            for ( auto j = 0u; j < mesh->primitives_count; ++j )
            {
                const tg3_primitive* primitive = &mesh->primitives[j];

                std::vector<uint32_t> localIndices;
                std::vector<resources::Vertex> localVertices;

                for ( auto k = 0u; k < primitive->attributes_count; ++k )
                {
                    const tg3_str_int_pair* attr = &primitive->attributes[k];
                    if ( strcmp( attr->key.data, "POSITION" ) == 0 )
                    {
                        const tg3_accessor* accessor = &m_model.accessors[attr->value];
                        KASSERT( accessor->type == TG3_TYPE_VEC3 &&
                                 accessor->component_type == TG3_COMPONENT_TYPE_FLOAT );
                        writeAttribute( &resources::Vertex::translation, attr, localVertices );
                    }

                    if ( strcmp( attr->key.data, "NORMAL" ) == 0 )
                    {
                        const tg3_accessor* accessor = &m_model.accessors[attr->value];
                        KASSERT( accessor->type == TG3_TYPE_VEC3 &&
                                 accessor->component_type == TG3_COMPONENT_TYPE_FLOAT );
                        writeAttribute( &resources::Vertex::normal, attr, localVertices );
                    }

                    if ( strcmp( attr->key.data, "TEXCOORD_0" ) == 0 )
                    {
                        const tg3_accessor* accessor = &m_model.accessors[attr->value];
                        KASSERT( accessor->type == TG3_TYPE_VEC2 &&
                                 accessor->component_type == TG3_COMPONENT_TYPE_FLOAT );
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

                uint32_t vertexCount = vertices.size();
                uint32_t indexCount = indices.size();

                for ( auto& v : localVertices )
                {
                    glm::vec4 p = local * glm::vec4( v.translation, 1.0f );
                    v.translation = glm::vec3{ p };
                    glm::mat3 nrmMat = glm::transpose( glm::inverse( glm::mat3( local ) ) );
                    v.normal = glm::normalize( nrmMat * v.normal );
                }

                vertices.insert( vertices.end(), localVertices.begin(), localVertices.end() );
                indices.insert( indices.end(), localIndices.begin(), localIndices.end() );

                submeshes.push_back( resources::Submesh{ .vertexOffset = vertexCount,
                                                         .indexOffset = indexCount,
                                                         .indexCount = static_cast<uint32_t>( localIndices.size() ),
                                                         .submeshMaterial = mat } );
            }
        }

        for ( auto c = 0u; c < node->children_count; ++c )
        {
            self( node->children[c], local );
        }
    };

    if ( m_model.scenes_count > 0 )
    {
        const tg3_scene* scene = &m_model.scenes[m_model.default_scene >= 0 ? m_model.default_scene : 0];
        for ( auto r = 0u; r < scene->nodes_count; ++r )
        {
            visitNode( scene->nodes[r], glm::mat4( 1.0f ) );
        }
    }
}

auto core::TinyGltfLoader::parseTextureData( resources::Mesh* pMesh )
    -> std::unordered_map<uint32_t, std::map<TextureType, std::string>>
{
    std::unordered_map<uint32_t, std::map<TextureType, std::string>> submeshMaterials;
    namespace fs = std::filesystem;

    auto getEnginePath = [pMesh]( const std::string& texturePathFromLoader ) -> std::string {
        if ( texturePathFromLoader.empty() )
            return { "" };

        // relative to its parent
        std::filesystem::path p{ texturePathFromLoader };
        std::filesystem::path meshPath{ pMesh->getPath() };
        std::filesystem::path path{ meshPath.parent_path() / "textures" / p.filename() };

        return path.string();
    };

    std::vector<resources::Submesh>& submeshes = pMesh->getSubmeshes();
    for ( uint32_t i = 0u; i < submeshes.size(); i++ )
    {
        resources::Submesh& submesh = submeshes.at( i );

        std::map<TextureType, std::string> texturePaths;

        std::string diffusePath{ "" };
        std::string emissivePath{ "" };
        std::string normalPath{ "" };

        if ( submesh.submeshMaterial.diffuseTextureIndex != -1 )
        {
            const tg3_str* uri = &m_model.images[submesh.submeshMaterial.diffuseTextureIndex].uri;
            if ( uri->data && uri->len > 0 )
                diffusePath = std::string{ uri->data, uri->len };
        }

        if ( submesh.submeshMaterial.normalTextureIndex != -1 )
        {
            const tg3_str* uri = &m_model.images[submesh.submeshMaterial.normalTextureIndex].uri;
            if ( uri->data && uri->len > 0 )
                normalPath = std::string{ uri->data, uri->len };
        }

        if ( submesh.submeshMaterial.emissiveTextureIndex != -1 )
        {
            const tg3_str* uri = &m_model.images[submesh.submeshMaterial.emissiveTextureIndex].uri;
            if ( uri->data && uri->len > 0 )
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
}
