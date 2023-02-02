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

 #include <private/module.h>
 #include <udjat/tools/net/ip.h>

 namespace Udjat {

	Network::Range::Range(const pugi::xml_node &node) : State(node) {


	}

	Network::Range::~Range() {
	}

	void Network::Range::parse(const char *range, sockaddr_storage &addr, uint16_t &mask) {

		memset(&addr,0,sizeof(addr));
		mask = 0;

		const char *masklen = strchr(range,'/');
		if(!masklen) {
			throw runtime_error("Range should be in the format IP/bits");
		}

		mask = atoi(masklen+1);
		if(!mask) {
			throw runtime_error("Invalid or unexpected mask");
		}

		if(*range == '!') {
			revert = true;
			range++;
			while(isspace(*range) && range < masklen) {
				range++;
			}
		}

		string str = string(range,(masklen-range));

		addr.ss_family = AF_INET;
		if(inet_pton(AF_INET,str.c_str(),&((sockaddr_in *) &addr)->sin_addr) == 0) {
			throw std::system_error(errno, std::system_category(), str);
		}

#ifdef DEBUG
		cout << range << " convert to '" << std::to_string(addr) << "'" << endl;
#endif // DEBUG


	}

	bool Network::Range::inRange(const sockaddr_in &ip, const sockaddr_in &addr, const sockaddr_in &netmask) const {

#ifdef DEBUG
		cout	<< "Checking " << std::to_string(ip)
				<< " in " << std::to_string(addr) << " " << std::to_string(netmask)
				<< endl;
#endif // DEBUG

		in_addr_t net  = (addr.sin_addr.s_addr & netmask.sin_addr.s_addr);
		in_addr_t base = (ip.sin_addr.s_addr & netmask.sin_addr.s_addr);

		bool inRange = (base == net) && ((base|net) == net);

#ifdef DEBUG
		cout << "Address is " << (inRange ? "in range" : "not in range") << endl;
#endif // DEBUG

		return inRange;

	}

	bool Network::Range::inRange(const sockaddr_storage &ip, const sockaddr &addr, const sockaddr &netmask) const {

		if(ip.ss_family != AF_INET) {
			throw runtime_error("Unexpected network family");
		}

		return inRange( *((sockaddr_in *) &ip), *((sockaddr_in *) &addr), *((sockaddr_in *) &netmask));

	}

    const sockaddr_in & Network::Range::getMask(sockaddr_in &netmask, uint16_t length) {

		memset(&netmask,0,sizeof(sockaddr_in));
		netmask.sin_family = AF_INET;

		for(size_t ix = 0; ix < length; ix++) {
			netmask.sin_addr.s_addr >>= 1;
			netmask.sin_addr.s_addr |= 0x80000000;
		}

		netmask.sin_addr.s_addr = htonl(netmask.sin_addr.s_addr);
		return netmask;

	}

 }
