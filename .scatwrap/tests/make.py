<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/make.py</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#!/usr/bin/env&nbsp;python3<br>
<br>
import&nbsp;licant<br>
from&nbsp;licant.cxx_modules&nbsp;import&nbsp;application<br>
from&nbsp;licant.modules&nbsp;import&nbsp;submodule,&nbsp;module<br>
from&nbsp;licant.libs&nbsp;import&nbsp;include<br>
<br>
licant.include(&quot;crow&quot;,&nbsp;&quot;../crow.g.py&quot;)<br>
<br>
tests_c&nbsp;=&nbsp;[<br>
]<br>
<br>
application(&quot;runtests&quot;,<br>
	sources&nbsp;=&nbsp;[<br>
		&quot;*.cpp&quot;,<br>
		&quot;brocker/*.cpp&quot;<br>
	],<br>
	mdepends=[&quot;crow&quot;,&nbsp;&quot;crow.udpgate&quot;],<br>
<br>
	cxxstd=&quot;gnu++17&quot;,<br>
	ccstd=&quot;c11&quot;,<br>
	cxx_flags&nbsp;=&nbsp;&quot;-g&nbsp;-Werror=all&nbsp;-Werror=pedantic&nbsp;-Weffc++&nbsp;-Wno-deprecated-declarations&quot;,<br>
	cc_flags&nbsp;=&nbsp;&quot;-g&nbsp;-Werror=all&nbsp;-Werror=pedantic&nbsp;-Wno-deprecated-declarations&quot;,<br>
<br>
	include_paths&nbsp;=&nbsp;[&quot;.&quot;],<br>
	libs&nbsp;=&nbsp;[&quot;igris&quot;,&nbsp;&quot;nos&quot;,&nbsp;&quot;pthread&quot;],<br>
)<br>
<br>
licant.ex(&quot;runtests&quot;)<br>
<!-- END SCAT CODE -->
</body>
</html>
