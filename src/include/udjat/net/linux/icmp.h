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
 #include <pugixml.hpp>
 #include <udjat/net/ip/address.h>

 namespace Udjat {

	namespace ICMP {

		enum Response : uint8_t {
			invalid,
			echo_reply,
			destination_unreachable,
			time_exceeded,
			timeout,
			network_unreachable
		};

		class Controller;

		class UDJAT_API Worker : public Udjat::IP::Address {
		private:

			friend class Controller;

			struct Timers {
				const time_t timeout;		///< @brief ICMP timeout.
				const time_t interval;		///< @brief ICMP packet interval.

				constexpr Timers(time_t t, time_t i) : timeout{t}, interval{i} {
				}

			} timers;

			uint64_t time = 0;				///< @brief Time of last response.
			bool busy = false;

		protected:

			virtual void set(const ICMP::Response response, const IP::Address &from) = 0;

			void start();
			void stop();

		public:
			Worker(time_t timeout = 5, time_t interval = 1);
			Worker(const pugi::xml_node &node, const char *addr = nullptr);

			virtual ~Worker();

			inline time_t interval() const noexcept {
				return timers.interval;
			}

			inline time_t timeout() const noexcept {
				return timers.timeout;
			}

			inline bool running() const noexcept {
				return busy;
			}

		};

	}

 }
