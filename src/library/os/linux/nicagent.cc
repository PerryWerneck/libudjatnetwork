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
 #include <udjat/net/nic/agent.h>
 #include <udjat/linux/network.h>
 #include <udjat/tools/logger.h>
 #include <udjat/net/ip/address.h>
 #include <string>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/handler.h>
 #include <list>

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

	class Nic::Agent::Controller : private MainLoop::Handler {
	private:
		static mutex guard;

		/*

			struct ifinfomsg {
                  unsigned char  ifi_family; AF_UNSPEC
                  unsigned short ifi_type;   Device type
                  int            ifi_index;  Interface index
                  unsigned int   ifi_flags;  Device flags
                  unsigned int   ifi_change; change mask
              };

		*/

		list<Nic::Agent *> nics;

		Nic::Agent * get_agent(int index) {

			char name[IF_NAMESIZE+1];
			memset(name,0,IF_NAMESIZE+1);
			if(if_indextoname(index,name)) {

				debug(name);
				for(auto nic : nics) {
					if(!strcasecmp(nic->std::string::c_str(),name)) {
						nic->intf.index = index;
						return nic;
					}
				}

			} else {

				debug("Cant get name for ",index);
				for(auto nic : nics) {
					if(nic->intf.index == index) {
						return nic;
					}
				}

			}

			return nullptr;

		}

		void refresh(struct ifinfomsg *ifi, bool enabled) {

			lock_guard<mutex> lock(guard);
			debug(__FUNCTION__,"(",ifi->ifi_index,"): ",ifi->ifi_flags);
			Nic::Agent *agent = get_agent(ifi->ifi_index);

			if(agent) {
				bool changed = false;
				if(agent->intf.flags != ifi->ifi_flags) {
					changed = true;
					agent->intf.flags = ifi->ifi_flags;
				}
				if(agent->intf.enabled != enabled) {
					changed = true;
					agent->intf.enabled = true;
				}

				if(changed) {
					agent->on_rtlink_event();
				}

			}

		}

		void new_link(struct ifinfomsg *ifi) {
			refresh(ifi,true);
		}

		void del_link(struct ifinfomsg *ifi) {
			refresh(ifi,false);
		}

		void handle_event(const Event) override {

			char buf[4096];
			struct iovec iov = { buf, sizeof buf };
			struct sockaddr_nl snl;
			struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
			struct nlmsghdr *h;
			//struct ifinfomsg *ifi = nullptr;

			int status = recvmsg(this->fd, &msg, 0);
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

				switch(h->nlmsg_type) {
				case RTM_NEWLINK:
					debug("RTM_NEWLINK");
					new_link((ifinfomsg *) NLMSG_DATA(h));
					break;

				case RTM_DELLINK:
					debug("RTM_DELLINK");
					del_link((ifinfomsg *) NLMSG_DATA(h));
					break;

				}

			}

		}

	public:
		static Controller & getInstance() {
			static Controller instance;
			return instance;
		}

		void push_back(Nic::Agent *agent) {

			lock_guard<mutex> lock(guard);
			nics.push_back(agent);

			if(this->fd != -1) {
				return;
			}

			// Start watcher.
			Logger::String{"Starting NIC watcher"}.trace("network");

			this->fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
			if(this->fd < 0) {
				throw system_error(errno,std::system_category(),"Cant get netlink socket");
			}

			struct sockaddr_nl addr;
			memset ((void *) &addr, 0, sizeof (addr));
			addr.nl_family = AF_NETLINK;
			addr.nl_pid = getpid ();
			addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;

			if(bind(this->fd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
				throw system_error(errno,std::system_category(),"Cant bind netlink socket");
			}

			Handler::set(oninput);
			Handler::enable();

		}

		void remove(Nic::Agent *nic) {
			lock_guard<mutex> lock(guard);
			nics.remove_if([nic](Nic::Agent *n){
				return n == nic;
			});

			if(!nics.empty()) {
				return;
			}

			Logger::String{"Stopping NIC watcher"}.trace("network");
			Handler::disable();
			::close(this->fd);
			this->fd = -1;

		}
	};

	mutex Nic::Agent::Controller::guard;

	Nic::Agent::Agent(const char *name) : Abstract::Agent{name}, std::string{name} {
		debug("-----------------------------> ",std::string::c_str());
	}

	Nic::Agent::Agent(const pugi::xml_node &node) : Abstract::Agent{node}, std::string{node.attribute("device-name").as_string()} {
		debug("-----------------------------> ",std::string::c_str());
	}

	void Nic::Agent::on_rtlink_event() {
		push([this](std::shared_ptr<Abstract::Agent> agent){
			Abstract::Agent::updated(true);
		});
	}

	void Nic::Agent::start() {
		Controller::getInstance().push_back(this);
	}

	void Nic::Agent::stop() {
		Controller::getInstance().remove(this);
	}


 }
