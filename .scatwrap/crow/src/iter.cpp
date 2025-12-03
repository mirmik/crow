<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/src/iter.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;crow/iter.h&quot;<br>
<br>
std::vector&lt;crow::gateway&nbsp;*&gt;&nbsp;crow::gates()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;crow::gateway&nbsp;*&gt;&nbsp;ret;<br>
&nbsp;&nbsp;&nbsp;&nbsp;crow::gateway&nbsp;*ref;<br>
&nbsp;&nbsp;&nbsp;&nbsp;dlist_for_each_entry(ref,&nbsp;&amp;crow::gateway_list,&nbsp;lnk)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ret.push_back(ref);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;ret;<br>
}<br>
<br>
std::vector&lt;crow::node&nbsp;*&gt;&nbsp;crow::nodes()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;crow::node&nbsp;*&gt;&nbsp;ret;<br>
&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(auto&nbsp;&amp;ref&nbsp;:&nbsp;crow::nodes_list)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ret.push_back(&amp;ref);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;ret;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
