<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>apps/crowker/webui.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#ifndef&nbsp;CROWKER_WEBUI_H<br>
#define&nbsp;CROWKER_WEBUI_H<br>
<br>
#include&nbsp;&lt;cstdint&gt;<br>
#include&nbsp;&lt;functional&gt;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;thread&gt;<br>
<br>
namespace&nbsp;crowker_webui<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;/**<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*&nbsp;@brief&nbsp;Initialize&nbsp;and&nbsp;start&nbsp;the&nbsp;web&nbsp;UI&nbsp;server<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*&nbsp;@param&nbsp;port&nbsp;HTTP&nbsp;port&nbsp;to&nbsp;listen&nbsp;on&nbsp;(default:&nbsp;8080)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*&nbsp;@param&nbsp;enable_log&nbsp;Enable&nbsp;HTTP&nbsp;request&nbsp;logging<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*&nbsp;@return&nbsp;true&nbsp;if&nbsp;server&nbsp;started&nbsp;successfully<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;start(uint16_t&nbsp;port&nbsp;=&nbsp;8080,&nbsp;bool&nbsp;enable_log&nbsp;=&nbsp;false);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;/**<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*&nbsp;@brief&nbsp;Stop&nbsp;the&nbsp;web&nbsp;UI&nbsp;server<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;stop();<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;/**<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*&nbsp;@brief&nbsp;Check&nbsp;if&nbsp;web&nbsp;UI&nbsp;server&nbsp;is&nbsp;running<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;is_running();<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;/**<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*&nbsp;@brief&nbsp;Get&nbsp;the&nbsp;port&nbsp;the&nbsp;server&nbsp;is&nbsp;listening&nbsp;on<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
&nbsp;&nbsp;&nbsp;&nbsp;uint16_t&nbsp;get_port();<br>
<br>
}&nbsp;//&nbsp;namespace&nbsp;crowker_webui<br>
<br>
#endif&nbsp;//&nbsp;CROWKER_WEBUI_H<br>
<!-- END SCAT CODE -->
</body>
</html>
