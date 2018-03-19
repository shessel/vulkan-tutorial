#include "Device.hpp"
#include "Swapchain.hpp"

namespace Render::Vulkan
{
std::shared_ptr<Swapchain> Device::createSwapchain(const VkSurfaceKHR& surface, const std::shared_ptr<const PhysicalDevice>& physicalDevice) {
    return std::make_shared<Swapchain>(surface, physicalDevice, this);
}
}
