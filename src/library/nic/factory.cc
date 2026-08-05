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
 #include <memory>
 #include <pugixml.hpp>
 #include <private/agents/nic.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Abstract::Agent> Nic::Agent::Factory(const Properties &props) {

		const auto device_name = props["device-name"];

		if(!device_name.empty()) {

			if(!(strcasecmp(device_name.c_str(),"*") && strcasecmp(device_name.c_str(),"all"))) {

				if(props.get("auto-detect",false)) {
					return make_shared<Nic::AutoDetect>(props);
				}

				return make_shared<Nic::List>(props);
			}

			return make_shared<Nic::Agent>(props);
		}


		throw std::system_error(EINVAL, std::system_category(),"device-name");

	}

 }
