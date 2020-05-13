**NUORDS PROXY CONFIGURATION EXAMPLES**  [(?)](README.md)

This document contains a web simple examples. For more options please see:

[How to configure NuoRDS Proxy.](CONFIG.md) 

**Two target servers with proxy on a third host**

The proxy server listens for IPv4 and IPv6 connections. 'client01' allows any client address. 'balancer01' distributes incoming connections from all allowed clients (client01) among all available servers (server01, server02). 
 
```
[proxy]

listeners = 0.0.0.0:3389 [::]:3389

[server.server01]

address = 192.168.0.101:3389

max_slots  = 10

[server.server02]

address = 192.168.0.102:3389

max_slots  = 10

[client.client01]

sources = 0.0.0.0/0 ::/0

[balancer.balancer01]

clients = all
servers = all

```

**Two target servers with proxy on the first host**

This configuration is similar to previous one, except we assume that the proxy server is istalled on the same host with the 'server01'. In order to avoid conflicts with the main protocol, the target port of the 'server01' should be changed. Please note that the 'server01' configuration must reflect the change of port. Also we assume that target server is NuoRDS Terminal Server. Therefore,to avoid conflicts with the info protocol, the info port should be changed as well. Ths change affects only proxy and should not be reflected by the server config.    

```
[proxy]

listeners = 0.0.0.0:3389 [::]:3389

info_port 4074

[server.server01]

address = 192.168.0.101:3390

max_slots  = 10

[server.server02]

address = 192.168.0.102:3389

max_slots  = 10

[client.client01]

sources = 0.0.0.0/0 ::/0

[balancer.balancer01]

clients = all
servers = all

```

**A non-caching IPv4 proxy with client filtering**

The proxy server listens for IPv4 connections only. 'client01' allows only connections from 192.168.0.X subnet. 'client02' blocks the rest of IP addresses. 'balancer01' distributes incoming connections from 'client01' to 'server01' and 'server02'. The Balancing metrics include memory and CPU usage.

```
[proxy]

listeners = 0.0.0.0:3389

[server.server01]

address = 192.168.0.101:3389

max_slots  = 10

[server.server02]

address = 192.168.0.102:3389

max_slots  = 10

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