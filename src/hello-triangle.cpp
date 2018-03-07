#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    const int WIDTH = 800;
    const int HEIGHT = 600;
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    struct QueueFamilyIndices {
        int graphicsFamily = -1;

        bool isComplete() {
            return graphicsFamily >= 0;
        }
    };

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        createVkInstance();
        createDebugCallback();
        selectPhysicalDevice();
    }

    void createVkInstance() {
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

        std::vector<const char*> requiredExtensions = getRequiredExtensions();
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

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        return extensions;
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

    void createDebugCallback() {
        if (!enableValidationLayers) {
            return;
        }

        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;

        auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

        if (func( instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug callback!");
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugReportFlagsEXT /*flags*/,
            VkDebugReportObjectTypeEXT /*objectType*/,
            uint64_t /*object*/,
            size_t /*location*/,
            int32_t /*code*/,
            const char */*layerPrefix*/,
            const char *msg,
            void */*userData*/) {
        std::cerr << "Validation Layer: " << msg << std::endl;
        return VK_FALSE;
    }

    void selectPhysicalDevice() {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Found no vulkan capable device");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }
        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Found no suitable device");
        }
    }

    bool isDeviceSuitable(const VkPhysicalDevice& device) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        QueueFamilyIndices indices = findQueueFamilyIndices(device);

        std::cout << properties.deviceName << std::endl;

        return indices.isComplete() &&
            (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
                properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ) &&
            features.geometryShader;
    }

    QueueFamilyIndices findQueueFamilyIndices(const VkPhysicalDevice& device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, properties.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (properties[i].queueCount > 0 &&  properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
        }

        return indices;
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
        func( instance, callback, nullptr);

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    GLFWwindow *window;
    VkInstance instance;
    VkDebugReportCallbackEXT callback;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
