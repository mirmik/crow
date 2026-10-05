<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/tower.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#ifndef&nbsp;CROW_TOWER_H<br>
#define&nbsp;CROW_TOWER_H<br>
<br>
#include&nbsp;&lt;crow/defs.h&gt;<br>
#include&nbsp;&lt;crow/gateway.h&gt;<br>
#include&nbsp;&lt;crow/packet_ptr.h&gt;<br>
#include&nbsp;&lt;igris/event/delegate.h&gt;<br>
#include&nbsp;&lt;nos/buffer.h&gt;<br>
#include&nbsp;&lt;string&gt;<br>
<br>
//&nbsp;Forward&nbsp;declaration&nbsp;of&nbsp;Tower&nbsp;class<br>
namespace&nbsp;crow<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;class&nbsp;Tower;<br>
}<br>
<br>
//&nbsp;Tower-specific&nbsp;functions<br>
namespace&nbsp;crow<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;Spin/thread&nbsp;functions&nbsp;(require&nbsp;Tower&nbsp;instance)<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;spin(Tower&nbsp;&amp;tower);<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;spin_with_select(Tower&nbsp;&amp;tower);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;stop_spin(bool&nbsp;wait&nbsp;=&nbsp;true);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;[[deprecated]]&nbsp;void&nbsp;spin_join();<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;join_spin();<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;set_spin_cancel_token();<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;start_spin_with_select_realtime(Tower&nbsp;&amp;tower,&nbsp;int&nbsp;abort_on_fault);<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;start_spin_realtime(Tower&nbsp;&amp;tower,&nbsp;int&nbsp;abort_on_fault);<br>
}<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
