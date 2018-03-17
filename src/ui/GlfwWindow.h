#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

class GlfwWindow {
public:
    GlfwWindow(int width, int height) : width(width), height(height) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    }

    ~GlfwWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    int getWidth() const {
        return width;
    }

    int getHeight() const {
        return height;
    }

    static void glfwSizeCallback(GLFWwindow *window, int width, int height) {
        GlfwWindow* windowWrapper = reinterpret_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
        windowWrapper->sizeCallback(*windowWrapper, width, height);
    }

    void setSizeCallback(void (*sizeCallback)(const GlfwWindow& /*window*/, int /*width*/, int /*height*/)) {
        this->sizeCallback = sizeCallback;
        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, glfwSizeCallback);
    }

    void setCallbackObject(void * callbackObject) {
        this->callbackObject = callbackObject;
    }

    void* getCallbackObject() const {
        return callbackObject;
    }

    std::vector<const char*> getRequiredVulkanExtensions() const {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        return extensions;
    }

    void createVulkanSurface(VkInstance& instance, VkSurfaceKHR *surface, VkAllocationCallbacks *allocationCallbacks = nullptr) const {
        if (glfwCreateWindowSurface(instance, window, allocationCallbacks, surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }
    }

    bool shouldClose() const {
        return glfwWindowShouldClose(window);
    }

    void pollEvents() const {
        glfwPollEvents();
    }

private:
    GLFWwindow *window;
    int width;
    int height;
    void *callbackObject;
    void (*sizeCallback)(const GlfwWindow& /*window*/, int /*width*/, int /*height*/);
};
