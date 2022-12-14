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

 #include <controller.h>
 #include <udjat/tools/ip.h>
 #include <udjat/tools/threadpool.h>
 #include <cstring>
 #include <sys/types.h>
 #include <unistd.h>
 #include <private/agents/host.h>
 #include <udjat/tools/logger.h>
 #include <netinet/ip_icmp.h>

 namespace Udjat {

	ICMP::Host::Host() {
		memset(&addr,0,sizeof(addr));
	}

	void ICMP::Host::start() {
		Controller::getInstance().insert(this);
	}

	ICMP::Host::Controller::Host::Host(ICMP::Host *h) : host{h} {

		static unsigned short id = 0;

		this->id = id++;
		timeout = ::time(0) + host->timeout;
		send();
	}

	bool ICMP::Host::Controller::Host::onTimer() {

 		time_t now = ::time(0);

		if(now > timeout) {
			host->set(Response::timeout,host->addr);
			return false;
		}

		if(now >= next) {
			send();
		}

		return true;
	}

	bool ICMP::Host::Controller::Host::onError(int code, const Controller::Payload &payload) {

		if(payload.id == this->id) {

			switch(code) {
			case ENETUNREACH:	// Network is unreachable
				host->set(Response::network_unreachable,host->addr);
				return true;

			default:
				cerr << "icmp\tError '" << strerror(code) << "' searching " << host->addr << endl;

			}

		}

		return false;
	}

	bool ICMP::Host::Controller::Host::onResponse(int icmp_type, const sockaddr_storage &addr, const Payload &payload) noexcept {

		if(payload.id != this->id) {
			return false;
		}

		try {

			switch(icmp_type) {
			case ICMP_ECHO: // Received my own Echo Request, ignore it.
				return false;

			case ICMP_ECHOREPLY: // Echo Reply
				{
					uint64_t now = ICMP::Host::Controller::getCurrentTime();

					if(payload.time >= now) {
						host->time = (payload.time - now);
					} else {
						host->time = (now - payload.time);
					}

					host->set(Response::echo_reply,addr);
				}
				break;

			case ICMP_DEST_UNREACH: // Destination Unreachable
				host->time = (uint64_t) -1;
				host->set(Response::destination_unreachable,addr);
				break;

			case ICMP_TIME_EXCEEDED: // Time Exceeded
				host->time = (uint64_t) -1;
				host->set(Response::time_exceeded,addr);
				break;

			default:
				clog << "Unexpected ICMP response '" << ((int) icmp_type) << "' from " << addr << endl;
			}

		} catch(const exception &e) {

			cerr << "Error processing ICMP response from " << addr << ": " << e.what() << endl;

		}

		return true;

	}

	void ICMP::Host::Controller::Host::send() noexcept {

		try {

			Payload packet;

			next = ::time(0) + host->interval;

			memset(&packet,0,sizeof(packet));
			packet.id 	= this->id;
			packet.seq	= ++this->packets;
			packet.time = getCurrentTime();

			Controller::getInstance().send(host->addr,packet);

		} catch(const exception &e) {

			cerr << "Error sending ICMP: " << e.what() << endl;

		}

	}

 }

