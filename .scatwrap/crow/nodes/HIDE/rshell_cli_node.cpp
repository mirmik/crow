<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/nodes/HIDE/rshell_cli_node.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/nodes/rshell_cli_node.h&gt;<br>
<br>
void&nbsp;crow::rshell_cli_node_base::incoming_packet(crow::packet&nbsp;*pack)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;sh&nbsp;=&nbsp;crow::node::subheader(pack);<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;data&nbsp;=&nbsp;crow::node_data(pack);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;char&nbsp;*ansbuf&nbsp;=&nbsp;(char&nbsp;*)malloc(answer_buffer_size);<br>
&nbsp;&nbsp;&nbsp;&nbsp;memset(ansbuf,&nbsp;0,&nbsp;answer_buffer_size);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;handle(data.data(),&nbsp;data.size(),&nbsp;ansbuf,&nbsp;answer_buffer_size&nbsp;-&nbsp;1);<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;anslen&nbsp;=&nbsp;strlen(ansbuf);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(anslen&nbsp;&gt;&nbsp;0)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::node::send(id,&nbsp;sh-&gt;sid,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;nos::buffer(pack-&gt;addrptr(),&nbsp;pack-&gt;addrsize()),<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;nos::buffer(ansbuf,&nbsp;anslen),&nbsp;2,&nbsp;200);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;free(ansbuf);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;crow::release(pack);<br>
&nbsp;&nbsp;&nbsp;&nbsp;return;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
