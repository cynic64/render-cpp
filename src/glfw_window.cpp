#include "glfw_window.hpp"

namespace glfw_window {
	GWindow::GWindow(uint32_t width, uint32_t height) {
		glfwInit();
	
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

		const char** raw_extensions;
		uint32_t extension_ct;
		raw_extensions = glfwGetRequiredInstanceExtensions(&extension_ct);
		req_instance_exts = {raw_extensions, raw_extensions + extension_ct};
	}

	auto GWindow::get_dims() const -> std::pair<int, int> {
		std::pair<int, int> dims;
		glfwGetFramebufferSize(window, &dims.first, &dims.second);

		return dims;
	}

	GWindow::~GWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}
