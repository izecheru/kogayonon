#define VMA_IMPLEMENTATION

#include "graphics/vulkan_device.hpp"
#include "graphics/shader_compiler.hpp"
#include "graphics/utils.hpp"
#include "graphics/vulkan_buffer.hpp"
#include "graphics/vulkan_image.hpp"
#include "precompiled/pch.hpp"
#include "utilities/utils/utils.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

static void destroyDebugUtilsMessengerEXT( VkInstance instance,
                                           VkDebugUtilsMessengerEXT debugMessenger,
                                           const VkAllocationCallbacks* pAllocator )
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
  if ( func != nullptr )
  {
    func( instance, debugMessenger, pAllocator );
  }
}

static auto createDebugUtilsMessengerEXT( VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger ) -> VkResult
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
  if ( func != nullptr )
  {
    return func( instance, pCreateInfo, pAllocator, pDebugMessenger );
  }
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

auto graphics::VulkanDevice::checkDeviceExtensionSupport( VkPhysicalDevice device ) -> bool
{
  uint32_t extensionCount{ 0u };
  vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );
  std::vector<VkExtensionProperties> availableExtensions( extensionCount );
  vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data() );

  std::set<std::string> requiredExtensions( deviceExtensions.begin(), deviceExtensions.end() );

  for ( const auto& extension : availableExtensions )
  {
    requiredExtensions.erase( extension.extensionName );
  }
  K_ASSERT( requiredExtensions.empty() && "Not all extensions supported" );
  return requiredExtensions.empty();
}

auto graphics::VulkanDevice::createDescriptorSetLayout( VkDescriptorSetLayout& layout,
                                                        VkDescriptorSetLayoutCreateInfo& layoutInfo ) const -> void
{
  VK_CALL( vkCreateDescriptorSetLayout( m_platform.device, &layoutInfo, nullptr, &layout ) );
}

auto graphics::VulkanDevice::createDescriptorSet() -> void
{
}

auto graphics::VulkanDevice::allocateDescriptorSet( VkDescriptorSet& descriptor,
                                                    VkDescriptorSetAllocateInfo& info ) const -> void
{
  VK_CALL( vkAllocateDescriptorSets( m_platform.device, &info, &descriptor ) );
}

auto graphics::VulkanDevice::updateDescriptorSet( std::initializer_list<VkWriteDescriptorSet> writes,
                                                  uint32_t writeCount,
                                                  uint32_t copyCount ) const -> void
{
  vkUpdateDescriptorSets( m_platform.device, writeCount, writes.data(), copyCount, nullptr );
}

auto graphics::VulkanDevice::destroyShaderModule( VkShaderModule module ) const -> void
{
  vkDestroyShaderModule( m_platform.device, module, nullptr );
}

auto graphics::VulkanDevice::destroyShaderModule( std::initializer_list<VkShaderModule> modules ) const -> void
{
  for ( auto shader : modules )
  {
    destroyShaderModule( shader );
  }
}

auto graphics::VulkanDevice::destroyImageView( VkImageView imageView ) const -> void
{
  vkDestroyImageView( m_platform.device, imageView, nullptr );
}

auto graphics::VulkanDevice::destroyImageViews( std::initializer_list<VkImageView> imageViews ) const -> void
{
  for ( auto imgView : imageViews )
  {
    destroyImageView( imgView );
  }
}

auto graphics::VulkanDevice::findQueueFamilies( VkPhysicalDevice& device ) -> QueueFamilyIndices
{
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount{ 0u };
  vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, nullptr );
  std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
  vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilies.data() );
  int i = 0;
  for ( const auto& queueFamily : queueFamilies )
  {
    if ( queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT )
    {
      indices.transferFamily = i;
      m_transferQueue.familyIndex = i;
    }

    // we need to find at least one queue family that supports the graphics bit
    if ( queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
    {
      indices.graphicsFamily = i;
      m_graphicsQueue.familyIndex = i;
    }

    VkBool32 presentSupport{ false };
    vkGetPhysicalDeviceSurfaceSupportKHR( device, i, m_platform.surface, &presentSupport );

    if ( presentSupport )
    {
      indices.presentFamily = i;
      m_presentQueue.familyIndex = i;
    }

    // early exit if we already found a family
    if ( indices.isComplete() )
      break;

    i++;
  }
  return indices;
}

