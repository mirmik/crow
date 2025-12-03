<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/src/address.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/address.h&gt;<br>
#include&nbsp;&lt;crow/warn.h&gt;<br>
<br>
namespace&nbsp;crow<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;hostaddr&nbsp;address(const&nbsp;nos::buffer&nbsp;&amp;in)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;uint8_t&gt;&nbsp;out;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out.resize(in.size());<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;len&nbsp;=&nbsp;hexer_s((uint8_t&nbsp;*)out.data(),&nbsp;in.size(),&nbsp;in.data());<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(len&nbsp;&lt;&nbsp;0)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;{};<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out.resize(len);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;out;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;hostaddr&nbsp;address_warned(const&nbsp;nos::buffer&nbsp;&amp;in)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;uint8_t&gt;&nbsp;out;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out.resize(in.size());<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;len&nbsp;=&nbsp;hexer_s((uint8_t&nbsp;*)out.data(),&nbsp;in.size(),&nbsp;in.data());<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(len&nbsp;==&nbsp;CROW_HEXER_MORE3_DOT)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::warn(&quot;crow::hexer:&nbsp;more&nbsp;then&nbsp;three&nbsp;symbols&nbsp;after&nbsp;dot.&quot;);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(len&nbsp;==&nbsp;CROW_HEXER_ODD_GRID)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::warn(&quot;crow::hexer:&nbsp;odd&nbsp;symbols&nbsp;after&nbsp;#&quot;);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(len&nbsp;==&nbsp;CROW_HEXER_UNDEFINED_SYMBOL)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::warn(&quot;crow::hexer:&nbsp;undefined&nbsp;symbol&quot;);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(len&nbsp;&lt;&nbsp;0)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;{};<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out.resize(len);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;out;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
}&nbsp;//&nbsp;namespace&nbsp;crow<br>
<br>
crow::hostaddr&nbsp;crow::crowker_address()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;crow::address(&quot;.12.127.0.0.1:10009&quot;);&nbsp;//&nbsp;default&nbsp;crowker&nbsp;address<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
