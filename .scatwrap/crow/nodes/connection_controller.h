<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/nodes/connection_controller.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;CROW_CONNECTION_CONTROLLER_H<br>
#define&nbsp;CROW_CONNECTION_CONTROLLER_H<br>
<br>
#include&nbsp;&lt;crow/nodes/node.h&gt;<br>
<br>
namespace&nbsp;crow<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;class&nbsp;connection_controller&nbsp;:&nbsp;public&nbsp;node<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::hostaddr&nbsp;addr;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;connection_controller(const&nbsp;crow::hostaddr&nbsp;&amp;)&nbsp;=&nbsp;default;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;on_connect()&nbsp;override&nbsp;{}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;on_disconnect()&nbsp;override&nbsp;{}<br>
&nbsp;&nbsp;&nbsp;&nbsp;};<br>
}&nbsp;//&nbsp;namespace&nbsp;crow<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
