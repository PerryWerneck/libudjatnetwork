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
 #include <private/module.h>
 #include <udjat/network/agents/host.h>
 #include <controller.h>
 #include <udjat/tools/xml.h>
 #include <udjat/network/resolver.h>
 #include <cstring>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <ifaddrs.h>
 #include <udjat/tools/intl.h>
 #include <sstream>
 #include <iomanip>

 namespace Udjat {

	Network::HostAgent::HostAgent(const pugi::xml_node &node) : Abstract::Agent(node) {

		// Do an ICMP check?
		icmp.check = getAttribute(node,"icmp",icmp.check);
		icmp.timeout = getAttribute(node,"icmp-timeout", (unsigned int) icmp.timeout);

		states.icmp = states.addr = Abstract::Agent::computeState();

	}

	Network::HostAgent::~HostAgent() {
	}

	std::shared_ptr<Abstract::State> Network::HostAgent::StateFactory(const pugi::xml_node &node) {

		/// @brief IP Address range state.
		class Range : public Network::Range {
		private:
			sockaddr_storage addr;
			uint16_t mask;

		public:
			Range(const pugi::xml_node &node) : Network::Range(node) {
				memset(&addr,0,sizeof(addr));
				parse(Attribute(node,"range").as_string(),addr,mask);
			}

		protected:
			bool test(const sockaddr_storage &ip) const override {

				// TODO: Implement IPV6 methods.

				if(addr.ss_family == AF_INET) {
					sockaddr_in netmask;
					return inRange(*((const sockaddr_in *) &ip), *((const sockaddr_in *) &this->addr), getMask(netmask,this->mask));
				} else {
					throw runtime_error("Unsupported address family");
				}

				return false;
			}

		};

		/// @brief Local network state
		class SameNetwork : public Network::Range {
		public:
			SameNetwork(const pugi::xml_node &node) : Network::Range(node) {
				revert = !Attribute(node,"same-network").as_bool(true);
			}

		protected:
			bool test(const sockaddr_storage &ip) const override {

				bool rc = false;

				struct ifaddrs *ifaces;

				if(getifaddrs(&ifaces)) {
					throw std::system_error(errno, std::system_category(), "Cant get network interface list");
				}

				try {

					for(ifaddrs *iface = ifaces; iface; iface = iface->ifa_next) {

						if(!iface->ifa_addr || iface->ifa_addr->sa_family != AF_INET) {
							continue;
						}

						if(inRange(ip,*iface->ifa_addr,*iface->ifa_netmask)) {
#ifdef DEBUG
							cout << "Found " << std::to_string(ip) << " in " << iface->ifa_name << endl;
#endif // DEBUG
							rc = true;
							break;

						}
					}

				} catch(...) {

					freeifaddrs(ifaces);
					throw;
				}

				freeifaddrs(ifaces);

				return rc;
			}
		};

		if(node.attribute("range")) {

			auto state = make_shared<Range>(node);
			states.available.push_back(state);
			return state;

		}

		if(node.attribute("same-network")) {

			auto state = make_shared<SameNetwork>(node);
			states.available.push_back(state);
			return state;

		}

		if(node.attribute("icmp-response")) {

			if(!icmp.check) {
				throw runtime_error("Can't use 'icmp-response' states without icmp='true' attribute on the agent definition");
			}

			ICMPResponse id = ICMPResponseFactory(node.attribute("icmp-response").as_string("undefined"));

			auto state = make_shared<ICMPResponseState>(node,id);
			states.available.push_back(state);
			return state;

		}

		return super::StateFactory(node);
	}

	void Network::HostAgent::set(const sockaddr_storage &addr) {

		// Check states.
		std::shared_ptr<State> detected;
		for(auto state : states.available) {

			State * st = dynamic_cast<State *>(state.get());

			if(st && st->isValid(addr)) {
				detected = state;
				break;
			}

		}

		if(detected) {
			states.addr = detected;
		} else {
			states.icmp = Abstract::Agent::computeState();
		}

		if(!icmp.check) {
			super::set(states.addr);
			return;
		}

		if(!states.icmp || states.addr->level() > states.icmp->level()) {
			super::set(states.addr);
		}

		if(addr.ss_family) {
			Controller::getInstance().insert(this,addr);
		} else {
			set(ICMPResponse::invalid);
		}

	}

	sockaddr_storage Network::HostAgent::resolv(sockaddr_storage &dnssrv, const char *hostname) {

		sockaddr_storage addr;

#ifdef DEBUG
		cout << "Checking " << hostname << endl;
#endif // DEBUG

		// Check DNS resolution.
		DNSResolver resolver{dnssrv};

		resolver.query(hostname);

		if(!resolver.size()) {
			throw runtime_error(string{"Can't resolve '"} + hostname + "'");
		}

		addr = resolver.begin()->getAddr();

		return addr;
	}

	std::string Udjat::Network::HostAgent::to_string() const noexcept {

		if(!icmp.time) {
			return "";
		}

		float value = ((float) icmp.time) / ((float) 1000000);

		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << value << " ms";

		return stream.str();

	}

	Udjat::Value & Udjat::Network::HostAgent::get(Udjat::Value &value) const {

		if(icmp.time) {
			value.set(((float) icmp.time) / ((float) 1000000));
		}

		return value;
	}

	Udjat::Value & Udjat::Network::HostAgent::getProperties(Udjat::Value &response) const noexcept {

		super::getProperties(response);

		if(states.addr) {
			auto &state = response["host"];
			states.addr->getProperties(state);
		}

		if(states.icmp) {
			auto &state = response["icmp"];
			states.icmp->getProperties(state);
			state["time"] = to_string();
		}

		return response;
	}


 }
