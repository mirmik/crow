<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/src/packet_ptr.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/packet_ptr.h&gt;<br>
#include&nbsp;&lt;crow/tower.h&gt;<br>
<br>
#include&nbsp;&lt;assert.h&gt;<br>
<br>
crow::packet_ptr::~packet_ptr()&nbsp;{&nbsp;clear();&nbsp;}<br>
<br>
void&nbsp;crow::packet_ptr::clear()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(pack)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;pack-&gt;refs--;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;assert(pack-&gt;refs&nbsp;&gt;=&nbsp;0);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(pack-&gt;refs&nbsp;==&nbsp;0)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::release(pack);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;pack&nbsp;=&nbsp;nullptr;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
