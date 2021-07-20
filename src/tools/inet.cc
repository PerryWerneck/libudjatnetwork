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

 #include <udjat/tools/inet.h>
 #include <arpa/inet.h>

 using namespace std;

 namespace std {

	string to_string(const struct sockaddr_in &addr, bool port) {

		string rc;

#ifdef _WIN32
		char ipaddr[INET_ADDRSTRLEN+1];
		sockaddr_in in;

		ipaddr[0] = 0;
		memcpy(&in,&addr,sizeof(in));

		getnameinfo((struct sockaddr *) &in,sizeof(in),ipaddr,INET_ADDRSTRLEN,NULL,0,NI_NUMERICHOST);

#else
		char ipaddr[100];
		ipaddr[0] = 0;
		inet_ntop(addr.sin_family,&addr.sin_addr,ipaddr,sizeof(ipaddr));
#endif // _WIN32

		rc.assign(ipaddr);

		if(addr.sin_port && port) {
			rc += ':';
			rc += to_string((int) htons(addr.sin_port));
		}

		return rc;

	}

	string to_string(const struct sockaddr_in6 &addr, bool port) {

		string rc;

		char ipaddr[256];
		ipaddr[0] = 0;

#ifdef _WIN32
		inet_ntop(addr.sin6_family,(PVOID) &addr.sin6_addr,ipaddr,sizeof(ipaddr));
#else
		inet_ntop(addr.sin6_family,&addr.sin6_addr,ipaddr,sizeof(ipaddr));
#endif // _WIN32

		rc.assign(ipaddr);

		if(addr.sin6_port && port) {
			rc += ':';
			rc += to_string(htons(addr.sin6_port));
		}

		return rc;

	}

	string to_string(const sockaddr_storage &addr, bool port) {

		if(addr.ss_family == AF_INET) {
			return to_string( (const struct sockaddr_in &) addr, port);
		}

		return to_string( (const struct sockaddr_in6 &) addr, port);

	}

 }

