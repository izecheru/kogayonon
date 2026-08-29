#pragma once
#include "renderer/node.hpp"
#include <entt/entt.hpp>

namespace graphics
{
class VulkanDevice;
} // namespace graphics

namespace rendering
{
class Blackboard;

class FrameGraph
{
public:
  explicit FrameGraph( graphics::VulkanDevice* device );
  ~FrameGraph() = default;

  /**
   * @brief Add a node to the graph
   * @param name Name of the node, Node_Name if empty()
   * @param nodeSetupFunc In this function the reads and writes are specified using the builder
   * @param renderFunc The function that is executed during the execution part of the frame graph
   * @return
   */
  template <typename SetupFunc, typename ExecuteFunc>
  auto addNode( std::string_view name, SetupFunc&& setupFunc, ExecuteFunc&& executeFunc ) -> void
  {
    auto node = std::make_unique<rendering::Node>();
    node->name = name.empty() ? "Node_Name" : std::string{ name };
    node->setupFunction = std::move( setupFunc );
    node->executeFunction = std::move( executeFunc );

    m_container.nodes.emplace_back( std::move( node ) );
  }

  /**
   * @brief Create the rendering resource a node needs
   * @param name Name of said resource (for debugging)
   * @param imageInfo
   * @param imageAllocInfo
   * @return Pointer to that FGResource
   */
  auto createResource( std::string_view name, VkImageCreateInfo imageInfo, VmaAllocationCreateInfo imageAllocInfo )
    -> FGResource*;

  auto getExecOrder() -> std::vector<Node*>&;
  auto getContainer() -> rendering::NodeContainer&;
  auto getBlackboard() -> Blackboard*;

  /**
   * @brief The compilation part of the graph resolves dependencies by adding edges where a relation between nodes is
   * discovered from the resources a specific node handles, be it read from or write to resources, then topologically
   * sorting the graph to get the right execution order
   * @return
   */
  auto compile() -> void;

  /**
   * @brief Clear the graph resources to prepare for recompilation
   * @return
   */
  auto clearGraph() -> void;

  /**
   * @brief Call the setup function for each node
   * @return
   */
  auto setupNodes() -> void;

  /**
   * @brief Execution part of the graph, where the rendering lambdas are all called and fed with the swapchain current
   * command buffer in use
   * @return
   */
  auto execute( VkCommandBuffer cmdBuff ) -> void;

  /**
   * @brief Recompile the graph, clears the container of data and compiles the graph again
   * @return
   */
  auto recompile() -> void;

private:
  /**
   * @brief Add an edge in the graph from a node to another if a dependency is found, eg. a node reads a resource that
   * another node writes
   * @param from
   * @param to
   * @return
   */
  auto addEdge( Node* from, Node* to ) -> void;

  /**
   * @brief Parse all the resources from the container and if a relationship is found, we create an edge then at the
   * end, the indegree is exactly the amount of edges a node has
   * @return
   */
  auto resloveDependencies() -> void;

  /**
   * @brief Topologically sort the graph based on nodes it depends on
   * @return
   */
  auto topoSort() -> void;

  /**
   * @brief Search for resource transitions that need a barrier set in place and execute it
   * @return
   */
  auto resolveResourceBarriers() -> void;

private:
  NodeContainer m_container;
  std::unique_ptr<Blackboard> m_blackboard;
  graphics::VulkanDevice* m_device;

  bool m_compiled{ false };
};
} // namespace rendering