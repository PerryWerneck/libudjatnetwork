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
 #include <pugixml.hpp>
 #include <sys/socket.h>

 namespace Udjat {

	namespace ICMP {

		enum Response : uint8_t {
			invalid,
			echo_reply,
			destination_unreachable,
			time_exceeded,
			timeout,
			network_unreachable
		};

		UDJAT_API Response ResponseFactory(const char *name);
		// UDJAT_API Response ResponseFactory(const pugi::xml_node &node);

		class UDJAT_API Host {
		private:
			class Controller;
			friend class Controller;

		protected:

			sockaddr_storage addr;	///< @brief ICMP host to check.
			time_t interval = 1;		///< @brief ICMP packet interval.
			time_t timeout = 5;			///< @brief ICMP timeout.
			uint64_t time = 0;			///< @brief Time of last response.

			/// @brief Start ICMP check.
			void start();

			virtual void set(const ICMP::Response response, const sockaddr_storage &from) = 0;

		public:
			Host();

			constexpr Host(const sockaddr_storage &a) : addr{a} {
			}

			inline void set(const sockaddr_storage &a) {
				addr = a;
			}


		};

	}

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::ICMP::Response response);

	inline ostream & operator<< (ostream& os, const Udjat::ICMP::Response response) {
			return os << to_string(response);
	}

 }