bool graphics::VulkanDevice::isDeviceSuitable( VkPhysicalDevice& device )
{
  // Get the device features and properties
  vkGetPhysicalDeviceProperties( device, &m_physicalDeviceProps );

  // Assign the limits to get for future stuff
  m_physicalDeviceLimits = m_physicalDeviceProps.limits;

  vkGetPhysicalDeviceFeatures( device, &m_physicalDeviceFeatures );
  vkGetPhysicalDeviceMemoryProperties( device, &m_physicalDeviceMemoryProps );
  auto indices = findQueueFamilies( device );
  auto extensionsSupported = checkDeviceExtensionSupport( device );

  if ( m_physicalDeviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
       m_physicalDeviceFeatures.geometryShader && indices.isComplete() && extensionsSupported )
    return true;

  return false;
}

auto graphics::VulkanDevice::getRequiredExtensions() -> std::vector<const char*>
{
  uint32_t extensionCount{ 0u };
  SDL_Vulkan_GetInstanceExtensions( m_window, &extensionCount, nullptr );
  K_INFO( "extension count from SDL_Vulkan_GetInstanceExtensions {}", extensionCount );
  std::vector<const char*> extensions( extensionCount );
  SDL_Vulkan_GetInstanceExtensions( m_window, &extensionCount, extensions.data() );

  if ( enableValidationLayers )
    extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

  return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                     void* pUserData )
{
  /*
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
  VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
   */
  switch ( messageSeverity )
  {
  case 1 << 0:
  case 1 << 4:
    K_INFO( "{}", pCallbackData->pMessage );
    break;
  case 1 << 8:
    K_WARN( "{}", pCallbackData->pMessage );
    break;
  case 1 << 12:
    K_ERROR( "{}", pCallbackData->pMessage );
    break;
  }

  return VK_FALSE;
}

auto graphics::VulkanDevice::checkValidationLayerSupport() -> bool
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

  std::vector<VkLayerProperties> availableLayers( layerCount );
  vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );

  for ( const char* layerName : validationLayers )
  {
    bool layerFound = false;

    for ( const auto& layerProperties : availableLayers )
    {
      if ( strcmp( layerName, layerProperties.layerName ) == 0 )
      {
        layerFound = true;
        break;
      }
    }

    if ( !layerFound )
    {
      return false;
    }
  }

  return true;
}

void graphics::VulkanDevice::shutdown() const
{
  // printLeaks();
  waitIdle();

  destroyCommandPool( m_commandPool );
  destroyCommandPool( m_transferCommandPool );

  vmaDestroyAllocator( m_allocator );
  destroyDebugUtilsMessengerEXT( m_platform.instance, m_debugMessenger, nullptr );
  vkDestroyDevice( m_platform.device, nullptr );
  vkDestroySurfaceKHR( m_platform.instance, m_platform.surface, nullptr );
  vkDestroyInstance( m_platform.instance, nullptr );
}

auto graphics::VulkanDevice::waitIdle() const -> void
{
  vkDeviceWaitIdle( m_platform.device );
}

auto graphics::VulkanDevice::getTransferQueue() -> GPUQueue&
{
  return m_transferQueue;
}

auto graphics::VulkanDevice::getGraphicsQueue() -> GPUQueue&
{
  return m_graphicsQueue;
}

auto graphics::VulkanDevice::getPresentQueue() -> GPUQueue&
{
  return m_presentQueue;
}

auto graphics::VulkanDevice::getPhysicalDevice() const -> VkPhysicalDevice
{
  return m_platform.physicalDevice;
}

auto graphics::VulkanDevice::getLogicalDevice() const -> VkDevice
{
  return m_platform.device;
}

auto graphics::VulkanDevice::getSurface() const -> VkSurfaceKHR
{
  return m_platform.surface;
}

graphics::VulkanDevice::VulkanDevice( SDL_Window* wnd )
    : m_window{ wnd }
{
  init();
  m_shaderCompiler = std::make_unique<ShaderCompiler>( m_platform.device );
}

graphics::VulkanDevice::~VulkanDevice()
{
  shutdown();
}

void graphics::VulkanDevice::init()
{
  K_INFO( "initializing device" );
  createInstance();
  setupDebug();
  createWindowSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  initMemoryAllocator();
  createDeviceCommandPool();
  K_INFO( "vulkan device initialized" );
}

auto graphics::VulkanDevice::createDeviceCommandPool() -> void
{
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = getGraphicsQueue().familyIndex;
  createCommandPool( poolInfo, m_commandPool );

  VkCommandPoolCreateInfo transferPoolInfo{};
  transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  transferPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  transferPoolInfo.queueFamilyIndex = getTransferQueue().familyIndex;
  createCommandPool( transferPoolInfo, m_transferCommandPool );
}

