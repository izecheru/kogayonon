#include "graphics/vulkan_device.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <set>
#include <spdlog/spdlog.h>
#include <stdexcept>

void DestroyDebugUtilsMessengerEXT( VkInstance instance,
                                    VkDebugUtilsMessengerEXT debugMessenger,
                                    const VkAllocationCallbacks* pAllocator )
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
  if ( func != nullptr )
  {
    func( instance, debugMessenger, pAllocator );
  }
}

VkResult CreateDebugUtilsMessengerEXT( VkInstance instance,
                                       const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator,
                                       VkDebugUtilsMessengerEXT* pDebugMessenger )
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

bool checkDeviceExtensionSupport( VkPhysicalDevice& device )
{
  // get all the extensions of the device
  uint32_t extensionCount{ 0u };
  vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );
  std::vector<VkExtensionProperties> availableExtensions( extensionCount );
  vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data() );

  // fill the set with the global deviceExtensions
  std::set<std::string> requiredExtensions( deviceExtensions.begin(), deviceExtensions.end() );

  // if the extension of the device is present in the set, or the extensions of the device contain the global
  // extension too, it will get erased and check if the required is empty
  for ( const auto& extension : availableExtensions )
  {
    requiredExtensions.erase( extension.extensionName );
  }

  return requiredExtensions.empty();
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
  // get the device features and properties
  vkGetPhysicalDeviceProperties( device, &m_physicalDeviceProps );
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
  spdlog::info( extensionCount );
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
    spdlog::info( "---VK_INFO---" );
    spdlog::info( pCallbackData->pMessage );
    spdlog::info( "------\n" );
    break;
  case 1 << 8:
    spdlog::warn( "---VK_WARN---" );
    spdlog::warn( pCallbackData->pMessage );
    spdlog::warn( "------\n" );
    break;
  case 1 << 12:
    spdlog::error( "---VK_ERR---" );
    spdlog::error( pCallbackData->pMessage );
    spdlog::error( "------\n" );
    break;
  }

  return VK_FALSE;
}

bool checkValidationLayerSupport()
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

void populateDebugMessengerCreateInfo( VkDebugUtilsMessengerCreateInfoEXT& info )
{
  info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  info.pfnUserCallback = debugCallback;
  info.pNext = nullptr;
}

void graphics::VulkanDevice::shutdown()
{
  vkDestroyDevice( m_platform.device, nullptr );
  vkDestroySurfaceKHR( m_platform.instance, m_platform.surface, nullptr );
  vkDestroyInstance( m_platform.instance, nullptr );
}

auto graphics::VulkanDevice::getGraphicsQueue() -> GPUQueue&
{
  return m_graphicsQueue;
}

auto graphics::VulkanDevice::getPresentQueue() -> GPUQueue&
{
  return m_presentQueue;
}

auto graphics::VulkanDevice::getPhysicalDevice() -> VkPhysicalDevice&
{
  return m_platform.physicalDevice;
}

auto graphics::VulkanDevice::getLogicalDevice() -> VkDevice&
{
  return m_platform.device;
}

auto graphics::VulkanDevice::getSurface() -> VkSurfaceKHR&
{
  return m_platform.surface;
}

graphics::VulkanDevice::VulkanDevice( SDL_Window* wnd )
    : m_window{ wnd }
{
  init();
}

void graphics::VulkanDevice::init()
{
  spdlog::info( "initializing device" );
  createInstance();
  setupDebug();
  createWindowSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  spdlog::info( "vulkan device initialized" );
}

void graphics::VulkanDevice::setupDebug()
{
  if ( !enableValidationLayers )
    return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  populateDebugMessengerCreateInfo( createInfo );
  if ( CreateDebugUtilsMessengerEXT( m_platform.instance, &createInfo, nullptr, &m_debugMessenger ) != VK_SUCCESS )
  {
    throw std::runtime_error( "failed to set up debug messenger!" );
  }
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

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if ( enableValidationLayers )
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>( validationLayers.size() );
    createInfo.ppEnabledLayerNames = validationLayers.data();

    populateDebugMessengerCreateInfo( debugCreateInfo );
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  }
  else
  {
    createInfo.enabledLayerCount = 0;

    createInfo.pNext = nullptr;
  }

  for ( auto& extension : extensions )
  {
    spdlog::info( "extension: {}", extension );
  }

  if ( vkCreateInstance( &createInfo, nullptr, &m_platform.instance ) != VK_SUCCESS )
  {
    throw std::runtime_error( "cannot create instance" );
  }
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
      spdlog::info( "found {}", props.deviceName );
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

  // specify the features
  VkPhysicalDeviceFeatures deviceFeatures{};
  VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
  dynamicRendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
  dynamicRendering.dynamicRendering = VK_TRUE;

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = &dynamicRendering;
  createInfo.queueCreateInfoCount = queueCreateInfos.size();
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount = 1;
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
}

auto graphics::VulkanDevice::getInstance() -> VkInstance&
{
  return m_platform.instance;
}
