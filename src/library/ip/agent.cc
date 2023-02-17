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
 #include <udjat/agent/state.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	IP::Agent::Agent(const char *name) : Abstract::Agent(name) {
	}

	IP::Agent::Agent(const pugi::xml_node &node, const char *addr) : IP::Address{addr}, Abstract::Agent{node}, ICMP::Worker{node} {
		icmp.check = getAttribute(node,"icmp",icmp.check);

		auto attr = node.attribute("ip");
		if(attr) {
			IP::Address::set(attr.as_string());
		}

	}

	void IP::Agent::start() {
	}

	void IP::Agent::set(const ICMP::Response response, const IP::Address &) {

		if(response == icmp.response) {
			return;
		}

		// Check for xml defined states.
		for(auto state : icmp.states) {
			if(state->id == response) {
				icmp.state = state;
				Logger::String{"Setting ICMP state to '",icmp.state->to_string(),"' (",response,")"}.trace(name());
				updated(true);
				return;
			}
		}

		// Use predefined state.
		icmp.state = ICMP::State::Factory(*this,response);
		Logger::String{"Setting ICMP state to '",icmp.state->to_string(),"' (",response,")"}.trace(name());
		updated(true);

	}

	std::shared_ptr<Abstract::State> IP::Agent::StateFactory(const pugi::xml_node &node) {

		if(Object::getAttribute(node,"icmp-response")) {
			auto state = ICMP::State::Factory(node);
			icmp.states.push_back(state);
			return state;
		}

		if(Object::getAttribute(node,"subnet")) {
			auto state = IP::State::Factory(node);
			ip.states.push_back(state);
			return state;
		}

		return super::StateFactory(node);

	}

	std::string IP::Agent::to_string() const noexcept {
		return std::to_string((IP::Address) *this);
	}

	Udjat::Value & IP::Agent::get(Udjat::Value &value) const {
		value.set(std::to_string((IP::Address) *this));
		return value;
	}

	Udjat::Value & IP::Agent::getProperties(Value &value) const noexcept {


		return super::getProperties(value);
	}

	bool IP::Agent::getProperty(const char *key, std::string &value) const noexcept {

		if(!strcasecmp(key,"ip")) {
			value = std::to_string((IP::Address) *this);
		}

		return super::getProperty(key,value);
	}

	std::shared_ptr<Abstract::State> IP::Agent::computeState() {

		auto computed_state = super::computeState();

		// Compute state from subnet.
		ip.state.reset();
		for(auto subnet : ip.states) {
			if(subnet->compare((const IP::Address) *this)) {
				ip.state = subnet;
				break;
			}
		}

		if(ip.state && *ip.state > *computed_state) {
			computed_state = ip.state;
		}

		if(icmp.state && *icmp.state > *computed_state) {
			computed_state = icmp.state;
		}

		return computed_state;
	}


	bool IP::Agent::refresh() {

		// Check IP state
		if(IP::Address::empty()) {

			if(ip.state) {
				info() << "No IP address, resetting state" << endl;
				ip.state.reset();
				set(ICMP::invalid,(IP::Address) *this);
			}

			return false;

		}

		if(icmp.check && !ICMP::Worker::running()) {
			ICMP::Worker::start(*this);
		}

		return false;
	}

 }