auto graphics::VulkanDevice::initMemoryAllocator() -> void
{
  VmaVulkanFunctions vulkanFunctions{};
  vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
  vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo allocatorCreateInfo{};
  allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
  allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
  allocatorCreateInfo.physicalDevice = m_platform.physicalDevice;
  allocatorCreateInfo.device = m_platform.device;
  allocatorCreateInfo.instance = m_platform.instance;
  allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

  VK_CALL( vmaCreateAllocator( &allocatorCreateInfo, &m_allocator ) );
}

void graphics::VulkanDevice::setupDebug()
{
  if ( !enableValidationLayers )
    return;

  VkDebugUtilsMessengerCreateInfoEXT info{};
  info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  info.pfnUserCallback = debugCallback;
  info.pNext = nullptr;

  VK_CALL( createDebugUtilsMessengerEXT( m_platform.instance, &info, nullptr, &m_debugMessenger ) );
}

void graphics::VulkanDevice::createInstance()
{
  if ( enableValidationLayers && !checkValidationLayerSupport() )
  {
    throw std::runtime_error( "validation layers requested but not available!!!" );
  }
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
  appInfo.apiVersion = VK_API_VERSION_1_4;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>( extensions.size() );
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT info{};
  if ( enableValidationLayers )
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>( validationLayers.size() );
    createInfo.ppEnabledLayerNames = validationLayers.data();

    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debugCallback;
    info.pNext = nullptr;
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&info;
  }
  else
  {
    createInfo.enabledLayerCount = 0;

    createInfo.pNext = nullptr;
  }

  for ( auto& extension : extensions )
  {
    K_INFO( "extension: {}", extension );
  }

  VK_CALL( vkCreateInstance( &createInfo, nullptr, &m_platform.instance ) );
}

void graphics::VulkanDevice::createWindowSurface()
{
  if ( SDL_Vulkan_CreateSurface( m_window, m_platform.instance, &m_platform.surface ) != SDL_TRUE )
  {
    throw std::runtime_error( "could not create surface" );
  }
}

void graphics::VulkanDevice::pickPhysicalDevice()
{
  // this should choose the physical device (GPU) we are going to use for the rendering
  uint32_t deviceCount{ 0u };
  vkEnumeratePhysicalDevices( m_platform.instance, &deviceCount, nullptr );

  // check to see if we have at leas one vulkan supporting gpu
  if ( deviceCount == 0 )
  {
    throw std::runtime_error( "failed to find GPU with Vulkan support, i'm sorry" );
  }
  std::vector<VkPhysicalDevice> devices( deviceCount );
  vkEnumeratePhysicalDevices( m_platform.instance, &deviceCount, devices.data() );

  // we will implement a multimap with device scores later
  // the score is based on properties and features of the gpu, the more features and
  // the better properties it has, the higher the score
  for ( auto& device : devices )
  {
    if ( isDeviceSuitable( device ) )
    {
      VkPhysicalDeviceProperties props;
      vkGetPhysicalDeviceProperties( device, &props );
      K_INFO( "Physical device found: {}", props.deviceName );
      m_platform.physicalDevice = device;
      break;
    }
  }
}

void graphics::VulkanDevice::createLogicalDevice()
{
  // get the indices of the phisycal device we picked earlier
  QueueFamilyIndices indices = findQueueFamilies( m_platform.physicalDevice );

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

  float queuePriority = 1.0f;
  for ( uint32_t queueFamily : uniqueQueueFamilies )
  {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back( queueCreateInfo );
  }

  // features
  VkPhysicalDeviceVulkan12Features features12{};
  features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  features12.runtimeDescriptorArray = VK_TRUE;
  features12.descriptorIndexing = VK_TRUE;
  // features12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
  features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
  features12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
  features12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
  features12.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
  features12.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
  features12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
  features12.descriptorBindingPartiallyBound = VK_TRUE;
  features12.descriptorBindingVariableDescriptorCount = VK_TRUE;

  features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
  features12.timelineSemaphore = VK_TRUE;
  features12.bufferDeviceAddress = VK_TRUE;

  VkPhysicalDeviceVulkan13Features features13{};
  features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  features13.synchronization2 = true;
  features13.dynamicRendering = true;
  features13.descriptorBindingInlineUniformBlockUpdateAfterBind = true;
  features13.pNext = &features12;

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.geometryShader = VK_TRUE;
  deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

  // for wireframe pipeline
  deviceFeatures.fillModeNonSolid = VK_TRUE;
  // this is too for the wireframe, without this we cannot change the line width
  deviceFeatures.wideLines = VK_TRUE;

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = &features13;
  createInfo.queueCreateInfoCount = queueCreateInfos.size();
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = deviceExtensions.size();
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  // enable validation layers
  if ( enableValidationLayers )
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>( validationLayers.size() );
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  if ( vkCreateDevice( m_platform.physicalDevice, &createInfo, nullptr, &m_platform.device ) != VK_SUCCESS )
  {
    throw std::runtime_error( "could not create logical device" );
  }

  vkGetDeviceQueue( m_platform.device, indices.graphicsFamily.value(), 0, &m_graphicsQueue.handle );
  vkGetDeviceQueue( m_platform.device, indices.presentFamily.value(), 0, &m_presentQueue.handle );
  vkGetDeviceQueue( m_platform.device, indices.transferFamily.value(), 0, &m_transferQueue.handle );
}

