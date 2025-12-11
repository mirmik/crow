#include <doctest/doctest.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>
#include <crow/tower_thread_executor.h>
#include <crow/gates/loopgate.h>

TEST_CASE("start-stop")
{
	crow::Tower tower;
	crow::loopgate gate;
	gate.bind(tower, 99);

	crow::TowerThreadExecutor executor(tower);
	executor.start();
	executor.stop(true);
} 