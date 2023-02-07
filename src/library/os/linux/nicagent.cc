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
 #include <private/linux/netlink.h>
 #include <udjat/tools/threadpool.h>
 #include <sys/ioctl.h>
 #include <unistd.h>

 /*
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
 */

 using namespace std;

 namespace Udjat {

	/*
	static const struct {
		unsigned int flag;
		const char *name;
	} flagnames[] = {
		{ IFF_UP,			"IFF_UP"			},
		{ IFF_BROADCAST,	"IFF_BROADCAST"		},
		{ IFF_DEBUG,		"IFF_DEBUG"			},
		{ IFF_LOOPBACK,		"IFF_LOOPBACK"		},
		{ IFF_POINTOPOINT,	"IFF_POINTOPOINT"	},
		{ IFF_RUNNING,		"IFF_RUNNING"		},
		{ IFF_NOARP,		"IFF_NOARP"			},
		{ IFF_PROMISC,		"IFF_PROMISC"		},
		{ IFF_NOTRAILERS,	"IFF_NOTRAILERS"	},
		{ IFF_ALLMULTI,		"IFF_ALLMULTI"		},
		{ IFF_MASTER,		"IFF_MASTER"		},
		{ IFF_SLAVE,		"IFF_SLAVE"			},
		{ IFF_MULTICAST,	"IFF_MULTICAST"		},
		{ IFF_PORTSEL,		"IFF_PORTSEL"		},
		{ IFF_AUTOMEDIA,	"IFF_AUTOMEDIA"		},
		{ IFF_DYNAMIC,		"IFF_DYNAMIC"		},
//						{ IFF_LOWER_UP,		"IFF_LOWER_UP"		},
//						{ IFF_DORMANT,		"IFF_DORMANT"		},
//						{ IFF_ECHO,			"IFF_ECHO"			},
	};
	*/

	mutex Nic::Agent::guard;
	std::list <Nic::Agent *> Nic::Agent::agents;

	unsigned int getFlags(const char *name) {

		int sock = socket(PF_INET, SOCK_DGRAM, 0);
		if(sock < 0) {
			cerr << name << "\tUnable to create socket: " << strerror(errno) << endl;
			return 0;
		}

		struct ifreq ifr;
		memset(&ifr,0,sizeof(ifr));
		strncpy(ifr.ifr_name,name,sizeof(ifr.ifr_name));

		if (ioctl(sock, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {

			if(errno == ENODEV) {
				clog << name << "\t" << strerror(errno) << endl;
 			} else {
 				cerr << name << "\t" << strerror(errno) << endl;
 			}
			ifr.ifr_flags = 0;

		}

		::close(sock);

		return ifr.ifr_flags;
	}

	Nic::Agent::Agent(const char *name) : Abstract::Agent{name}, std::string{name} {
		intf.flags = getFlags(std::string::c_str());
	}

	Nic::Agent::Agent(const pugi::xml_node &node) : Abstract::Agent{node}, std::string{node.attribute("device-name").as_string()} {
		intf.flags = getFlags(std::string::c_str());
	}

	Nic::Agent::~Agent() {
		stop(); // Just in case.
	}

	void Nic::Agent::start() {

		intf.index = if_nametoindex(std::string::c_str());

		{
			lock_guard<mutex> lock(guard);
			agents.push_back(this);
		}

		// Interface was added.
		NetLink::Controller::getInstance().push_back(this,RTM_NEWLINK,[this](const void *m){

			const ifinfomsg *ifi = (const ifinfomsg *) m;

			char name[IF_NAMESIZE+1];
			memset(name,0,IF_NAMESIZE+1);
			if(!if_indextoname(ifi->ifi_index,name)) {
				cerr << "nic\tUnable to get name of interface " << ifi->ifi_index << ": " << strerror(errno) << endl;
				return;
			}

			lock_guard<mutex> lock(guard);
			for(Agent *agent : agents) {

				if(strcmp(agent->std::string::c_str(),name)) {
					continue;
				}

				Logger::String{"Interface link was crated (RTM_NEWLINK)"}.trace(agent->name());

				agent->intf.index = ifi->ifi_index;

				if(agent->intf.flags != ifi->ifi_flags) {
					agent->intf.flags = ifi->ifi_flags;
					agent->push([this](std::shared_ptr<Abstract::Agent>){
						this->updated(true);
					});
				}

			}

		});

		// Interface was removed.
		NetLink::Controller::getInstance().push_back(this,RTM_DELLINK,[this](const void *m) {

			// Link was removed
			const ifinfomsg *ifi = (const ifinfomsg *) m;

			lock_guard<mutex> lock(guard);
			for(Agent *agent : agents) {
				if(agent->intf.index == ifi->ifi_index && agent->intf.flags != ifi->ifi_flags) {

					Logger::String{"Interface link was removed (RTM_DELLINK)"}.trace(agent->name());

					agent->intf.flags = ifi->ifi_flags;
					agent->push([this](std::shared_ptr<Abstract::Agent>){
						this->updated(true);
					});

				}
			}

		});
	}

	void Nic::Agent::stop() {

		{
			lock_guard<mutex> lock(guard);
			agents.remove_if([this](Agent *agent) {
				return agent == this;
			});
		}

		NetLink::Controller::getInstance().remove(this);
	}


 }
