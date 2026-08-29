#pragma once
#include <vulkan/vulkan.h>

#include "precompiled/pch.hpp"
#include "utilities/utils/utils.hpp"

inline auto formatSize( VkDeviceSize size ) -> std::string
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

constexpr const char* vkResultToString( VkResult result )
{
  switch ( result )
  {
  case VK_SUCCESS:
    return "VK_SUCCESS";
  case VK_NOT_READY:
    return "VK_NOT_READY";
  case VK_TIMEOUT:
    return "VK_TIMEOUT";
  case VK_EVENT_SET:
    return "VK_EVENT_SET";
  case VK_EVENT_RESET:
    return "VK_EVENT_RESET";
  case VK_INCOMPLETE:
    return "VK_INCOMPLETE";
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    return "VK_ERROR_OUT_OF_HOST_MEMORY";
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
  case VK_ERROR_INITIALIZATION_FAILED:
    return "VK_ERROR_INITIALIZATION_FAILED";
  case VK_ERROR_DEVICE_LOST:
    return "VK_ERROR_DEVICE_LOST";
  case VK_ERROR_MEMORY_MAP_FAILED:
    return "VK_ERROR_MEMORY_MAP_FAILED";
  case VK_ERROR_LAYER_NOT_PRESENT:
    return "VK_ERROR_LAYER_NOT_PRESENT";
  case VK_ERROR_EXTENSION_NOT_PRESENT:
    return "VK_ERROR_EXTENSION_NOT_PRESENT";
  case VK_ERROR_FEATURE_NOT_PRESENT:
    return "VK_ERROR_FEATURE_NOT_PRESENT";
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "VK_ERROR_INCOMPATIBLE_DRIVER";
  case VK_ERROR_TOO_MANY_OBJECTS:
    return "VK_ERROR_TOO_MANY_OBJECTS";
  case VK_ERROR_FORMAT_NOT_SUPPORTED:
    return "VK_ERROR_FORMAT_NOT_SUPPORTED";
  case VK_ERROR_FRAGMENTED_POOL:
    return "VK_ERROR_FRAGMENTED_POOL";
  case VK_ERROR_UNKNOWN:
    return "VK_ERROR_UNKNOWN";
  case VK_ERROR_VALIDATION_FAILED:
    return "VK_ERROR_VALIDATION_FAILED";
  case VK_ERROR_OUT_OF_POOL_MEMORY:
    return "VK_ERROR_OUT_OF_POOL_MEMORY";
  case VK_ERROR_INVALID_EXTERNAL_HANDLE:
    return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
  case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
    return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
  case VK_ERROR_FRAGMENTATION:
    return "VK_ERROR_FRAGMENTATION";
  case VK_PIPELINE_COMPILE_REQUIRED:
    return "VK_PIPELINE_COMPILE_REQUIRED";
  case VK_ERROR_NOT_PERMITTED:
    return "VK_ERROR_NOT_PERMITTED";
  case VK_ERROR_SURFACE_LOST_KHR:
    return "VK_ERROR_SURFACE_LOST_KHR";
  case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
    return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
  case VK_SUBOPTIMAL_KHR:
    return "VK_SUBOPTIMAL_KHR";
  case VK_ERROR_OUT_OF_DATE_KHR:
    return "VK_ERROR_OUT_OF_DATE_KHR";
  case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
    return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
  case VK_ERROR_INVALID_SHADER_NV:
    return "VK_ERROR_INVALID_SHADER_NV";
  case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
    return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
  case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
    return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
  case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
    return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
  case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
    return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
  case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
    return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
  case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
    return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
  case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
    return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
  case VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT:
    return "VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT";
  case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
    return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
  case VK_THREAD_IDLE_KHR:
    return "VK_THREAD_IDLE_KHR";
  case VK_THREAD_DONE_KHR:
    return "VK_THREAD_DONE_KHR";
  case VK_OPERATION_DEFERRED_KHR:
    return "VK_OPERATION_DEFERRED_KHR";
  case VK_OPERATION_NOT_DEFERRED_KHR:
    return "VK_OPERATION_NOT_DEFERRED_KHR";
  case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
    return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
  case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
    return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
  case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
    return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
  case VK_PIPELINE_BINARY_MISSING_KHR:
    return "VK_PIPELINE_BINARY_MISSING_KHR";
  case VK_ERROR_NOT_ENOUGH_SPACE_KHR:
    return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
  case VK_RESULT_MAX_ENUM:
    return "VK_RESULT_MAX_ENUM";
  default:
    return "Unknown VkResult";
  }
}

/**
 * @brief Got this from here https://gist.github.com/Pikachuxxxx/1bfb4a5eca2593fbf0dc409953ad9bde
 */
