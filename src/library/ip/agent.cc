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

	IP::Agent::Agent(const pugi::xml_node &node) : Abstract::Agent(node), ICMP::Worker(node) {
		icmp = getAttribute(node,"icmp",icmp);
	}

	void IP::Agent::set(const ICMP::Response response, const IP::Address &from) {

		Logger::String{"Setting ICMP state to '",response,"'"}.trace(name());

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


		if(icmp) {
			ICMP::Worker::start(*this);
		}

		return false;
	}



 }
