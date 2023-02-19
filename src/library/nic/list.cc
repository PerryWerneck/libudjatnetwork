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

	Nic::List::List(const pugi::xml_node &node) : Udjat::Agent<unsigned int>{node}, auto_detect{node.attribute("auto-detect").as_bool(false)} {
		init();
	}

	bool Nic::List::refresh() {
		return set(active());
	}

	Value & Nic::List::getProperties(Value &value) const noexcept {

		Abstract::Object::getProperty("nics",value["nics"]);
		Abstract::Object::getProperty("active",value["active"]);

		// TODO: List all interfaces.

		return super::getProperties(value);
	}

 }

