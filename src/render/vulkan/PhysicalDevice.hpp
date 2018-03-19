#pragma once

#include <cstring>
#include <memory>
#include <set>
#include <vector>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "QueueFamilyIndices.hpp"
#include "SwapchainCapabilities.hpp"

namespace Render::Vulkan
{
class Instance;

class PhysicalDevice {
public:
    PhysicalDevice(VkPhysicalDevice physicalDevice) : device(physicalDevice) {}

    PhysicalDevice(PhysicalDevice&& other) : device(other.device) {
        other.device = VK_NULL_HANDLE;
    }

    PhysicalDevice& operator=(PhysicalDevice&& other) {
        this->device = other.device;
        other.device = VK_NULL_HANDLE;
        return *this;
    }

    VkPhysicalDevice getHandle() const {
        return device;
    }

    std::shared_ptr<Device> createDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions, bool enableValidationLayers) const {
        QueueFamilyIndices indices = findQueueFamilyIndices(surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<int> uniqueQueueFamilyIndices = {indices.graphicsFamily, indices.presentFamily};

        for (auto index : uniqueQueueFamilyIndices) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = index;
            queueCreateInfo.queueCount = 1;
            float queuePriority = 1.0f;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        createInfo.enabledExtensionCount = deviceExtensions.size();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        VkDevice deviceToCreate;
        if (vkCreateDevice(device, &createInfo, nullptr, &deviceToCreate) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }

        return std::shared_ptr<Device>(new Device(deviceToCreate));
    }

    QueueFamilyIndices findQueueFamilyIndices(VkSurfaceKHR surface) const {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, properties.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface, &presentSupport);
            if (properties[i].queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
            }

            if (properties[i].queueCount > 0 && properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
        }

        return indices;
    }

    SwapchainCapabilities querySwapchainCapabilities(VkSurfaceKHR surface) const {
        SwapchainCapabilities swapchainCapabilities;
        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, nullptr);
        if (surfaceFormatCount > 0) {
            swapchainCapabilities.surfaceFormats.resize(surfaceFormatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, swapchainCapabilities.surfaceFormats.data());
        }

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchainCapabilities.surfaceCapabilities);

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount > 0) {
            swapchainCapabilities.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, swapchainCapabilities.presentModes.data());
        }
        return swapchainCapabilities;
    }

private:
    friend Instance;

    PhysicalDevice(const PhysicalDevice& other) = delete;

    PhysicalDevice& operator=(const PhysicalDevice& other) = delete;

    bool isSuitable(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) const {
        QueueFamilyIndices indices = findQueueFamilyIndices(surface);
        bool extensionsSupported = checkSupportedDeviceExtensions(deviceExtensions);
        SwapchainCapabilities swapchainCapabilities = querySwapchainCapabilities(surface);
        return indices.isComplete()
            && extensionsSupported
            && !swapchainCapabilities.presentModes.empty()
            && !swapchainCapabilities.surfaceFormats.empty();
    }

    bool checkSupportedDeviceExtensions(const std::vector<const char*>& deviceExtensions) const {
        uint32_t extensionPropertyCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionPropertyCount, nullptr);
        if (extensionPropertyCount > 0) {
            std::vector<VkExtensionProperties> extensionProperties(extensionPropertyCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionPropertyCount, extensionProperties.data());

            for (const auto deviceExtension : deviceExtensions) {
                bool extensionFound = false;
                for (const auto& extensionProperty : extensionProperties) {
                    if (strcmp(extensionProperty.extensionName, deviceExtension) == 0) {
                        extensionFound = true;
                        break;
                    }
                }
                if (!extensionFound) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    VkPhysicalDevice device;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };
};
}
