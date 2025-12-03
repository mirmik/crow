<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/proto/acceptor.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/proto/acceptor.h&gt;<br>
#include&nbsp;&lt;crow/proto/channel.h&gt;<br>
#include&nbsp;&lt;crow/tower.h&gt;<br>
#include&nbsp;&lt;crow/warn.h&gt;<br>
<br>
#include&nbsp;&lt;nos/trace.h&gt;<br>
<br>
void&nbsp;crow::acceptor::incoming_packet(crow::packet&nbsp;*pack)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;crow::subheader_channel&nbsp;*shc&nbsp;=&nbsp;crow::get_subheader_channel(pack);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(shc-&gt;ftype&nbsp;==&nbsp;Frame::HANDSHAKE_REQUEST)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;ch&nbsp;=&nbsp;init_channel();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(ch-&gt;id&nbsp;==&nbsp;0)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ch-&gt;bind(dynport());<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ch-&gt;wait_handshake_request();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ch-&gt;incoming_packet(pack);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;else<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::release(pack);<br>
}<br>
<br>
void&nbsp;crow::acceptor::undelivered_packet(crow::packet&nbsp;*pack)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;crow::release(pack);<br>
}<br>
<br>
uint16_t&nbsp;crow::dynport()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;crow::warn(&quot;TODO:&nbsp;crow::dynport&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;512;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
