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

 // References:
 //
 // https://www.linuxquestions.org/questions/linux-networking-3/howto-find-gateway-address-through-code-397078/
 // https://gist.github.com/javiermon/6272065#file-gateway_netlink-c
 //

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/net/gateway.h>
 #include <udjat/net/ip/address.h>

 #include <sys/socket.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <linux/netlink.h>
 #include <linux/rtnetlink.h>
 #include <arpa/inet.h>
 #include <unistd.h>
 #include <net/if.h>
 #include <iostream>
 #include <udjat/tools/intl.h>
 #include <private/linux/netlink.h>

 #include <stdexcept>
 #include <system_error>

 #define BUFFER_SIZE 4096

 using namespace std;

 namespace Udjat {

	IP::Gateway::Gateway() : Udjat::IP::Agent{"gateway"} {
	}

	IP::Gateway::Gateway(const pugi::xml_node &node) : Udjat::IP::Agent{node} {
	}

	void IP::Gateway::start() {

		// https://stackoverflow.com/questions/11788326/extract-current-route-from-netlink-message-code-attached

		// Route was added.
		NetLink::Controller::getInstance().push_back(this,RTM_NEWROUTE,[this](const void *m){

			debug("------------------------------------------ RTM_NEWROUTE");

			// struct rtmsg *route_entry = (struct rtmsg *) m;

		});

		// Route was removed.
		NetLink::Controller::getInstance().push_back(this,RTM_DELROUTE,[this](const void *m){

			debug("------------------------------------------ RTM_DELROUTE");

			// struct rtmsg *route_entry = (struct rtmsg *) m;


		});

	}

	void IP::Gateway::stop() {
		NetLink::Controller::getInstance().remove(this);
	}

	bool IP::Gateway::detect() {

		sockaddr_storage gateway;
		uint32_t msgseq = 0;
		int received_bytes;
		struct nlmsghdr *nlh;

		memset(&gateway,0,sizeof(gateway));

		int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

		if(sock < 0) {
			throw std::system_error(errno, std::system_category(), _("Cant get netlink socket"));
		}

		try {

			// 1 Sec Timeout to avoid stall
			struct timeval tv;
			memset(&tv,0,sizeof(tv));
			tv.tv_sec = 1;
			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));

			// Fill in the nlmsg header
			char msgbuf[BUFFER_SIZE];
			memset(msgbuf, 0, sizeof(msgbuf));
			struct nlmsghdr *nlmsg = (struct nlmsghdr *) msgbuf;

			nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
			nlmsg->nlmsg_type = RTM_GETROUTE; 					// Get the routes from kernel routing table .
			nlmsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;	// The message is a request for dump.
			nlmsg->nlmsg_seq = msgseq++;						// Sequence of the message packet.
			nlmsg->nlmsg_pid = getpid();						// PID of process sending the request.

			// Send
			if(send(sock, nlmsg, nlmsg->nlmsg_len, 0) < 0) {
				throw std::system_error(errno, std::system_category(), _("Cant send netlink message"));
			}

			// receive response
			char buffer[BUFFER_SIZE];
			memset(buffer, 0, sizeof(buffer));
			char *ptr = buffer;

			int msg_len = 0;

			do {

				received_bytes = recv(sock, ptr, sizeof(buffer) - msg_len, 0);
				if (received_bytes < 0) {
					throw std::system_error(errno, std::system_category(), _("Cant receive netlink response"));
				}

				nlh = (struct nlmsghdr *) ptr;

				// Check if the header is valid
				if((NLMSG_OK(nlmsg, received_bytes) == 0) || (nlmsg->nlmsg_type == NLMSG_ERROR)) {
					throw runtime_error(_("Error in received packet"));
				}

				// If we received all data break
				if (nlh->nlmsg_type == NLMSG_DONE) {
					break;
				} else {
					ptr += received_bytes;
					msg_len += received_bytes;
				}

				// Break if its not a multi part message
				if ((nlmsg->nlmsg_flags & NLM_F_MULTI) == 0) {
					break;
				}

			} while ((nlmsg->nlmsg_seq != msgseq) || (nlmsg->nlmsg_pid != (unsigned) getpid()));

			// parse response
			struct rtmsg *route_entry;
			struct rtattr *route_attribute;
			int route_attribute_len = 0;
			for ( ; NLMSG_OK(nlh, received_bytes); nlh = NLMSG_NEXT(nlh, received_bytes)) {

				// Get the route data
				route_entry = (struct rtmsg *) NLMSG_DATA(nlh);

				// We are just interested in main routing table
				if (route_entry->rtm_table != RT_TABLE_MAIN) {
					continue;
				}

				route_attribute = (struct rtattr *) RTM_RTA(route_entry);
				route_attribute_len = RTM_PAYLOAD(nlh);

				// Loop through all attributes
				for(;RTA_OK(route_attribute, route_attribute_len); route_attribute = RTA_NEXT(route_attribute, route_attribute_len)) {

					switch(route_attribute->rta_type) {
					case RTA_OIF:
						{
							char interface[IF_NAMESIZE];
							if_indextoname(*(int *)RTA_DATA(route_attribute), interface);
							this->intf = interface;
						}
						break;

					case RTA_GATEWAY:
						{
							if((unsigned) route_attribute_len > sizeof(struct sockaddr_in)) {
								throw runtime_error(_("Invalid size on RTA_GATEWAY"));
							}
							struct sockaddr_in *addr = (struct sockaddr_in *) &gateway;
							addr->sin_family = AF_INET;
							memcpy(&addr->sin_addr,RTA_DATA(route_attribute),sizeof(addr->sin_addr));
						}
						break;

					default:
						break;
					}

				}

			}

		} catch(...) {

			::close(sock);
			throw;

		}

		::close(sock);

		bool changed = !equal(gateway,*this);
		if(changed) {
			IP::Address::set(gateway);
			info() << "Default gateway changed to " << to_string() << endl;
		}

		return changed;

	}

 }


 /*
 #include <sys/socket.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <linux/netlink.h>
 #include <linux/rtnetlink.h>
 #include <arpa/inet.h>
 #include <unistd.h>
 #include <net/if.h>
 #include <iostream>
 #include <udjat/tools/intl.h>

 #include <stdexcept>
 #include <system_error>

 #include <udjat/tools/net/gateway.h>

 #define BUFFER_SIZE 4096

 using namespace std;
 namespace Udjat {

	Network::DefaultGateway::DefaultGateway() {
		refresh();
	}

	const Network::DefaultGateway & Network::DefaultGateway::refresh() {

		clear();

		int msgseq = 0;
		int received_bytes;
		struct nlmsghdr *nlh;

		int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

		if(sock < 0) {
			throw std::system_error(errno, std::system_category(), _("Cant get netlink socket"));
		}

		try {

			// 1 Sec Timeout to avoid stall
			struct timeval tv;
			memset(&tv,0,sizeof(tv));
			tv.tv_sec = 1;
			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));

			// Fill in the nlmsg header
			char msgbuf[BUFFER_SIZE];
			memset(msgbuf, 0, sizeof(msgbuf));
			struct nlmsghdr *nlmsg = (struct nlmsghdr *) msgbuf;

			nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
			nlmsg->nlmsg_type = RTM_GETROUTE; 					// Get the routes from kernel routing table .
			nlmsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;	// The message is a request for dump.
			nlmsg->nlmsg_seq = msgseq++;						// Sequence of the message packet.
			nlmsg->nlmsg_pid = getpid();						// PID of process sending the request.

			// Send
			if(send(sock, nlmsg, nlmsg->nlmsg_len, 0) < 0) {
				throw std::system_error(errno, std::system_category(), _("Cant send netlink message"));
			}

			// receive response
			char buffer[BUFFER_SIZE];
			memset(buffer, 0, sizeof(buffer));
			char *ptr = buffer;

			int msg_len = 0;

			do {

				received_bytes = recv(sock, ptr, sizeof(buffer) - msg_len, 0);
				if (received_bytes < 0) {
					throw std::system_error(errno, std::system_category(), _("Cant receive netlink response"));
				}

				nlh = (struct nlmsghdr *) ptr;

				// Check if the header is valid
				if((NLMSG_OK(nlmsg, received_bytes) == 0) || (nlmsg->nlmsg_type == NLMSG_ERROR)) {
					throw runtime_error(_("Error in received packet"));
				}

				// If we received all data break
				if (nlh->nlmsg_type == NLMSG_DONE) {
					break;
				} else {
					ptr += received_bytes;
					msg_len += received_bytes;
				}

				// Break if its not a multi part message
				if ((nlmsg->nlmsg_flags & NLM_F_MULTI) == 0) {
					break;
				}

			} while ((nlmsg->nlmsg_seq != msgseq) || (nlmsg->nlmsg_pid != getpid()));

			// parse response
			struct rtmsg *route_entry;
			struct rtattr *route_attribute;
			int route_attribute_len = 0;
			for ( ; NLMSG_OK(nlh, received_bytes); nlh = NLMSG_NEXT(nlh, received_bytes)) {

				// Get the route data
				route_entry = (struct rtmsg *) NLMSG_DATA(nlh);

				// We are just interested in main routing table
				if (route_entry->rtm_table != RT_TABLE_MAIN) {
					continue;
				}

				route_attribute = (struct rtattr *) RTM_RTA(route_entry);
				route_attribute_len = RTM_PAYLOAD(nlh);

				// Loop through all attributes
				for(;RTA_OK(route_attribute, route_attribute_len); route_attribute = RTA_NEXT(route_attribute, route_attribute_len)) {

					switch(route_attribute->rta_type) {
					case RTA_OIF:
						{
							char interface[IF_NAMESIZE];
							if_indextoname(*(int *)RTA_DATA(route_attribute), interface);
							this->intf = interface;
						}
						break;

					case RTA_GATEWAY:

						if(route_attribute_len > sizeof(struct sockaddr_in)) {
							throw runtime_error(_("Invalid size on RTA_GATEWAY"));
						}

						{
							sockaddr_storage storage;
							memset(&storage,0,sizeof(storage));

							struct sockaddr_in *addr = (struct sockaddr_in *) &storage;
							addr->sin_family = AF_INET;

							memcpy(&addr->sin_addr,RTA_DATA(route_attribute),sizeof(addr->sin_addr));

							*((sockaddr_storage *) this) = storage;
						}
						break;

					default:
						break;
					}

				}

			}

		} catch(...) {

			::close(sock);
			throw;

		}

		::close(sock);

		return *this;
	}

 }
 */
