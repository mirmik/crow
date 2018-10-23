#include <crow/tower.h>
#include <chrono>

extern "C" uint16_t crow_millis() {
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return millis;
}
