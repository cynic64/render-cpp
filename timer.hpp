#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include "iostream"

namespace timer {
	class Timer {
	public:
		Timer() {
			begin = std::chrono::high_resolution_clock::now();
		}

		double get_elapsed() {
			auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - begin);
			return elapsed.count();
		}

		void print_fps(int frames) {
			auto fps = (double) frames / get_elapsed();
			std::cout << "FPS: " << fps << std::endl;
		}

	private:
		std::chrono::high_resolution_clock::time_point begin;
	};
}

#endif // TIMER_H
