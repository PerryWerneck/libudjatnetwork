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
 #include <pugixml.hpp>
 #include <iostream>
 #include <udjat/net/ip/address.h>
 #include <udjat/agent/state.h>

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

		UDJAT_API Response ResponseFactory(const char *name);

		class UDJAT_API State : public Abstract::State {
		public:
			const ICMP::Response id;

			State(const pugi::xml_node &node, const ICMP::Response i) : Abstract::State{node}, id{i} {
			}

			State(const char *name, const Level level, const char *summary, const char *body, const ICMP::Response i)
				: Abstract::State{name,level,summary,body}, id{i} {
			}

			static std::shared_ptr<State> Factory(const pugi::xml_node &node);
			static std::shared_ptr<State> Factory(const ICMP::Response id);

		};

		class Controller;

		class UDJAT_API Worker {
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

		public:
			Worker(time_t timeout = 5, time_t interval = 1);
			Worker(const pugi::xml_node &node);

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

			void start(const IP::Address &addr);
			void stop();
		};

		/*
		// UDJAT_API Response ResponseFactory(const pugi::xml_node &node);

		class UDJAT_API Host {
		private:
			class Controller;
			friend class Controller;

		protected:


			/// @brief Start ICMP check.
			void start();

			virtual void set(const ICMP::Response response, const sockaddr_storage &from) = 0;

		public:
			Host();

			constexpr Host(const sockaddr_storage &a) : addr{a} {
			}

			inline void set(const sockaddr_storage &a) {
				addr = a;
			}

		};
		*/

	}

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::ICMP::Response response);

	inline ostream & operator<< (ostream& os, const Udjat::ICMP::Response response) {
		return os << ((const char *) to_string(response));
	}

 }

