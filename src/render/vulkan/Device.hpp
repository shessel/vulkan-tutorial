#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include "QueueFamilyIndices.hpp"

namespace Render::Vulkan
{
class Device {
public:
    Device(VkDevice device) : device(device) {}

    Device(Device&& other) : device(other.device) {
        other.device = VK_NULL_HANDLE;
    }

    ~Device() {
        vkDestroyDevice(device, nullptr);
    }

    Device& operator=(Device&& other) {
        device = other.device;
        other.device = VK_NULL_HANDLE;
        return *this;
    }

    operator VkDevice&() {
        return device;
    }

    operator const VkDevice&() const {
        return device;
    }

    VkQueue getQueue(uint32_t index) {
        VkQueue result;
        vkGetDeviceQueue( device, index, 0, &result);
        return result;
    }

private:
    VkDevice device;
    Device(const Device&){};
    Device& operator=(const Device& other);
};
}
