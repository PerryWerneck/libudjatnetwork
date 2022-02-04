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
 #include <udjat/tools/inet.h>
 #include <udjat/tools/threadpool.h>
 #include <cstring>
 #include <sys/types.h>
 #include <unistd.h>

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
 #include <netinet/ip_icmp.h>

 namespace Udjat {

	Network::Agent::Controller::Host::Host(Network::Agent *a, const sockaddr_storage &i) : agent(a), addr(i) {

		static unsigned short id = 0;

		this->id = id++;
		timeout = time(0) + agent->icmp.timeout;
		send();
	}

	bool Network::Agent::Controller::Host::onTimer() {

		time_t now = time(0);

		if(now > timeout) {
			agent->set(ICMPResponse::timeout);
			return false;
		}

		if(now >= next) {
			send();
		}

		return true;
	}

	bool Network::Agent::Controller::Host::onResponse(int icmp_type, const sockaddr_storage &addr, const Payload &payload) noexcept {

		if(payload.id != this->id) {
			return false;
		}

		try {

			switch(icmp_type) {
			case ICMP_ECHO: // Received my own Echo Request, ignore it.
				return false;

			case ICMP_ECHOREPLY: // Echo Reply
				agent->info() << "Got response from " << std::to_string(addr) << endl;
				agent->set(ICMPResponse::echo_reply);
				break;

			case ICMP_DEST_UNREACH: // Destination Unreachable
				agent->error() << "Received 'Destination Unreachable' from " << std::to_string(addr) << endl;
				agent->set(ICMPResponse::destination_unreachable);
				break;

			case ICMP_TIME_EXCEEDED: // Time Exceeded
				agent->error() << "Received 'Time Exceeded' from " << std::to_string(addr) << endl;
				agent->set(ICMPResponse::time_exceeded);
				break;

			}

		} catch(const exception &e) {

			agent->failed("Error processing ICMP response", e);

		}

		return true;

	}

	void Network::Agent::Controller::Host::send() noexcept {

		try {

			Payload packet;

			next = time(0) + agent->icmp.interval;

			memset(&packet,0,sizeof(packet));
			packet.id 	= this->id;
			packet.seq	= ++this->packets;

			// Get current time
			struct timespec tm;
			clock_gettime(CLOCK_MONOTONIC_RAW, &tm);
			packet.time = tm.tv_sec;
			packet.time *= 1000000;
			packet.time += tm.tv_nsec;

			Controller::getInstance().send(addr,packet);

		} catch(const exception &e) {

			agent->failed("Error sending ICMP",e);

		}

	}

 }

