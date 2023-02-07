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

 // References:
 //
 // https://www.linuxquestions.org/questions/linux-networking-3/howto-find-gateway-address-through-code-397078/
 // https://gist.github.com/javiermon/6272065#file-gateway_netlink-c
 //

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/net/gateway.h>

 using namespace std;

 namespace Udjat {

	bool IP::Gateway::refresh() {

		bool changed = detect();

		if(IP::Agent::refresh()) {
			changed = true;
		}

		return changed;
	}

 }
