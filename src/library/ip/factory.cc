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
 #include <udjat/tools/properties.h>
 #include <memory>

 #include <udjat/net/gateway.h>
 #include <udjat/net/ip/agent.h>
 #include <udjat/net/dns/agent.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Abstract::Agent> IP::Agent::Factory(const Properties &props) {

		
		switch(props.get("type","host").select("host","default-gateway",nullptr)) {
		case 0:	// IP based host
			return make_shared<Udjat::IP::Agent>(props);
			break;

		case 1: // Default gateway
			return make_shared<Udjat::IP::Gateway>(props);

		default:
			if(props.contains("hostname")) {
				return make_shared<Udjat::DNS::Agent>(props);
			} else if(props.contains("ip")) {
				return make_shared<Udjat::IP::Agent>(props);
			}
		}

		throw runtime_error("Cant identify network host type, missing attribute 'ip' or 'hostname");

	}

 }
