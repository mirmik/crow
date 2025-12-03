<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/brocker/crowker_api.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/brocker/crowker_api.h&gt;<br>
#include&nbsp;&lt;crow/brocker/crowker_pubsub_node.h&gt;<br>
#include&nbsp;&lt;crow/nodes/pubsub_defs.h&gt;<br>
#include&nbsp;&lt;igris/util/bug.h&gt;<br>
<br>
std::vector&lt;client&nbsp;*&gt;&nbsp;crow::crowker::clients()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;client&nbsp;*&gt;&nbsp;ret;<br>
&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(auto&nbsp;*client&nbsp;:&nbsp;crowker_implementation::crow_client::clients())<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ret.push_back(client);<br>
&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(auto&nbsp;*client&nbsp;:&nbsp;crowker_implementation::tcp_client::clients())<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ret.push_back(client);<br>
&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(auto&nbsp;*api&nbsp;:&nbsp;apivec)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(auto&nbsp;*client&nbsp;:&nbsp;api-&gt;get_clients())<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ret.push_back(client);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;ret;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
