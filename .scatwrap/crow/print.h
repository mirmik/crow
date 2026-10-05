<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/print.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#ifndef&nbsp;CROW_PRINT_H<br>
#define&nbsp;CROW_PRINT_H<br>
<br>
#include&nbsp;&lt;crow/packet.h&gt;<br>
#include&nbsp;&lt;string&gt;<br>
<br>
namespace&nbsp;crow<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;diagnostic(const&nbsp;char&nbsp;*notation,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::packet&nbsp;*pack,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;label,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint16_t&nbsp;dbg_size);<br>
}<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
