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

 #include "private.h"
 #include <udjat/network/agent.h>
 #include <udjat/tools/xml.h>
 #include <udjat/network/resolver.h>
 #include <cstring>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <ifaddrs.h>

 namespace Udjat {

	Network::Agent::Factory::Factory() : Udjat::Factory("network-host",&moduleinfo) {
	}

	void Network::Agent::Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {
		parent.insert(make_shared<Network::Agent>(node));
	}

	Network::Agent::Agent(const pugi::xml_node &node) : Network::HostCheck(node) {

		memset(&addr,0,sizeof(addr));

		// Do an ICMP check?
		icmp.check = Udjat::Attribute(node,"icmp").as_bool(true);
		icmp.timeout = Udjat::Attribute(node,"icmp-timeout").as_uint(icmp.timeout);

		// Get dns-server.
		const char *dnssrv = Udjat::Attribute(node, "dns-server").as_string();
		dns.check = Udjat::Attribute(node,"dns").as_bool(dnssrv[0] != 0);

		// Host name to check.
		hostname = Udjat::Attribute(node,"host").c_str();

		if(dns.check) {

			// Will check DNS resolution, get the DNS server addr.

			if(dnssrv[0]) {

				// Resolve DNS server.
				DNSResolver resolver;
				resolver.query(dnssrv);

				if(resolver.size()) {
					this->addr = resolver.begin()->getAddr();
				} else {
					throw runtime_error(string{"Can't resolve '"} + dnssrv + "'");
				}

			}

		} else {

			// Will not check DNS resolution, get host address.

			DNSResolver resolver;
			resolver.query(hostname);

			if(resolver.size()) {
				this->addr = resolver.begin()->getAddr();
			} else {
				throw runtime_error(string{"Can't resolve '"} + hostname + "'");
			}

		}

		load(node);

	}

	Network::Agent::~Agent() {
	}

	bool Network::Agent::hasStates() const noexcept {
		return !this->states.empty();
	}

	void Network::Agent::append_state(const pugi::xml_node &node) {

		/// @brief IP Address range state.
		class Range : public Network::Range {
		private:
			sockaddr_storage addr;
			uint16_t mask;

		public:
			Range(const pugi::xml_node &node) : Network::Range(node) {
				parse(Attribute(node,"range").as_string(),addr,mask);
			}

		protected:
			bool test(const sockaddr_storage &ip) const override {

				// TODO: Implement IPV6 methods.

				if(addr.ss_family == AF_INET) {
					sockaddr_in netmask;
					return inRange(*((const sockaddr_in *) &ip), *((const sockaddr_in *) &this->addr), getMask(netmask,this->mask));
				} else {
					throw runtime_error("Unsupported address family");
				}

				return false;
			}

		};

		/// @brief Local network state
		class SameNetwork : public Network::Range {
		public:
			SameNetwork(const pugi::xml_node &node) : Network::Range(node) {
				revert = !Attribute(node,"same-network").as_bool(true);
			}

		protected:
			bool test(const sockaddr_storage &ip) const override {

				bool rc = false;

				struct ifaddrs *ifaces;

				if(getifaddrs(&ifaces)) {
					throw std::system_error(errno, std::system_category(), "Cant get network interface list");
				}

				try {

					for(ifaddrs *iface = ifaces; iface; iface = iface->ifa_next) {

						if(!iface->ifa_addr || iface->ifa_addr->sa_family != AF_INET) {
							continue;
						}

						if(inRange(ip,*iface->ifa_addr,*iface->ifa_netmask)) {
#ifdef DEBUG
							cout << "Found " << std::to_string(ip) << " in " << iface->ifa_name << endl;
#endif // DEBUG
							rc = true;
							break;

						}
					}

				} catch(...) {

					freeifaddrs(ifaces);
					throw;
				}

				freeifaddrs(ifaces);

				return rc;
			}
		};

		if(node.attribute("range")) {
			if(!dns.check) {
				throw runtime_error("Can't use 'range' states without dns='true' attribute on the agent");
			}
			states.push_back(make_shared<Range>(node));
			return;

		} else if(node.attribute("same-network")) {
			if(!dns.check) {
				throw runtime_error("Can't use 'range' states without dns='true' attribute on the agent");
			}
			states.push_back(make_shared<SameNetwork>(node));
			return;

		}


		super::append_state(node);

	}

	void Network::Agent::refresh() {

		sockaddr_storage addr;

		std::shared_ptr<Abstract::State> selected;

#ifdef DEBUG
		info("Checking '{}'",hostname);
#endif // DEBUG

		if(dns.check) {

			// Check DNS resolution.
			DNSResolver resolver{this->addr};

			resolver.query(hostname);

			if(!resolver.size()) {
				throw runtime_error(string{"Can't resolve '"} + hostname + "'");
			}

			addr = resolver.begin()->getAddr();

#ifdef DEBUG
			info("{}: '{}' = '{}'",__FUNCTION__,hostname, std::to_string(addr));
#endif // DEBUG

			// Check states.
			for(auto state : states) {

				State * st = dynamic_cast<State *>(state.get());

				if(st && (!selected || st->getLevel() > selected->getLevel())) {
					if(st->isValid(addr)) {
						selected = state;
					}
#ifdef DEBUG
					else {
						cout << "State " << st->getName() << " was rejected" << endl;
					}
#endif // DEBUG
				}

			}

		} else {

			addr = this->addr;

		}

		if(icmp.check) {
			check(addr);
		}

		//
		// Set current state.
		//
		if(!selected) {
			selected = super::stateFromValue();
		}

		activate(selected);

	}

 }
