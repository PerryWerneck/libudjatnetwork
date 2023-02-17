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
 #include <udjat/net/nic/state.h>
 #include <udjat/net/nic/agent.h>

 #include <net/if.h>

 using namespace std;

 namespace Udjat {

	static const struct {
		unsigned int	  flag;
		const char		* active;
		const char 		* inactive;
	} flagstates[] = {
		{ IFF_UP,				"up",			"down"			},
		{ IFF_RUNNING,			"running",		"not-running"	},
		{ IFF_PROMISC,			"promisc",		"not-promisc"	},
		{ IFF_POINTOPOINT,		"p2p",			"not-p2p"		},
		{ IFF_NOARP,			"no-arp",		"arp"			},
		{ IFF_MULTICAST,		"multicast",	"no-multicast"	},
	};

	std::shared_ptr<Nic::State> Nic::State::Factory(const pugi::xml_node &node) {

		/// @brief Test if flag is 'on'
		class StateActive : public Nic::State {
		private:
			const unsigned int flags;

		public:
			StateActive(unsigned int f, const pugi::xml_node &node) : Nic::State{node}, flags{f} {
			}

			bool compare(const Nic::Agent &agent) override {
				unsigned int agflags = agent.flags();
				return agent.exist() && (agflags && (agflags & this->flags));
			}

		};

		/// @brief Test if flag is 'off'
		class StateInactive : public Nic::State {
		private:
			const unsigned int flags;

		public:
			StateInactive(unsigned int f, const pugi::xml_node &node) : Nic::State{node}, flags{f} {
			}

			bool compare(const Nic::Agent &agent) override {
				unsigned int agflags = agent.flags();
				return agent.exist() && (agflags && !(agflags & this->flags));
			}

		};

		const char * state = node.attribute("device-state").as_string(node.attribute("name").as_string());
		if(!state[0]) {
			throw runtime_error("The required attribute 'device-state' is missing");
		}

		for(size_t ix = 0; ix < N_ELEMENTS(flagstates); ix++) {

			if(!strcasecmp(state,flagstates[ix].active)) {
				return make_shared<StateActive>(flagstates[ix].flag,node);
			}

			if(!strcasecmp(state,flagstates[ix].inactive)) {
				return make_shared<StateInactive>(flagstates[ix].flag,node);
			}

		}

		// @brief Test if interface exists.
		class StateExistant : public Nic::State {
		private:
			bool revert;

		public:
			StateExistant(const pugi::xml_node &node, bool r) : Nic::State{node}, revert{r} {
			}

			bool compare(const Nic::Agent &agent) override {
				return (revert ? !agent.intf.exist : agent.intf.exist);
			}

		};

		if(!strcasecmp(state,"found")) {
			return make_shared<StateExistant>(node,false);
		}

		if(!strcasecmp(state,"not-found")) {
			return make_shared<StateExistant>(node,true);
		}


		throw runtime_error("Required attribute 'device-state' is missing or invalid");

	}

 }
