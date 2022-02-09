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

 #include "private.h"
 #include <udjat/network/resolver.h>

 namespace Udjat {

	enum AgentType : uint8_t {
		standard_host,
		default_gateway,
	};

 	Network::Agent::Factory::Factory() : Udjat::Factory("network-host",moduleinfo) {
	}

	bool Network::Agent::Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {

		/// @brief Standard agent.
		class StandardAgent : public Network::Agent {
		private:
			/// @brief Host to check.
			const char * hostname = nullptr;

			/// @brief DNS Addr if check.dns is true or host addr if check.dns is false.
			sockaddr_storage addr;

			struct {
				bool check = true;	///< @brief Check DNS resolution.
			} dns;

		public:
			StandardAgent(const pugi::xml_node &node) : Network::Agent(node) {

				memset(&addr,0,sizeof(addr));

				// Get dns-server.
				const char *dnssrv = Udjat::Attribute(node, "dns-server").as_string();
				dns.check = Udjat::Attribute(node,"dns").as_bool(dnssrv[0] != 0);

				// Host name to check.
				hostname = Udjat::Attribute(node,"host").c_str();

				if(dns.check) {

					// Will check DNS resolution, get the DNS server addr.
					if(dnssrv[0]) {

						// Resolve DNS server.
						DNSResolver resolver;
						resolver.query(dnssrv);

						if(resolver.size()) {
							this->addr = resolver.begin()->getAddr();
						} else {
							throw runtime_error(string{"Can't resolve '"} + dnssrv + "'");
						}

					}

				} else {

					// Will not check DNS resolution, get host address.

					DNSResolver resolver;
					resolver.query(hostname);

					if(resolver.size()) {
						this->addr = resolver.begin()->getAddr();
					} else {
						throw runtime_error(string{"Can't resolve '"} + hostname + "'");
					}

				}

				checkStates();

			}

			bool refresh() override {

				// Start with a clean state.
				selected.reset();

				if(dns.check) {
					set(resolv(this->addr,hostname));
				} else {
					set(this->addr);
				}

				//
				// Set current state.
				//
				if(selected) {
					activate(selected);
				} else {
					activate(super::stateFromValue());
				}

				return true;

			}

		};

		/// @brief Gateway Agent
		class GatewayAgent : public Network::Agent {
		public:
			GatewayAgent(const pugi::xml_node &node) : Network::Agent(node) {

				if(!icmp.check) {
					throw runtime_error("Gateway agent requires icmp='true'");
				}

				checkStates();

			}

			bool refresh() override {

				// Start with a clean state.
				selected.reset();
				set(DefaultGateway().refresh());
				return true;
			}

		};

		switch(Attribute(node,"type").select("default","default-gateway",nullptr)) {
		case standard_host:
			parent.insert(make_shared<StandardAgent>(node));
			break;

		case default_gateway:
			parent.insert(make_shared<GatewayAgent>(node));
			break;

		default:
			return false;
		}

		return true;

	}

 }
