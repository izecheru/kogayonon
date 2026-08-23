#include "graphics/vulkan_pipeline.hpp"
#include "graphics/utils.hpp"
#include "resources/vertex.hpp"
#include <vulkan/vulkan_core.h>

graphics::VulkanPipeline::VulkanPipeline( const VulkanPipelineSpec& spec, VulkanContext* pContext )
    : m_spec{ spec }
    , m_vkCtx{ pContext }
    , m_pipeline{ VK_NULL_HANDLE }
{
  create( spec, pContext );
}

void graphics::VulkanPipeline::bind( VkCommandBuffer& cmd, VkPipelineBindPoint bindPoint ) const
{
  vkCmdBindPipeline( cmd, bindPoint, m_pipeline );
}

auto graphics::VulkanPipeline::getLayout() const -> VkPipelineLayout
{
  return m_layout;
}

auto graphics::VulkanPipeline::create( const VulkanPipelineSpec& spec, VulkanContext* vkCtx ) -> void
{
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = spec.vertexModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = spec.fragmentModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &spec.vertexBindingDescription,
    .vertexAttributeDescriptionCount = static_cast<uint32_t>( spec.vertexAttributesDescription.size() ),
    .pVertexAttributeDescriptions = spec.vertexAttributesDescription.data(),
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
    .polygonMode = spec.options.polyMode,
    .cullMode = spec.options.cullMode,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .lineWidth = spec.options.lineWidth,
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

  // If we have a line width bigger than 1 and the pipeline is a wireframe one, we add the dynamic state line width to
  // the dynamicStates
  if ( spec.options.lineWidth > 1.0f )
  {
    dynamicStates.push_back( VK_DYNAMIC_STATE_LINE_WIDTH );
  }

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size() );
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = std::size( spec.descriptorLayout );
  pipelineLayoutInfo.pSetLayouts = spec.descriptorLayout.data();

  if ( spec.pushConstantSize != 0u )
  {
    // create the mesh constants here
    VkPushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = spec.pushConstantSize;
    pushConstant.stageFlags = spec.pushConstantVisibility;

    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
  }
  else // no push constants
  {
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
  }

  vkCtx->device->createPipelineLayout( pipelineLayoutInfo, m_layout );

  VkPipelineRenderingCreateInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  if ( spec.colorAttachmentCount == 0u )
  {
    renderingInfo.colorAttachmentCount = 0;
    renderingInfo.pColorAttachmentFormats = VK_NULL_HANDLE;
  }
  else
  {
    renderingInfo.colorAttachmentCount = spec.colorAttachmentCount;
    renderingInfo.pColorAttachmentFormats = spec.colorAttachmentFormat.data();
  }

  // TODO change the format to come from the spec
  renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
  renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

  VkPipelineDepthStencilStateCreateInfo depthStencil{ .sType =
                                                        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                                                      .depthTestEnable = spec.options.depthTestEnable,
                                                      .depthWriteEnable = spec.options.depthWriteEnable,
                                                      .depthCompareOp = spec.options.depthCompareOp,
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
  vkCtx->device->createGraphicsPipeline( m_pipeline, pipelineInfo );
}

auto graphics::VulkanPipeline::getPipeline() const -> VkPipeline
{
  return m_pipeline;
}