auto graphics::VulkanDevice::getInstance() const -> VkInstance
{
  return m_platform.instance;
}

auto graphics::VulkanDevice::getLimits() -> VkPhysicalDeviceLimits&
{
  return m_physicalDeviceLimits;
}

auto graphics::VulkanDevice::setDebugName( VkObjectType objType, uint64_t handle, std::string_view name ) -> void
{
#ifndef _DEBUG
  return;
#endif

  if ( name.empty() )
  {
    return;
  }

  if ( m_setDebugNameFunc == VK_NULL_HANDLE )
  {
    m_setDebugNameFunc =
      (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr( m_platform.device, "vkSetDebugUtilsObjectNameEXT" );
  }

  const std::string nullTerminatedName( name );

  VkDebugUtilsObjectNameInfoEXT nameInfo{};
  nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
  nameInfo.objectType = objType;
  nameInfo.objectHandle = handle;
  nameInfo.pObjectName = nullTerminatedName.c_str();

  m_setDebugNameFunc( m_platform.device, &nameInfo );
}

void graphics::VulkanDevice::createBuffer( VulkanBuffer& vulkanBuffer,
                                           VkBufferCreateInfo& createInfo,
                                           VmaAllocationCreateInfo& usage,
                                           std::string_view bufferName )
{
  VK_CALL(
    vmaCreateBuffer( m_allocator, &createInfo, &usage, &vulkanBuffer.vkBuffer, &vulkanBuffer.vmaAllocation, nullptr ) );

  if ( !bufferName.empty() )
  {
    K_INFO( "[BUFF_ALLOC] {} {}", bufferName, formatSize( static_cast<double>( createInfo.size ) ) );
    setName( bufferName, vulkanBuffer.vmaAllocation );
    setDebugName( vulkanTypeToObject<VkBuffer>(), reinterpret_cast<uint64_t>( vulkanBuffer.vkBuffer ), bufferName );
  }
  else
  {

    K_INFO( "[BUFF_ALLOC] {}", formatSize( static_cast<double>( createInfo.size ) ) );
  }
}

void graphics::VulkanDevice::createBuffer( FrameInFlightVulkanBuffer& vulkanBuffer,
                                           VkBufferCreateInfo& createInfo,
                                           VmaAllocationCreateInfo& usage,
                                           std::string_view name )
{
  uint32_t index{ 0u };
  for ( auto& buffer : vulkanBuffer.buffers )
  {
    std::string buffName = std::string{ name } + std::to_string( index++ );
    createBuffer( buffer, createInfo, usage, buffName );
  }
}

void graphics::VulkanDevice::createImage( VkImage& image,
                                          VkImageCreateInfo& imageCreateInfo,
                                          VmaAllocationCreateInfo& usage,
                                          VmaAllocation& allocation,
                                          std::string_view imageName )
{
  auto imgExtent = imageCreateInfo.extent;
  auto mipLevels =
    static_cast<uint32_t>( std::floor( std::log2( std::max( imgExtent.width, imgExtent.height ) ) ) ) + 1;

  imageCreateInfo.mipLevels = mipLevels;

  VK_CALL( vmaCreateImage( m_allocator, &imageCreateInfo, &usage, &image, &allocation, nullptr ) );

  std::string size = formatSize( static_cast<double>( allocation->GetSize() ) );
  if ( !imageName.empty() )
  {
    K_INFO( "[IMG_ALLOC] {} size {}", imageName, size );
    setName( imageName, allocation );
    setDebugName( vulkanTypeToObject<VkImage>(), reinterpret_cast<uint64_t>( image ), imageName );
  }
  else
  {
    K_INFO( "[IMG_ALLOC] size {}", size );
  }
}

void graphics::VulkanDevice::createImage( VulkanImage& image,
                                          VkImageCreateInfo& imageCreateInfo,
                                          VmaAllocationCreateInfo& usage,
                                          std::string_view imageName )
{
  createImage( image.vkImage, imageCreateInfo, usage, image.vmaAllocation, imageName );
}

auto graphics::VulkanDevice::getAllocator() -> VmaAllocator&
{
  return m_allocator;
}

auto graphics::VulkanDevice::createStagingBuffer( VkBufferCreateInfo& createInfo, VmaAllocationCreateInfo& usage )
  -> VulkanBuffer
{
  VulkanBuffer stageBuffer{};
  createBuffer( stageBuffer, createInfo, usage );
  return stageBuffer;
}

auto graphics::VulkanDevice::formatSize( VkDeviceSize size ) -> std::string
{
  const double KB = 1024.0;
  const double MB = KB * 1024.0;
  const double GB = MB * 1024.0;

  std::ostringstream oss;
  oss << std::fixed << std::setprecision( 2 );

  if ( size >= GB )
    oss << ( size / GB ) << " GB";
  else if ( size >= MB )
    oss << ( size / MB ) << " MB";
  else if ( size >= KB )
    oss << ( size / KB ) << " KB";
  else
    oss << size << " B";

  return oss.str();
}

auto graphics::VulkanDevice::invalidateAllocation( VulkanBuffer& vulkanBuffer,
                                                   VkDeviceSize offset,
                                                   VkDeviceSize size ) const -> void
{
  vmaInvalidateAllocation( m_allocator, vulkanBuffer.vmaAllocation, offset, size );
}

auto graphics::VulkanDevice::destroyImage( VkImage& image, VmaAllocation& allocation ) -> void
{
  if ( image == VK_NULL_HANDLE )
    return;

  auto info = getAllocInfo( allocation );
  if ( info.pName )
  {
    K_INFO( "[IMG_DEALLOC] {}, size {}", info.pName, formatSize( info.size ) );
  }

  vmaDestroyImage( m_allocator, image, allocation );
}

auto graphics::VulkanDevice::destroyImages( const std::initializer_list<std::tuple<VkImage&, VmaAllocation&>>& images )
  -> void
{
  for ( auto& [image, alloc] : images )
  {
    destroyImage( image, alloc );
  }
}

auto graphics::VulkanDevice::printLeaks() const -> void
{
  char* statsString = nullptr;
  VmaAllocatorCreateInfo info{};
  vmaBuildStatsString( m_allocator, &statsString, VK_TRUE );
  printf_s( "%s", statsString );
  // K_INFO( statsString );
  vmaFreeStatsString( m_allocator, statsString );
}

auto graphics::VulkanDevice::setName( std::string_view name, VmaAllocation& allocation ) const -> void
{
#ifndef _DEBUG
  return;
#endif

  auto allocName = std::string{ name };
  vmaSetAllocationName( m_allocator, allocation, allocName.c_str() );
}

auto graphics::VulkanDevice::destroyBuffer( VulkanBuffer& buff ) -> void
{
  auto info = getAllocInfo( buff.vmaAllocation );

  if ( info.pName )
  {
    K_INFO( "[BUFF_DEALLOC] {}, size {}", info.pName, formatSize( info.size ) );
  }
  else
  {
    K_INFO( "[BUFF_DEALLOC] size {}", formatSize( info.size ) );
  }

  vmaDestroyBuffer( m_allocator, buff.vkBuffer, buff.vmaAllocation );
}

auto graphics::VulkanDevice::destroyBuffer( FrameInFlightVulkanBuffer& buff ) -> void
{
  for ( auto& buff : buff.buffers )
  {
    destroyBuffer( buff );
  }
}

auto graphics::VulkanDevice::getAllocInfo( VmaAllocation allocation ) const -> VmaAllocationInfo
{
  VmaAllocationInfo info{};
  vmaGetAllocationInfo( m_allocator, allocation, &info );
  return info;
}

auto graphics::VulkanDevice::destroyPipeline( VkPipeline pipeline ) const -> void
{
  if ( pipeline == VK_NULL_HANDLE )
    return;

  vkDestroyPipeline( m_platform.device, pipeline, nullptr );
}

auto graphics::VulkanDevice::destroyPipelineLayout( VkPipelineLayout layout ) const -> void
{
  if ( layout == VK_NULL_HANDLE )
    return;

  vkDestroyPipelineLayout( m_platform.device, layout, nullptr );
}

auto graphics::VulkanDevice::destroyDescriptorPool( VkDescriptorPool pool ) const -> void
{
  if ( pool == VK_NULL_HANDLE )
    return;

  vkDestroyDescriptorPool( m_platform.device, pool, nullptr );
}

auto graphics::VulkanDevice::destroySampler( VkSampler sampler ) const -> void
{
  if ( sampler == VK_NULL_HANDLE )
    return;

  vkDestroySampler( m_platform.device, sampler, nullptr );
}

auto graphics::VulkanDevice::destroyDescriptorSetLayout( VkDescriptorSetLayout layout ) const -> void
{
  if ( layout == VK_NULL_HANDLE )
    return;

  vkDestroyDescriptorSetLayout( m_platform.device, layout, nullptr );
}

auto graphics::VulkanDevice::destroyCommandPool( VkCommandPool pool ) const -> void
{
  vkDestroyCommandPool( m_platform.device, pool, nullptr );
}

auto graphics::VulkanDevice::destroySemaphore( VkSemaphore semaphore ) const -> void
{
  vkDestroySemaphore( m_platform.device, semaphore, nullptr );
}

auto graphics::VulkanDevice::destroyFence( VkFence fence ) const -> void
{
  vkDestroyFence( m_platform.device, fence, nullptr );
}

auto graphics::VulkanDevice::destroySwapchain( VkSwapchainKHR swapchain ) const -> void
{
  vkDestroySwapchainKHR( m_platform.device, swapchain, nullptr );
}

auto graphics::VulkanDevice::createSwapchain( VkSwapchainCreateInfoKHR info, VkSwapchainKHR* swapchain ) const -> void
{
  VK_CALL( vkCreateSwapchainKHR( m_platform.device, &info, nullptr, swapchain ) );
}

auto graphics::VulkanDevice::getSwapchainImagesKHR( std::vector<VulkanImage>& swapchainImages,
                                                    VkSwapchainKHR swapchain,
                                                    uint32_t& imageCount ) const -> void
{
  VK_CALL( vkGetSwapchainImagesKHR( m_platform.device, swapchain, &imageCount, nullptr ) );
  std::vector<VkImage> images;
  images.resize( imageCount );
  VK_CALL( vkGetSwapchainImagesKHR( m_platform.device, swapchain, &imageCount, images.data() ) );

  swapchainImages.clear();
  swapchainImages.resize( imageCount );
  for ( auto i = 0u; i < imageCount; i++ )
  {
    swapchainImages.at( i ).vkImage = std::move( images.at( i ) );
  }
}

auto graphics::VulkanDevice::beginSingleTimeCommands() const -> VkCommandBuffer
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = m_commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  VK_CALL( vkAllocateCommandBuffers( m_platform.device, &allocInfo, &commandBuffer ) );

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CALL( vkBeginCommandBuffer( commandBuffer, &beginInfo ) );

  return commandBuffer;
}

