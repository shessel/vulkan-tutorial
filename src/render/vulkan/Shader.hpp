#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace Render::Vulkan
{
class Shader {
public:
    Shader(const std::string& fileName, const std::shared_ptr<const Device>& device) : device(device) {
        auto shaderCode = readFile(fileName);
        shaderModule = createShaderModule(shaderCode);
    }

    ~Shader() {
        vkDestroyShaderModule(device->getHandle(), shaderModule, nullptr);
    }

    VkShaderModule getHandle() const {
        return shaderModule;
    }

private:
    static std::vector<char> readFile(const std::string& fileName) {
        std::ifstream fileStream(fileName, std::ios::ate | std::ios::binary);
        if (!fileStream.is_open()) {
            throw std::runtime_error("failed to open file " + fileName);
        }
        std::vector<char> fileContent(fileStream.tellg());
        fileStream.seekg(0);
        fileStream.read(fileContent.data(), fileContent.size());
        fileStream.close();
        return fileContent;
    }

    VkShaderModule createShaderModule(const std::vector<char>& byteCode) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = byteCode.size();
        shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device->getHandle(), &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module");
        }
        return shaderModule;
    }

    const std::shared_ptr<const Device>& device;
    VkShaderModule shaderModule;
};
}
