<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/address.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/address.h&gt;<br>
#include&nbsp;&lt;doctest/doctest.h&gt;<br>
<br>
TEST_CASE(&quot;address&quot;)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;addr&nbsp;=&nbsp;crow::address(&quot;#F2.12.127.0.0.1:10009&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;res&nbsp;=&nbsp;crow::hostaddr(<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;uint8_t&gt;{0xF2,&nbsp;12,&nbsp;127,&nbsp;0,&nbsp;0,&nbsp;1,&nbsp;0x27,&nbsp;0x19});<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_EQ(addr,&nbsp;res);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
