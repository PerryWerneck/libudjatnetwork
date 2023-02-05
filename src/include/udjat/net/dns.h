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

 #ifdef _WIN32
	#include <udjat/net/win32/dns.h>
 #else
	#include <udjat/net/linux/dns.h>
 #endif // _WIN32

  namespace Udjat {

	namespace DNS {

		enum Response : uint8_t {
			invalid,
			cant_resolve_server_address,
			cant_resolve_address,
			dns_ok
		};

		UDJAT_API Response ResponseFactory(const char *name);

		class UDJAT_API State : public Abstract::State {
		public:
			const DNS::Response id;

			State(const pugi::xml_node &node, const DNS::Response i) : Abstract::State{node}, id{i} {
			}

			State(const char *name, const Level level, const char *summary, const char *body, const DNS::Response i)
				: Abstract::State{name,level,summary,body}, id{i} {
			}

			static std::shared_ptr<State> Factory(const pugi::xml_node &node);
			static std::shared_ptr<State> Factory(const DNS::Response id);

		};


	}

  }

