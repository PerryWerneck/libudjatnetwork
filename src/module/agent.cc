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
 #include <udjat/tools/inet.h>
 #include <arpa/inet.h>
 #include <cstring>

 namespace Udjat {

	class Network::Agent::State : public Abstract::State {
	public:
		State(const pugi::xml_node &node) : Abstract::State(node) {
		}

		virtual ~State() {
		}

		virtual bool test(const sockaddr_storage &addr) {
			return false;
		}

	};

	Network::Agent::Factory::Factory() : Udjat::Factory("network-host",&moduleinfo) {
	}

	void Network::Agent::Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {
		parent.insert(make_shared<Network::Agent>(node));
	}

	Network::Agent::Agent(const pugi::xml_node &node) {

		memset(&addr,0,sizeof(addr));

		// Do an ICMP check?
		check.icmp = Udjat::Attribute(node,"icmp").as_bool(true);

		// Get dns-server.
		const char *dnssrv = Udjat::Attribute(node, "dns-server").as_string();
		check.dns = Udjat::Attribute(node,"dns").as_bool(dnssrv[0] != 0);

		// Host name to check.
		hostname = Udjat::Attribute(node,"host").c_str();

		if(check.dns) {

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

		/// @brief Range check.
		class Range : public Network::Agent::State {
		private:
			in_addr addr;
			uint16_t mask;
			bool revert = false;

		public:
			Range(const pugi::xml_node &node) : State(node) {

				const char *range = Attribute(node,"range").as_string();
				const char *mask = strchr(range,'/');
				if(!mask) {
					throw runtime_error("Range should be in the format IP/bits");
				}

				this->mask = atoi(mask+1);
				if(!this->mask) {
					throw runtime_error("Invalid or unexpected mask");
				}

				if(*range == '!') {
					revert = true;
					range++;
					while(isspace(*range) && range < mask) {
						range++;
					}
				}

				string addr = string(range,(mask-range));

				if(inet_pton(AF_INET,addr.c_str(),&this->addr) == 0) {
					throw std::system_error(errno, std::system_category(), addr);
				}

//#ifdef DEBUG
//				cout << "Range = '" << std::to_string(this->addr) << "'" << endl;
//#endif // DEBUG

			}

			virtual ~Range() {
			}

			bool test(const sockaddr_storage &addr) override {

				if(addr.ss_family != AF_INET)
					return false;

				uint32_t netmask = 0;
				for(size_t ix = 0; ix < this->mask; ix++) {
					netmask >>= 1;
					netmask |= 0x80000000;
				}

				netmask = htonl(netmask);

				uint32_t ip = htonl(((sockaddr_in *) &ip)->sin_addr.s_addr);
				uint32_t netstart = (this->addr.s_addr & netmask); 	// first ip in subnet
				uint32_t netend = (netstart | ~netmask); 			// last ip in subnet

				bool rc = ( (ip >= htonl(netstart)) && (ip <= htonl(netend)) );

#ifdef DEBUG
				{
					sockaddr_in i;
					i.sin_family = AF_INET;
					i.sin_addr.s_addr = netstart;

					cout << "network starts on " << std::to_string(i);

					i.sin_addr.s_addr = netend;
					cout << " and ends on " << std::to_string(i) << endl;

				}

				cout	<< std::to_string(addr)
						<< " is " << (rc ? "in range" : "not in range")
						<< " of " << std::to_string(this->addr)
						<< endl;
#endif // DEBUG

				if(revert)
					return !rc;

				return ip;
			}

		};

		if(node.attribute("range")) {
			states.push_back(make_shared<Range>(node));
			return;
		}


		super::append_state(node);

	}

	void Network::Agent::refresh() {

		std::shared_ptr<Abstract::State> selected;

#ifdef DEBUG
		info("Checking '{}'",hostname);
#endif // DEBUG

		if(check.dns) {

			// Check DNS resolution.
			DNSResolver resolver{this->addr};

			resolver.query(hostname);

			if(!resolver.size()) {
				throw runtime_error(string{"Can't resolve '"} + hostname + "'");
			}

			sockaddr_storage addr = resolver.begin()->getAddr();

#ifdef DEBUG
			info("'{}' = '{}'",hostname, std::to_string(addr));
#endif // DEBUG

			// Check states.
			for(auto state : states) {

				State * st = dynamic_cast<State *>(state.get());
				if(st && st->test(addr)) {

					// IP test is valid, check it.
					if(!selected || st->getLevel() > selected->getLevel()) {
						selected = state;
					}

				}

			}

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
