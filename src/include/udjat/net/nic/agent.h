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
 #include <udjat/agent/abstract.h>
 #include <udjat/agent.h>
 #include <udjat/net/nic/state.h>

 namespace Udjat {

	namespace Nic {

		class State;
		class Controller;

		/// @brief Simple network interface agent.
		class UDJAT_API Agent : public Abstract::Agent, public std::string  {
		private:
			friend class State;

#ifndef _WIN32
			struct {
				int index = 0;
				bool netlink = false;		///< @brief True on RTM_NEWLINK, false on RTM_DELLINK.
				bool exist = false;			///< @brief True if cant get device info.
				unsigned int flags = 0;		///< @brief Interface flags.
			} intf;

#endif // !_WIN32

			std::vector<std::shared_ptr<Nic::State>> states;			///< @brief XML defined NIC states.

		protected:
			std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node) override;
			std::shared_ptr<Abstract::State> computeState() override;

		public:

			static std::shared_ptr<Abstract::Agent> Factory(const pugi::xml_node &node);

			Agent(const char *name = "");
			Agent(const pugi::xml_node &node, const char *name = "");
			virtual ~Agent();

			void start() override;
			void stop() override;
			bool refresh() override;

			/// @brief Check for interface link.
			/// @return true if the interface is 'up' and has an active link.
			// bool link();

#ifndef _WIN32
			inline unsigned int flags() const noexcept {
				return intf.flags;
			}

			inline bool exist() const noexcept {
				return intf.netlink;
			}
#endif // !_WIN32

		};

		/// @brief Container with all network interfaces.
		class UDJAT_API List : public Udjat::Agent<unsigned int>  {
		public:
			List(const pugi::xml_node &node);

			void start() override;
			void stop() override;

			/// @brief Get number of active interfaces.
			size_t active();

			bool refresh() override;

			Udjat::Value & getProperties(Value &value) const override;
			bool getProperty(const char *key, std::string &value) const override;

			bool getProperties(const char *path, Value &value) const override;

		};

	}

 }

