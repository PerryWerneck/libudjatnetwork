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
 #include <pugixml.hpp>
 #include <udjat/net/ip/address.h>
 #include <udjat/net/ip/agent.h>
 #include <udjat/net/icmp.h>
 #include <udjat/net/dns.h>
 #include <udjat/net/dns/agent.h>
 #include <udjat/agent/state.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	DNS::Agent::Agent(const char *name) : IP::Agent(name) {
	}

	DNS::Agent::Agent(const pugi::xml_node &node) : IP::Agent{node} {

		server.name = getAttribute(node,"dns","");
		hostname = getAttribute(node,"hostname","");

	}

	std::shared_ptr<Abstract::State> DNS::Agent::computeState() {

		auto state = IP::Agent::computeState();

		if(this->state && *this->state > *state) {
			state = this->state;
		}

		return state;
	}

	std::shared_ptr<Abstract::State> DNS::Agent::StateFactory(const pugi::xml_node &node) {

		if(Object::getAttribute(node,"dns-state")) {
			auto state = DNS::State::Factory(node);
			states.push_back(state);
			return state;
		}

		return IP::Agent::StateFactory(node);

	}

	Udjat::Value & DNS::Agent::getProperties(Value &value) const noexcept {


		return IP::Agent::getProperties(value);
	}

	bool DNS::Agent::set(const DNS::Response response, const char *name) {

		if(state && state->id == response) {
			return false;
		}

		for(auto state : states) {
			if(state->id == response) {
				state = state;
				info() << name << ": " << state->to_string() << endl;
				return true;
			}
		}

		state = DNS::State::Factory(response);
		info() << name << ": " << state->to_string() << endl;

		return true;
	}


	bool DNS::Agent::refresh() {

		debug("----------------------------------------------------------------",name());
		bool rc = false;

		// Check hostname?
		if(!(hostname && *hostname)) {
			return set(DNS::invalid,"");
		}

		// Check for DNS
		try {

			// Do we need the DNS server addr?
			if(server.name && *server.name && !server.ip) {
				//
				// Resolve DNS server address using the system DNS server.
				//
				DNS::Resolver resolver;
				resolver.query(server.name);
				if(resolver.empty()) {
					IP::Address::clear();
					return set(DNS::cant_resolve_server_address,server.name);
				}

				server.ip.set(resolver.begin()->getAddr());

			}

			// Using a custom DNS server?
			if(server.ip) {

				DNS::Resolver resolver{server.ip};
				resolver.query(hostname);
				if(resolver.empty()) {
					IP::Address::clear();
					return set(DNS::cant_resolve_address,hostname);
				}

				IP::Address::set(resolver.begin()->getAddr());

			} else {

				DNS::Resolver resolver;
				resolver.query(hostname);
				if(resolver.empty()) {
					IP::Address::clear();
					return set(DNS::cant_resolve_address,hostname);
				}

				IP::Address::set(resolver.begin()->getAddr());
			}

			if(set(DNS::dns_ok,hostname)) {
				rc = true;
			}

		} catch(...) {
			server.ip.clear();
			IP::Address::clear();
			throw;
		}

		if(IP::Agent::refresh()) {
			rc = true;
		}

		debug("-------------------------------------------------------------");
		debug("Agent '",name(),"' refresh(), ends with rc=",(rc ? "Updated" : "no updated"));
		return rc;
	}

 }
