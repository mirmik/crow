<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/nodes/service_node.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/nodes/pubsub_defs.h&gt;<br>
#include&nbsp;&lt;crow/nodes/service_node.h&gt;<br>
#include&nbsp;&lt;crow/warn.h&gt;<br>
#include&nbsp;&lt;nos/print.h&gt;<br>
<br>
void&nbsp;crow::service_node::incoming_packet(crow::packet&nbsp;*pack)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;curpack&nbsp;=&nbsp;pack;<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;&amp;subheader&nbsp;=&nbsp;pack-&gt;subheader&lt;consume_subheader&gt;();<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;data&nbsp;=&nbsp;subheader.message();<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;reply_theme_length&nbsp;=&nbsp;data.data()[0];<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;message&nbsp;=&nbsp;nos::buffer{data.data()&nbsp;+&nbsp;1&nbsp;+&nbsp;reply_theme_length,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;data.size()&nbsp;-&nbsp;1&nbsp;-&nbsp;reply_theme_length};<br>
&nbsp;&nbsp;&nbsp;&nbsp;dlg(message.data(),&nbsp;message.size(),&nbsp;*this);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;release&nbsp;after&nbsp;reply<br>
&nbsp;&nbsp;&nbsp;&nbsp;crow::release(pack);<br>
}<br>
<br>
void&nbsp;crow::service_node::reply(const&nbsp;char&nbsp;*answ,&nbsp;size_t&nbsp;size)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;&amp;subheader&nbsp;=&nbsp;curpack-&gt;subheader&lt;consume_subheader&gt;();<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;data&nbsp;=&nbsp;subheader.message();<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;reply_theme_length&nbsp;=&nbsp;data.data()[0];<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;reply_theme&nbsp;=&nbsp;nos::buffer(data.data()&nbsp;+&nbsp;1,&nbsp;reply_theme_length);<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(reply_theme&nbsp;!=&nbsp;&quot;__noanswer__&quot;)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;publish(curpack-&gt;addr(),&nbsp;subheader.sid,&nbsp;reply_theme,&nbsp;{answ,&nbsp;size},&nbsp;qos,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ackquant);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
