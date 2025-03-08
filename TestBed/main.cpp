#include "engine.hpp"
#include <iostream>

int main() {
    baldwin::Engine engine{ 800, 600, baldwin::RenderAPI::Vulkan };

    try {
	engine.init();
	engine.run();
	engine.cleanup();
    } catch (const std::exception& e) {
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
