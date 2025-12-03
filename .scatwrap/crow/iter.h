<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/iter.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#ifndef&nbsp;CROW_ITER_H<br>
#define&nbsp;CROW_ITER_H<br>
<br>
#include&nbsp;&lt;crow/gateway.h&gt;<br>
#include&nbsp;&lt;crow/proto/node.h&gt;<br>
<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
namespace&nbsp;crow<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;crow::gateway&nbsp;*&gt;&nbsp;gates();<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;crow::node&nbsp;*&gt;&nbsp;nodes();<br>
}<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
