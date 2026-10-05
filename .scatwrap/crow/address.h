<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/address.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#ifndef&nbsp;CROW_ADDRESS_H<br>
#define&nbsp;CROW_ADDRESS_H<br>
<br>
#include&nbsp;&lt;crow/hexer.h&gt;<br>
#include&nbsp;&lt;crow/hostaddr.h&gt;<br>
#include&nbsp;&lt;igris/compiler.h&gt;<br>
#include&nbsp;&lt;nos/buffer.h&gt;<br>
#include&nbsp;&lt;stdint.h&gt;<br>
#include&nbsp;&lt;stdlib.h&gt;<br>
<br>
namespace&nbsp;crow<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;hostaddr&nbsp;crowker_address();<br>
&nbsp;&nbsp;&nbsp;&nbsp;hostaddr&nbsp;address(const&nbsp;nos::buffer&nbsp;&amp;in);<br>
&nbsp;&nbsp;&nbsp;&nbsp;hostaddr&nbsp;address_warned(const&nbsp;nos::buffer&nbsp;&amp;in);<br>
}<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
