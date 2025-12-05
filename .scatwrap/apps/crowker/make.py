<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>apps/crowker/make.py</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#!/usr/bin/env&nbsp;python3<br>
#coding:&nbsp;utf-8<br>
<br>
import&nbsp;licant<br>
import&nbsp;licant.install<br>
from&nbsp;licant.cxx_modules&nbsp;import&nbsp;application<br>
from&nbsp;licant.libs&nbsp;import&nbsp;include<br>
import&nbsp;os<br>
<br>
licant.include(&quot;crow&quot;)<br>
<br>
application(&quot;crowker&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;sources=[<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;main.cpp&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;control_node.cpp&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;],<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;mdepends=[<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;crow&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;crow.crowker&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;crow.udpgate&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;],<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;optimize=&quot;-O0&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cc_flags=&quot;-flto&nbsp;-fPIC&nbsp;-Wextra&nbsp;-Wall&nbsp;-ffunction-sections&nbsp;-fdata-sections&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cxx_flags=&quot;-flto&nbsp;-fPIC&nbsp;-Wextra&nbsp;-Wall&nbsp;-ffunction-sections&nbsp;-fdata-sections&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ld_flags=&quot;-flto&nbsp;-fPIC&nbsp;-static&nbsp;-Wl,--whole-archive&nbsp;-lpthread&nbsp;-Wl,--no-whole-archive&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;libs=[&quot;readline&quot;,&nbsp;&quot;nos&quot;,&nbsp;&quot;igris&quot;]<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;)<br>
<br>
licant.install.install_application(tgt=&quot;install_crowker&quot;,&nbsp;src=&quot;crowker&quot;,&nbsp;dst=&quot;crowker&quot;)<br>
<br>
if&nbsp;__name__&nbsp;==&nbsp;&quot;__main__&quot;:<br>
&nbsp;&nbsp;&nbsp;&nbsp;licant.install.install_application(tgt=&quot;install&quot;,&nbsp;src=&quot;crowker&quot;,&nbsp;dst=&quot;crowker&quot;)<br>
&nbsp;&nbsp;&nbsp;&nbsp;licant.ex(&quot;crowker&quot;)<br>
<!-- END SCAT CODE -->
</body>
</html>
