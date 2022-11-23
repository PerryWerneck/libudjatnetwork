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
 #include <udjat/tools/net/nic.h>
 #include <sys/socket.h>
 #include <list>

 namespace Udjat {

	namespace Network {

		enum NIC_STATE : unsigned short {

			NIC_STATE_UNDEFINED,	///< @brief Undefined state.

			NIC_STATE_OFFLINE,		///< @brief No active interface.
			NIC_STATE_ONLINE,		///< @brief Just one active interface.
			NIC_STATE_MULTIPLE,		///< @brief More than one active interface.

		};

		NIC_STATE NicStateFactory(const char *name);

		namespace Agent {

			std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node);

			/// @brief Network agent factory.
			class UDJAT_API Factory : public Udjat::Factory {
			public:
				Factory();
				std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object &parent, const pugi::xml_node &node) const override;
			};

			/// @brief State of one single interface.
			class UDJAT_API Interface : public Udjat::Agent<unsigned short> {
			private:
				const char * interface_name;

			public:

				Interface(const pugi::xml_node &node);
				virtual ~Interface();

				// std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node) override;

				bool refresh() override;

				bool getProperty(const char *key, std::string &value) const noexcept override;

			};

			/// @brief State of ALL network interfaces (except loopback).
			class UDJAT_API Interfaces : public Udjat::Agent<unsigned short> {
			private:

				/// @brief Network interfaces.
				struct Interface {
					std::string name;
					bool active = false;

					Interface(const char *n) : name{n} {
					}

				};

				std::list<Interface> interfaces;

				/// @brief Find interface by name, insert one if needed.
				struct Interface & find_interface(const char *name);

			public:

				Interfaces(const pugi::xml_node &node);
				virtual ~Interfaces();

				// std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node) override;

				bool refresh() override;

				bool getProperty(const char *key, std::string &value) const noexcept override;

			};

		}

	}

 }
