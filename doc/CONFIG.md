**NUORDS PROXY CONFIGURATION**  [(?)](README.md)

This document describes advanced options that you can use to tune the proxy server. Before proceeding with advanced tuning, we strongly recommedn that you test one of [simple configuration examples](CONFIG.md) 

**General proxy server options**

General proxy server options should be located under **\[proxy\]** section of configuration file. This section is singular and should not be extended with a name.

- The proxy listeners (IPv4|IPv6, TCP).
  
  **Property:** listeners  
  **Format:** Comma|Space-separated list of addresses  
  **Default**: 0.0.0.0:3389 [::]:3389  
  
  See [Network Address format](ADDRESS.md) for details. If you are running any target server on the same host, you shoud change the target server port to avoid conflicts.

- The NuoRDS info protocol availability.
  
  **Property:** info_enabled  
  **Format:** 0|1  
  **Default**: 1  
  
  If none of target servers is NuoRDS, then disable the info protocol by setting "0".
  
- The NuoRDS info protocol port (UDP).
  
  **Property:** info_port  
  **Format:** Integer  
  **Default**: 4073  

  If none of target servers is NuoRDS, then disable the NuoRDS info protocol by setting info_port = 0. If you are running NuoRDS Server on the same host, you should change this info port to avoid conflicts.
  
- The info request period in seconds.
  
  **Property:** info_period  
  **Format:** Integer,  10+  
  **Default**: 15  
  
  The info request period is a time after which a usage info should be requested from a server.
  
- The status check period in seconds.
  
  **Property:** status_period  
  **Format:** Integer,  60+  
  **Default**: 300  
  
  The status check period is a time after which availability of a server must be re-checked.
  
- The routing cache timeout in seconds.

  **Property:** cache_timeout  
  **Format:** Integer, 0|10+  
  **Default**: 60  

  Set "0" to disable routing cache. 
  
- The connection timeout in seconds.
  
  **Property:** conn_timeout  
  **Format:** Integer,  10...900  
  **Default**: 30  
  
  The connection timeout is a time within which onnection to the server must be established.
  
- Maximum number of connection faults.

  **Property:** max_faults  
  **Format:** Integer, 1+  
  **Default**: 2  

  A faulty server will be disabled till the next check.
  
- Socket buffering mode.
  
  **Property:** socket_mode  
  **Format:** Integer, see Values  
  **Default**: 0  
  
  **Values:**  
  0 - Use operating system defaults.  
  1 - Align socket buffers with a peer.  
  2 - Use 'max_buffer' for socket buffers.   
  
  See also 'max_buffer' and 'buffer_mode'.
  
- Internal buffering mode.

  **Property:** buffer_mode  
  **Format:** Integer, see Values  
  **Default**: 0  

  **Values:**  
  0 - Use proxy application defaults.  
  1 - Align internal buffers with a socket.  
  2 - Use 'max_buffer' for internal buffers.  
  
  See also 'max_buffer' and 'socket_mode'.
  
- Maximum IO buffer size.
  
  **Property:** max_buffer  
  **Format:** Integer, 8760...131072  
  **Default**: 131072  
  
  See also 'socket_mode' and 'buffer_mode'.
 
  A [server.NAME] section describes a target server.
  Add a [server.NAME] section for each target server.
  IMPORTANT: To avoid conflicts, do not use NAME "all".

[server.server01]

  The server address (IPv4|IPv6, TCP).

  **Property:** address
  **Format:** Network Address.
  **Default**: No Default

  See 'Network Address format' for details.

address = 172.16.0.2:3389

  The server info port (UDP).

  **Property:** info_port
  **Format:** Integer
  **Default**: 4073
  
  The server info port is used only when info  
  protocol is enabled and check_mode is 2 or 3.

;info_port = 4073

  The status and info check mode.

  **Property:** check_mode
  **Format:** Integer, see Values
  **Default**: 2

  **Values:**
  0 - Never check the server status or info.
  1 - Check the server availability only.
  2 - Request info and check resources usage.
  3 - Request info and synchronize all limits.

  For advanced modes (2,3), make sure that the
  NuoRDS info protocol is enabled in proxy config.

;check_mode = 2

  The maximum server weight.

  **Property:** max_weight
  **Format:** Integer,  1...100
  **Default**: 100

  Higher weight means higher probability
  of receiving connections from a balancer. 

;max_weight = 100

  The maximum number of connections. 

  **Property:** max_slots
  **Format:** Integer
  **Default**: 100

  Set "0" to disable this server.

;max_slots = 100

  The redundant server flag. 

  **Property:** redundant
  **Format:** 0/1
  **Default**: 0

  A redundant server will not be involved as long as at
  least one non-redundant server has at least one slot. 

;redundant = 0



[server.server02]

address = 172.16.0.2:3389

;check_mode = 2
;info_port = 4073
;max_weight = 100
;max_slots = 100
;redundant = 0


  A [client.NAME] section describes a client or a group.
  IMPORTANT: To avoid conflicts, do not use NAME "all".

[client.subnets]

  The client connection sources (CIDRs)

  **Property:** sources
  **Format:** Comma|Space-separated list of CIDRs
  **Default**: No default

  Set 0.0.0.0/0 to accept any range of IPv4 addresses. 
  Set ::/0 to accept any range of IPv6 addresses.

sources = 192.168.0.0/16 10.0.0.0/8 172.16.0.0/12 fec0::0/16 fe80::/10

  Maximal number of incoming client connections.

  **Property:** max_slots
  **Format:** Integer
  **Default**: 1000

  Set "0" to disable this client group.

;max_slots = 1000



[client.localhost]

sources = 127.0.0.1/24 ::1/120
max_slots = 1



[client.general]

sources = 0.0.0.0/0 ::/0
max_slots = 0


  A [balancer.NAME] section describes a balancing module, 
  that is distributing client connections among servers.

[balancer.balancer01]

  List of client groups.

  **Property:** clients
  **Format:** Comma|Space-separated list of names
  **Default**: all

  The listed client groups will be handled by this balancer.

clients = subnets localhost general

  List of target servers.

  **Property:** servers
  **Format:** Comma|Space-separated list of names
  **Default**: all

  The listed servers will receive connections form this balancer.

servers = server01 server02

  The balancing method.

  **Property:** method
  **Format:** See values
  **Default**: bybusyness

  **Values:**
  byrequests - based on utilized resources.
  bybusyness - based on available resources.
 
;method = bybusyness

  List of the balancer metrics.

  **Property:** metrics
  **Format:** See values
  **Default**: cap

  **Values:**
  cap - server capacity (maximum slots vs current slots) 
  mem - server memory usage (check_mode 2+ is required) 
  cpu - server CPU usage (check_mode 2+ is required)  

;metrics = cap mem cpu

 

------------------------------
[< NuoRDS Proxy](README.md) 
