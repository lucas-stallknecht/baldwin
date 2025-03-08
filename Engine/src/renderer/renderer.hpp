#pragma once

#include <GLFW/glfw3.h>

namespace baldwin {

class Renderer {
  public:
    virtual bool init(GLFWwindow* window, int width, int height,
		      bool tripleBuffering = false) = 0;
    virtual void run(int frame) = 0;
    virtual void cleanup() = 0;
};

} // namespace baldwin
