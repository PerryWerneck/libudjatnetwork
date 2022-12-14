/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/network/ipaddress.h>
 #include <exception>
 #include <arpa/inet.h>
 #include <stdexcept>
 #include <system_error>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	const string IP::Address::to_string(bool port) const {

		unsigned int portnumber = 0;
		char ipaddr[INET6_ADDRSTRLEN+1];
		ipaddr[0] = 0;

		switch(addr.ss_family) {
		case 0:
			break;

		case AF_INET:	// IPV4
			{
				sockaddr_in * in = ((sockaddr_in *) & addr);

				if(!inet_ntop(AF_INET,&in->sin_addr,ipaddr,sizeof(ipaddr))) {
					throw std::system_error(errno, std::system_category(), "Cant convert IP to string");
				}

				portnumber = htons(in->sin_port);

			}
			break;

		case AF_INET6:	// IPV6
			{
				sockaddr_in6 * in = ((sockaddr_in6 *) & addr);

				if(!inet_ntop(AF_INET6,&in->sin6_addr,ipaddr,sizeof(ipaddr))) {
					throw std::system_error(errno, std::system_category(), "Cant convert IP to string");
				}

				portnumber = htons(in->sin6_port);

			}
			break;

		default:
			throw runtime_error("Invalid network family");
		}

		if(ipaddr[0]) {
			string rc = ipaddr;
			if(port && portnumber) {
				rc += ":";
				rc += to_string(portnumber);
			}
			return rc;
		}

		return "";

	}

 }
