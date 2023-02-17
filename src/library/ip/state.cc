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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/object.h>
 #include <pugixml.hpp>
 #include <udjat/net/ip/address.h>
 #include <udjat/net/ip/state.h>
 #include <udjat/net/interface.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Abstract::IP::State> IP::State::Factory(const pugi::xml_node &node) {

		const struct {
			const char *name;
			bool revert;
		} locals[] = {
			{ "local", 			false 	},
			{ "!remote", 		false	},
			{ "not remote", 	false	},
			{ "internal", 		false	},
			{ "mine", 			false	},
			{ "remote", 		true	},
			{ "!local", 		true	},
			{ "not local", 		true	},
			{ "external", 		true	},
			{ "!mine", 			true	},
			{ "not mine", 		true	},

		};

		const char *subnet = Object::getAttribute(node,"subnet").as_string();

		class Internal : public Abstract::IP::State, public IP::SubNet {
		private:
			bool revert;

		protected:

			/// @brief Test an IPV4 address range.
			bool compare(const sockaddr_in &addr) const override {
				bool found = SubNet::for_each([this,&addr](const SubNet &subnet){
					if(subnet.contains(addr)) {
						*((SubNet *)this) = subnet;
						return true;
					}
					return false;
				});
				return (revert ? !found : found);
			}

			/// @brief Test an IPV6 address range.
			bool compare(const sockaddr_in6 &addr) const override {
				bool found = SubNet::for_each([this,&addr](const SubNet &subnet){
					if(subnet.contains(addr)) {
						*((SubNet *)this) = subnet;
						return true;
					}
					return false;
				});
				return (revert ? !found : found);
			}

		public:
			Internal(const pugi::xml_node &node, bool r) : Abstract::IP::State{node}, revert{r} {
			}

			bool empty() const noexcept override {
				return IP::SubNet::empty();
			}

			std::string to_string() const noexcept override {
				if(Object::properties.summary[0]) {
					return Object::properties.summary;
				}
				return IP::SubNet::to_string();
			}

		};

		for(size_t ix = 0; ix < N_ELEMENTS(locals);ix++) {
			if(!strcasecmp(subnet,locals[ix].name)) {
				return make_shared<Internal>(node,locals[ix].revert);
			}
		}

		// Create default state.
		return make_shared<IP::State>(node);
	}

	IP::State::State(const char *subnet) : Abstract::IP::State{subnet}, IP::SubNet{subnet} {
	}

	IP::State::State(const pugi::xml_node &node) : Abstract::IP::State{node}, IP::SubNet{node} {
	}

	std::string IP::State::to_string() const noexcept {

		if(Object::properties.summary[0]) {
			return Object::properties.summary;
		}

		return IP::SubNet::to_string();
	}

	bool IP::State::empty() const noexcept {
		return IP::SubNet::empty();
	}

	/// @brief Test an IPV4 address range.
	bool IP::State::compare(const sockaddr_in &addr) const {
		return IP::SubNet::contains(addr);
	}

	/// @brief Test an IPV6 address range.
	bool IP::State::compare(const sockaddr_in6 &addr) const {
		return IP::SubNet::contains(addr);
	}

	bool Abstract::IP::State::compare(const sockaddr_storage &addr) const {

		switch(addr.ss_family) {
		case AF_INET:
			return compare(*((const sockaddr_in *) &addr));

		case AF_INET6:
			return compare(*((const sockaddr_in6 *) &addr));

		case 0:
			return empty();

		default:
			throw runtime_error("Invalid address family");
		}


	}

 }
