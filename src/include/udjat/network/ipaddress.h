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

 /*
 #include <udjat/defs.h>
 #include <netdb.h>
 #include <string>
 #include <cstring>

 namespace Udjat {

	namespace IP {

		/// @brief IP Adress.
		class UDJAT_API Address {
		private:
			sockaddr_storage addr;

		public:

			Address() {
				memset(&addr,0,sizeof(addr));
			};

			Address(const sockaddr_storage &addr) : Address() {
				memcpy(&this->addr,&addr,sizeof(addr));
			}

			Address(const struct sockaddr_in &addr) : Address() {
				memcpy(&this->addr,&addr,sizeof(addr));
			}

			Address(const struct sockaddr_in6 &addr) : Address() {
				memcpy(&this->addr,&addr,sizeof(addr));
			}

			Address(const struct in_addr &addr) : Address() {
				memcpy(&this->addr,&addr,sizeof(addr));
			}

			void clear() {
				memset(&this->addr,0,sizeof(addr));
			}

			/// @brief Resolve hostname and setup storage.
			Address(const char *hostname);

			const std::string to_string(bool port = true) const;

			inline void set(const sockaddr_storage &addr)  {
				this->addr = addr;
			}

			inline void get(sockaddr_storage &addr) const {
				addr = this->addr;
			}

		};

	}

 }

 namespace std {

	UDJAT_API string to_string(const Udjat::IP::Address &addr);

	inline ostream& operator<< (ostream& os, const Udjat::IP::Address &ip) {
		return os << ip.to_string();
	}

 }
 */
