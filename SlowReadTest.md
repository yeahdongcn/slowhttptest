#How to run Slow Read test

# Slow Read Denial of Service attack #

Idea is to send legitimate HTTP requests, and read responses slowly aiming to keep as many connections as possible in active state.


# Details #

Attack exploits the fact that most of modern web servers are not limiting the connection duration if there is a data flow going on, and with possiblity to prolong TCP connection virtually forever with zero or minimal data flow by manipulating TCP receive window size value, it is possible to acquire concurent connections pool of the application.
Possibility to prolong TCP connection is described in several vulnerability reports: [MS09-048](http://www.microsoft.com/technet/security/Bulletin/MS09-048.mspx), [CVE-2008-4609](http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2008-4609), [CVE-2009-1925](http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2009-1925), [CVE-2009-1926](http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2009-1926).

Prerequisites for the successful attack are:
- victim server should accept connections with advertised window smaller than server socket send buffer, the smaller the better
– attacker needs to request a resource that doesn't fit into server's socket send buffer, which is usually between 64K and 128K. To fill up server socket's send buffer for sure, consider using HTTP pipelining (-k argument of slowhttptest)

slowhttptest controls the incoming data rate by manipulating receive buffer size through SO\_RCVBUF socket option _and_ by varying reading rate from it by application.
Note, that different operating systems might have different behavior. For example, OSX uses the value we set SO\_RCVBUF to in initial SYN packet, while Linux systems double this value (to allow space for bookkeeping overhead) when it is set using setsockopt. Minimum doubled value for Linux systems is 256 or the first value of /proc/sys/net/ipv4/tcp\_rmem, whichever is larger. Also, changing receive buffer size on OSX doesn't work if connecting to localhost.

For SSL connections, slow reading from receive buffer engages after SSL handshake is finished. However, as initial window size is smaller than usual, handshake might require more TCP packets and last longer than usual.

[My write up](http://blog.shekyan.com/2012/01/are-you-ready-for-slow-reading.html) on the topic reveals even more details.

# Example #

Actual example of usage:
```
./slowhttptest -c 1000 -X -g -o slow_read_stats -r 200 -w 512 -y 1024 -n 5 -z 32 -k 3 -u https://myseceureserver/resources/index.html -p 3 
```

-X starts Slow Read test with 1000 connections, creating 200 connections per second.
Initial SYN packet for every connection would have random advertised window size value between 512 and 1024, and application would read 32 bytes every 5 seconds from each socket's receive buffer.
To multiply overall response size, we use pipeline factor 3 to request the same resource 3 times per socket.
Probe connection would consider server DoSed, if no response was received after 3 seconds.