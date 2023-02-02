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
 #include <udjat/agent/state.h>
 #include <udjat/tools/net/icmp.h>
 #include <udjat/tools/net/ip.h>

 namespace Udjat {

	namespace Network {

		/// @brief Agent to check for DNS resolution and ICMP test.
		class UDJAT_API HostAgent : public Udjat::Abstract::Agent, private ICMP::Host {
		public:
			class State;

		private:

			struct {
				/// @brief The state from IP addr.
				std::shared_ptr<Abstract::State> addr;

				/// @brief The state from ICMP.
				std::shared_ptr<Abstract::State> icmp;

				/// @brief Agent states.
				std::vector<std::shared_ptr<State>> available;

			} states;

			struct {
				bool dns = false;		///< @brief Do DNS check.
				bool icmp = true;		///< @brief Do ICMP check.
			} check;

		protected:

			/// @brief Set address (do an ICMP check if necessary).
			void set(const sockaddr_storage &addr);

			/// @brief Do a DNS check.
			static sockaddr_storage resolv(sockaddr_storage &dnssrv, const char *hostname);

			void set(const ICMP::Response response, const sockaddr_storage &from) override;

		public:

			class State;

			class Factory : public Udjat::Factory {
			public:
				Factory();
				std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object &parent, const pugi::xml_node &node) const override;
			};

			HostAgent(const pugi::xml_node &node);
			virtual ~HostAgent();

			std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node) override;

			std::string to_string() const noexcept override;

			Udjat::Value & get(Udjat::Value &value) const;

			Value & getProperties(Value &response) const noexcept override;

 		};


	}

 }
