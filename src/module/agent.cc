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

 namespace Udjat {

	class Network::Agent::State : public Abstract::State {
	public:
		State(const pugi::xml_node &node) : Abstract::State(node) {
		}

		virtual ~State() {
		}

	};

	Network::Agent::Factory::Factory() : Udjat::Factory("network-host",&moduleinfo) {
	}

	void Network::Agent::Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {
		parent.insert(make_shared<Network::Agent>(node));
	}

	Network::Agent::Agent(const pugi::xml_node &node) {

		check.dns = Udjat::Attribute(node,"dns").as_bool(true);
		check.icmp = Udjat::Attribute(node,"icmp").as_bool(true);
		hostname = Udjat::Attribute(node,"host").c_str();

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
		public:
			Range(const pugi::xml_node &node) : State(node) {
			}

			virtual ~Range() {
			}
		};

		if(node.attribute("range")) {
			states.push_back(make_shared<Range>(node));
			return;
		}


		super::append_state(node);

	}

	void Network::Agent::refresh() {

	}

 }
