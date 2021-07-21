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

 #pragma once

 #include <udjat/defs.h>
 #include <netdb.h>
 #include <string>

 namespace std {

	UDJAT_API string to_string(const sockaddr_storage &addr, bool port = false);
	UDJAT_API string to_string(const struct sockaddr_in &addr, bool port = false);
	UDJAT_API string to_string(const struct sockaddr_in6 &addr, bool port = false);
	UDJAT_API string to_string(const struct in_addr &addr);

	inline ostream& operator<< (ostream& os, const sockaddr_storage &addr) {
		return os << to_string(addr);
	}

 }

