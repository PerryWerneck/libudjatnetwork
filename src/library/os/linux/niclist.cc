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
 #include <udjat/tools/string.h>
 #include <udjat/tools/file.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	void Nic::List::init() {

	}

	static bool carrier(const char *name) {
		return stoi(File::Text{String{"/sys/class/net/",name,"/carrier"}}.c_str()) != 0;
	}

	size_t Nic::List::active() {
		size_t active = 0;
		Network::Interface::for_each([&active](const Network::Interface &interface) {
			if(interface.up() && carrier(interface.name())) {
				active++;
			}
			return false;
		});
		return active;
	}

	bool Nic::List::getProperty(const char *key, std::string &value) const noexcept {

		if(!strcasecmp(key,"active")) {
			// Find first active nic.
			Network::Interface::for_each([&value](const Network::Interface &interface) {
				if(interface.up() && carrier(interface.name())) {
					value = interface.name();
					return true;
				}
				return false;
			});

			return true;

		}

		if(!strcasecmp(key,"nics")) {
			// Get all interfaces.
			Network::Interface::for_each([&value](const Network::Interface &interface) {
				if(!value.empty()) {
					value += ",";
				}
				value += interface.name();
				return false;
			});

			return true;
		}

		return super::getProperty(key,value);
	}


 }
