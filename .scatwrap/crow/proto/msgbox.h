<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/proto/msgbox.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;CROW_MSGBOX_H<br>
#define&nbsp;CROW_MSGBOX_H<br>
<br>
#include&nbsp;&lt;crow/proto/node.h&gt;<br>
#include&nbsp;&lt;igris/datastruct/dlist.h&gt;<br>
#include&nbsp;&lt;igris/sync/semaphore.h&gt;<br>
#include&nbsp;&lt;igris/sync/syslock.h&gt;<br>
<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
#define&nbsp;CROW_MSGBOX_STATE_NONE&nbsp;0<br>
#define&nbsp;CROW_MSGBOX_STATE_SEND&nbsp;1<br>
#define&nbsp;CROW_MSGBOX_STATE_RECEIVE&nbsp;2<br>
<br>
namespace&nbsp;crow<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;class&nbsp;msgbox&nbsp;:&nbsp;public&nbsp;crow::node<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;igris_sem_t&nbsp;message_lock&nbsp;=&nbsp;{};<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;struct&nbsp;dlist_head&nbsp;messages&nbsp;=&nbsp;DLIST_HEAD_INIT(messages);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;msgbox();<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::node_packet_ptr&nbsp;query(nodeid_t&nbsp;rid,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;crow::hostaddr_view&nbsp;&amp;addr,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;nos::buffer&nbsp;data,&nbsp;uint8_t&nbsp;qos,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint16_t&nbsp;ackquant);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::node_packet_ptr&nbsp;receive();<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::packet_ptr&nbsp;reply(crow::node_packet_ptr&nbsp;msg,&nbsp;nos::buffer&nbsp;data,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint8_t&nbsp;qos,&nbsp;uint16_t&nbsp;ackquant);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;incoming_packet(crow::packet&nbsp;*pack)&nbsp;override;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;undelivered_packet(crow::packet&nbsp;*pack)&nbsp;override;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;~msgbox()&nbsp;override;<br>
&nbsp;&nbsp;&nbsp;&nbsp;};<br>
}&nbsp;//&nbsp;namespace&nbsp;crow<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
