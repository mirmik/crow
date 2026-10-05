<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/proto/node-sync.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/proto/node.h&gt;<br>
#include&nbsp;&lt;igris/osinter/wait.h&gt;<br>
<br>
int&nbsp;crow::node::waitevent()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;intptr_t&nbsp;ret;<br>
&nbsp;&nbsp;&nbsp;&nbsp;wait_current_schedee(&amp;waitlnk,&nbsp;0,&nbsp;(void&nbsp;**)&amp;ret);<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;ret;<br>
}<br>
<br>
void&nbsp;crow::node::notify_one(intptr_t&nbsp;future)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;unwait_one(&amp;waitlnk,&nbsp;(intptr_t)future);<br>
}<br>
<br>
void&nbsp;crow::node::notify_all(intptr_t&nbsp;future)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;unwait_all(&amp;waitlnk,&nbsp;(intptr_t)future);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
