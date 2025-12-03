<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/functional.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;CROW_FUNCTIONAL_H<br>
#define&nbsp;CROW_FUNCTIONAL_H<br>
<br>
#include&nbsp;&lt;crow/nodes/subscriber_node.h&gt;<br>
#include&nbsp;&lt;functional&gt;<br>
#include&nbsp;&lt;memory&gt;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;unordered_map&gt;<br>
<br>
namespace&nbsp;crow<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;extern&nbsp;std::list&lt;std::unique_ptr&lt;crow::subscriber_node&gt;&gt;&nbsp;subscribers;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;crow::subscriber_node&nbsp;&amp;subscribe(nos::buffer&nbsp;theme,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::function&lt;void(nos::buffer)&gt;);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;publish(nos::buffer&nbsp;theme,&nbsp;nos::buffer&nbsp;data);<br>
}<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
