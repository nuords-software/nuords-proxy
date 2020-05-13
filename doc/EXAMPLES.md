**NUORDS PROXY CONFIGURATION EXAMPLES**  [(?)](README.md)

**A simple two-target configuration**

This configuration distributes incoming RDP connection among two target server. Each target server can accept 10 simultanous connections. The client01 group represents all IPv4 and IPv6 addresses.  
------------------------------
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
------------------------------
[< NuoRDS Proxy](README.md) 
