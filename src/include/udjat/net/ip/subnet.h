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

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/net/ip/address.h>

 namespace Udjat {

	namespace IP {

		class UDJAT_API SubNet : public IP::Address {
		public:
			unsigned int bits = 0;

		protected:
			void set(const char *subnet);

		public:

			SubNet() {
			}

			/// @brief Create subnet from string xxx.xxx.xxx.xxx/bits
			SubNet(const char *subnet);

			/// @brief Create subnet from XML node.
			SubNet(const pugi::xml_node &node, const char *attrname="subnet");

			/// @brief Check if IPv4 address is in the subnet.
			bool contains(const sockaddr_in &addr) const;

			/// @brief Check if IPv6 address is in the subnet.
			bool contains(const sockaddr_in6 &addr) const;

			/// @brief Check if IP address is in the subnet.
			bool contains(const sockaddr_storage &addr) const;

			/// @brief Check if IP address is in the subnet.
			bool contains(const IP::Address &value) const;

			std::string to_string() const noexcept;

		};


	}

 }

