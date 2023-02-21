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
 #include <udjat/tools/object.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	IP::SubNet::SubNet(const char *subnet) {
		set(subnet);
	}

	IP::SubNet::SubNet(const pugi::xml_node &node, const char *attrname) {
		set(Object::getAttribute(node,attrname).as_string());
	}

	void IP::SubNet::set(const char *subnet) {

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

	/// @brief Test an IPV4 address range.
	bool IP::SubNet::contains(const sockaddr_in &addr) const {

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

#ifdef _WIN32
		unsigned long net  = (addr.sin_addr.s_addr & netmask.sin_addr.s_addr);
		unsigned long base = (subnet.sin_addr.s_addr & netmask.sin_addr.s_addr);
#else
		in_addr_t net  = (addr.sin_addr.s_addr & netmask.sin_addr.s_addr);
		in_addr_t base = (subnet.sin_addr.s_addr & netmask.sin_addr.s_addr);
#endif // _WIN32

		debug("base==net: ", (base==net), "  (base|net)==net: ", ((base|net) == net))

		return (base == net) && ((base|net) == net);

	}

	/// @brief Test an IPV6 address range.
	bool IP::SubNet::contains(const sockaddr_in6 &) const {
		throw std::system_error(ENOTSUP, std::system_category(),"No support for IPV6");
	}

	bool IP::SubNet::contains(const sockaddr_storage &addr) const {

		switch(addr.ss_family) {
		case 0:
			return empty();

		case AF_INET:
			return contains(*((const sockaddr_in *) &addr));

		case AF_INET6:
			return contains(*((const sockaddr_in6 *) &addr));

		default:
			throw std::system_error(ENOTSUP, std::system_category(),"Invalid address family");
		}

	}

	bool IP::SubNet::contains(const IP::Address &value) const {
		return contains((const sockaddr_storage) value);
	}

	std::string IP::SubNet::to_string() const noexcept {

		if(!ss_family) {
			return "None";
		}

		std::string rc{std::to_string((IP::Address) *this)};
		rc += "/";
		rc += std::to_string(bits);

		return rc;
	}

 }

