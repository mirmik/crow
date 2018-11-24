#include <crow/tower.h>
#include <chrono>

uint16_t crow::millis() {
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return (uint16_t) millis;
}
