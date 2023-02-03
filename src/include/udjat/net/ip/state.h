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
 #include <udjat/agent/state.h>
 #include <pugixml.hpp>

 namespace Udjat {

	namespace IP {

		class UDJAT_API State : public Abstract::State, public IP::Address {
		private:
			unsigned int bits = 0;

			void set(const char *subnet);

			/// @brief Test an IPV4 address range.
			bool compare(const sockaddr_in &subnet, const sockaddr_in &addr) const;

			/// @brief Test address range.
			bool compare(const sockaddr_storage &subnet, const sockaddr_storage &addr) const;

		public:

			/// @brief Create state from subnet on format xxx.xxx.xxx.xxx/bits
			State(const char *subnet);

			/// @brief Create state from XML node.
			State(const pugi::xml_node &node);

			/// @brief Test if IP is in range.
			/// @param value IP to test.
			/// @return true if IP is in state range.
			bool compare(const IP::Address &value);

			std::string to_string() const noexcept override;

		};

	}

 }
