#pragma once
#include "graphics/vulkan_image.hpp"
#include "precompiled/pch.hpp"
#include "utilities/utils/utils.hpp"
#include <entt/entt.hpp>

namespace rendering
{
struct Node;
struct NodeContainer;
struct NodeBuilder;
struct Blackboard;

enum class FGResourceAccessType
{
  None,
  Write,
  Read,
};

enum class FGResourceType
{
  None,          // undefined
  Color,         // basic color
  Depth,         // basic depth
  ColorTransfer, // for entity  picking
  Shader         // for shadow mapping for example, this will be sampled in the shader
};

struct FGResourceState
{
  FGResourceType type;
  FGResourceAccessType accessType;
};

/**
 * @brief FrameGraph resource
 */
struct FGResource
{
  graphics::VulkanImage vulkanImage;
  FGResourceState lastState;
  std::set<Node*> writerNodes;
  std::set<Node*> readerNodes;
};

struct Node
{
  /**
   * @brief This node should not be taken into account
   * MUST NOT generate edges, culling a node means recompiling the graph
   */
  bool culled{ false };

  /**
   * @brief Name of the Node
   */
  std::string name;

  /**
   * @brief Count of edges in the graph
   */
  uint32_t indegree{ 0u };

  /**
   * @brief Nodes that produce resources we need to use in the current node, so we wait for them to finish before
   * using the resource
   */
  std::set<Node*> dependsOn; // this must contain only unique entries hence set is used

  /**
   * @brief Nodes that consume resources produced by this current node, those have to wait for this one to finish
   * execution
   */
  std::set<Node*> consumers; // this must contain only unique entries hence set is used

  // TODO maybe turn those into sets too, idk if a node should write or read the same resource twice
  /**
   * @brief List of resources this Node produces
   */
  std::vector<FGResource*> writes;

  /**
   * @brief List of resources this Node consumes
   */
  std::vector<FGResource*> reads;

  /**
   * @brief Node setup function
   */
  std::function<void( NodeBuilder&, Blackboard* )> setupFunction;

  /**
   * @brief Rendering function
   */
  std::function<void( VkCommandBuffer )> executeFunction;

  /**
   * @brief This is filled at graph build time and using this state we can define the future
   * needed barriers for image layout transitions
   */
  std::unordered_map<FGResource*, FGResourceState> resourceExpectedState;

  /**
   * @brief Actual transition data per FGResource*, those should be grouped and call the vkCmdImageBarrier once per node
   */
  std::vector<std::tuple<FGResource*, VkImageMemoryBarrier2>> resourceBarriers;
  std::vector<VkImageMemoryBarrier2> barriers;
};

/**
 * @brief Container for nodes and resources, here we have the execution order of those too
 */
struct NodeContainer
{
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<std::unique_ptr<FGResource>> resources;
  std::vector<Node*> executionOrder;
};

/**
 * @brief Build a node writes and reads
 */
class NodeBuilder
{
public:
  explicit NodeBuilder( Node* nodeHandle, NodeContainer* container )
      : m_nodeHandle{ nodeHandle }
      , m_nodeContainer{ container }
  {
  }

  auto inline write( FGResource* resource, FGResourceType type ) const -> void
  {
    using enum FGResourceAccessType;

    resource->writerNodes.insert( m_nodeHandle );
    m_nodeHandle->writes.push_back( resource );

    m_nodeHandle->resourceExpectedState[resource] =
      FGResourceState{ .type = type, .accessType = FGResourceAccessType::Write };
  }

  auto inline read( FGResource* resource, FGResourceType type ) -> void
  {
    using enum FGResourceAccessType;

    resource->readerNodes.insert( m_nodeHandle );
    m_nodeHandle->reads.push_back( resource );

    m_nodeHandle->resourceExpectedState[resource] =
      FGResourceState{ .type = type, .accessType = FGResourceAccessType::Read };
  }

private:
  Node* m_nodeHandle;
  NodeContainer* m_nodeContainer;
};
} // namespace rendering