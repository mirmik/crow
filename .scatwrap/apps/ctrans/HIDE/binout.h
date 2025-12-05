<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>apps/ctrans/HIDE/binout.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&lt;igris/buffer.h&gt;<br>
#include&nbsp;&lt;string&gt;<br>
<br>
#include&nbsp;&lt;crow/packet.h&gt;<br>
<br>
std::string&nbsp;flt32_output(void&nbsp;*tgt);<br>
std::string&nbsp;int32_output(void&nbsp;*tgt);<br>
<br>
void&nbsp;binout_mode_prepare(const&nbsp;std::string&nbsp;&amp;fmt);<br>
void&nbsp;output_binary(nos::buffer&nbsp;data,&nbsp;crow::packet&nbsp;*pack);<br>
<!-- END SCAT CODE -->
</body>
</html>
