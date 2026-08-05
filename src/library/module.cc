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

 #define LOG_DOMAIN "network"

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/module.h>
 #include <udjat/module/network.h>
 #include <udjat/agent.h>
 #include <udjat/action.h>
 #include <udjat/net/ip/agent.h>
 #include <udjat/net/nic/agent.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <udjat/net/dns.h>
 #include <memory>

 using namespace std;

 namespace Udjat {

	Udjat::Module * Network::Module::Factory(const char *name) {

		/// @brief Nic agent factor.
		class NicFactory : public Udjat::Abstract::Agent::Factory {
		public:
			NicFactory() : Factory("network-interface") {
				debug("---> Network::Module::NicFactory()");
			}

			std::shared_ptr<Abstract::Agent> AgentFactory(const Properties &props) const override {
				debug("Building Nic::Agent '",props["name"].c_str(),"' from XML");
				return Nic::Agent::Factory(props);
			}

		};

		/// @brief IP based agents factory.
		class HostFactory : public Udjat::Abstract::Agent::Factory, public Udjat::Action::Factory {
		public:
			HostFactory() : Abstract::Agent::Factory("network-host"), Action::Factory("wait-for-host") {
				debug("---> Network::Module::HostFactory()");
			}

			std::shared_ptr<Abstract::Agent> AgentFactory(const Properties &props) const override{
				debug("Building IP::Agent '",props["name"].c_str(),"' from XML");
				return IP::Agent::Factory(props);
			}

			std::shared_ptr<Action> ActionFactory(const Properties &props) const override {

				class WaitForHost : public Action {
				private:
					const char *hostname;
					time_t timeout;
					time_t interval;

				public:
					WaitForHost(const Properties &props) 
					: Action{props}, hostname{props["host"].as_quark()}, timeout{props.get("timeout",120)},
						 interval{props.get("interval",5)} {

						if(!(hostname && *hostname)) {
							throw std::runtime_error("Required attribute 'host' is missing or empty.");
						}

					}
					
					~WaitForHost() override {
					}

					int call(Udjat::Request &request, Udjat::Response &response, bool except) override {

						response["host"] = hostname;

						try {

							int status = DNS::Resolver{}.wait(hostname,timeout,interval);
							response["reachable"] = status;

						} catch(const std::exception &e) {

							response.failed(e);

							if(except) {
								throw;
							}

							return -1;

						}

						return 0;
					}

					bool activate() noexcept override {

						Logger::String{"Waiting for host '",hostname,"'..."}.trace();
					
						DNS::Resolver resolver;

						if(resolver.wait(hostname,timeout,interval) == 0) {
							Logger::String{"Host '",hostname,"' is reachable."}.info();
						} else {
							Logger::String{"Timeout reached waiting for host '",hostname,"'."}.warning();
							return false;
						}

						return true;
					}
				};

				return make_shared<WaitForHost>(props);

			}

		};

		class Module : public Udjat::Module {
		private:
			HostFactory hFactory;
			NicFactory nFactory;

		public:

			Module(const char *name) : Udjat::Module(name,PACKAGE_DESCRIPTION) {
				debug("---> Network::Module::Module()");
				autoclean();
			};

			~Module() override {
			};

		};

		return new Module(name);

	}

	Network::Module::Module(const char *name) : Udjat::Module{name,PACKAGE_DESCRIPTION} {
	}

	Network::Module::~Module() {
	}


 }

