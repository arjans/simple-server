simple-server
=============

A simple web server written in C.

Setup
-----
To install and run:

Clone the repo: ```git clone https://github.com/jns2/simple-server.git```

Run the Makefile: ```make```

Run the executable: ```./simpleserver```

Usage
-----
To test the server, run:
```curl localhost:3500```
in a different window from where you ran the executable ```./simpleserver```.

Or point your browser to:
[localhost:3500](http://localhost:3500/)

In the window where you're running simpleserver, you should see output like this, where you can see the IP address the request came form, how much data was received over the socket, the content of the request, and what was sent back in response:
<pre>
===OPEN SOCKET===
++listener: got packet from 127.0.0.1
++recv()'d 424 bytes of data

GET /test.css HTTP/1.1
Host: localhost:3500
Connection: keep-alive
Cache-Control: no-cache
Pragma: no-cache
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_4) AppleWebKit/537.1 (KHTML, like Gecko) Chrome/21.0.1180.77 Safari/537.1
Accept: text/css,*/*;q=0.1
Referer: http://localhost:3500/
Accept-Encoding: gzip,deflate,sdch
Accept-Language: en-US,en;q=0.8
Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3


++file searched: ./test.css
body { font: 20px Helvetica }
===SOCKET CLOSED===
</pre>
