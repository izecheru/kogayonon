#include "core/systems/scene_rendering_system.hpp"
#include "graphics/utils.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "precompiled/pch.hpp"

core::SceneRenderingSystem::SceneRenderingSystem( graphics::VulkanDevice* pDevice,
                                                  graphics::VulkanSwapchain* pSwapchain )
    : m_pDevice{ pDevice }
    , m_pSwapchain{ pSwapchain }
{
  createDescriptorPool();
  createDescriptorSetLayout();
  createGraphicsPipeline();
}

void core::SceneRenderingSystem::render( VkCommandBuffer& cmd )
{
  // TODO(kogayonon) render entities from the current scene
  VkBuffer vertexBuffers[] = { m_vertexBuffer };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers( cmd, 0, 1, vertexBuffers, nullptr );
}

void core::SceneRenderingSystem::createGraphicsPipeline()
{
  auto vert = std::filesystem::absolute( "." ) / "engine_resources\\shaders\\vert.spv";
  auto frag = std::filesystem::absolute( "." ) / "engine_resources\\shaders\\frag.spv";

  auto vertShaderCode = readFile( vert.string() );
  auto fragShaderCode = readFile( frag.string() );

  auto vertModule = createShaderModule( vertShaderCode, m_pDevice->getLogicalDevice() );
  auto fragModule = createShaderModule( fragShaderCode, m_pDevice->getLogicalDevice() );

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

  // TODO(kogayonon) provide shader binding and attrib descriptions
  auto bindingDesc = Vertex::getBindingDescription();
  auto attribDesc = Vertex::getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &bindingDesc,
    .vertexAttributeDescriptionCount = static_cast<uint32_t>( attribDesc.size() ),
    .pVertexAttributeDescriptions = attribDesc.data(),
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size() );
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                 .setLayoutCount = 1,
                                                 .pSetLayouts = &m_descriptorSetLayout };

  if ( vkCreatePipelineLayout( m_pDevice->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout ) !=
       VK_SUCCESS )
  {
    throw std::runtime_error( "failed to create pipeline layout!" );
  }

  VkPipelineRenderingCreateInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachmentFormats = &m_pSwapchain->getSwapchainImageFormat();
  renderingInfo.depthAttachmentFormat = findDepthFormat( &m_pDevice->getPhysicalDevice() );
  renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

  VkPipelineDepthStencilStateCreateInfo depthStencil{ .sType =
                                                        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                                                      .depthTestEnable = VK_TRUE,
                                                      .depthWriteEnable = VK_TRUE,
                                                      .depthCompareOp = VK_COMPARE_OP_LESS,
                                                      .depthBoundsTestEnable = VK_FALSE,
                                                      .stencilTestEnable = VK_FALSE };

  VkGraphicsPipelineCreateInfo pipelineInfo{
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &renderingInfo,
    .stageCount = 2,
    .pStages = shaderStages,
    .pVertexInputState = &vertexInputInfo,
    .pInputAssemblyState = &inputAssembly,
    .pViewportState = &viewportState,
    .pRasterizationState = &rasterizer,
    .pMultisampleState = &multisampling,
    .pDepthStencilState = &depthStencil,
    .pColorBlendState = &colorBlending,
    .pDynamicState = &dynamicState,
    .layout = m_pipelineLayout,
    .renderPass = VK_NULL_HANDLE,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
  };

  if ( vkCreateGraphicsPipelines(
         m_pDevice->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_basicGeometryPipeline ) !=
       VK_SUCCESS )
  {
    throw std::runtime_error( "failed to create graphics pipeline!" );
  }
  vkDestroyShaderModule( m_pDevice->getLogicalDevice(), vertModule, nullptr );
  vkDestroyShaderModule( m_pDevice->getLogicalDevice(), fragModule, nullptr );
}

void core::SceneRenderingSystem::createDescriptorPool()
{
}

void core::SceneRenderingSystem::createDescriptorSetLayout()
{
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>( bindings.size() );
  layoutInfo.pBindings = bindings.data();

  if ( vkCreateDescriptorSetLayout( m_pDevice->getLogicalDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout ) !=
       VK_SUCCESS )
  {
    throw std::runtime_error( "failed to create descriptor set layout!" );
  }
}
