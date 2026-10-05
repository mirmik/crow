<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/brocker/options.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#ifndef&nbsp;CROWKER_OPTIONS_H<br>
#define&nbsp;CROWKER_OPTIONS_H<br>
<br>
#include&nbsp;&lt;stdint.h&gt;<br>
<br>
namespace&nbsp;crowker_implementation<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;struct&nbsp;options<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint8_t&nbsp;qos&nbsp;=&nbsp;0;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint16_t&nbsp;ackquant&nbsp;=&nbsp;200;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;options(uint8_t&nbsp;qos,&nbsp;uint16_t&nbsp;ackquant)&nbsp;:&nbsp;qos(qos),&nbsp;ackquant(ackquant)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;options()&nbsp;=&nbsp;default;<br>
&nbsp;&nbsp;&nbsp;&nbsp;};<br>
}&nbsp;//&nbsp;namespace&nbsp;crowker_implementation<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
