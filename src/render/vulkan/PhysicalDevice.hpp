#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace Render::Vulkan
{
struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool isComplete() {
        return graphicsFamily >= 0 && presentFamily >= 0;
    }
};

struct SwapChainCapabilities {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
};

class PhysicalDevice {
public:
    PhysicalDevice(VkPhysicalDevice physicalDevice) : device(physicalDevice) {}

    operator VkPhysicalDevice&() {
        return device;
    }

    operator const VkPhysicalDevice&() const {
        return device;
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
};
}
