<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/subscriber_node.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/nodes/subscriber_node.h&gt;<br>
#include&nbsp;&lt;doctest/doctest.h&gt;<br>
<br>
void&nbsp;foo(nos::buffer&nbsp;data)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;(void)data;<br>
}<br>
<br>
TEST_CASE(&quot;doctest&quot;)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;crow::subscriber_node&nbsp;node(foo);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;node.bind(13);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
