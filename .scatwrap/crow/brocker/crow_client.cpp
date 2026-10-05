<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/brocker/crow_client.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#include&nbsp;&quot;crow_client.h&quot;<br>
#include&nbsp;&lt;crow/pubsub/pubsub.h&gt;<br>
<br>
std::map&lt;std::string,&nbsp;crowker_implementation::crow_client&gt;<br>
&nbsp;&nbsp;&nbsp;&nbsp;crowker_implementation::crow_client::allsubs;<br>
<br>
void&nbsp;crowker_implementation::crow_client::publish(<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;theme,<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;data,<br>
&nbsp;&nbsp;&nbsp;&nbsp;crowker_implementation::options&nbsp;opts)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;(void)theme;<br>
&nbsp;&nbsp;&nbsp;&nbsp;(void)data;<br>
&nbsp;&nbsp;&nbsp;&nbsp;(void)opts;<br>
<br>
#ifdef&nbsp;CROW_PUBSUB_PROTOCOL_SUPPORTED<br>
&nbsp;&nbsp;&nbsp;&nbsp;::crow::publish({(uint8_t&nbsp;*)addr.data(),&nbsp;addr.size()},&nbsp;theme.c_str(),<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{data.data(),&nbsp;data.size()},&nbsp;opts.qos,&nbsp;opts.ackquant,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(uint8_t)crow::pubsub_type::MESSAGE);<br>
#endif<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
