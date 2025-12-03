<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/main.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#define&nbsp;DOCTEST_CONFIG_IMPLEMENT<br>
#include&nbsp;&quot;doctest/doctest.h&quot;<br>
#include&nbsp;&lt;crow/gates/udpgate.h&gt;<br>
#include&nbsp;&lt;crow/gates/loopgate.h&gt;<br>
#include&nbsp;&lt;crow/tower.h&gt;<br>
#ifdef&nbsp;__WIN32__<br>
#include&nbsp;&lt;winsock2.h&gt;<br>
WSADATA&nbsp;wsaData;<br>
#endif<br>
<br>
crow::udpgate&nbsp;udpgate;<br>
crow::loopgate&nbsp;loopgate;<br>
int&nbsp;main(int&nbsp;argc,&nbsp;char**&nbsp;argv)<br>
{<br>
#ifdef&nbsp;__WIN32__<br>
int&nbsp;iResult;<br>
<br>
//&nbsp;Initialize&nbsp;Winsock<br>
iResult&nbsp;=&nbsp;WSAStartup(MAKEWORD(2,2),&nbsp;&amp;wsaData);<br>
if&nbsp;(iResult&nbsp;!=&nbsp;0)&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;printf(&quot;WSAStartup&nbsp;failed:&nbsp;%d\n&quot;,&nbsp;iResult);<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;1;<br>
}<br>
#endif<br>
<br>
	crow::retransling_allowed&nbsp;=&nbsp;true;<br>
	loopgate.bind(99);<br>
	udpgate.bind(12);<br>
	udpgate.open(10099);<br>
<br>
	doctest::Context&nbsp;context;<br>
<br>
	int&nbsp;res&nbsp;=&nbsp;context.run();&nbsp;//&nbsp;run<br>
<br>
	if&nbsp;(context.shouldExit())&nbsp;//&nbsp;important&nbsp;-&nbsp;query&nbsp;flags&nbsp;(and&nbsp;--exit)&nbsp;rely&nbsp;on&nbsp;the&nbsp;user&nbsp;doing&nbsp;this<br>
		return&nbsp;res;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;propagate&nbsp;the&nbsp;result&nbsp;of&nbsp;the&nbsp;tests<br>
	int&nbsp;client_stuff_return_code&nbsp;=&nbsp;0;<br>
	return&nbsp;res&nbsp;+&nbsp;client_stuff_return_code;&nbsp;//&nbsp;the&nbsp;result&nbsp;from&nbsp;doctest&nbsp;is&nbsp;propagated&nbsp;here&nbsp;as&nbsp;well<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
