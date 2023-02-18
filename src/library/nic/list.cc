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
 #include <udjat/net/nic/agent.h>
 #include <udjat/tools/logger.h>
 #include <udjat/net/interface.h>

 using namespace std;

 namespace Udjat {

	Nic::List::List(bool a) : auto_detect{a} {
		init();
	}

	Nic::List::List(const pugi::xml_node &node) : Udjat::Agent<unsigned int>{node}, auto_detect{node.attribute("auto-detect").bool(false)} {
		init();
	}

	bool Nic::List::refresh() {

		// Count interfaces.
		unsigned int interfaces = 0;

		Network::Interface::for_each([&interfaces](const Interface &) {
			interfaces++;
		});

		return set(interfaces);
	}

	bool Nic::List::getProperty(const char *key, std::string &value) const noexcept {

		if(!strcasecmp(key,"active-nic")) {
			// Find first active nic.
			Network::Interface::for_each([&value](const Interface &interface) {
				if(interface.active()) {
					value = interface.name();
					return true;
				}
			});

			return true;

		}

		if(!strcasecmp(key,"active-nics")) {
			// Get all interfaces.
			Network::Interface::for_each([&value](const Interface &interface) {
				if(interface.active()) {
					if(!value.empty()) {
						value += ",";
					}
					value += interface.name();
					return true;
				}
			});

			return true;
		}

		return super::getProperty(key,value);

	}

	Value & Nic::List::getProperties(Value &value) const noexcept {

		getProperty("active-nic",value["active-nic"]);
		getProperty("active-nics",value["active-nics"]);

		// TODO: List all interfaces.

		return super::getProperties(value);
	}

 }

