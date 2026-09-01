#include "renderer/frame_graph.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "renderer/blackboard.hpp"
#include "renderer/node.hpp"
#include "utilities/utils/utils.hpp"

rendering::FrameGraph::FrameGraph( graphics::VulkanDevice* device )
    : m_device{ device }
    , m_blackboard{ std::make_unique<Blackboard>() }
{
}

auto rendering::FrameGraph::createResource( std::string_view name,
                                            VkImageCreateInfo imageInfo,
                                            VmaAllocationCreateInfo imageAllocInfo ) -> rendering::FGResource*
{
  m_container.resources.emplace_back( std::make_unique<FGResource>() );

  std::string resName =
    name.empty() ? "fg_resource_" + std::to_string( m_container.resources.size() ) : std::string{ name };
  FGResource* res = m_container.resources.back().get();
  res->lastState = FGResourceState{ .type = FGResourceType::None, .accessType = FGResourceAccessType::None };

  VkImageAspectFlags aspect{ VK_IMAGE_ASPECT_COLOR_BIT };
  if ( imageInfo.format == VK_FORMAT_D32_SFLOAT )
  {
    aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
  }
  else if ( imageInfo.format == VK_FORMAT_R32_SINT )
  {
    aspect = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  m_device->createImage( res->vulkanImage, imageInfo, imageAllocInfo, resName );
  m_device->createImageView( res->vulkanImage.vkImageView,
                             res->vulkanImage.vkImage,
                             imageInfo.format,
                             aspect,
                             std::string{ resName + "_imageView" } );

  return m_container.resources.back().get();
}

void rendering::FrameGraph::addEdge( Node* from, Node* to )
{
  if ( from == to )
    return;

  to->dependsOn.insert( from );
  from->consumers.insert( to );
}

auto rendering::FrameGraph::compile() -> void
{
  setupNodes();
  resloveDependencies();
  topoSort();
  resolveResourceBarriers();
  m_compiled = true;
}

auto rendering::FrameGraph::clearGraph() -> void
{
  m_container.executionOrder.clear();

  for ( std::unique_ptr<FGResource>& resource : m_container.resources )
  {
    resource->lastState = FGResourceState{ .type = FGResourceType::None, .accessType = FGResourceAccessType::None };
    resource->readerNodes.clear();
    resource->writerNodes.clear();
  }

  for ( std::unique_ptr<Node>& node : m_container.nodes )
  {
    node->consumers.clear();
    node->dependsOn.clear();
    node->reads.clear();
    node->writes.clear();
    node->resourceBarriers.clear();
    node->resourceExpectedState.clear();
    node->indegree = 0u;
  }
}

auto rendering::FrameGraph::setupNodes() -> void
{
  for ( std::unique_ptr<Node>& node : m_container.nodes )
  {
    NodeBuilder builder{ node.get(), &m_container };
    node->setupFunction( builder, m_blackboard.get() );
  }
}

auto rendering::FrameGraph::execute( VkCommandBuffer cmdBuff ) -> void
{
  for ( auto& node : m_container.executionOrder )
  {
    std::vector<VkImageMemoryBarrier2> builtBarriers{};

    for ( auto& [resource, transition] : node->resourceBarriers )
    {
      VkImageMemoryBarrier2 transitionBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                               .srcStageMask = transition.srcStage,
                                               .srcAccessMask = transition.srcAccess,
                                               .dstStageMask = transition.newStage,
                                               .dstAccessMask = transition.newAccess,
                                               .oldLayout = transition.oldLayout,
                                               .newLayout = transition.newLayout,
                                               .image = resource->vulkanImage.vkImage,
                                               .subresourceRange = {
                                                 .aspectMask = transition.aspect,
                                                 .baseMipLevel = 0,
                                                 .levelCount = 1,
                                                 .baseArrayLayer = 0,
                                                 .layerCount = 1,
                                               } };

      builtBarriers.push_back( transitionBarrier );
    }

    VkDependencyInfo dependency{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = static_cast<uint32_t>( builtBarriers.size() ),
      .pImageMemoryBarriers = builtBarriers.data(),
    };

    m_device->cmdPipelineBarrier2( cmdBuff, dependency );

    node->executeFunction( cmdBuff );
  }
}

