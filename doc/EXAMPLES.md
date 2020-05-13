**NUORDS PROXY CONFIGURATION EXAMPLES**  [(?)](README.md)  
  
This document contains a few simple examples. For more options please see:  
  
[NuoRDS Proxy configuration.](CONFIG.md)  
  
**Two target servers with proxy on a third host.**  
  
The proxy server listens for IPv4 and IPv6 connections. 'client01' allows connection from any client address. 'balancer01' distributes connections from all allowed clients (client01) among all available servers (server01, server02). The proxy server caches all routes with default timeout (60 sec).  
  
```  
[proxy]  
  
listeners = 0.0.0.0:3389 [::]:3389  
  
[server.server01]  
  
address = 192.168.0.101:3389  
  
max_slots = 10  
  
[server.server02]  
  
address = 192.168.0.102:3389  
  
max_slots = 10  
  
[client.client01]  
  
sources = 0.0.0.0/0 ::/0  
  
[balancer.balancer01]  
  
clients = all  
servers = all  
  
```  
  
**Two target servers with proxy on the first host.**  
  
This configuration is similar to previous one, except we assume that the proxy server is installed on the same host with the 'server01'. In order to avoid conflicts with the main protocol, the target port of the 'server01' has been changed. Please note that configuration of the 'server01' software must reflect the changed port. Also we assume that target server is NuoRDS Terminal Server. Therefore, to avoid conflicts with the info protocol, the info port has also been changed. The info port change affects proxy only and should not be reflected by any server configuration.  
  
```  
[proxy]  
  
listeners = 0.0.0.0:3389 [::]:3389  
  
info_port = 4074  
  
[server.server01]  
  
address = 192.168.0.101:3390  
  
max_slots = 10  
  
[server.server02]  
  
address = 192.168.0.102:3389  
  
max_slots = 10  
  
[client.client01]  
  
sources = 0.0.0.0/0 ::/0  
  
[balancer.balancer01]  
  
clients = all  
servers = all  
  
```  
  
**A non-caching IPv4 proxy with client filtering.**  
  
The proxy server listens for IPv4 connections only. 'client01' allows only connections from '192.168.0.X' subnet. 'client02' blocks the rest of IP addresses. 'balancer01' distributes incoming connections from 'client01' to 'server01' and 'server02'. The balancing metrics include memory and CPU usage. The proxy server never caches any routes.  
  
```  
[proxy]  
  
listeners = 0.0.0.0:3389  
  
cache_timeout = 0  
  
[server.server01]  
  
address = 192.168.0.101:3389  
  
max_slots = 10  
  
[server.server02]  
  
address = 192.168.0.102:3389  
  
max_slots = 10  
  
[client.client01]  
  
sources = 192.168.0.0/24  
  
[client.client02]  
  
sources = 0.0.0.0/0  
  
max_slots = 0  
  
[balancer.balancer01]  
  
clients = client01  
servers = server01 server02  
metrics = cap mem cpu  
  
```  
  
------------------------------  
[< NuoRDS Proxy](README.md)    
  