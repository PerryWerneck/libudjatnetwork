# Network agent module for udjat

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
![CodeQL](https://github.com/PerryWerneck/udjat-module-network/workflows/CodeQL/badge.svg?branch=master)
[![build result](https://build.opensuse.org/projects/home:PerryWerneck:udjat/packages/udjat-module-network/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:PerryWerneck:udjat/udjat-module-network)

### Examples

Example of configuration files for [Udjat](../../../udjat)

1. Warning if hostname is not in the same network.

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<config log-debug='yes' log-trace='yes'>

	<!-- Load network agents -->
	<module name='users' required='yes' />

	<network-host name='cdn' hostname='cdn.werneck.eti.br' icmp='true' icmp-timeout='60' update-timer='60'>
	
		<!-- subnet states -->
		<state name='local' subnet='local' level='ready' summary='CDN Server is local' />
		<state name='remote' subnet='remote' level='warning' summary='CDN Server is NOT local' />
		
		<!-- DNS states -->
		<state name='not-found' dns-state='cant-resolve-host' level='error' summary='Cant resolve CDN hostname' />
			
		<!-- ICMP states -->
		<state name='active' icmp-response='echo-reply' level='ready' summary='CDN is active' />
		<state name='unreachable' icmp-response='destination-unreachable' level='error' summary='CDN is not reachable' />
		<state name='time-exceeded' icmp-response='time-exceeded' level='error' summary='CDN is not available' />
		<state name='timeout' icmp-response='timeout' level='error' summary='No ICMP response from CDN' />
			
	</network-host>


</config>
```

