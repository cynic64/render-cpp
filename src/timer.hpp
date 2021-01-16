#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include "iostream"

namespace timer {
	namespace chrono = std::chrono;

	class Timer {
	public:
		Timer() {
			begin = chrono::high_resolution_clock::now();
		}

		auto get_elapsed() -> double {
			auto now = chrono::high_resolution_clock::now();
			auto elapsed = chrono::duration_cast<chrono::duration<double>>(now - begin);
			return elapsed.count();
		}

		void print_fps(int frames) {
			auto fps = static_cast<double>(frames) / get_elapsed();
			std::cout << "FPS: " << fps << std::endl;
		}

	private:
		chrono::high_resolution_clock::time_point begin;
	};
}

#endif // TIMER_H
