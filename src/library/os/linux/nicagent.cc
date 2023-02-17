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

 using namespace std;

 namespace Udjat {

	static void logflags(const char *name, unsigned int flags) {
		Logger::String text{"Flags:"};

		static const struct {
			unsigned int flag;
			const char *name;
		} names[] = {
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

		for(size_t ix = 0; ix < N_ELEMENTS(names); ix++) {

			if(!(flags & names[ix].flag)) {
				continue;
			}

			text += " ";
			text += names[ix].name;

		}

		text.write(Logger::Trace,name);
	}

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
		intf.exist = (intf.flags != 0);
	}

	Nic::Agent::Agent(const pugi::xml_node &node) : Abstract::Agent{node}, std::string{node.attribute("device-name").as_string()} {
		intf.flags = getFlags(std::string::c_str());
		intf.exist = (intf.flags != 0);
	}

	Nic::Agent::~Agent() {
		stop(); // Just in case.
	}

	void Nic::Agent::start() {

		intf.index = if_nametoindex(std::string::c_str());

		// Interface was added.
		NetLink::Controller::getInstance().push_back(this,RTM_NEWLINK,[this](const void *m){

			const ifinfomsg *ifi = (const ifinfomsg *) m;

			char name[IF_NAMESIZE+1];
			memset(name,0,IF_NAMESIZE+1);
			if(!if_indextoname(ifi->ifi_index,name)) {
				cerr << "nic\tUnable to get name of interface " << ifi->ifi_index << ": " << strerror(errno) << endl;
				return;
			}

			debug("RTM_NEWLINK on '",this->name(),"'");

			if(strcmp(this->std::string::c_str(),name)) {
				debug("'",this->std::string::c_str(),"' ignoring event from '",name,"'");
				return;
			}

			this->intf.index = ifi->ifi_index;

			Logger::String{"'RTM_NEWLINK' on interface ",ifi->ifi_index," with index ",this->intf.index}.trace(this->name());

			if(this->intf.flags != ifi->ifi_flags) {

				if(Logger::enabled(Logger::Trace)) {
					logflags(this->name(),ifi->ifi_flags);
				}

				this->intf.flags = ifi->ifi_flags;
				this->intf.exist = true;
				this->push([this](std::shared_ptr<Abstract::Agent>){
					this->updated(true);
				});

			}

		});

		// Interface was removed.
		NetLink::Controller::getInstance().push_back(this,RTM_DELLINK,[this](const void *m) {

			const ifinfomsg *ifi = (const ifinfomsg *) m;

			// Link was removed
			if(this->intf.index != ifi->ifi_index) {
				debug("Ignoring RTM_DELLINK to index '",ifi->ifi_index,"' mine is '",this->intf.index,"'");
				return;
			}

			Logger::String{"'RTM_DELLINK' on interface ",ifi->ifi_index," with index ",this->intf.index}.trace(this->name());

			if(this->intf.flags != ifi->ifi_flags) {

				if(Logger::enabled(Logger::Trace)) {
					logflags(this->name(),ifi->ifi_flags);
				}

				this->intf.flags = ifi->ifi_flags;
				this->intf.exist = false;
				this->push([this](std::shared_ptr<Abstract::Agent>){
					this->updated(true);
				});
			}


		});
	}

	void Nic::Agent::stop() {
		NetLink::Controller::getInstance().remove(this);
	}

 }
