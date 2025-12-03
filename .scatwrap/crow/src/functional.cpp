<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/src/functional.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/address.h&gt;<br>
#include&nbsp;&lt;crow/functional.h&gt;<br>
<br>
namespace&nbsp;crow<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::list&lt;std::unique_ptr&lt;crow::subscriber_node&gt;&gt;&nbsp;subscribers;<br>
}<br>
<br>
crow::subscriber_node&nbsp;&amp;crow::subscribe(nos::buffer&nbsp;theme,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::function&lt;void(nos::buffer)&gt;&nbsp;func)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;addr&nbsp;=&nbsp;crow::crowker_address();<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;node&nbsp;=&nbsp;std::make_unique&lt;crow::subscriber_node&gt;(addr,&nbsp;theme,&nbsp;func);<br>
&nbsp;&nbsp;&nbsp;&nbsp;node-&gt;install_keepalive(2000);<br>
&nbsp;&nbsp;&nbsp;&nbsp;subscribers.push_back(std::move(node));<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;*subscribers.back();<br>
}<br>
<br>
void&nbsp;crow::publish(nos::buffer&nbsp;theme,&nbsp;nos::buffer&nbsp;data)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;addr&nbsp;=&nbsp;crow::crowker_address();<br>
&nbsp;&nbsp;&nbsp;&nbsp;crow::publisher_node&nbsp;node(addr,&nbsp;theme);<br>
&nbsp;&nbsp;&nbsp;&nbsp;node.bind();<br>
&nbsp;&nbsp;&nbsp;&nbsp;node.publish(data);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