auto graphics::VulkanDevice::beginSingleTimeCommands( VkCommandPool pool ) const -> VkCommandBuffer
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = pool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  VK_CALL( vkAllocateCommandBuffers( m_platform.device, &allocInfo, &commandBuffer ) );

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CALL( vkBeginCommandBuffer( commandBuffer, &beginInfo ) );

  return commandBuffer;
}

auto graphics::VulkanDevice::createFence( VkFence& fence, VkFenceCreateInfo info ) const -> void
{
  VK_CALL( vkCreateFence( m_platform.device, &info, nullptr, &fence ) );
}

// auto graphics::VulkanDevice::endSingleTimeCommands( VkCommandBuffer commandBuffer, VkQueue queue, VkFence fence )
// const
//   -> void
//{
//   vkEndCommandBuffer( commandBuffer );
//
//   VkSubmitInfo submitInfo{};
//   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//   submitInfo.commandBufferCount = 1;
//   submitInfo.pCommandBuffers = &commandBuffer;
//
//   VK_CALL( vkQueueSubmit( queue, 1, &submitInfo, fence ) );
//
//   vkWaitForFences( m_platform.device, 1, &fence, VK_TRUE, UINT64_MAX );
//   vkResetFences( m_platform.device, 1, &fence );
//
//   vkFreeCommandBuffers( m_platform.device, m_commandPool, 1, &commandBuffer );
// }

