#include "first_app.hpp"

// std
#include <stdexcept>
#include <array>
#include <cassert>

namespace lve
{
	FirstApp::FirstApp()
	{
		loadModels();
		createPipelineLayout();
		recreateSwapChain();
		// createPipeline();
		createCommandBuffers();
	}

	FirstApp::~FirstApp() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }
	void FirstApp::run()
	{
		while (!lveWindow.shouldClose())
		{
			glfwPollEvents();
			drawFrame();
		}

		// CPU阻塞直到GPU完成所有工作（防止验证层消息）
		vkDeviceWaitIdle(lveDevice.device());
	}
	// void FirstApp::sierpinski(
	// 	std::vector<LveModel::Vertex> &vertices,
	// 	int depth,
	// 	glm::vec2 left,
	// 	glm::vec2 right,
	// 	glm::vec2 top)
	// {
	// 	if (depth <= 0)
	// 	{
	// 		vertices.push_back({top});
	// 		vertices.push_back({right});
	// 		vertices.push_back({left});
	// 	}
	// 	else
	// 	{
	// 		auto leftTop = 0.5f * (left + top);
	// 		auto rightTop = 0.5f * (right + top);
	// 		auto leftRight = 0.5f * (left + right);
	// 		sierpinski(vertices, depth - 1, left, leftRight, leftTop);
	// 		sierpinski(vertices, depth - 1, leftRight, right, rightTop);
	// 		sierpinski(vertices, depth - 1, leftTop, rightTop, top);
	// 	}
	// }

	void FirstApp::loadModels()
	{
		std::vector<LveModel::Vertex> vertices{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
		lveModel = std::make_unique<LveModel>(lveDevice, vertices);
		// other graphics
		//  std::vector<LveModel::Vertex> vertices{};
		//  sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
		//  lveModel = std::make_unique<LveModel>(lveDevice, vertices);
	}

	void FirstApp::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}
	void FirstApp::recreateSwapChain()
	{
		auto extent = lveWindow.getExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = lveWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(lveDevice.device());

		if (lveSwapChain == nullptr)
		{
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
		}
		else
		{
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, std::move(lveSwapChain));
			if (lveSwapChain->imageCount() != commandBuffers.size())
			{
				freeCommandBuffers();
				createCommandBuffers();
			}
		}

		createPipeline();
	}

	void FirstApp::createPipeline()
	{
		assert(lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.pipelineLayout = pipelineLayout;
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = lveSwapChain->getRenderPass();

		lve_pipeline = std::make_unique<LvePipeline>(
			lveDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig);
	}
	void FirstApp::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			lveDevice.device(),
			lveDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	void FirstApp::createCommandBuffers()
	{
		commandBuffers.resize(lveSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers");
		}
	}

	void FirstApp::recordCommandBuffer(int imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lveSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		lve_pipeline->bind(commandBuffers[imageIndex]);
		lveModel->bind(commandBuffers[imageIndex]);
		lveModel->draw(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}
	void FirstApp::drawFrame()
	{
		uint32_t imageIndex;
		auto result = lveSwapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image");
		}
		//将提供的命令缓冲区提交到设备
		result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			lveWindow.wasWindowResized())
		{
			lveWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}
	}
}