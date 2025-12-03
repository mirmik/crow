<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/brocker/tcp_client.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#include&nbsp;&quot;tcp_client.h&quot;<br>
<br>
std::map&lt;nos::inet::netaddr,&nbsp;crowker_implementation::tcp_client&gt;<br>
&nbsp;&nbsp;&nbsp;&nbsp;crowker_implementation::tcp_client::allsubs;<br>
void&nbsp;crowker_implementation::tcp_client::publish(const&nbsp;std::string&nbsp;&amp;theme,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;data,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;options&nbsp;opts)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;(void)opts;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;str&nbsp;=&nbsp;nos::format(&quot;p{:f02&gt;}{}{:f06&gt;}{}&quot;,&nbsp;theme.size(),&nbsp;theme,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;data.size(),&nbsp;data);<br>
&nbsp;&nbsp;&nbsp;&nbsp;sock-&gt;write(str.data(),&nbsp;str.size());<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
