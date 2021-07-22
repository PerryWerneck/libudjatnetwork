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
 #include <udjat/factory.h>
 #include <udjat/agent.h>
 #include <udjat/state.h>
 #include <sys/socket.h>

 namespace Udjat {

	namespace Network {

		enum ICMPResponse : uint8_t {
			echo_reply,
			destination_unreachable,
			time_exceeded,
			timeout
		};

		class UDJAT_API Agent : public Udjat::Abstract::Agent {
		public:
			class State;

		private:

			class Controller;
			friend class Controller;

			struct {
				bool check = true;		///< @brief Do ICMP check.
				time_t interval = 1;	///< @brief ICMP packet interval.
				time_t timeout = 5;		///< @brief ICMP timeout.
			} icmp;

			struct {
				bool check = true;	///< @brief Check DNS resolution.
			} dns;

			/// @brief DNS Addr if check.dns is true or host addr if check.dns is false.
			sockaddr_storage addr;

			/// @brief Host to check.
			const char * hostname = nullptr;

			/// @brief Agent states.
			std::vector<std::shared_ptr<State>> states;

		public:

			class State;

			class Factory : public Udjat::Factory {
			public:
				Factory();
				void parse(Abstract::Agent &parent, const pugi::xml_node &node) const override;
			};

			Agent(const pugi::xml_node &node);
			virtual ~Agent();

			bool hasStates() const noexcept override;
			void append_state(const pugi::xml_node &node) override;
			void refresh() override;

			void set(ICMPResponse response);

		};


	}

 }