auto rendering::FrameGraph::recompile() -> void
{
  K_INFO( "[FrameGraph] Recompiling!!!" );
  m_compiled = false;

  clearGraph();
  compile();
}

auto rendering::FrameGraph::resloveDependencies() -> void
{
  for ( std::unique_ptr<FGResource>& resource : m_container.resources )
  {
    if ( resource->writerNodes.empty() ) // don't go and loop through writers if we don't have any
    {
      continue;
    }

    Node* writer = *resource->writerNodes.begin(); // todo this might need to go from the end to the beginning
    for ( Node* reader : resource->readerNodes )
    {
      if ( reader->culled || writer->culled )
        continue;

      K_INFO( "[FrameGraph] Found edge from {} to {}", writer->name, reader->name );
      addEdge( writer, reader );
    }
  }

  for ( std::unique_ptr<Node>& node : m_container.nodes )
  {
    if ( node->culled )
      continue;

    for ( FGResource* res : node->reads )
    {
      if ( res->writerNodes.empty() )
      {
        throw std::runtime_error( "Resource was never written to!!!" );
      }
    }
    // indegree is just the number of edges a node has
    node->indegree = static_cast<uint32_t>( node->dependsOn.size() );
  }
}

auto rendering::FrameGraph::topoSort() -> void
{
  std::queue<Node*> ready{};
  std::unordered_map<Node*, uint32_t> remaining{};

  for ( std::unique_ptr<Node>& node : m_container.nodes )
  {
    if ( node->culled )
      continue;

    remaining[node.get()] = node->indegree;
    if ( node->indegree == 0 )
    {
      ready.push( node.get() );
    }
  }

  while ( !ready.empty() )
  {
    Node* n = ready.front();
    ready.pop();
    m_container.executionOrder.push_back( n );

    for ( Node* consumer : n->consumers )
    {
      if ( consumer->culled )
        continue;

      if ( --remaining[consumer] == 0 )
      {
        ready.push( consumer );
      }
    }
  }

  // those must be the same otherwise we have a cycle in our (should be) Direct Acyclic Graph
  size_t size = m_container.executionOrder.size();
  size_t count = std::count_if( m_container.nodes.begin(),
                                m_container.nodes.end(),
                                []( const std::unique_ptr<Node>& node ) { return !node->culled; } );

  K_ASSERT( size == count && "Cycle detected" );

  for ( Node* node : m_container.executionOrder )
  {
    K_INFO( "[FrameGraph] node {}", node->name );
  }
}

