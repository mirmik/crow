<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>crow/hexer.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
/**&nbsp;@file&nbsp;*/<br>
<br>
#ifndef&nbsp;CROW_HEXER_H<br>
#define&nbsp;CROW_HEXER_H<br>
<br>
#include&nbsp;&lt;igris/compiler.h&gt;<br>
#include&nbsp;&lt;stdint.h&gt;<br>
#include&nbsp;&lt;stdlib.h&gt;<br>
<br>
#define&nbsp;CROW_HEXER_UNDEFINED_SYMBOL&nbsp;-1<br>
#define&nbsp;CROW_HEXER_MORE3_DOT&nbsp;-4<br>
#define&nbsp;CROW_HEXER_ODD_GRID&nbsp;-5<br>
<br>
///&nbsp;Утилита&nbsp;для&nbsp;удобной&nbsp;записи&nbsp;хекс&nbsp;адреса.<br>
///&nbsp;Пример:&nbsp;.12.127.0.0.1#2714&nbsp;&nbsp;-&gt;&nbsp;0C7F0000012714<br>
///&nbsp;Пример:&nbsp;.12.127.0.0.1:10004&nbsp;-&gt;&nbsp;0C7F0000012714<br>
///&nbsp;Пример:&nbsp;.12.127:.1:10004&nbsp;-&gt;&nbsp;0C7F0000012714<br>
///&nbsp;Пример:&nbsp;.12#7F000001:10004&nbsp;-&gt;&nbsp;0C7F0000012714<br>
///&nbsp;Пример:&nbsp;#0C#7F000001#2714&nbsp;-&gt;&nbsp;0C7F0000012714<br>
///&nbsp;Пример:&nbsp;#0C7F0000012714&nbsp;-&gt;&nbsp;0C7F0000012714<br>
int&nbsp;hexer(uint8_t&nbsp;*dst,&nbsp;size_t&nbsp;maxsz,&nbsp;const&nbsp;char&nbsp;*src,&nbsp;size_t&nbsp;srcsz);<br>
int&nbsp;hexer_s(uint8_t&nbsp;*dst,&nbsp;size_t&nbsp;maxsz,&nbsp;const&nbsp;char&nbsp;*src);<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
