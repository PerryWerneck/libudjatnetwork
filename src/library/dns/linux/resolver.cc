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

 #include <config.h>
 #include <udjat/net/dns.h>
 #include <cstring>
 #include <netdb.h>
 #include <iostream>
 #include <netinet/in.h>
 #include <arpa/nameser.h>
 #include <resolv.h>

 using namespace std;

 namespace Udjat {

	mutex DNS::Resolver::guard;

	DNS::Exception::Exception(int code) : runtime_error{hstrerror(code)}, err{code} {
	}

	DNS::Resolver::Resolver() {

		std::lock_guard<std::mutex> lock(guard);

		memset(&this->state,0,sizeof(this->state));

		if(res_ninit(&this->state) != 0) {

			// Erro na inicialização
			throw std::system_error(EINVAL, std::system_category());

		}

	}

	DNS::Resolver::Resolver(const struct sockaddr_storage &server) : DNS::Resolver{} {
		set(server);
	}

	DNS::Resolver::~Resolver() {
		std::lock_guard<std::mutex> lock(guard);
		res_nclose(&this->state);
	}

	/// @brief Set Address of the nameserver.
	void DNS::Resolver::set(const struct sockaddr_storage &server) {

		if(!server.ss_family) {
			return;
		}

		std::lock_guard<std::mutex> lock(guard);

		if(server.ss_family == AF_INET) {
			this->state.nsaddr_list[0].sin_addr = ((struct sockaddr_in *) &server)->sin_addr;
			this->state.nscount = 1;
		}

		throw runtime_error("Invalid record type when setting DNS server");

	}

	/// @brief Run DNS query.
	DNS::Resolver & DNS::Resolver::query(ns_class cls, ns_type type, const char *name) {

		debug("Resolving '",name,"'");

		if(!(name && *name)) {
			throw runtime_error("Cant resolve an empty hostname");
		}

		std::lock_guard<std::mutex> lock(guard);

		records.clear();

		unsigned char query_buffer[1024];

		int szResponse = res_nquery(&this->state, name, cls, type, query_buffer, sizeof(query_buffer));

		if (szResponse < 0) {
			throw DNS::Exception(h_errno);
		}

		ns_msg	msg;
		ns_rr	rr;
		ns_initparse(query_buffer, szResponse, &msg);

		for(int x = 0; x < ns_msg_count(msg, ns_s_an); x++) {

			ns_parserr(&msg, ns_s_an, x, &rr);
			records.emplace_back(msg, rr);

		}

		return *this;

	}

 }
