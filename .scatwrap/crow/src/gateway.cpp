<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/src/gateway.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/gateway.h&gt;<br>
#include&nbsp;&lt;crow/tower.h&gt;<br>
#include&nbsp;&lt;crow/tower_cls.h&gt;<br>
<br>
#include&nbsp;&lt;igris/sync/syslock.h&gt;<br>
<br>
//&nbsp;C++&nbsp;gateway&nbsp;binding&nbsp;with&nbsp;explicit&nbsp;tower<br>
int&nbsp;crow::gateway::bind(Tower&nbsp;&amp;tower,&nbsp;int&nbsp;id)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;_tower&nbsp;=&nbsp;&amp;tower;<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;tower.bind_gateway(this,&nbsp;id);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
