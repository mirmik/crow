<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/src/hostaddr.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/address.h&gt;<br>
#include&nbsp;&lt;crow/hostaddr.h&gt;<br>
#include&nbsp;&lt;crow/hostaddr_view.h&gt;<br>
<br>
crow::hostaddr_view&nbsp;crow::hostaddr::view()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;hostaddr_view(_addr.data(),&nbsp;_addr.size());<br>
}<br>
<br>
crow::hostaddr::operator&nbsp;crow::hostaddr_view()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;view();<br>
}<br>
<br>
crow::hostaddr::hostaddr(const&nbsp;crow::hostaddr_view&nbsp;&amp;h)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;_addr&nbsp;=&nbsp;std::vector&lt;uint8_t&gt;((uint8_t&nbsp;*)h.data(),<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(uint8_t&nbsp;*)h.data()&nbsp;+&nbsp;h.size());<br>
}<br>
<br>
crow::hostaddr::hostaddr(const&nbsp;std::string&nbsp;&amp;str)&nbsp;:&nbsp;hostaddr(crow::address(str))<br>
{<br>
}<br>
<br>
std::string&nbsp;crow::hostaddr::to_string()&nbsp;const<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;view().to_string();<br>
}<br>
<br>
const&nbsp;crow::hostaddr_view&nbsp;crow::hostaddr::view()&nbsp;const<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;hostaddr_view(_addr.data(),&nbsp;_addr.size());<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
