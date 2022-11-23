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
 #include <cstdio>
 #include <unistd.h>
 #include <fcntl.h>
 #include <unordered_set>

 using namespace std;

 namespace Udjat {

	Network::Agent::Interfaces::Interfaces(const pugi::xml_node &node) : Udjat::Agent<unsigned short>(node,NIC_STATE_UNDEFINED) {
	}

	Network::Agent::Interfaces::~Interfaces() {
	}

	bool Network::Agent::Interfaces::getProperty(const char *key, std::string &value) const noexcept {

		if(!strcasecmp(key,"active-nic")) {
			// Find first active nic.
			for(const Interface &interface : interfaces) {
				if(interface.active) {
					value = interface.name;
					return true;
				}
			}
		}

		if(!strcasecmp(key,"active-nics")) {
			// Get all interfaces.
			for(const Interface &interface : interfaces) {
				if(interface.active) {
					if(!value.empty()) {
						value += ",";
					}
					value += interface.name;
				}
			}
			return true;
		}

		return false;

	}

	bool Network::Agent::Interfaces::refresh() {

		unordered_set<string> nics;

		// Load network cards.
		Network::Interface::for_each([this,&nics](const Network::Interface &intf){

			if(!intf.loopback()) {

				Interface &state = find_interface(intf.name());

				bool active = intf.up();
				if(active) {

					// Check link
					string path{"/sys/class/net/"};
					path += intf.name();
					path += "/carrier";

					if(access(path.c_str(),R_OK)) {
						error() << "Can't acess " << path << endl;
					} else {
						FILE *in = fopen(path.c_str(),"r");
						if(in) {
							if(fgetc(in) == '0') {
								if(state.active) {
									warning() << "No carrier on interface " << intf.name() << endl;
								}
								active = false;
							}
							fclose(in);
						}
					}

				}

				if(state.active != active) {
					info() << "Interface " << state.name << " is now " << (intf.up() ? "ACTIVE" : "INACTIVE") << endl;
					state.active = active;
				}

				nics.emplace(intf.name());

			}

			return false;
		});

		// Detect removed interfaces.
		interfaces.remove_if([this,nics](Interface &intf){
			if(nics.find(intf.name) == nics.end()) {
				info() << "Interface " << intf.name << " was removed" << endl;
				return true;
			}
			return false;
		});

		size_t active_nics = 0;
		for(Interface & interface : interfaces) {
			if(interface.active) {
				active_nics++;
			}
		}

		debug("Detected nics: ",interfaces.size(), " Active nics: ", active_nics);

		switch(active_nics) {
		case 0:
			return set(NIC_STATE_OFFLINE);

		case 1:
			return set(NIC_STATE_ONLINE);

		default:
			return set(NIC_STATE_MULTIPLE);

		}

		return set(NIC_STATE_UNDEFINED);
	}

	Network::Agent::Interfaces::Interface & Network::Agent::Interfaces::find_interface(const char *name) {

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
