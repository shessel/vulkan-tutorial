#pragma once
#include <vulkan/vulkan.h>

#include "PhysicalDevice.hpp"

namespace Render::Vulkan
{
class Instance {
public:
    Instance(std::vector<const char*> requiredExtensions) {
        createVkInstance(requiredExtensions);
    }

    ~Instance() {
        auto vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
        for (const auto& callback : debugCallbacks) {
            vkDestroyDebugReportCallbackEXT( instance, callback, nullptr);
        }

        vkDestroyInstance(instance, nullptr);
    }

    operator VkInstance&() {
        return instance;
    }

    operator const VkInstance&() const {
        return instance;
    }

    void createDebugCallback(PFN_vkDebugReportCallbackEXT debugCallback) {
        if (!enableValidationLayers) {
            return;
        }

        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;

        auto vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
        VkDebugReportCallbackEXT callback;

        if (vkCreateDebugReportCallbackEXT( instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug callback!");
        }

        debugCallbacks.push_back(callback);
    }

    PhysicalDevice selectDefaultPhysicalDeviceForSurface(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Found no vulkan capable device");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            PhysicalDevice physicalDevice(device);
            if (physicalDevice.isSuitable(surface, deviceExtensions)) {
                return physicalDevice;
                break;
            }
        }

        throw std::runtime_error("Found no suitable device");
    }

private:
    VkInstance instance;
    std::vector<VkDebugReportCallbackEXT> debugCallbacks;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    void createVkInstance(std::vector<const char*> requiredExtensions) {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layers not available");
        }
        VkApplicationInfo applicationInfo = {};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName = "Hello Triangle";
        applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pEngineName = "No Engine";
        applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &applicationInfo;

        if (enableValidationLayers) {
            requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        if (!checkExtensions(requiredExtensions)) {
            std::cout << "missing extension" << std::endl;
        }

        createInfo.enabledExtensionCount = requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = validationLayers.size();
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    bool checkValidationLayerSupport() {
        uint32_t propertyCount;
        vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);

        std::vector<VkLayerProperties> layerProperties(propertyCount);
        vkEnumerateInstanceLayerProperties(&propertyCount, layerProperties.data());

        for (const auto& layer : validationLayers) {
            bool found = false;
            for (const auto& layerProperty : layerProperties) {
                if (strcmp(layerProperty.layerName, layer) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }

        return true;
    }

    bool checkExtensions(const std::vector<const char*>& extensions) {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

        for (const auto& extension : extensions) {
            bool found = false;
            for (const auto& extensionProperty : extensionProperties) {
                if (strcmp(extension, extensionProperty.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }
};
}
