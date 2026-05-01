#include "graphics/vulkan_pipeline.hpp"
#include <vulkan/vulkan_core.h>
#include "graphics/utils.hpp"
#include "resources/vertex.hpp"

graphics::VulkanPipeline::VulkanPipeline( const VulkanPipelineSpec& spec, VulkanContext* pContext )
    : m_spec{ spec }
    , m_pVkContext{ pContext }
{
  auto vert = std::filesystem::absolute( "." ) / "engine_resources\\shaders\\vulkan_vertex.spv";
  auto frag = std::filesystem::absolute( "." ) / "engine_resources\\shaders\\vulkan_fragment.spv";

  auto vertShaderCode = readFile( vert.string() );
  auto fragShaderCode = readFile( frag.string() );

  auto vertModule = createShaderModule( vertShaderCode, m_pVkContext->device->getLogicalDevice() );
  auto fragModule = createShaderModule( fragShaderCode, m_pVkContext->device->getLogicalDevice() );

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

  auto bindingDesc = resources::Vertex::getBindingDescription();
  auto attribDesc = resources::Vertex::getAttributeDescriptions();

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

  VkPipelineRasterizationStateCreateInfo rasterizer{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE /*VK_CULL_MODE_BACK_BIT*/,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .lineWidth = 1.0f,
  };

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

  // create the mesh constants here
  VkPushConstantRange pushConstant{};
  pushConstant.offset = 0;
  pushConstant.size = spec.pushSize;
  pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                 .setLayoutCount = std::size( spec.descriptorsLayout ),
                                                 .pSetLayouts = spec.descriptorsLayout.data(),
                                                 .pushConstantRangeCount = 1,
                                                 .pPushConstantRanges = &pushConstant };

  VK_CALL(
    vkCreatePipelineLayout( m_pVkContext->device->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_layout ) );

  VkPipelineRenderingCreateInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachmentFormats = &m_pVkContext->swapchain->getSwapchainImageFormat();
  renderingInfo.depthAttachmentFormat = findDepthFormat( &m_pVkContext->device->getPhysicalDevice() );
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
    .layout = m_layout,
    .renderPass = VK_NULL_HANDLE,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
  };

  VK_CALL( vkCreateGraphicsPipelines(
    m_pVkContext->device->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline ) );

  vkDestroyShaderModule( m_pVkContext->device->getLogicalDevice(), vertModule, nullptr );
  vkDestroyShaderModule( m_pVkContext->device->getLogicalDevice(), fragModule, nullptr );
}

void graphics::VulkanPipeline::bind( VkCommandBuffer& cmd ) const
{
  vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline );
}
