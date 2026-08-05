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
 #include <udjat/net/ip/address.h>
 #include <udjat/net/ip/agent.h>
 #include <udjat/net/icmp.h>
 #include <udjat/net/dns.h>
 #include <udjat/agent/state.h>
 #include <udjat/tools/properties.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	IP::Agent::Agent(const char *name) : Abstract::Agent(name) {
	}

	IP::Agent::Agent(const Properties &props, const char *addr) : Abstract::Agent{props}, ICMP::Worker{props,addr} {
		icmp.check = props.get("icmp",icmp.check);
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

	std::shared_ptr<Abstract::State> IP::Agent::StateFactory(const Properties &props) {

		if(props.contains("icmp-response")) {
			auto state = ICMP::State::Factory(props);
			icmp.states.push_back(state);
			return state;
		}

		if(props.contains("subnet")) {
			auto state = IP::State::Factory(props);
			ip.states.push_back(state);
			return state;
		}

		return super::StateFactory(props);

	}

	std::string IP::Agent::to_string() const noexcept {
		return std::to_string((IP::Address) *this);
	}

	Udjat::Value & IP::Agent::get(Udjat::Value &value) const {
		value.set(std::to_string((IP::Address) *this));
		return value;
	}

	Udjat::Variant & IP::Agent::get_properties(Variant &value) const {

		if(icmp.check) {
			ICMP::Worker::get_properties(value);
		}

		return super::get_properties(value);
	}

	bool IP::Agent::get_property(const char *key, Variant &value) const {

		if(ICMP::Worker::get_property(key, value)) {
			return true;
		}

		return super::get_property(key,value);
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


	bool IP::Agent::refresh(bool) {

		// Check IP state
		if(IP::Address::empty()) {

			if(ip.state) {
				Logger::String {"No IP address, resetting state"}.info(name());
				ip.state.reset();
				set(ICMP::invalid,(IP::Address) *this);
			}

			return false;

		}

		if(icmp.check && !ICMP::Worker::running()) {
			ICMP::Worker::start();
		}

		return false;
	}

 }
