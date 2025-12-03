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
<br>
TEST_CASE(&quot;start-stop&quot;)&nbsp;<br>
{<br>
	crow::start_spin();<br>
	crow::stop_spin(true);<br>
}&nbsp;<br>
<!-- END SCAT CODE -->
</body>
</html>
