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
 #include <udjat/defs.h>
 #include <udjat/tools/intl.h>
 #include <udjat/net/icmp.h>
 #include <udjat/net/ip/address.h>
 #include <iostream>
 #include <private/module.h>
 #include <udjat/tools/string.h>
 #include <udjat/agent/level.h>

 using namespace std;

 namespace Udjat {

	static const struct icmp_state {
		const char *name;
		const ICMP::Response id;
		const Level level;
		const char *label;
		const char *summary;
		const char *body;
	} icmp_states[] = {
		{
			"invalid",
			ICMP::Response::invalid,
			Level::error,
			N_("Invalid IP"),
			N_("${name} address is invalid"),
			N_("Unable to get a valid IP address for ${name}.")
		},
		{
			"echo-reply",
			ICMP::Response::echo_reply,
			Level::ready,
			N_("Active"),
			N_("${name} is active"),
			N_("Got ICMP echo reply from host.")
		},
		{
			"destination-unreachable",
			ICMP::Response::destination_unreachable,
			Level::error,
			N_("Unreachable"),
			N_("${name} is not reachable"),
			N_("Destination Unreachable. The gateway doesnt know how to get to the defined network.")
		},
		{
			"time-exceeded",
			ICMP::Response::time_exceeded,
			Level::error,
			N_("Timeout"),
			N_("${name} is not acessible"),
			N_("Time Exceeded. The ICMP request has been discarded because it was 'out of time'.")
		},
		{
			"timeout",
			ICMP::Response::timeout,
			Level::error,
			N_("Unavailable"),
			N_("${name} is not available"),
			N_("No ICMP response from host.")
		},
		{
			"network-unreachable",
			ICMP::Response::network_unreachable,
			Level::error,
			N_("Unreachable"),
			N_("Network is not reachable"),
			N_("The entire network is unreachable.")
		},

	};

	std::shared_ptr<ICMP::State> ICMP::State::Factory(const pugi::xml_node &node) {

		class State : public ICMP::State {
		private:
			Udjat::String summary;
			Udjat::String body;

		public:
			State(const pugi::xml_node &node, const icmp_state &state) : ICMP::State{node,state.id} {

				if(!Object::properties.label[0]) {
#ifdef GETTEXT_PACKAGE
					Object::properties.label = dgettext(GETTEXT_PACKAGE,state.label);
#else
					Object::properties.label = state.label;
#endif // GETTEXT_PACKAGE
				}

				if(!properties.body[0]) {
#ifdef GETTEXT_PACKAGE
					body = dgettext(GETTEXT_PACKAGE,state.body);
#else
					body = state.body;
#endif // GETTEXT_PACKAGE
					properties.body = body.expand().c_str();
				}

				if(!Object::properties.summary[0]) {
#ifdef GETTEXT_PACKAGE
					summary = dgettext(GETTEXT_PACKAGE,state.summary);
#else
					summary = state.summary;
#endif // GETTEXT_PACKAGE
					Object::properties.summary = summary.expand().c_str();
				}
			}

		};

		ICMP::Response id{ResponseFactory(node.attribute("icmp-response").as_string())};

		for(const icmp_state &st : icmp_states) {

			if(st.id == id) {
				return make_shared<State>(node,st);
			}

		}

		throw runtime_error("The required attribute 'icmp-response' is missing or invalid");

	}

	std::shared_ptr<ICMP::State> ICMP::State::Factory(const ICMP::Response id) {

		class State : public ICMP::State {
		public:
#ifdef GETTEXT_PACKAGE
			State(const icmp_state &state) : ICMP::State{
				dgettext(GETTEXT_PACKAGE,state.name),
				state.level,
				dgettext(GETTEXT_PACKAGE,state.summary),
				dgettext(GETTEXT_PACKAGE,state.body),
				state.id
			} {
				Object::properties.label = dgettext(GETTEXT_PACKAGE,state.label);
			}
#else
			State(const icmp_state &state) : ICMP::State{
				state.name,
				state.level,
				state.summary,
				state.body,
				state.id
			} {
				Object::properties.label = state.label;
			}
#endif // GETTEXT_PACKAGE

		};

		for(const icmp_state &st : icmp_states) {

			if(st.id == id) {
				return make_shared<State>(st);
			}

		}

		throw runtime_error("The required attribute 'icmp-response' is missing or invalid");

	}


 }


