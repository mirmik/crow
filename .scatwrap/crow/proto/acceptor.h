<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/proto/acceptor.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;CROW_ACCEPTOR_H<br>
#define&nbsp;CROW_ACCEPTOR_H<br>
<br>
#include&nbsp;&lt;crow/proto/channel.h&gt;<br>
#include&nbsp;&lt;crow/proto/node.h&gt;<br>
<br>
namespace&nbsp;crow<br>
{<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;struct&nbsp;acceptor&nbsp;:&nbsp;public&nbsp;crow::node<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;igris::delegate&lt;crow::channel&nbsp;*&gt;&nbsp;init_channel;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;acceptor()&nbsp;=&nbsp;default;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;acceptor(igris::delegate&lt;crow::channel&nbsp;*&gt;&nbsp;init_channel)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;:&nbsp;init_channel(init_channel)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;init(int&nbsp;id,&nbsp;igris::delegate&lt;crow::channel&nbsp;*&gt;&nbsp;init_channel)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;this-&gt;init_channel&nbsp;=&nbsp;init_channel;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;bind(id);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;incoming_packet(crow::packet&nbsp;*pack)&nbsp;override;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;undelivered_packet(crow::packet&nbsp;*pack)&nbsp;override;<br>
&nbsp;&nbsp;&nbsp;&nbsp;};<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;__BEGIN_DECLS<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;static&nbsp;inline&nbsp;acceptor&nbsp;*<br>
&nbsp;&nbsp;&nbsp;&nbsp;create_acceptor(uint16_t&nbsp;port,&nbsp;igris::delegate&lt;crow::channel&nbsp;*&gt;&nbsp;dlg)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;asrv&nbsp;=&nbsp;new&nbsp;crow::acceptor(dlg);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;asrv-&gt;bind(port);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;asrv;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;uint16_t&nbsp;dynport();<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;__END_DECLS<br>
<br>
}&nbsp;//&nbsp;namespace&nbsp;crow<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
