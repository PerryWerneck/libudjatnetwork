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
 #include <udjat/tools/file/text.h>
 #include <udjat/tools/string.h>

 using namespace std;

 namespace Udjat {

 	Nic::List::List(const pugi::xml_node &node) : Udjat::Agent<unsigned int>{node} {
	}

	bool Nic::List::refresh() {
		return set(active());
	}

	bool Nic::List::getProperties(const char *path, Value &value) const {

		if(super::getProperties(path,value)) {
			return true;
		}

		if(!*path) {
			return false;

		}

#ifdef _WIN32

		// FIX-ME: Why Network::Interface::for_each isnt working on windows?
		return false;

#else
		return Network::Interface::for_each([&value,path](const Network::Interface &interface) {
			if(!strcasecmp(interface.name(),path)) {

				value["carrier"] = stoi(File::Text{String{"/sys/class/net/",interface.name(),"/carrier"}}.c_str()) != 0;

				interface.getProperties(value);
				return true;
			}
			return false;
		});
#endif // _WIN32

	}

	Value & Nic::List::getProperties(Value &value) const {

		Abstract::Object::getProperty("nics",value["nics"]);
		Abstract::Object::getProperty("active",value["active"]);

		// TODO: List all interfaces.

		return super::getProperties(value);
	}

 }

