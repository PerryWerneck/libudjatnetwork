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
 #include <private/agents/nic.h>
 #include <udjat/net/nic/agent.h>
 #include <private/linux/netlink.h>
 #include <udjat/net/interface.h>
 #include <memory>

 using namespace std;

 namespace Udjat {

	Nic::AutoDetect::AutoDetect(const pugi::xml_node &node) : Nic::Agent{node} {

		Network::Interface::for_each([this,node](const Network::Interface &interface) {
			shared_ptr<Abstract::Agent> agent = make_shared<Nic::Agent>(node,interface.name());
			Abstract::Agent::push_back(agent);
			return false;
		});

	}

	Nic::AutoDetect::~AutoDetect() {
		NetLink::Controller::getInstance().remove(this);
	}

	void Nic::AutoDetect::start() {
		Abstract::Agent::start();
	}

	void Nic::AutoDetect::stop() {
		NetLink::Controller::getInstance().remove(this);
		Abstract::Agent::stop();
	}

 }
