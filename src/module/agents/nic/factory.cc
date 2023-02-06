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

 /*
 #include <config.h>
 #include <private/module.h>
 #include <udjat/net/nic/agent.h>
 #include <agent.h>

 namespace Udjat {

 	Network::Agent::Factory::Factory() : Udjat::Factory("network-interface",moduleinfo) {
	}

	std::shared_ptr<Abstract::Agent> Network::Agent::Factory::AgentFactory(const Abstract::Object UDJAT_UNUSED(&parent), const pugi::xml_node &node) const {

		// Has device name? If yes create device agent
		if(node.attribute("device-name")) {
			return make_shared<Udjat::Nic::Agent>(node);
		}

		// No device name, create container.
		return make_shared<Udjat::Nic::List>(node);

	}

 }
 */
