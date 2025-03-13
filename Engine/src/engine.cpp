#include "engine.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <imgui.h>

#include "renderer/vulkan/vk_renderer.hpp"

namespace baldwin {

Engine* loadedEngine = nullptr;
Engine& get() { return *loadedEngine; }

Engine::Engine(int width, int height, RenderAPI api)
  : _width(width)
  , _height(height)
  , _api(api) {
    switch (_api) {
	    /* Always defaults to vulkan for now */
	default:
	    _renderer = std::move(std::make_unique<vk::VulkanRenderer>());
    }
}

bool Engine::init() {
    assert(loadedEngine == nullptr);

    assert(initWindow() == true);
    assert(initImgui() == true);
    assert(_renderer->init(_window, _width, _height, false) == true);

    std::cout << "- Engine init\n";

    return true;
}

bool Engine::initWindow() {
    assert(glfwInit() == 1);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    /*glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);*/
    _window = glfwCreateWindow(
      _width, _height, "Baldwin Engine", nullptr, nullptr);
    glfwMakeContextCurrent(_window);

    return _window != nullptr;
}

bool Engine::initImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    return true;
}

void Engine::run() {
    std::cout << "- Engine run\n";
    while (!glfwWindowShouldClose(_window)) {
	glfwPollEvents();
	_renderer->newImguiFrame();
	ImGui::NewFrame();
	{
	    ImGui::Begin("Settings");
	    ImGui::End();
	}
	ImGui::EndFrame();
	ImGui::Render();
	_renderer->run(_frame);
	_frame++;
    }
}

void Engine::cleanup() {
    std::cout << "- Engine cleanup\n";
    _renderer->cleanup();
    glfwDestroyWindow(_window);
    glfwTerminate();
}

} // namespace baldwin
