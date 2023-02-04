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
 #include <udjat/net/ip/agent.h>
 #include <udjat/net/icmp.h>

 namespace Udjat {

	IP::Agent::Agent(const char *name) : Abstract::Agent(name) {
	}

	IP::Agent::Agent(const pugi::xml_node &node, const char *addr) : IP::Address{addr}, Abstract::Agent{node}, ICMP::Worker{node} {
		icmp.check = getAttribute(node,"icmp",icmp.check);

#ifdef DEBUG
		if(!IP::Address::empty()) {
			debug("ICMP Check is ",(icmp.check ? "active" : "inactive")," on ",to_string());
		}

#endif // DEBUG
	}

	void IP::Agent::start() {
	}

	void IP::Agent::set(const ICMP::Response response, const IP::Address &from) {

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
		icmp.state = ICMP::State::Factory(response);
		Logger::String{"Setting ICMP state to '",icmp.state->to_string(),"' (",response,")"}.trace(name());
		updated(true);

	}

	std::shared_ptr<Abstract::State> IP::Agent::computeState() {

		auto state = super::computeState();

		if(ip.state && *ip.state > *state) {
			state = ip.state;
		}

		if(icmp.state && *icmp.state > *state) {
			state = icmp.state;
		}

		return state;
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

	bool IP::Agent::refresh() {

		debug("----------------------------------------------------------------",name());
		bool rc = false;

		// Check IP state
		{
			std::shared_ptr<IP::State> state;
			for(auto subnet : ip.states) {
				if(subnet->compare((const IP::Address) *this)) {
					state = subnet;
					break;
				}
			}

			if(state.get() != ip.state.get()) {

				// IP state has changed.

				ip.state = state;
				if(ip.state) {
					Logger::String{"Setting IP state to '",ip.state->to_string(),"'"}.trace(name());
				} else {
					Logger::String{"Cleaning IP state"}.trace(name());
				}
				rc = true;

			}

		}

		if(icmp.check && !ICMP::Worker::running()) {
			ICMP::Worker::start(*this);
		}

		debug("-------------------------------------------------------------");
		debug("Agent '",name(),"' refresh(), ends with rc=",(rc ? "Updated" : "no updated"));
		return rc;
	}

 }
