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
 #include <mutex>
 #include <udjat/tools/handler.h>
 #include <list>
 #include <functional>
 #include <asm/types.h>
 #include <linux/netlink.h>
 #include <linux/rtnetlink.h>
 #include <functional>

 namespace Udjat {

	namespace NetLink {

		class UDJAT_PRIVATE Controller  : private MainLoop::Handler {
		private:
			std::mutex guard;

			struct Listener {
				uint16_t	  message;	///< @brief The message type.
				void 		* object;	///< @brief The listener object.
				const std::function<void(const void *)> method;

				Listener(void *o, uint16_t m, const std::function<void(const void *)> l)
					: message{m}, object{o}, method{l} { }
			};

			std::list<Listener> listeners;

			void handle_event(const Event) override;

			Controller();

		public:
			~Controller();

			static Controller & getInstance();

			void push_back(void *object, uint16_t message, std::function<void(const void *)> method);
			void remove(void *object);

		};

	}

 }