static std::unordered_map<VkResult, std::string> ErrorDescriptions = {
  { VK_SUCCESS, "Command successfully completed" },
  { VK_NOT_READY, "A fence or query has not yet completed" },
  { VK_TIMEOUT, "A wait operation has not completed in the specified time" },
  { VK_EVENT_SET, "An event is signaled" },
  { VK_EVENT_RESET, "An event is unsignaled" },
  { VK_INCOMPLETE, "A return array was too small for the result" },
  { VK_SUBOPTIMAL_KHR,
    "A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface "
    "successfully." },
  { VK_THREAD_IDLE_KHR,
    "A deferred operation is not complete but there is currently no work for this thread to do at the time of this "
    "call." },
  { VK_THREAD_DONE_KHR,
    "A deferred operation is not complete but there is no work remaining to assign to additional threads." },
  { VK_OPERATION_DEFERRED_KHR, "A deferred operation was requested and at least some of the work was deferred." },
  { VK_OPERATION_NOT_DEFERRED_KHR, "A deferred operation was requested and no operations were deferred." },
  { VK_PIPELINE_COMPILE_REQUIRED_EXT,
    "A requested pipeline creation would have required compilation, but the application requested compilation to not "
    "be performed." },
  { VK_ERROR_OUT_OF_HOST_MEMORY, "A host memory vmaAllocation has failed." },
  { VK_ERROR_OUT_OF_DEVICE_MEMORY, "A device memory vmaAllocation has failed." },
  { VK_ERROR_INITIALIZATION_FAILED,
    "Initialization of an object could not be completed for implementation-specific reasons." },
  { VK_ERROR_DEVICE_LOST, "The logical or physical device has been lost. See Lost Device" },
  { VK_ERROR_MEMORY_MAP_FAILED, "Mapping of a memory object has failed." },
  { VK_ERROR_LAYER_NOT_PRESENT, "A requested layer is not present or could not be loaded." },
  { VK_ERROR_EXTENSION_NOT_PRESENT, "A requested extension is not supported." },
  { VK_ERROR_FEATURE_NOT_PRESENT, "A requested feature is not supported." },
  { VK_ERROR_INCOMPATIBLE_DRIVER,
    "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for "
    "implementation-specific reasons." },
  { VK_ERROR_TOO_MANY_OBJECTS, "Too many objects of the type have already been created." },
  { VK_ERROR_FORMAT_NOT_SUPPORTED, "A requested format is not supported on this device." },
  { VK_ERROR_FRAGMENTED_POOL,
    "A pool vmaAllocation has failed due to fragmentation of the pool’s memory. This must only be returned if no attempt "
    "to allocate host or device memory was made to accommodate the new vmaAllocation. This should be returned in "
    "preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the implementation is certain that the pool vmaAllocation "
    "failure was due to fragmentation." },
  { VK_ERROR_SURFACE_LOST_KHR, "A surface is no longer available." },
  { VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
    "The requested window is already in use by Vulkan or another API in a manner which prevents it from being used "
    "again." },
  { VK_ERROR_OUT_OF_DATE_KHR,
    "A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation "
    "requests using the swapchain will fail. Applications must query the new surface properties and recreate their "
    "swapchain if they wish to continue presenting to the surface." },
  { VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,
    "The display used by a swapchain does not use the same presentable image vkLayout, or is incompatible in a way "
    "that "
    "prevents sharing an image." },
  { VK_ERROR_INVALID_SHADER_NV,
    "One or more shaders failed to compile or link. More details are reported back to the application via "
    "VK_EXT_debug_report if enabled." },
  { VK_ERROR_OUT_OF_POOL_MEMORY,
    "A pool memory vmaAllocation has failed. This must only be returned if no attempt to allocate host or device memory "
    "was made to accommodate the new vmaAllocation. If the failure was definitely due to fragmentation of the pool, "
    "VK_ERROR_FRAGMENTED_POOL should be returned instead." },
  { VK_ERROR_INVALID_EXTERNAL_HANDLE, "An external handle is not a valid handle of the specified type." },
  { VK_ERROR_FRAGMENTATION, "A descriptor pool creation has failed due to fragmentation." },
  { VK_ERROR_INVALID_DEVICE_ADDRESS_EXT, "A buffer creation failed because the requested address is not available." },
  { VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
    "A buffer creation or memory vmaAllocation failed because the requested address is not available. A shader group "
    "handle assignment failed because the requested shader group handle information is no longer valid." },
  { VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
    "An operation on a swapchain created with VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT failed as it did not "
    "have exlusive full-screen access. This may occur due to implementation-dependent reasons, outside of the "
    "application’s control." },
  { VK_ERROR_UNKNOWN,
    "An unknown error has occurred; either the application has provided invalid input, or an implementation failure "
    "has occurred." } };

#define VK_CALL( x ) VK_CHECK_CALL( x )

#define VK_CHECK_CALL( x ) VulkanCheckErrorStatus( x, __FILE__, __LINE__ )

static bool VulkanCheckErrorStatus( VkResult x, const char* file, int line )
{
  if ( x != VK_SUCCESS )
  {
    auto filePath = std::filesystem::path{ file };
    K_ERROR( "VK_CALL failed -> {} {} {}", line, filePath.filename().string(), vkResultToString( x ) );
    return true;
  }
  else
  {
    return false;
  }
}

static auto findMemoryType( VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties )
  -> uint32_t
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProperties );

  for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
  {
    if ( ( typeFilter & ( 1 << i ) ) && ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties )
    {
      return i;
    }
  }

  throw std::runtime_error( "failed to find suitable memory type!" );
}

static auto readFile( const std::string& filePath ) -> std::vector<char>
{
  std::ifstream file( filePath, std::ios::ate | std::ios::binary );

  if ( !file.is_open() )
  {
    throw std::runtime_error( "failed to open shader file!\n" );
  }
  size_t fileSize{ static_cast<size_t>( file.tellg() ) };
  std::vector<char> buffer( fileSize );
  file.seekg( 0 );
  file.read( buffer.data(), fileSize );
  file.close();

  return buffer;
}
