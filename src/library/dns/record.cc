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
 #include <udjat/tools/net/dns.h>
 #include <string>
 #include <cstring>
// #include <udjat/tools/inet.h>
 #include <udjat/tools/net/ip.h>

 using namespace std;

 namespace Udjat {

	Network::DNSResolver::Record::Record() {
		memset(&addr,0,sizeof(addr));
	}

	static string getValue(const ns_msg &msg, const ns_rr &rr) {

		size_t szBuff = 1024;
		char *buffer = new char[szBuff+1];
		const unsigned char * msgPtr = ns_msg_base(msg);

		memset(buffer,0,szBuff);

		auto n = dn_expand(msgPtr, msgPtr + ns_msg_size(msg), ns_rr_rdata(rr), buffer, szBuff);
		if(n < 0) {
			throw runtime_error("DN Expand has failed");
		}

		string rc{buffer};

		delete[] buffer;

		return rc;
	}

	static void getValue(const ns_msg UDJAT_UNUSED(&msg), const ns_rr &rr, sockaddr_storage &addr) {

		int rdlen 		= ns_rr_rdlen(rr);
		const unsigned char * rdata	= ns_rr_rdata(rr);

		if (rdlen != (size_t) NS_INADDRSZ) {
			throw runtime_error("RR format error");
		}

		memset(&addr,0,sizeof(addr));
		addr.ss_family = AF_INET;

		((struct sockaddr_in *) &addr)->sin_addr.s_addr = ((struct in_addr *) rdata)->s_addr;

	}

	Network::DNSResolver::Record::Record(const ns_msg &msg, const ns_rr &rr) : Network::DNSResolver::Record() {

		this->ttl	= (uint32_t) ns_rr_ttl(rr);
		this->name	= ns_rr_name(rr);
		this->type	= ns_rr_type(rr);
		this->cls	= ns_rr_class(rr);

		// https://docstore.mik.ua/orelly/networking_2ndEd/dns/ch15_02.htm
		// https://github.com/lattera/glibc/blob/master/resolv/ns_print.c
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wswitch"
		switch(this->type) {
		case ns_t_a:
			getValue(msg,rr,this->addr);
			if(this->value.empty()) {
				this->value = std::to_string(this->addr);
			}
			break;

		case ns_t_cname:
		case ns_t_mb:
		case ns_t_mg:
		case ns_t_mr:
		case ns_t_ns:
		case ns_t_ptr:
		case ns_t_dname:
			this->value = getValue(msg, rr);
			break;
		}
		#pragma GCC diagnostic pop


	}

 }

