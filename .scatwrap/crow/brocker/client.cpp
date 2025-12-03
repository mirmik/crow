<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/brocker/client.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#include&nbsp;&quot;client.h&quot;<br>
#include&nbsp;&quot;crowker.h&quot;<br>
#include&nbsp;&lt;nos/log.h&gt;<br>
<br>
crowker_implementation::client::~client()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;detach_from_themes();<br>
}<br>
<br>
crowker_implementation::client::client()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;name&nbsp;is&nbsp;random&nbsp;string<br>
&nbsp;&nbsp;&nbsp;&nbsp;_name&nbsp;=&nbsp;std::to_string((uint64_t)this);<br>
}<br>
<br>
void&nbsp;crowker_implementation::client::detach_from_themes()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::lock_guard&lt;std::recursive_mutex&gt;&nbsp;guard(mtx);<br>
&nbsp;&nbsp;&nbsp;&nbsp;while&nbsp;(!_thms.empty())<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;it&nbsp;=&nbsp;_thms.begin();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::crowker::instance()-&gt;unlink_theme_client(it-&gt;first,&nbsp;this);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;_thms.clear();<br>
}<br>
<br>
void&nbsp;crowker_implementation::client::forgot_theme(theme&nbsp;*thm)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::lock_guard&lt;std::recursive_mutex&gt;&nbsp;guard(mtx);<br>
&nbsp;&nbsp;&nbsp;&nbsp;_thms.erase(thm);<br>
}<br>
<br>
crowker_implementation::options<br>
crowker_implementation::client::get_theme_options(theme&nbsp;*thm)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::lock_guard&lt;std::recursive_mutex&gt;&nbsp;guard(mtx);<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(_thms.count(thm)&nbsp;==&nbsp;0)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;throw&nbsp;std::runtime_error(&quot;Theme&nbsp;not&nbsp;found&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;_thms[thm];<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
