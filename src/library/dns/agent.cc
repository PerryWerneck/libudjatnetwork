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

#ifndef _WIN32
  #include <netdb.h>
#endif // _WIN32

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
			auto state = DNS::State::Factory(*this,node);
			states.push_back(state);
			return state;
		}

		return IP::Agent::StateFactory(node);

	}

	Udjat::Value & DNS::Agent::getProperties(Value &value) const noexcept {

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

	bool DNS::Agent::set(int code, const char *name) {

		if(state && state->compare(code)) {
			debug("DNS State not changed");
			return false;
		}

		for(auto state : states) {
			if(state->compare(code)) {
				debug("Found predefined state for response ",code);
				this->state = state;
				info() << name << ": " << state->to_string() << endl;
				return true;
			}
		}

		debug("Using standard state for response ",code);
		this->state = DNS::State::Factory(*this,code);
		info() << state->to_string() << endl;

		return true;
	}


	bool DNS::Agent::refresh() {

		bool rc = false;

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
#ifdef _WIN32
					#warning Implement
#else
					set(HOST_NOT_FOUND,server.name);
#endif // _WIN32
				}

				server.ip.set(resolver.begin()->getAddr());

			}

			// Using a custom DNS server?
			if(server.ip) {

				DNS::Resolver resolver{server.ip};
				resolver.query(hostname);
				if(resolver.empty()) {
					IP::Address::clear();
#ifdef _WIN32
					#warning Implement
#else
					set(HOST_NOT_FOUND,server.name);
#endif // _WIN32
				}

				IP::Address::set(resolver.begin()->getAddr());

			} else {

				DNS::Resolver resolver;
				resolver.query(hostname);
				if(resolver.empty()) {
					IP::Address::clear();
#ifdef _WIN32
					#warning Implement
#else
					set(HOST_NOT_FOUND,server.name);
#endif // _WIN32
				}

				IP::Address::set(resolver.begin()->getAddr());
			}

#ifdef _WIN32
			#warning Implement
#else
			set(NETDB_SUCCESS,server.name);
#endif // _WIN32

		} catch(const DNS::Exception &e) {

			server.ip.clear();
			IP::Address::clear();

			Logger::String{"DNS Query has failed: ",e.what()," (",e.code(),")"}.trace(name());
			return set(e.code(),hostname);

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
