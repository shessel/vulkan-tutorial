#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "QueueFamilyIndices.hpp"

namespace Render::Vulkan
{
struct SwapChainCapabilities {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
};

class PhysicalDevice {
public:
    PhysicalDevice(VkPhysicalDevice physicalDevice) : device(physicalDevice) {}

    PhysicalDevice& operator=(const PhysicalDevice& other) {
        device = other.device;
        return *this;
    }

    operator VkPhysicalDevice&() {
        return device;
    }

    operator const VkPhysicalDevice&() const {
        return device;
    }

    Device createDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions, bool enableValidationLayers) {
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

        return Device(deviceToCreate);
    }

    bool isSuitable(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
        QueueFamilyIndices indices = findQueueFamilyIndices(surface);
        bool extensionsSupported = checkSupportedDeviceExtensions(deviceExtensions);
        SwapChainCapabilities swapChainCapabilities = querySwapChainCapabilities(surface);
        return indices.isComplete()
            && extensionsSupported
            && !swapChainCapabilities.presentModes.empty()
            && !swapChainCapabilities.surfaceFormats.empty();
    }

    QueueFamilyIndices findQueueFamilyIndices(VkSurfaceKHR surface) {
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

    SwapChainCapabilities querySwapChainCapabilities(VkSurfaceKHR surface) {
        SwapChainCapabilities swapChainCapabilities;
        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, nullptr);
        if (surfaceFormatCount > 0) {
            swapChainCapabilities.surfaceFormats.resize(surfaceFormatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, swapChainCapabilities.surfaceFormats.data());
        }

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainCapabilities.surfaceCapabilities);

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount > 0) {
            swapChainCapabilities.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, swapChainCapabilities.presentModes.data());
        }
        return swapChainCapabilities;
    }

private:
    bool checkSupportedDeviceExtensions(const std::vector<const char*>& deviceExtensions) {
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
