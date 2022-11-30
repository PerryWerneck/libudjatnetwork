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
 #include <private/agents/host.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/handler.h>
 #include <udjat/tools/net/icmp.h>
 #include <mutex>
 #include <memory>
 #include <iostream>
 #include <list>

 using namespace std;

 namespace Udjat {

 	class ICMP::Host::Controller : private MainLoop::Timer, private MainLoop::Handler {
	public:

		#pragma pack(1)
		/// @brief ICMP payload.
		struct Payload {
			uint16_t	id;
			uint16_t	seq;
			uint64_t	time;
		};
		#pragma pack()

		static uint64_t getCurrentTime() noexcept;

	private:

		static recursive_mutex guard;

		class Host {
		private:

			ICMP::Host *host;

			uint16_t id = 0;
			uint16_t packets = 0;
			time_t timeout;
			time_t next = 0;

		public:

			Host(ICMP::Host *host);

			bool onTimer();
			void send() noexcept;

			inline bool operator ==(const ICMP::Host *host) const noexcept {
				return host == this->host;
			}

			/// @brief Process response.
			/// @return true if the response was processed and host can be removed.
			bool onResponse(int icmp_type, const sockaddr_storage &addr, const Controller::Payload &payload) noexcept;

		};

		list<Host> hosts;

		Controller();

		void start();
		void stop();

		void on_timer() override;
		void handle_event(const Event event) override;


	public:

		static Controller & getInstance();

		~Controller();

		void insert(ICMP::Host *host);
		void remove(ICMP::Host *host);

		void send(const sockaddr_storage &addr, const Payload &payload);

	};


 }
