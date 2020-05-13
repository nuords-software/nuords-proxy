**NUORDS PROXY CONFIGURATION EXAMPLES**  [(?)](README.md)

This document contains a simple examples only. For more option please see:

[How to configure NuoRDS Proxy.](CONFIG.md) 

**Two target servers with the proxy on a third host**

This configuration distributes incoming RDP connection among two target server. Each target server can accept 10 simultanous connections. The client01 group represents all IPv4 and IPv6 addresses.  

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

max_slots = 1000

[balancer.balancer01]

clients = all
servers = all
metrics = cap

```

**Two target servers with the proxy on the first host**

This configuration is similar to previous one, except we assume that the proxy server is istalled on the same host with server01. In order to avoid conflicts with the main protocol, the target port of server01 should be changed. Please note that the server configuration must reflect the change of port. Also we assume that target server is NuoRDS Terminal Server. Therefore,to avoid conflicts with the info protocol, the info port should be changed as well. Ths change affects only proxy and should not be reflected by the server config.    

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

max_slots = 1000

[balancer.balancer01]

clients = all
servers = all
metrics = cap

```


------------------------------
[< NuoRDS Proxy](README.md) 
