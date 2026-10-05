<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/src/variants/warn.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/warn.h&gt;<br>
#include&nbsp;&lt;igris/dprint.h&gt;<br>
<br>
#if&nbsp;!defined(MEMORY_ECONOMY)<br>
void&nbsp;crow::warn(nos::buffer&nbsp;msg)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;WARN:&nbsp;&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;debug_write(msg.data(),&nbsp;msg.size());<br>
&nbsp;&nbsp;&nbsp;&nbsp;dln();<br>
}<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
