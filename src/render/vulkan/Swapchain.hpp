#pragma once

#include <algorithm>
#include <limits>
#include <vector>

#include <vulkan/vulkan.h>

#include "PhysicalDevice.hpp"
#include "SwapchainCapabilities.hpp"

namespace Render::Vulkan
{
class Device;
class Swapchain {
public:
    Swapchain(const VkSurfaceKHR& surface, const std::shared_ptr<const PhysicalDevice>& physicalDevice, const Device *const device) : device(device->getHandle()) {
        SwapchainCapabilities swapchainCapabilities = physicalDevice->querySwapchainCapabilities(surface);
        surfaceFormat = chooseSurfaceFormat(swapchainCapabilities.surfaceFormats);
        presentMode = choosePresentMode(swapchainCapabilities.presentModes);
        extent = chooseSwapExtent(swapchainCapabilities.surfaceCapabilities);

        uint32_t imageCount = swapchainCapabilities.surfaceCapabilities.minImageCount + 1;
        if (swapchainCapabilities.surfaceCapabilities.maxImageCount > 0 && imageCount > swapchainCapabilities.surfaceCapabilities.maxImageCount) {
            imageCount = swapchainCapabilities.surfaceCapabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = surface;
        swapChainCreateInfo.minImageCount = imageCount;
        swapChainCreateInfo.imageFormat = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent = extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = physicalDevice->findQueueFamilyIndices(surface);
        uint32_t queueFamiliyIndices[] = {
            static_cast<uint32_t>(indices.graphicsFamily),
            static_cast<uint32_t>(indices.presentFamily)
        };

        if (indices.graphicsFamily == indices.presentFamily) {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        } else {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = sizeof(queueFamiliyIndices)/sizeof(queueFamiliyIndices[0]);
            swapChainCreateInfo.pQueueFamilyIndices = queueFamiliyIndices;
        }

        swapChainCreateInfo.preTransform = swapchainCapabilities.surfaceCapabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device->getHandle(), &swapChainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain");
        }

        vkGetSwapchainImagesKHR(device->getHandle(), swapchain, &imageCount, nullptr);
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(device->getHandle(), swapchain, &imageCount, images.data());

        createImageViews();
    }

    Swapchain(const Swapchain& other) = delete;
    Swapchain& operator=(const Swapchain& other) = delete;

    Swapchain(Swapchain&& other) {
        using std::swap;
        device = other.device;
        swapchain = other.swapchain;
        extent = other.extent;
        surfaceFormat = other.surfaceFormat;
        presentMode = other.presentMode;
        swap(images, other.images);
        other.swapchain = VK_NULL_HANDLE;
    }

    ~Swapchain() {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    VkSwapchainKHR getHandle() const {
        return swapchain;
    }

    VkFormat getImageFormat() const {
        return surfaceFormat.format;
    }

    VkExtent2D getExtent() const {
        return extent;
    }

    const std::vector<VkImage>& getImages() const {
        return images;
    }

    const std::vector<VkImageView> getImageViews() const {
        return imageViews;
    }

private:
    VkDevice device;
    VkSwapchainKHR swapchain;
    VkExtent2D extent;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& supportedFormats) {
        if (supportedFormats.size() == 1 && supportedFormats[0].format == VK_FORMAT_UNDEFINED) {
            return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }

        for (const auto& format : supportedFormats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        // fallback option
        return supportedFormats[0];
    }

    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& supportedModes) {
        for (const auto& mode : supportedModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return mode;
            } else if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                return mode;
            }
        }
        // fallback option
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            // TODO: replace this
            int width = 100;//window.getWidth();
            int height = 100;//window.getHeight();
            VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(actualExtent.width, capabilities.maxImageExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(actualExtent.height, capabilities.maxImageExtent.height));
            return actualExtent;
        }
    }

    void createImageViews() {
        imageViews.resize(images.size());
        for (size_t i = 0; i < images.size(); ++i) {
            VkImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = images[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = getImageFormat();
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image view");
            }
        }
    }
};
}