auto graphics::VulkanDevice::getCommandPool() -> VkCommandPool
{
  return m_commandPool;
}

auto graphics::VulkanDevice::getTransferCommandPool() -> VkCommandPool
{
  return m_transferCommandPool;
}

auto graphics::VulkanDevice::endSingleTimeCommands( VkCommandBuffer commandBuffer,
                                                    VkQueue queue,
                                                    VkSubmitInfo submitInfo,
                                                    VkCommandPool pool ) const -> void
{
  vkEndCommandBuffer( commandBuffer );

  VK_CALL( vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE ) );
  VK_CALL( vkQueueWaitIdle( queue ) );

  vkFreeCommandBuffers( m_platform.device, pool, 1, &commandBuffer );
}

auto graphics::VulkanDevice::endSingleTimeCommands( VkCommandBuffer commandBuffer,
                                                    VkQueue queue,
                                                    VkCommandPool pool ) const -> void
{
  vkEndCommandBuffer( commandBuffer );

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  VK_CALL( vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE ) );
  VK_CALL( vkQueueWaitIdle( queue ) );

  vkFreeCommandBuffers( m_platform.device, pool, 1, &commandBuffer );
}

auto graphics::VulkanDevice::copyBuffer( VkBuffer src, VkBuffer dst, VkDeviceSize size ) const -> void
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands( m_transferCommandPool );

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer( commandBuffer, src, dst, 1, &copyRegion );

  endSingleTimeCommands( commandBuffer, m_transferQueue.handle, m_transferCommandPool );
}

