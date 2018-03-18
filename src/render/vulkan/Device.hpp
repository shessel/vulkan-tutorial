#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include "QueueFamilyIndices.hpp"

namespace Render::Vulkan
{
class Device {
public:
    Device(VkDevice device) : device(device) {}

    Device(const Device&) = delete;

    Device(Device&& other) : device(other.device) {
        other.device = VK_NULL_HANDLE;
    }

    ~Device() {
        vkDestroyDevice(device, nullptr);
    }

    Device& operator=(const Device& other) = delete;

    Device& operator=(Device&& other) {
        device = other.device;
        other.device = VK_NULL_HANDLE;
        return *this;
    }

    VkQueue getQueue(uint32_t index) {
        VkQueue result;
        vkGetDeviceQueue( device, index, 0, &result);
        return result;
    }

    VkDevice getHandle() const {
        return device;
    }

private:
    VkDevice device;
};
}
