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

 #include <private/module.h>
 #include <udjat/tools/net/dns.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/method.h>
 #include <udjat/tools/net/gateway.h>

 namespace Udjat {

	enum AgentType : uint8_t {
		standard_host,
		default_gateway,
	};

 	Network::HostAgent::Factory::Factory() : Udjat::Factory("network-host",moduleinfo) {
	}

	std::shared_ptr<Abstract::Agent> Network::HostAgent::Factory::AgentFactory(const Abstract::Object &parent, const pugi::xml_node &node) const {

		/// @brief Standard agent.
		class StandardAgent : public Network::HostAgent {
		private:
			/// @brief Host to check.
			const char * hostname = nullptr;

			/// @brief DNS Addr if check.dns is true or host addr if check.dns is false.
			sockaddr_storage addr;

		public:
			StandardAgent(const pugi::xml_node &node) : Network::HostAgent(node) {

				memset(&addr,0,sizeof(addr));

				auto ipaddr = getAttribute(node,"ip");
				if(ipaddr) {

					// Have an IP addr.
					check.dns = false;

					if(inet_pton(AF_INET,ipaddr.as_string(),&((sockaddr_in *) &addr)->sin_addr) != 0) {
						addr.ss_family = AF_INET;
						debug(ipaddr.as_string()," is an IPV4 address");
					} else if(inet_pton(AF_INET6,ipaddr.as_string(),&((sockaddr_in6 *) &addr)->sin6_addr) != 0) {
						addr.ss_family = AF_INET6;
						debug(ipaddr.as_string()," is an IPV6 address");
					} else {
						throw std::system_error(errno, std::system_category(), ipaddr.as_string());
					}

				} else {

					// Get dns-server.
					const char *dnssrv = getAttribute(node, "dns-server").as_string();
					check.dns = getAttribute(node,"network-host","dns",dnssrv[0] != 0);

					// Host name to check.
					hostname = getAttribute(node,"host","");
					if(!*hostname) {
						throw runtime_error("Missing required attribute 'host'");
					}

					if(check.dns) {

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

				}

			}

			bool refresh() override {

				if(check.dns) {
					set(resolv(this->addr,hostname));
				} else {
					set(this->addr);
				}

				return true;

			}

			bool getProperty(const char *key, std::string &value) const noexcept override {

				if(!(strcasecmp(key,"host") && strcasecmp(key,"hostname")) ) {
					value = hostname;
					return true;
				}

				return Network::HostAgent::getProperty(key,value);
			}

		};

		/// @brief Gateway Agent
		class GatewayAgent : public Network::HostAgent {
		public:
			GatewayAgent(const pugi::xml_node &node) : Network::HostAgent(node) {

				if(!check.icmp) {
					throw runtime_error("Gateway agent requires icmp='true'");
				}

			}

			bool refresh() override {

				sockaddr_storage addr = Network::DefaultGateway().refresh();

				set(addr);

				return true;

			}

		};


		class URLAgent : public Udjat::Agent<int> {
		private:
			const char *url;
			HTTP::Method method;

		public:
			URLAgent(const pugi::xml_node &node) : super(node,0), url(Quark(node,"url").c_str()), method(HTTP::MethodFactory(node.attribute("http-method").as_string("head"))) {

				if(!url[0]) {
					throw runtime_error("Required attribute 'url' is missing");
				}


			}

			bool refresh() override {
				int value = Udjat::URL{this->url}.test(method);
				return set(value);
			}

		};

		String type{node.attribute("type").as_string("default")};

		switch(type.select("default","default-gateway",nullptr)) {
		case standard_host:
			if(node.attribute("url")) {
				return make_shared<URLAgent>(node);
			}
			return make_shared<StandardAgent>(node);

		case default_gateway:
			return make_shared<GatewayAgent>(node);

		default:
			Factory::error() << "Unexpected node type '" << type.c_str() << "'" << endl;
		}

		return Udjat::Factory::AgentFactory(parent,node);

	}

 }