auto graphics::VulkanDevice::createSampler( VkSampler& sampler ) const -> void
{
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties( m_platform.physicalDevice, &properties );

  VkSamplerCreateInfo samplerInfo{
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = VK_FILTER_NEAREST,
    .minFilter = VK_FILTER_NEAREST,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    //.anisotropyEnable = VK_FALSE,
    //.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
    .compareEnable = VK_FALSE,
    //.compareOp = VK_COMPARE_OP_ALWAYS,
    //.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    //.unnormalizedCoordinates = VK_FALSE,
  };

  VK_CALL( vkCreateSampler( m_platform.device, &samplerInfo, nullptr, &sampler ) );
}

auto graphics::VulkanDevice::createImageView( VkImageView& imageView,
                                              VkImage& image,
                                              VkFormat format,
                                              VkImageAspectFlags aspectFlags,
                                              std::string_view imageViewName ) -> void
{
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VK_CALL( vkCreateImageView( m_platform.device, &viewInfo, nullptr, &imageView ) );

  setDebugName( vulkanTypeToObject<VkImageView>(), reinterpret_cast<uint64_t>( imageView ), imageViewName );
}

auto graphics::VulkanDevice::transitionImageLayout( VkImage image, VkImageMemoryBarrier2 imageBarrier ) -> void
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands( m_commandPool );

  VkImageAspectFlags aspect{ VK_IMAGE_ASPECT_COLOR_BIT };
  if ( imageBarrier.oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL )
  {
    aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
  }

  VkDependencyInfo transferDepInfo{
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &imageBarrier };

  vkCmdPipelineBarrier2( commandBuffer, &transferDepInfo );

  endSingleTimeCommands( commandBuffer, getGraphicsQueue().handle, m_commandPool );
}

auto graphics::VulkanDevice::transitionImageLayout( VulkanImage image, VkImageMemoryBarrier2 imageBarrier ) -> void
{
  transitionImageLayout( image.vkImage, imageBarrier );
}

auto graphics::VulkanDevice::transitionImageLayout( VkImage image,
                                                    VkCommandBuffer cmdBuffer,
                                                    VkImageMemoryBarrier2 imageBarrier ) -> void
{

  VkDependencyInfo transferDepInfo{
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &imageBarrier };

  vkCmdPipelineBarrier2( cmdBuffer, &transferDepInfo );
}

auto graphics::VulkanDevice::transitionImageLayout( VulkanImage image,
                                                    VkCommandBuffer cmdBuffer,
                                                    VkImageMemoryBarrier2 imageBarrier ) -> void
{
  transitionImageLayout( image.vkImage, cmdBuffer, imageBarrier );
}

auto graphics::VulkanDevice::copyBufferToImage(
  VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkImageMemoryBarrier2 imageBarrier ) -> void
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands( m_transferCommandPool );

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = { width, height, 1 };

  vkCmdCopyBufferToImage( commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

  VkDependencyInfo dependency{
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .imageMemoryBarrierCount = 1,
    .pImageMemoryBarriers = &imageBarrier,
  };

  vkCmdPipelineBarrier2( commandBuffer, &dependency );

  endSingleTimeCommands( commandBuffer, getTransferQueue().handle, m_transferCommandPool );
}

