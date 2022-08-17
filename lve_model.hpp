#pragma once

#include "lve_device.hpp"

//默认情况下它将使用度而非弧度
#define GLM_FORCE_RADIANS
// glm的深度缓冲值从0开始非-1 -  1
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve
{
    class LveModel
    {
    public:
        struct Vertex
        {
            glm::vec2 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };
        LveModel(LveDevice &DEVICE, const std::vector<Vertex> &vertices);
        ~LveModel();

        LveModel(const LveModel &) = delete;
        LveModel operator=(const LveModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

        LveDevice &lveDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };

}