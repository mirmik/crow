<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/src/variants/allocation_pool.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/packet.h&gt;<br>
#include&nbsp;&lt;igris/container/pool.h&gt;<br>
<br>
#include&nbsp;&lt;igris/sync/syslock.h&gt;<br>
<br>
extern&nbsp;bool&nbsp;__live_diagnostic_enabled;<br>
igris::pool&nbsp;_crow_packet_pool;<br>
int&nbsp;crow_allocated_count&nbsp;=&nbsp;0;<br>
<br>
igris::pool&nbsp;*crow::get_package_pool()&nbsp;{&nbsp;return&nbsp;&amp;_crow_packet_pool;&nbsp;}<br>
<br>
void&nbsp;crow::engage_packet_pool(void&nbsp;*zone,&nbsp;size_t&nbsp;zonesize,&nbsp;size_t&nbsp;elsize)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;_crow_packet_pool.init(zone,&nbsp;zonesize,&nbsp;elsize);<br>
}<br>
<br>
void&nbsp;crow::deallocate_packet(crow::packet&nbsp;*pack)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;assert(pack);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;system_lock();<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(pack)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow_allocated_count--;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;_crow_packet_pool.put(pack);<br>
&nbsp;&nbsp;&nbsp;&nbsp;system_unlock();<br>
}<br>
<br>
crow::compacted_packet&nbsp;*crow_allocate_packet(size_t&nbsp;adlen)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;(void)&nbsp;adlen;<br>
&nbsp;&nbsp;&nbsp;&nbsp;system_lock();<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;*ret&nbsp;=&nbsp;_crow_packet_pool.get();<br>
&nbsp;&nbsp;&nbsp;&nbsp;memset(ret,&nbsp;0,&nbsp;_crow_packet_pool.element_size());<br>
&nbsp;&nbsp;&nbsp;&nbsp;new&nbsp;(ret)&nbsp;crow::compacted_packet;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(ret)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow_allocated_count++;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;system_unlock();<br>
&nbsp;&nbsp;&nbsp;&nbsp;pack-&gt;set_destructor(crow::deallocate_packet);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;(crow::compacted_packet&nbsp;*)ret;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
