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
#include&nbsp;&lt;crow/tower.h&gt;<br>
#ifdef&nbsp;__WIN32__<br>
#include&nbsp;&lt;winsock2.h&gt;<br>
WSADATA&nbsp;wsaData;<br>
#endif<br>
<br>
int&nbsp;main(int&nbsp;argc,&nbsp;char**&nbsp;argv)<br>
{<br>
#ifdef&nbsp;__WIN32__<br>
	int&nbsp;iResult;<br>
<br>
	//&nbsp;Initialize&nbsp;Winsock<br>
	iResult&nbsp;=&nbsp;WSAStartup(MAKEWORD(2,2),&nbsp;&amp;wsaData);<br>
	if&nbsp;(iResult&nbsp;!=&nbsp;0)&nbsp;{<br>
		printf(&quot;WSAStartup&nbsp;failed:&nbsp;%d\n&quot;,&nbsp;iResult);<br>
		return&nbsp;1;<br>
	}<br>
#endif<br>
<br>
	doctest::Context&nbsp;context;<br>
	context.applyCommandLine(argc,&nbsp;argv);<br>
<br>
	int&nbsp;res&nbsp;=&nbsp;context.run();<br>
<br>
	if&nbsp;(context.shouldExit())<br>
		return&nbsp;res;<br>
<br>
	return&nbsp;res;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
