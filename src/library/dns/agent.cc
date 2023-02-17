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
 #include <netdb.h>

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

		debug("---------------------------------------------------- ", name(),"::",__FUNCTION__);
		debug("IP STATE=",state->to_string());

		if(!this->state) {
			debug("Cant have an state, using the IP");
			return state;
		}

		if(*this->state > *state) {
			debug("Using my state: ",this->state->c_str());
			state = this->state;
		}
#ifdef DEBUG
		else {
			debug("Using IP Agent state: ",this->state->c_str());
		}
#endif // DEBUG

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

		debug("----------------------------------------------- ",name(),"::",__FUNCTION__);

		if(state) {
			value["dns"] = state->to_string();
		} else {
			value["dns"] = "";
		}

		return IP::Agent::getProperties(value);
	}

	bool DNS::Agent::getProperty(const char *key, std::string &value) const noexcept {

		if(!strcasecmp(key,"hostname")) {
			value = hostname;
			return true;
		}

		return IP::Agent::getProperty(key,value);
	}

	bool DNS::Agent::set(const DNS::Response response, const char *name) {

		debug("---------------------------------------------");

		if(state && state->id == response) {
			debug("DNS State not changed");
			return false;
		}

		for(auto state : states) {
			if(state->id == response) {
				debug("Found predefined state for response ",(int) response);
				this->state = state;
				info() << name << ": " << state->to_string() << endl;
				return true;
			}
		}

		debug("Using standard state for response ",(int) response);
		this->state = DNS::State::Factory(*this,response);
		info() << name << ": " << state->to_string() << endl;

		return true;
	}


	bool DNS::Agent::refresh() {

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

		} catch(const DNS::Exception &e) {

			server.ip.clear();
			IP::Address::clear();

			Logger::String{"DNS Query has failed: ",e.what()," (",e.code(),")"}.trace(name());

#ifdef _WIN32

			throw;

#else

			switch(e.code()) {
			case HOST_NOT_FOUND:
				if(set(DNS::host_not_found,hostname)) {
					debug("DNS State has changed");
					return true;
				}
				return false;

			case NO_DATA:
				if(set(DNS::host_not_found,hostname)) {
					debug("DNS State has changed");
					return true;
				}
				return false;

			default:
				throw;
			}

#endif // _WIN32


		} catch(...) {

			server.ip.clear();
			IP::Address::clear();
			throw;

		}

		if(IP::Agent::refresh()) {
			rc = true;
		}

		return rc;
	}

 }
