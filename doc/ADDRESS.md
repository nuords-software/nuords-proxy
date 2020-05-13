**NUORDS PROXY ADDRESS FORMAT** [(?)](../README.md)

NuoRDS Proxy supports both IPv4 and IPv6 connections.
In order to avoid collisions please use format below.


**A generic network address**

- An internet address consists of two colon-separated 
components - host and port.
- If the port is omitted, a default port will be used.


**An explicit IPv4 address**

- The host component represents an IPv4 literal (rfc791).
- The host component must not be enclosed into brackets.
- The host component '0.0.0.0' describes 'any' address.

  Examples:

  192.168.0.100   
  192.168.0.100:3389


**An explicit IPv6 address**

- The host component represents an IPv6 literal (rfc3513).
- The host component must be enclosed into square brackets.
- The host component '::' describes 'any' IPv6 address.

  Examples:

  [fec0::1]   
  [fec0::1]:3389


**A hostname based address**

- The host component represents a domain name (FQDN).
- If the preferred address family is IPv6, then the
host component must be enclosed into square brackets.

  Examples:

  myhost.com   
  myhost.com:3389   
  [myhost.com]    
  [myhost.com]:3389   

