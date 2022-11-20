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

 #include <config.h>
 #include <udjat/network/agents/nic.h>
 #include <udjat/tools/net/nic.h>
 #include <udjat/tools/logger.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	Network::NICAgent::NICAgent(const pugi::xml_node &node) : Udjat::Agent<unsigned short>(node,NIC_STATE_UNDEFINED) {
	}

	Network::NICAgent::~NICAgent() {
	}

	bool Network::NICAgent::getProperty(const char *key, std::string &value) const noexcept {


		return false;

	}

	bool Network::NICAgent::refresh() {

		size_t active = 0;

		// Load network cards.
		Network::Interface::for_each([this,&active](const Network::Interface &intf){

			if(!intf.loopback()) {

				Interface &state = find_interface(intf.name());

				if(state.up != intf.up()) {
					info() << "Interface " << state.name << " is now " << (intf.up() ? "UP" : "DOWN") << endl;
					state.up = intf.up();
				}

				if(intf.up()) {
					active++;
				}

			}

			return false;
		});

		debug("Detected nics: ",interfaces.size(), " Active nics: ", active);

		switch(active) {
		case 0:
			return set(NIC_STATE_OFFLINE);

		case 1:
			return set(NIC_STATE_SINGLE);

		case 2:
			return set(NIC_STATE_MULTIPLE);

		}

		return set(NIC_STATE_UNDEFINED);
	}

	Network::NICAgent::Interface & Network::NICAgent::find_interface(const char *name) {

		for(Interface &interface : interfaces) {
			if(strcasecmp(interface.name.c_str(),name) == 0) {
				return interface;
			}
		}

		info() << "Detected new interface '" << name << "'" << endl;
		interfaces.emplace_back(name);

		return interfaces.back();

	}

 }