auto rendering::FrameGraph::resolveResourceBarriers() -> void
{
  for ( Node* node : m_container.executionOrder )
  {
    for ( FGResource* resource : node->reads )
    {
      if ( resource->lastState.accessType == FGResourceAccessType::Write ) // RAW
      {
        const FGResourceState& desiredState = node->resourceExpectedState[resource];
        bool depth = desiredState.type == FGResourceType::Depth;

        graphics::ImageTransitionData& currentData = resource->vulkanImage.transition;
        graphics::ImageTransitionData transition{};

        if ( desiredState.type == FGResourceType::ColorTransfer )
        {
          transition = graphics::ImageTransitionData{ .srcStage = currentData.newStage,
                                                      .newStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                                      .srcAccess = currentData.newAccess,
                                                      .newAccess = VK_ACCESS_2_TRANSFER_READ_BIT,
                                                      .oldLayout = currentData.newLayout,
                                                      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                      .aspect = VK_IMAGE_ASPECT_COLOR_BIT };
        }
        else if ( desiredState.type == FGResourceType::Depth ) // if we want to read a previously written depth image
        {

          transition =
            graphics::ImageTransitionData{ .srcStage = currentData.newStage,
                                           .newStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                           .srcAccess = currentData.newAccess,
                                           .newAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                           .oldLayout = currentData.newLayout,
                                           .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
                                           .aspect = VK_IMAGE_ASPECT_DEPTH_BIT };
        }
        else // if we don't have a color transfer or a previously written detph image, this also covers the Shader type
             // and the Color case
        {
          if ( depth )
          {
            transition.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
          }
          else
          {
            transition.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
          }

          transition = graphics::ImageTransitionData{
            .srcStage = currentData.newStage,
            .newStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .srcAccess = currentData.newAccess,
            .newAccess = VK_ACCESS_2_SHADER_READ_BIT,
            .oldLayout = currentData.newLayout,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
          };
        }

        currentData = transition;
        node->resourceBarriers.push_back( { resource, transition } );
        resource->lastState = desiredState;
      }
    }
    for ( FGResource* resource : node->writes )
    {
      // undefined barriers
      if ( resource->lastState.accessType == FGResourceAccessType::None ) // Undefined case
      {
        const FGResourceState& desiredState = node->resourceExpectedState[resource];
        graphics::ImageTransitionData transition;

        if ( desiredState.type == FGResourceType::Color && desiredState.accessType == FGResourceAccessType::Write )
        {
          transition = graphics::ImageTransitionData{ .srcStage = VK_PIPELINE_STAGE_2_NONE,
                                                      .newStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                      .srcAccess = VK_ACCESS_2_NONE,
                                                      .newAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                                      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                      .aspect = VK_IMAGE_ASPECT_COLOR_BIT };
        }
        else if ( desiredState.type == FGResourceType::Depth && desiredState.accessType == FGResourceAccessType::Write )
        {
          transition = graphics::ImageTransitionData{ .srcStage = VK_PIPELINE_STAGE_2_NONE,
                                                      .newStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                                                      .srcAccess = VK_ACCESS_2_NONE,
                                                      .newAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                      .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                                      .aspect = VK_IMAGE_ASPECT_DEPTH_BIT };
        }

        resource->vulkanImage.transition = transition;
        node->resourceBarriers.push_back( { resource, transition } );
        resource->lastState = desiredState;
      }
      else if ( resource->lastState.accessType == FGResourceAccessType::Write ) // WAW
      {
        const FGResourceState& desiredState = node->resourceExpectedState[resource];
        bool depth = desiredState.type == FGResourceType::Depth;

        graphics::ImageTransitionData& currentData = resource->vulkanImage.transition;

        graphics::ImageTransitionData transition{
          .srcStage = currentData.newStage,
          .newStage =
            depth ? VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT : VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
          .srcAccess = currentData.newAccess,
          .newAccess = depth ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
          .oldLayout = currentData.newLayout,
          .newLayout = depth ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
          .aspect = depth ? static_cast<VkImageAspectFlags>( VK_IMAGE_ASPECT_DEPTH_BIT )
                          : static_cast<VkImageAspectFlags>( VK_IMAGE_ASPECT_COLOR_BIT ),
        };

        currentData = transition;
        node->resourceBarriers.push_back( { resource, transition } );
        resource->lastState = desiredState;
      }
      else if ( resource->lastState.accessType == FGResourceAccessType::Read ) // WAR
      {
        const FGResourceState& desiredState = node->resourceExpectedState[resource];
        bool depth = desiredState.type == FGResourceType::Depth;

        graphics::ImageTransitionData& currentData = resource->vulkanImage.transition;

        graphics::ImageTransitionData transition{
          .srcStage = currentData.newStage,
          .newStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
          .srcAccess = currentData.newAccess,
          .newAccess = VK_ACCESS_2_SHADER_READ_BIT,
          .oldLayout = currentData.newLayout,
          .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
          .aspect = depth ? static_cast<VkImageAspectFlags>( VK_IMAGE_ASPECT_DEPTH_BIT )
                          : static_cast<VkImageAspectFlags>( VK_IMAGE_ASPECT_COLOR_BIT ),
        };

        currentData = transition;
        node->resourceBarriers.push_back( { resource, transition } );
        resource->lastState = desiredState;
      }
    }
  }
}

auto rendering::FrameGraph::getBlackboard() -> rendering::Blackboard*
{
  return m_blackboard.get();
}

auto rendering::FrameGraph::getExecOrder() -> std::vector<Node*>&
{
  return m_container.executionOrder;
}

auto rendering::FrameGraph::getContainer() -> rendering::NodeContainer&
{
  return m_container;
}
