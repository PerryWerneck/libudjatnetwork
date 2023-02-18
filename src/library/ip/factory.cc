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
 #include <memory>

 #include <udjat/net/gateway.h>
 #include <udjat/net/ip/agent.h>
 #include <udjat/net/dns/agent.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Abstract::Agent> IP::Agent::Factory(const pugi::xml_node &node) {

		switch(String{node,"type","host"}.select("host","default-gateway",nullptr)) {
		case 0:	// IP based host
			return make_shared<Udjat::IP::Agent>(node);
			break;

		case 1: // Default gateway
			return make_shared<Udjat::IP::Gateway>(node);

		default:
			if(node.attribute("hostname")) {
				return make_shared<Udjat::DNS::Agent>(node);
			} else if(node.attribute("ip")) {
				return make_shared<Udjat::IP::Agent>(node);
			}
		}

		throw runtime_error("Cant identify network host type, missing attribute 'ip' or 'hostname");

	}

 }
