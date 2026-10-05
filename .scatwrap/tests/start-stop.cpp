<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/start-stop.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;doctest/doctest.h&gt;<br>
#include&nbsp;&lt;crow/tower.h&gt;<br>
#include&nbsp;&lt;crow/tower_cls.h&gt;<br>
#include&nbsp;&lt;crow/tower_thread_executor.h&gt;<br>
#include&nbsp;&lt;crow/gates/loopgate.h&gt;<br>
<br>
TEST_CASE(&quot;start-stop&quot;)<br>
{<br>
	crow::Tower&nbsp;tower;<br>
	crow::loopgate&nbsp;gate;<br>
	gate.bind(tower,&nbsp;99);<br>
<br>
	crow::TowerThreadExecutor&nbsp;executor(tower);<br>
	executor.start();<br>
	executor.stop(true);<br>
}&nbsp;<br>
<!-- END SCAT CODE -->
</body>
</html>
