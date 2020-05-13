**NUORDS PROXY SERVER**

NuoRDS Proxy (proxy) is a protocol agnostic reverse proxy and IP-based load balancer. Originally designed for NuoRDS Terminal Server for Mac, the NuoRDS Proxy can in fact distribute other connection based protocols, such as VNC, ARD, etc. However, keep in mind that usage of NuoRDS Proxy with request/response protocols, such as HTTP, may not be effective. For request/response protocols, we recommend that you use a protocol dependent solution.

The proxy distributes client connections based on capacity of a target server. If the target server is NuoRDS Terminal Server, then other parameters, such CPU and memory usage, can be added into equation.

The proxy can also filter connections and cache a target server selection. Since these features are based on client's IP, they are more effective in a local network or VPN, where each client has an unique IP address. 

[How to build NuoRDS Proxy.](doc/BUILD.md)

[How to install NuoRDS Proxy.](doc/INSTALL.md)

[How to configure NuoRDS Proxy.](doc/CONFIG.md)

[NuoRDS Proxy address format.](doc/ADDRESS.md)
