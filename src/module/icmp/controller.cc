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
 #include <unistd.h>
 #include <netdb.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/inet.h>

 namespace Udjat {

	 recursive_mutex Network::Agent::Controller::guard;

	 Network::Agent::Controller::Controller() {
	 }

	 Network::Agent::Controller::~Controller() {
		lock_guard<recursive_mutex> lock(guard);
	 	stop();
	 }

	 Network::Agent::Controller & Network::Agent::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller instance;
		return instance;
	 }

	 void Network::Agent::Controller::stop() {

#ifdef DEBUG
		cout << "Stopping ICMP Worker" << endl;
#endif // DEBUG

		MainLoop::getInstance().remove(this);

	 	if(sock > 0) {
			close(sock);
			sock = -1;
	 	}

	 }

	 void Network::Agent::Controller::start() {

		if(sock > 0) {
			return;
		}

		try {

#ifdef DEBUG
			cout << "Starting ICMP Worker" << endl;
#endif // DEBUG

			// Create socket
			{
				// Reference: https://github.com/schweikert/fping/blob/develop/src/socket4.c
				protoent * proto = getprotobyname("icmp");
				if(!proto) {
					throw runtime_error("ICMP: Unknown protocol");
				}

				sock = socket(AF_INET, SOCK_RAW, proto->p_proto);
				if(sock < 0) {
					throw std::system_error(errno, std::system_category(), "Cant create ICMP socket");
				}

				// Set non-blocking
				int flags;

				if ((flags = fcntl(sock, F_GETFL, 0)) < 0) {
					throw std::system_error(errno, std::system_category(), "Cant get ICMP socket flags");
				}

				if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0){
					throw std::system_error(errno, std::system_category(), "Cant set ICMP socket flags");
				}

			}

			// Listen for package
			MainLoop::getInstance().insert(this,sock,MainLoop::oninput,[this](const MainLoop::Event event) {

				lock_guard<recursive_mutex> lock(guard);


				return true;

			});

			// Timer for packet sent.
			MainLoop::getInstance().insert(this,1000L,[this]() {

				ThreadPool::getInstance().push([this]() {

					lock_guard<recursive_mutex> lock(guard);

					// Send packets.
					hosts.remove_if([](Host &host) {
						return !host.onTimer();
					});

					if(hosts.empty()) {
						stop();
					}

				});

				return true;

			});

		} catch(...) {

			stop();
			throw;

		}


	 }

	 void Network::Agent::Controller::insert(Network::Agent *agent, const sockaddr_storage &addr) {

		lock_guard<recursive_mutex> lock(guard);

		start();
		hosts.emplace_back(agent,addr);

	 }

	 void Network::Agent::Controller::remove(Network::Agent *agent) {

		lock_guard<recursive_mutex> lock(guard);

		hosts.remove_if([agent](Host &h) {
			return h == agent;
		});

		if(hosts.empty()) {
			stop();
		}

	 }

	void Network::Agent::Controller::send(const sockaddr_storage &addr, const Payload &payload) {

		// TODO: Add IPV6 support.

		if(addr.ss_family != AF_INET) {
			throw runtime_error("Unsupported family");
		}

#ifdef DEBUG
		cout 	<< "Sending ICMP " << payload.id << "." << payload.seq
				<< " to " << std::to_string(addr) << endl;
#endif // DEBUG


	}

 }
