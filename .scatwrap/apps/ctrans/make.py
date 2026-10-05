<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>apps/ctrans/make.py</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#!/usr/bin/env&nbsp;python3<br>
#coding:&nbsp;utf-8<br>
<br>
import&nbsp;os<br>
<br>
import&nbsp;licant<br>
import&nbsp;licant.install<br>
from&nbsp;licant.cxx_modules&nbsp;import&nbsp;application<br>
from&nbsp;licant.libs&nbsp;import&nbsp;include<br>
<br>
defines&nbsp;=&nbsp;[&quot;NOTRACE=1&quot;]<br>
<br>
application(&quot;ctrans&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;sources=[<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;main.cpp&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;],<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;mdepends=[<br>
#&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;crow&quot;,<br>
&nbsp;#&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;crow.crowker&quot;,<br>
&nbsp;&nbsp;#&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;crow.udpgate&quot;,<br>
&nbsp;&nbsp;&nbsp;#&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;crow.tcpgate&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;#&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;crow.serial_gstuff&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;],<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;defines=defines,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;optimize=&quot;-O0&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cc_flags=&quot;-flto&nbsp;-fPIC&nbsp;-Wextra&nbsp;-Wall&nbsp;-ffunction-sections&nbsp;-fdata-sections&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cxx_flags=&quot;-flto&nbsp;-fPIC&nbsp;-Wextra&nbsp;-Wall&nbsp;-ffunction-sections&nbsp;-fdata-sections&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ld_flags=&quot;-flto&nbsp;-fPIC&nbsp;-static&nbsp;-Wl,--whole-archive&nbsp;-lpthread&nbsp;-Wl,--strip-all&nbsp;-Wl,--no-whole-archive&nbsp;-ffunction-sections&nbsp;-fdata-sections&nbsp;-Wl,--gc-sections&quot;,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;libs=[&quot;pthread&quot;,&nbsp;&quot;readline&quot;,&nbsp;&quot;igris&quot;,&nbsp;&quot;nos&quot;,&nbsp;&quot;crow&quot;],<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cxxstd=&quot;c++17&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;)<br>
<br>
<br>
@licant.routine<br>
def&nbsp;doc():<br>
&nbsp;&nbsp;&nbsp;&nbsp;os.system(&quot;cd&nbsp;doc;&nbsp;./make.sh&quot;)<br>
<br>
<br>
licant.install.install_application(tgt=&quot;install_ctrans&quot;,&nbsp;src=&quot;ctrans&quot;,&nbsp;dst=&quot;ctrans&quot;)<br>
<br>
if&nbsp;__name__&nbsp;==&nbsp;&quot;__main__&quot;:<br>
&nbsp;&nbsp;&nbsp;&nbsp;licant.install.install_application(tgt=&quot;install&quot;,&nbsp;src=&quot;ctrans&quot;,&nbsp;dst=&quot;ctrans&quot;)<br>
&nbsp;&nbsp;&nbsp;&nbsp;licant.ex(&quot;ctrans&quot;)<br>
<!-- END SCAT CODE -->
</body>
</html>
