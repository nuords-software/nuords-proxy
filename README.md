**NUORDS PROXY SERVER** [a relative link]../README.md


NuoRDS Proxy (proxy) is a protocol agnostic reverse proxy and IP-based load balancer. Originally designed for NuoRDS Terminal Server for Mac, the NuoRDS Proxy can in fact distribute other connection based protocols, such as VNC, ARD, etc. However, keep in mind that usage of NuoRDS Proxy with request/response protocols, such as HTTP, may not be effective. For a request/response protocol, we recommend that you use a protocol dependent solution.

The proxy distributes client connections based on capacity of a target server. If the target server is NuoRDS Terminal Server, then other parameters, such CPU and memory usage, can be added into equation.

The proxy can also filter connections and cache a target server selection. Since these features are based on client's IP, they are more effective in a local network or VPN, where each client has an unique IP address. 

[a relative link to HOW TO BUILD NUORDS PROXY SERVER](doc/BUILD.md)

[a relative link](doc/ADDRESS.md)
 
[a relative link](doc/INSTALL.md)

[a relative link](doc/CONFIG.md)
