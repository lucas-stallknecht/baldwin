#pragma once
#include <GLFW/glfw3.h>

#include <memory>

#include "renderer/renderer.hpp"

namespace baldwin {

enum RenderAPI { Vulkan, DirectX12 };

class Engine {
  public:
    static Engine& get();
    Engine(int width, int height, RenderAPI api);

    bool init();
    void run();
    void cleanup();

  private:
    bool initWindow();

    int _frame = 0;
    int _width, _height;
    GLFWwindow* _window = nullptr;
    const RenderAPI _api;
    std::unique_ptr<Renderer> _renderer;
};

} // namespace baldwin
