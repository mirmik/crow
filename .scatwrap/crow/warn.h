<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/warn.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#ifndef&nbsp;CROW_WARN_H<br>
#define&nbsp;CROW_WARN_H<br>
<br>
#include&nbsp;&lt;nos/buffer.h&gt;<br>
<br>
namespace&nbsp;crow<br>
{<br>
#if&nbsp;defined(MEMORY_ECONOMY)<br>
&nbsp;&nbsp;&nbsp;&nbsp;static&nbsp;inline&nbsp;void&nbsp;warn(nos::buffer)&nbsp;{}<br>
#else<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;warn(nos::buffer&nbsp;msg);<br>
#endif<br>
}<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
