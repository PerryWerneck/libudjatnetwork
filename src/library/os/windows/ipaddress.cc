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

/*
 #pragma once
 #include <config.h>
 #include <udjat/network/ipaddress.h>
 #include <exception>

 using namespace std;

 namespace Udjat {

	const string IP::Address::to_string(bool port) {

		string rc;
		unsigned int portnumber = 0;
		const char *result = NULL;
		char ipaddr[INET_ADDRSTRLEN+1];
		ipaddr[0] = 0;

		switch(ss.family) {
		case 0:
			break;

		case AF_INET:	// IPV4
			inet_ntop(
				((sockaddr_in *) &addr)->sin_family,
				(PVOID) &addr,
				ipaddr,
				sizeof(ipaddr)
			);
			portnumber = htons(((sockaddr_in *) &addr)->sin_port);
			break;

		case AF_INET6:	// IPV6
			inet_ntop(
				((sockaddr_in6 *) &addr)->sin6_family,
				(PVOID) this,
				ipaddr,
				sizeof(ipaddr)
			);
			portnumber = htons(((sockaddr_in6 *) &addr)->sin6_port);
			break;

		default:
			throw runtime_error("Invalid network family")
		}

		if(result) {
			if(port && portnumber) {
				result += ":";
				result += to_string(portnumber);
			}
			return result;
		}

		throw std::system_error(errno, std::system_category(), "Cant convert IP to string");

	}

 }
*/
