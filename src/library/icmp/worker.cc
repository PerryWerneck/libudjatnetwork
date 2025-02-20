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
 #include <udjat/net/icmp.h>
 #include <udjat/tools/object.h>

 #ifdef _WIN32
	#include <private/windows/icmp_controller.h>
 #else
	#include <private/linux/icmp_controller.h>
 #endif // _WIN32

 namespace Udjat {

	ICMP::Worker::Worker(time_t timeout, time_t interval) : timers{timeout,interval} {
	}

	ICMP::Worker::Worker(const pugi::xml_node &node, const char *addr)
		: Worker(Object::getAttribute(node,"icmp-timeout", (unsigned int) 5),Object::getAttribute(node,"icmp-interval", (unsigned int) 1)) {

		if(addr && *addr) {
			IP::Address::set(addr);
		} else {
			auto attr = node.attribute("ip");
			if(attr) {
				IP::Address::set(attr.as_string());
			}
		}

	}

	ICMP::Worker::~Worker() {
		stop();
	}

	void ICMP::Worker::start() {
		Controller::getInstance().insert(*this);
	}

	void ICMP::Worker::stop() {
		Controller::getInstance().remove(*this);
	}

	bool ICMP::Worker::getProperty(const char *key, std::string &value) const {

		if(!strcasecmp(key,"ip")) {
			value = std::to_string((IP::Address) *this);
			return true;
		}

		return false;

	}

	Value & ICMP::Worker::getProperties(Value &value) const {

#ifdef _WIN32

#else

		value["icmp-timeout"] = timers.timeout;
		value["icmp-interval"] = timers.interval;
		value["icmp-time"] = ( ((float) time) / ((float)1000000));
		value["icmp-running"] = busy;

#endif // _WIN32

		return value;

	}

 }

