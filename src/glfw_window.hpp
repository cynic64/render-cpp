#ifndef GLFW_WINDOW_H
#define GLFW_WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace glfw_window {
	struct GWindow {
		GLFWwindow* window;

		// Pointers are valid until GLFW terminates (so until the GWindow is
		// destroyed)
		std::vector<const char*> req_instance_exts;

		GWindow(uint32_t width, uint32_t height);

		auto get_dims() const -> std::pair<int, int>;

		~GWindow();
	};
}

#endif // GLFW_WINDOW_H
