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

	namespace Nic {

		class Agent;

		class UDJAT_API State : public Abstract::State {
		public:

			static std::shared_ptr<State> Factory(const pugi::xml_node &node);

			/// @brief Create state from XML node.
			State(const pugi::xml_node &node) : Abstract::State{node} {
			}

			virtual bool compare(const Nic::Agent &agent) = 0;

		};

	}

 }
