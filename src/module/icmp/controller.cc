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
 #include <controller.h>
 #include <unistd.h>
 #include <netdb.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/inet.h>
 #include <netinet/ip_icmp.h>

 namespace Udjat {

	#pragma pack(1)
	struct Packet {
		struct icmp icmp;
		struct Network::HostAgent::Controller::Payload payload;
	};
	#pragma pack()

	recursive_mutex Network::HostAgent::Controller::guard;

	Network::HostAgent::Controller::Controller() : MainLoop::Handler(-1, MainLoop::Handler::oninput) {
	}

	Network::HostAgent::Controller::~Controller() {
		lock_guard<recursive_mutex> lock(guard);
	 	stop();
	}

	Network::HostAgent::Controller & Network::HostAgent::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	void Network::HostAgent::Controller::stop() {

		this->Handler::disable();
		this->Timer::disable();
		this->Handler::close();

		Logger::String{"ICMP listener disabled"}.write(Logger::Trace,"ICMP");

	}

	void Network::HostAgent::Controller::handle_event(const Event event) {

#ifdef DEBUG
		cout << "*** EVENT on ICMP listener" << endl;
#endif // DEBUG

		lock_guard<recursive_mutex> lock(guard);

		if(event & MainLoop::Handler::oninput) {

			// Receive packet.
			#pragma pack(1)
			struct {
				struct iphdr    hdr;
				struct Packet   packet;
			} in;
			#pragma pack()

			struct sockaddr_storage addr;
			socklen_t szAddr = sizeof(addr);

			memset(&in,0,sizeof(in));
			memset(&addr,0,sizeof(addr));

			int rc = recvfrom(fd,&in,sizeof(in),MSG_DONTWAIT,(struct sockaddr *) &addr,&szAddr);
			if(rc < 0) {
				cerr << "ICMP\tError '" << strerror(errno) << "' receiving ICMP packet" << endl;
				return;
			}

			if(rc != sizeof(in)) {
#ifdef DEBUG
				cout << "ICMP\tIgnoring packet with invalid size" << endl;
#endif // DEBUG
				return;
			}

			if(in.packet.icmp.icmp_id != htons((uint16_t) getpid())) {
				cout << "ICMP\tIgnoring packet with invalid id" << endl;
				return;
			}

#ifdef DEBUG
			cout << "ICMP\tReceived packet " << htons(in.packet.icmp.icmp_seq) << " from " << std::to_string(addr) << endl;
#endif // DEBUG

			hosts.remove_if([&in,&addr](Host &host) {
				return host.onResponse(in.packet.icmp.icmp_type,addr,in.packet.payload);
			});

		}


	}

	void Network::HostAgent::Controller::on_timer() {

		ThreadPool::getInstance().push([this]() {

			lock_guard<recursive_mutex> lock(guard);

			// Send packets.
			hosts.remove_if([](Host &host) {
				return !host.onTimer();
			});

			if(hosts.empty()) {
				Logger::String{"No more hosts, disabling ICMP listener"}.write(Logger::Trace,"ICMP");
				stop();
			}

		});

	}

	void Network::HostAgent::Controller::start() {

		try {

			Logger::String{"Starting ICMP Listener"}.write(Logger::Trace,"ICMP");

			// Create socket
			if(fd <= 0) {

				// Reference: https://github.com/schweikert/fping/blob/develop/src/socket4.c
				protoent * proto = getprotobyname("icmp");
				if(!proto) {
					throw runtime_error("ICMP: Unknown protocol");
				}

				fd = socket(AF_INET, SOCK_RAW, proto->p_proto);
				if(fd < 0) {
					throw std::system_error(errno, std::system_category(), "Cant create ICMP socket");
				}

				// Set non-blocking
				int flags;

				if ((flags = fcntl(fd, F_GETFL, 0)) < 0) {
					throw std::system_error(errno, std::system_category(), "Cant get ICMP socket flags");
				}

				if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0){
					throw std::system_error(errno, std::system_category(), "Cant set ICMP socket flags");
				}

			}

			// Timer for packet sent.
			this->Timer::reset(1000L);
			if(!this->Timer::enabled()) {
				this->Timer::enable();
			}

			if(!this->Handler::enabled()) {
				Logger::String{"Enabling ICMP listener"}.write(Logger::Trace,"ICMP");
				this->Handler::enable();
			}

		} catch(...) {

			stop();
			throw;

		}


	}

	void Network::HostAgent::Controller::insert(Network::HostAgent *agent, const sockaddr_storage &addr) {

		lock_guard<recursive_mutex> lock(guard);

		start();
		hosts.emplace_back(agent,addr);

	}

	void Network::HostAgent::Controller::remove(Network::HostAgent *agent) {

		lock_guard<recursive_mutex> lock(guard);

		hosts.remove_if([agent](Host &h) {
			return h == agent;
		});

		if(hosts.empty()) {
			stop();
		}

	}

	static unsigned short in_chksum(const unsigned short *buf, int sz) {

		int nleft = sz;
		int sum = 0;
		const unsigned short *w = buf;
		unsigned short ans = 0;

		while (nleft > 1) {
			sum += *w++;
			nleft -= 2;
		}

		if (nleft == 1) {
			*(unsigned char *) (&ans) = *(unsigned char *) w;
			sum += ans;
		}

		sum = (sum >> 16) + (sum & 0xFFFF);
		sum += (sum >> 16);
		ans = ~sum;
		return ans;
	}

	void Network::HostAgent::Controller::send(const sockaddr_storage &addr, const Payload &payload) {

		if(fd < 0) {
			throw runtime_error("ICMP Controller is not available");
		}

		// TODO: Add IPV6 support.

		if(addr.ss_family != AF_INET) {
			throw runtime_error("Unsupported family");
		}

		Logger::String(
			"Sending ICMP ", payload.id ,".", payload.seq, " to ", std::to_string(addr)
#ifdef DEBUG
			, " on socket ", fd
#endif // DEBUG
		).write(Logger::Debug,"ICMP");

		// Send package
		Packet packet;

		memset(&packet,0,sizeof(packet));
		packet.payload = payload;

		static uint16_t seq = 0;
		packet.icmp.icmp_type = ICMP_ECHO;
		packet.icmp.icmp_seq = htons(++seq);
		packet.icmp.icmp_id = htons(getpid());
		packet.icmp.icmp_cksum = in_chksum((unsigned short *) &packet, sizeof(packet));

		if(sendto(fd, (char *) &packet, sizeof(packet), 0, (const sockaddr *) &addr, sizeof(addr)) != sizeof(packet)) {
			throw std::system_error(errno, std::system_category(), "Can't send ICMP packet");
		}

	}

 }
