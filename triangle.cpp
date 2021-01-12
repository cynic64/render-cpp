#include "vk_base.hpp"

#include <iostream>
#include <string>
#include <memory>

int main() {
	vk_base::Base b;
	std::cout << "Using device: " << b.phys_dev_name << std::endl;
}
