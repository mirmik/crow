<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/proto/channel-sync.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/proto/channel.h&gt;<br>
<br>
int&nbsp;crow::channel::connect(const&nbsp;uint8_t&nbsp;*raddr,&nbsp;uint16_t&nbsp;rlen,&nbsp;nodeid_t&nbsp;rid,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint8_t&nbsp;qos,&nbsp;uint16_t&nbsp;ackquant)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;handshake(raddr,&nbsp;rlen,&nbsp;rid,&nbsp;qos,&nbsp;ackquant);<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;ret&nbsp;=&nbsp;waitevent();<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;ret;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
