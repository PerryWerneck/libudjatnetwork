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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/network/agents/host.h>
 #include <iostream>
 #include <udjat/tools/inet.h>
 #include <arpa/inet.h>

 using namespace std;

 extern const Udjat::ModuleInfo moduleinfo;

 namespace Udjat {

	namespace Network {

		/// @brief Abstract network agent state.
		class HostAgent::State : public Abstract::State {
		protected:
			bool revert = false;

		protected:
			virtual bool test(const sockaddr_storage &addr) const;

		public:
			State(const char *name, const Level level, const char *summary, const char *body) : Abstract::State(name,level,summary,body) {
			}

			State(const pugi::xml_node &node) : Abstract::State(node) {
			}

			virtual ~State() {
			}

			/// @brief True if the state can be used.
			bool isValid(const sockaddr_storage &addr) const noexcept;

			/// @brief True if the response can be used.
			virtual bool isValid(const ICMPResponse response) const noexcept;

		};

		/// @brief Network range state.
		class Range : public Network::HostAgent::State {
		protected:
			/// @brief Teste an IPV4 address range.
			bool inRange(const sockaddr_in &ip, const sockaddr_in &addr, const sockaddr_in &netmask) const;

			/// @brief Teste an IPV4 address range.
			bool inRange(const sockaddr_storage &ip, const sockaddr &addr, const sockaddr &netmask) const;

			/// @brief Get range from string.
			void parse(const char *range, sockaddr_storage &addr, uint16_t &mask);

			/// @brief Build netmask from length.
			static const sockaddr_in & getMask(sockaddr_in &netmask, uint16_t length);

		public:
			Range(const pugi::xml_node &node);
			virtual ~Range();

		};

	}


 }
