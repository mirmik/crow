<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/src/variants/print-debug.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&lt;crow/print.h&gt;<br>
#include&nbsp;&lt;igris/dprint.h&gt;<br>
<br>
void&nbsp;crow::diagnostic(const&nbsp;char&nbsp;*notation,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;crow::packet&nbsp;*pack,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;label,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint16_t&nbsp;dbg_size)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;(void)dbg_size;&nbsp;//&nbsp;Not&nbsp;used&nbsp;in&nbsp;debug&nbsp;variant<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(!label.empty())<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;[&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;dpr(label.c_str());<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;]&nbsp;&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(notation);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;:&nbsp;(&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;ptr:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dprptr(pack);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;qos:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(pack-&gt;quality());<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;,ack:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr((uint8_t)pack-&gt;ack());<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;,atim:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr((uint16_t)pack-&gt;ackquant());<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;,alen:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr((uint8_t)pack-&gt;addrsize());<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;,flen:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr((uint8_t)pack-&gt;full_length());<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;,type:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr((uint8_t)pack-&gt;type());<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;,addr:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;debug_writehex(pack-&gt;addrptr(),&nbsp;pack-&gt;addrsize());<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;,stg:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(pack-&gt;stage());<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;,rescount:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr((uint8_t)pack-&gt;_ackcount);<br>
&nbsp;&nbsp;&nbsp;&nbsp;dpr(&quot;,data:&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;debug_write(pack-&gt;dataptr(),&nbsp;20&nbsp;&gt;&nbsp;pack-&gt;datasize()&nbsp;?&nbsp;pack-&gt;datasize()&nbsp;:&nbsp;20);<br>
&nbsp;&nbsp;&nbsp;&nbsp;debug_putchar(')');<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
