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
 #include <udjat/defs.h>
 #include <udjat/network/agents/nic.h>
 #include <udjat/tools/object.h>
 #include <string>
 #include <unistd.h>
 #include <fcntl.h>

 using namespace std;

 namespace Udjat {

	Network::NIC_STATE Network::NicStateFactory(const char *name) {

		static const char *names[] = {
			"undefined",
			"offline",
			"online",
			"multiple",
		};

		for(size_t ix = 0; ix < N_ELEMENTS(names);ix++) {

			if(!strcasecmp(name,names[ix])) {
				return (NIC_STATE) ix;
			}

		}

		throw runtime_error(string{"Invalid Network state name '"} + name + "'");

	}

	Network::NIC_STATE Network::NicStateFactory(const pugi::xml_node &node) {

		static const char * names[] = {
			"network-state",
			"interface-state",
			"state-name",
			"name"
		};

		// First check for 'value=' attribute for compatibility.
		pugi::xml_attribute attr = node.attribute("value");
		if(attr) {
			unsigned short value = attr.as_uint();
			if(value > N_ELEMENTS(names)) {
				clog << "Unexpected state value '" << value << "'" << endl;
			}
			return (NIC_STATE) value;
		}

		// Then check for other attributes.
		for(size_t ix = 0; ix < N_ELEMENTS(names); ix++) {
			attr = Object::getAttribute(node,names[ix],false);
			if(attr) {
				return NicStateFactory(attr.as_string("undefined"));
			}
		}

		return NIC_STATE_UNDEFINED;

	}

	bool Network::Agent::has_link(const char *name) {

		// Check link
		string path{"/sys/class/net/"};
		path += name;
		path += "/carrier";

		bool link = false;

		if(access(path.c_str(),R_OK)) {
			cerr << "Can't access " << path << endl;
		} else {
			FILE *in = fopen(path.c_str(),"r");
			if(in) {
				if(fgetc(in) != '0') {
					link = true;
				}
				fclose(in);
			}
		}

		return link;

	}

 }
