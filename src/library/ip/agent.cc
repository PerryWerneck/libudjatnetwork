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

	bool IP::Agent::set(std::shared_ptr<Abstract::State> state) {

		if(*icmp.state > *state) {
			state = icmp.state;
		}

		return super::set(state);
	}

	void IP::Agent::set(const ICMP::Response response, const IP::Address &from) {

		if(response == icmp.response) {
			return;
		}

		icmp.response = response;

		Logger::String{"Setting ICMP state to '",response,"'"}.trace(name());

		if(!icmp.states.empty()) {

			// Check for xml defined states.
			for(auto state : icmp.states) {
				if(state->id == response) {
					set(icmp.state = state);
					return;
				}
			}
		}

		// Use predefined state.
		set(icmp.state = ICMP::State::Factory(response));

	}

	std::shared_ptr<Abstract::State> IP::Agent::computeState() {
		auto state = super::computeState();

		if(icmp.state && *icmp.state > *state) {
			state = icmp.state;
		}

		return state;
	}

	std::shared_ptr<Abstract::State> IP::Agent::StateFactory(const pugi::xml_node &node) {

		pugi::xml_attribute attr;

		attr = Object::getAttribute(node,"icmp-response");
		if(attr) {
			auto state = ICMP::State::Factory(node);
			icmp.states.push_back(state);
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

		if(icmp.check) {
			ICMP::Worker::start(*this);
		}

		return false;
	}

 }
