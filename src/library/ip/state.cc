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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/object.h>
 #include <pugixml.hpp>
 #include <udjat/net/ip/address.h>
 #include <udjat/net/ip/state.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Abstract::IP::State> IP::State::Factory(const pugi::xml_node &node) {

		const char *subnet = Object::getAttribute(node,"subnet").as_string();

		/*
		if(!strncasecmp(subnet,"local")) {

			// Create local subnet.
			class State : public IP::State {
			};


		}
		*/

		// Create default state.
		return make_shared<IP::State>(node);
	}

	IP::State::State(const char *subnet) : Abstract::IP::State{subnet} {
		set(subnet);
	}

	IP::State::State(const pugi::xml_node &node) : Abstract::IP::State{node} {
		set(Object::getAttribute(node,"subnet").as_string());
	}

	void IP::State::set(const char *subnet) {

		const char *ptr = strchr(subnet,'/');
		if(!ptr) {
			throw runtime_error("Subnet should be in the format addr/mask");
		}

		bits = stoi(ptr+1);
		debug("bits=",bits);
		debug("ip=",string{subnet,(size_t) (ptr-subnet) }.c_str());

		IP::Address::set(string{subnet,(size_t) (ptr-subnet) }.c_str());

		debug("Subnet state '",to_string(),"' built");

	}

	std::string IP::State::to_string() const noexcept {

		if(Object::properties.summary[0]) {
			return Object::properties.summary;
		}

		std::string rc{std::to_string((IP::Address) *this)};
		rc += "/";
		rc += std::to_string(bits);

		return rc;
	}

	/// @brief Test an IPV4 address range.
	bool IP::State::compare(const sockaddr_in &addr) const {

		if(this->ss_family != AF_INET) {
			return false;
		}

		sockaddr_storage sn{*this};
		sockaddr_in subnet = *((const sockaddr_in *) &sn);

		sockaddr_in netmask;
		memset(&netmask,0,sizeof(sockaddr_in));
		netmask.sin_family = AF_INET;

		for(size_t ix = 0; ix < bits; ix++) {
			netmask.sin_addr.s_addr >>= 1;
			netmask.sin_addr.s_addr |= 0x80000000;
		}
		netmask.sin_addr.s_addr = htonl(netmask.sin_addr.s_addr);

		debug("subnet=",std::to_string(subnet));
		debug("netmask=",std::to_string(netmask));
		debug("ip=",std::to_string(addr));

		in_addr_t net  = (addr.sin_addr.s_addr & netmask.sin_addr.s_addr);
		in_addr_t base = (subnet.sin_addr.s_addr & netmask.sin_addr.s_addr);

		debug("base==net: ", (base==net), "  (base|net)==net: ", ((base|net) == net))

		return (base == net) && ((base|net) == net);


	}

	/// @brief Test an IPV6 address range.
	bool IP::State::compare(const sockaddr_in6 &addr) const {
		throw runtime_error("Unsupported network family");
	}

	bool Abstract::IP::State::compare(const Udjat::IP::Address &value) {
		return compare((const sockaddr_storage) value);
	}

	bool Abstract::IP::State::compare(const sockaddr_storage &addr) const {

		switch(addr.ss_family) {
		case AF_INET:
			return compare(*((const sockaddr_in *) &addr));

		case AF_INET6:
			return compare(*((const sockaddr_in6 *) &addr));

		default:
			throw runtime_error("Invalid address family");
		}


	}

	/*
	bool IP::State::compare(const sockaddr_in &subnet, const sockaddr_in &addr) const {

		sockaddr_in netmask;
		memset(&netmask,0,sizeof(sockaddr_in));
		netmask.sin_family = AF_INET;

		for(size_t ix = 0; ix < bits; ix++) {
			netmask.sin_addr.s_addr >>= 1;
			netmask.sin_addr.s_addr |= 0x80000000;
		}
		netmask.sin_addr.s_addr = htonl(netmask.sin_addr.s_addr);

		debug("subnet=",std::to_string(subnet));
		debug("netmask=",std::to_string(netmask));
		debug("ip=",std::to_string(addr));

		in_addr_t net  = (addr.sin_addr.s_addr & netmask.sin_addr.s_addr);
		in_addr_t base = (subnet.sin_addr.s_addr & netmask.sin_addr.s_addr);

		debug("base==net: ", (base==net), "  (base|net)==net: ", ((base|net) == net))

		return (base == net) && ((base|net) == net);

	}
	*/


 }
