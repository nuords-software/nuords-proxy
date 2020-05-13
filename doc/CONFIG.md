**NUORDS PROXY CONFIGURATION**  [(?)](README.md)  
  
This document describes advanced properties that you can use to tune the proxy server. Before proceeding with advanced tuning, we strongly recommedn that you test one of [simple configuration examples](CONFIG.md)  
  
**General properties**  
  
General properties should be located in the **\[proxy\]** section. This section is singular, you should not add any extensions.  
  
- The proxy listeners (IPv4|IPv6, TCP).  
  
  **Property:** listeners  
  **Format:** Comma|Space-separated list of addresses  
  **Default**: 0.0.0.0:3389 [::]:3389  
  
  See ['NuoRDS Proxy address format'](ADDRESS.md) for details. If you are running any target server on the same host, you shoud change the target server port to avoid conflicts.  
  
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
  
  The connection timeout is a time within which the connection to the server must be established.  
  
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
  
**A target server**  
  
A target server properties should be located in a **\[server.NAME\]** section. Add a new section with a unique NAME extension for each target server.  
  
  **Example:** \[server.server01\]  
  
- The server address (IPv4|IPv6, TCP).  
  
  **Property:** address  
  **Format:** Network Address.  
  **Default**: No Default  
  
  See ['NuoRDS Proxy address format'](ADDRESS.md) for details.  
  
- The server info port (UDP).  
  
  **Property:** info_port  
  **Format:** Integer  
  **Default**: 4073  
  
  The server info port is used only when 'info_enabled' is not zero and 'check_mode' is 2 or 3.  
  
- The status and info check mode.  
  
  **Property:** check_mode  
  **Format:** Integer, see Values  
  **Default**: 2  
  **Values:**  
  0 - Never check the server status or info.  
  1 - Check the server availability only.  
  2 - Request info and check resources usage.  
  3 - Request info and synchronize all limits.  
  
  For advanced modes (2,3), make sure that the 'info_enabled' is not zero.  
  
- The maximum server weight.  
  
  **Property:** max_weight  
  **Format:** Integer,  1...100  
  **Default**: 100  
  
  Higher weight means higher probability of receiving a connection.  
  
- The maximum number of connections.  
  
  **Property:** max_slots  
  **Format:** Integer  
  **Default**: 100  
  
  Set '0' to disable this server.  
  
- The redundant server flag.  
  
  **Property:** redundant  
  **Format:** 0/1  
  **Default**: 0  
  
  A redundant server will not be involved as long as at least one non-redundant server has at least one slot available.  
  
**A client group**  
  
A client group properties should be placed in a **\[client.NAME\]** section. Add a new section with a unique NAME extension for each client group.  
  
- The client connection sources (CIDRs)  
  
  **Property:** sources  
  **Format:** Comma|Space-separated list of CIDRs  
  **Default**: No default  
  
  Add '0.0.0.0/0' to accept any range of IPv4 addresses. Add '::/0' to accept any range of IPv6 addresses.  
  
- Maximal number of incoming client connections.  
  
  **Property:** max_slots  
  **Format:** Integer  
  **Default**: 1000  
  
  Set '0' to prevent connections from this group.  
  
  A [balancer.NAME] section describes a balancing module,  
  that is distributing client connections among servers.  
  
**A balancing unit**  
  
A balancing unit properties should be placed in a **\[balancer.NAME\]** section. Add a new section with a unique NAME extension for each balancing unit.  
  
- List of client groups.  
  
  **Property:** clients  
  **Format:** Comma|Space-separated list of client group names (extensions only)  
  **Default**: all  
  
  The listed client groups will be handled by this balancing unit.  
  
- List of target servers.  
  
  **Property:** servers  
  **Format:** Comma|Space-separated list of server names (extensions only)  
  **Default**: all  
  
  The listed servers will receive connections from the listed client groups  
  
- The balancing method.  
  
  **Property:** method  
  **Format:** See values  
  **Default**: bybusyness  
  **Values:**  
  byrequests - based on resources utilization.  
  bybusyness - based on resources availability.  
  
- List of the balancing metrics.  
  
  **Property:** metrics  
  **Format:** See values  
  **Default**: cap  
  **Values:**  
  cap - server capacity  
  mem - server memory usage  
  cpu - server CPU usage  
  
  The memory and CPU usage metrics reuire 'check_mode' 2 or 3.  
  
------------------------------
[< NuoRDS Proxy](README.md) 
  