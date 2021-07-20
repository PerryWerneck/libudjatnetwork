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

		public:
			Range(const pugi::xml_node &node) : State(node) {
			}

			virtual ~Range() {
			}

			bool test(const sockaddr_storage &addr) override {

				// TODO: Implement

				return false;
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
