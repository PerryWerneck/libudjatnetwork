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
 #include <udjat/tools/net/ip.h>
 #include <udjat/net/icmp.h>

 namespace Udjat {

	namespace IP {

		class Agent : public Udjat::IP::Address, public Abstract::Agent, private ICMP::Worker  {
		private:
			bool icmp = true;	// Is ICMP check enabled?

		protected:

			virtual void set(const ICMP::Response response, const IP::Address &from) override;

		public:
			Agent(const char *name = "");
			Agent(const pugi::xml_node &node);

			/// @brief Do an ICMP check
			/// @return true if the state has changed.
			bool refresh() override;

			std::string to_string() const noexcept override;

			Udjat::Value & get(Udjat::Value &value) const override;

			Udjat::Value & getProperties(Value &value) const noexcept override;

		};

	}

 }

