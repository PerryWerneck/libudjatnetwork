/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/net/ip/subnet.h>
 #include <udjat/tools/logger.h>
 #include <udjat/win32/ip.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	bool IP::SubNet::for_each(const std::function<bool(const SubNet &subnet)> &method) {

		// https://learn.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_addresses_lh
		return Win32::for_each([&method](const IP_ADAPTER_ADDRESSES &address){

#ifndef DEBUG
			#error Pending implementation
#endif // DEBUG

			return false;
		});

		/*
		struct ifaddrs *ifaces;

		if(getifaddrs(&ifaces)) {
			throw std::system_error(errno, std::system_category(), "Cant get network interface list");
		}

		debug("Got interfaces");

		bool rc = false;
		try {

			for(ifaddrs *iface = ifaces; iface; iface = iface->ifa_next) {

				if(!(iface->ifa_addr && iface->ifa_netmask)) {
					debug("Ignoring interface '",iface->ifa_name,"'");
					continue;
				}

				// Only AF_INET for now.
				if(!iface->ifa_addr || iface->ifa_addr->sa_family != AF_INET) {
					debug("Interface '",iface->ifa_name,"' is not AF_INET");
					continue;
				}

				SubNet subnet;
				subnet.IP::Address::set((sockaddr_in *) iface->ifa_addr);

				unsigned long s_addr = htonl(((sockaddr_in *) iface->ifa_netmask)->sin_addr.s_addr);

				subnet.bits = 0;
				while(s_addr) {
					if(s_addr & 0x80000000) {
						subnet.bits++;
					}
					s_addr <<= 1;
				}

				debug(iface->ifa_name,": ",subnet.to_string());
				if(method(subnet)) {
					rc = true;
					break;
				}

			}

		} catch(...) {

			freeifaddrs(ifaces);
			throw;

		}

		freeifaddrs(ifaces);
		*/

	}

 }
