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
 #include <resolv.h>
 #include <arpa/nameser.h>
 #include <mutex>
 #include <vector>
 #include <string>
 #include <sys/socket.h>

 namespace Udjat {

	namespace DNS {

		class Resolver;

		/// @brief DNS Record
		class UDJAT_API Record {
		private:
			friend class Resolver;

			/// @brief TTL value from the record
			uint32_t ttl;

			std::string name;

			ns_type type;
			ns_class cls;

			/// @brief IP address
			sockaddr_storage addr;

			/// @brief Record value
			std::string value;

		public:
			Record();
			Record(const ns_msg &msg, const ns_rr &rr);

			/// @brief Get TTL value from the record;
			inline uint32_t getTTL() const noexcept {
				return this->ttl;
			}

			inline const char * getName() const noexcept {
				return this->name.c_str();
			}

			inline ns_type getType() const noexcept {
				return this->type;
			}

			inline ns_class getClass() const noexcept {
				return this->cls;
			}

			inline const sockaddr_storage & getAddr() const {
				return this->addr;
			}

			inline std::string toString() const {
				return value;
			}

			inline const char * c_str() const {
				return value.c_str();
			}

			inline operator sockaddr_storage() const {
				return this->addr;
			}

		};

		class UDJAT_API Resolver {
		private:
			struct __res_state state;
			static std::mutex guard;
			std::vector<Record> records;

		public:

			Resolver();
			Resolver(const struct sockaddr_storage &server);
			~Resolver();

			inline std::vector<Record>::const_iterator begin() const {
				return records.begin();
			}

			inline std::vector<Record>::const_iterator end() const {
				return records.end();
			}

			inline bool empty() const noexcept {
				return records.empty();
			}

			inline size_t size() const noexcept {
				return records.size();
			}

			/// @brief Set Address of the nameserver.
			void set(const struct sockaddr_storage &server);

			/// @brief Query service hosts.
			///
			/// @param cls		The class of data being looked for.
			/// @param type	The type of request being made.
			/// @param domain	The pointer to the domain name.
			///
			Resolver & query(ns_class cls, ns_type type, const char *name);

			/// @brief Run DNS query.
			inline Resolver & query(const char *name) {
				return query(ns_c_in, ns_t_a, name);
			}

		};

	}

 }
