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

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/linux/netlink.h>
 #include <udjat/tools/logger.h>

 // https://stackoverflow.com/questions/7225888/how-can-i-monitor-the-nic-statusup-down-in-a-c-program-without-polling-the-ker
 #include <asm/types.h>
 #include <sys/socket.h>
 #include <unistd.h>
 #include <errno.h>
 #include <stdio.h>
 #include <string.h>
 #include <net/if.h>
 #include <netinet/in.h>
 #include <linux/netlink.h>
 #include <linux/rtnetlink.h>
 #include <stdlib.h>
 #include <sys/time.h>
 #include <sys/types.h>

 using namespace std;

 namespace Udjat {

	NetLink::Controller & NetLink::Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	NetLink::Controller::Controller() {
	}

	NetLink::Controller::~Controller() {
	}

	void NetLink::Controller::handle_event(const Event) {

		char buf[4096];
		struct iovec iov = { buf, sizeof buf };
		struct sockaddr_nl snl;
		struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
		struct nlmsghdr *h;

		int status = recvmsg(Handler::values.fd, &msg, 0);
		if (status < 0) {
			if(errno == EWOULDBLOCK || errno == EAGAIN)
				return;
			throw system_error(errno,std::system_category(),"Cant read netlink socket");
		}

		for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, (unsigned int) status); h = NLMSG_NEXT (h, status)) {

			//Finish reading
			if (h->nlmsg_type == NLMSG_DONE)
				return;

			// Message is some kind of error
			if (h->nlmsg_type == NLMSG_ERROR) {
				cerr << "netlink\tMessage is an error - decode TBD" << endl;
				return;
			}

			for(Listener &listener : listeners) {

				if(listener.message == h->nlmsg_type) {

					try {

						listener.method(NLMSG_DATA(h));

					} catch(const std::exception &e) {

						cerr << "netlink\tError processing message: " << e.what() << endl;

					} catch(...) {

						cerr << "netlink\tUnexpected error processing message" << endl;

					}
				}
			}
		}
	}

	void NetLink::Controller::push_back(void *object, uint16_t message, std::function<void(const void *)> method) {

		lock_guard<mutex> lock(guard);
		listeners.emplace_back(object,message,method);

		if(Handler::values.fd != -1) {
			return;
		}

		// Start watcher.
		Logger::String{"Starting watcher"}.trace("netlink");

		Handler::values.fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
		if(Handler::values.fd < 0) {
			throw system_error(errno,std::system_category(),"Cant get netlink socket");
		}

		struct sockaddr_nl addr;
		memset ((void *) &addr, 0, sizeof (addr));
		addr.nl_family = AF_NETLINK;
		addr.nl_pid = getpid ();
		addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE;

		if(bind(Handler::values.fd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
			throw system_error(errno,std::system_category(),"Cant bind netlink socket");
		}

		Handler::set(oninput);
		Handler::enable();

	}

	void NetLink::Controller::remove(void *object) {

		lock_guard<mutex> lock(guard);
		listeners.remove_if([object](Listener &l){
			return l.object == object;
		});

		if(!listeners.empty()) {
			return;
		}

		Logger::String{"Stopping watcher"}.trace("netlink");
		Handler::disable();
		::close(Handler::values.fd);
		Handler::values.fd = -1;
	}


 }
