<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/brocker/crow_client.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#ifndef&nbsp;CROW_BROCKER_CROW_SUBSCRIBER_H<br>
#define&nbsp;CROW_BROCKER_CROW_SUBSCRIBER_H<br>
<br>
#include&nbsp;&lt;map&gt;<br>
#include&nbsp;&lt;string&gt;<br>
<br>
#include&nbsp;&quot;client.h&quot;<br>
#include&nbsp;&quot;options.h&quot;<br>
<br>
namespace&nbsp;crowker_implementation<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;class&nbsp;crow_client&nbsp;:&nbsp;public&nbsp;client<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;addr&nbsp;=&nbsp;{};<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;static&nbsp;std::map&lt;std::string,&nbsp;crow_client&gt;&nbsp;allsubs;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;static&nbsp;crow_client&nbsp;*get(const&nbsp;std::string&nbsp;&amp;addr)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;&amp;allsubs[addr];<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;static&nbsp;std::vector&lt;client&nbsp;*&gt;&nbsp;clients()<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;client&nbsp;*&gt;&nbsp;ret;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(auto&nbsp;&amp;pair&nbsp;:&nbsp;allsubs)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ret.push_back(&amp;pair.second);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;ret;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;publish(const&nbsp;std::string&nbsp;&amp;theme,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;data,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;options&nbsp;opts)&nbsp;override;<br>
&nbsp;&nbsp;&nbsp;&nbsp;};<br>
}&nbsp;//&nbsp;namespace&nbsp;crowker_implementation<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
