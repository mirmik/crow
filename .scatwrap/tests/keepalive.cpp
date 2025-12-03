<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/keepalive.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;chrono&gt;<br>
#include&nbsp;&lt;crow/proto/node.h&gt;<br>
#include&nbsp;&lt;crow/tower.h&gt;<br>
#include&nbsp;&lt;doctest/doctest.h&gt;<br>
#include&nbsp;&lt;iostream&gt;<br>
#include&nbsp;&lt;thread&gt;<br>
<br>
static&nbsp;int&nbsp;a&nbsp;=&nbsp;0;<br>
static&nbsp;int&nbsp;b&nbsp;=&nbsp;0;<br>
<br>
class&nbsp;test_keepalive_node&nbsp;:&nbsp;public&nbsp;crow::node,&nbsp;public&nbsp;crow::alived_object<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;&amp;ptr;<br>
<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;test_keepalive_node(int&nbsp;&amp;ptr)&nbsp;:&nbsp;ptr(ptr)&nbsp;{}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;keepalive_handle()&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ptr++;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;incoming_packet(crow::packet&nbsp;*)&nbsp;override&nbsp;{}<br>
};<br>
<br>
TEST_CASE(&quot;keepalive&quot;)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;test_keepalive_node&nbsp;an(a);<br>
&nbsp;&nbsp;&nbsp;&nbsp;test_keepalive_node&nbsp;bn(b);<br>
&nbsp;&nbsp;&nbsp;&nbsp;an.install_keepalive(10);<br>
&nbsp;&nbsp;&nbsp;&nbsp;bn.install_keepalive(20);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;int64_t&nbsp;start&nbsp;=&nbsp;igris::millis();<br>
&nbsp;&nbsp;&nbsp;&nbsp;while&nbsp;(igris::millis()&nbsp;-&nbsp;start&nbsp;&lt;&nbsp;41)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::onestep();<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_UNARY(a&nbsp;==&nbsp;5&nbsp;||&nbsp;a&nbsp;==&nbsp;4);<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_UNARY(b&nbsp;==&nbsp;3&nbsp;||&nbsp;b&nbsp;==&nbsp;2);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