auto graphics::VulkanDevice::copyImageToBuffer( VkImage image, VkBuffer buffer, VkOffset3D offset, VkExtent3D extent )
  -> void
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands( m_transferCommandPool );

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = offset;
  region.imageExtent = extent;

  VkImageMemoryBarrier2 textureToColor{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                                        .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                        .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
                                        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                        .image = image,
                                        .subresourceRange = {
                                          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                          .baseMipLevel = 0,
                                          .levelCount = 1,
                                          .baseArrayLayer = 0,
                                          .layerCount = 1,
                                        } };

  VkDependencyInfo dependency{
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .imageMemoryBarrierCount = 1,
    .pImageMemoryBarriers = &textureToColor,
  };

  vkCmdPipelineBarrier2( commandBuffer, &dependency );

  vkCmdCopyImageToBuffer( commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region );

  endSingleTimeCommands( commandBuffer, getTransferQueue().handle, m_transferCommandPool );
}

auto graphics::VulkanDevice::copyImageToBuffer(
  VkImage image, VkCommandBuffer cmdBuffer, VkBuffer buffer, VkOffset3D offset, VkExtent3D extent, bool applyBarrier )
  -> void
{
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = offset;
  region.imageExtent = extent;

  if ( applyBarrier )
  {

    VkImageMemoryBarrier2 textureToColor{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                          .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                          .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                                          .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                          .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
                                          .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                          .image = image,
                                          .subresourceRange = {
                                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                            .baseMipLevel = 0,
                                            .levelCount = 1,
                                            .baseArrayLayer = 0,
                                            .layerCount = 1,
                                          } };

    VkDependencyInfo dependency{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &textureToColor,
    };
    vkCmdPipelineBarrier2( cmdBuffer, &dependency );
  }

  vkCmdCopyImageToBuffer( cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region );
}

auto graphics::VulkanDevice::createCommandPool( VkCommandPoolCreateInfo createInfo, VkCommandPool& pool ) const -> void
{
  vkCreateCommandPool( m_platform.device, &createInfo, nullptr, &pool );
}

auto graphics::VulkanDevice::createShaderModule( const std::string& shaderName, const std::string& shaderEntryFunc )
  -> VkShaderModule
{
  return m_shaderCompiler->createShaderModule( shaderName, shaderEntryFunc );
}

auto graphics::VulkanDevice::destroyShaderModule( VkShaderModule module ) -> void
{
  vkDestroyShaderModule( m_platform.device, module, nullptr );
}

auto graphics::VulkanDevice::createPipelineLayout( VkPipelineLayoutCreateInfo layout,
                                                   VkPipelineLayout& pipelineLayout ) const -> void
{
  VK_CALL( vkCreatePipelineLayout( m_platform.device, &layout, nullptr, &pipelineLayout ) );
}

auto graphics::VulkanDevice::createGraphicsPipeline( VkPipeline& pipeline,
                                                     VkGraphicsPipelineCreateInfo createInfo ) const -> void
{
  VK_CALL( vkCreateGraphicsPipelines( m_platform.device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline ) );
}

auto graphics::VulkanDevice::findSupportedFormat( VkPhysicalDevice device,
                                                  const std::vector<VkFormat>& candidates,
                                                  VkImageTiling tiling,
                                                  VkFormatFeatureFlags features ) -> VkFormat
{
  for ( VkFormat format : candidates )
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties( device, format, &props );

    if ( tiling == VK_IMAGE_TILING_LINEAR && ( props.linearTilingFeatures & features ) == features )
    {
      return format;
    }
    else if ( tiling == VK_IMAGE_TILING_OPTIMAL && ( props.optimalTilingFeatures & features ) == features )
    {
      return format;
    }
  }

  throw std::runtime_error( "failed to find supported format!" );
}

auto graphics::VulkanDevice::findDepthFormat( VkPhysicalDevice device ) -> VkFormat
{
  return findSupportedFormat( device,
                              { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                              VK_IMAGE_TILING_OPTIMAL,
                              VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
}

auto graphics::VulkanDevice::getDeviceProperties() const -> VkPhysicalDeviceProperties
{
  return m_physicalDeviceProps;
}

auto graphics::VulkanDevice::getDeviceMemoryProperties() const -> VkPhysicalDeviceMemoryProperties
{
  return m_physicalDeviceMemoryProps;
}

auto graphics::VulkanDevice::createTimelineSemaphore( VkSemaphore& timeline,
                                                      VkSemaphoreTypeCreateInfo info,
                                                      VkSemaphoreCreateInfo createInfo ) const -> void
{
  VK_CALL( vkCreateSemaphore( m_platform.device, &createInfo, NULL, &timeline ) );
}

auto graphics::VulkanDevice::cmdPipelineBarrier2( VkCommandBuffer cmdBuffer, VkDependencyInfo dependencyInfo ) -> void
{
  vkCmdPipelineBarrier2( cmdBuffer, &dependencyInfo );
}
