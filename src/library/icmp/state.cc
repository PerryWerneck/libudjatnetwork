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
 #include <udjat/tools/net/ip.h>
 #include <private/agents/host.h>
 #include <iostream>
 #include <private/module.h>
 #include <udjat/tools/string.h>
 #include <udjat/agent/level.h>

 using namespace std;

 namespace Udjat {

	/*
	static const struct {
		const char *name;
		const ICMP::Response id;
		const Level level;
		const char *summary;
		const char *body;
	} icmp_states[] = {
		{
			"invalid",
			ICMP::Response::invalid,
			Level::error,
			N_("${name} address is invalid"),
			N_("Unable to get a valid IP address for ${name}.")
		},
		{
			"echo-reply",
			ICMP::Response::echo_reply,
			Level::ready,
			N_("${name} is active"),
			N_("Got ICMP echo reply from host.")
		},
		{
			"destination-unreachable",
			ICMP::Response::destination_unreachable,
			Level::error,
			N_("${name} is not reachable"),
			N_("Destination Unreachable. The gateway doesnt know how to get to the defined network.")
		},
		{
			"time-exceeded",
			ICMP::Response::time_exceeded,
			Level::error,
			N_("${name} is not acessible"),
			N_("Time Exceeded. The ICMP request has been discarded because it was 'out of time'.")
		},
		{
			"timeout",
			ICMP::Response::timeout,
			Level::error,
			N_("${name} is not available"),
			N_("No ICMP response from host.")
		},
		{
			"network-unreachable",
			ICMP::Response::network_unreachable,
			Level::error,
			N_("Network is not reachable"),
			N_("The entire network is unreachable.")
		},

	};

	void Udjat::Network::HostAgent::set(const ICMP::Response response, const sockaddr_storage &from) {

		trace() << "Got response '" << response << "' from " << from << endl;

		std::shared_ptr<State> detected;

		for(auto state : states.available) {

			State * st = dynamic_cast<State *>(state.get());

			if(st && st->isValid(response)) {
				detected = state;
				break;
			}

		}

		if(!detected) {

			// Scan for predefined states.
			for(size_t ix = 0; ix < N_ELEMENTS(icmp_states); ix++) {

				if(icmp_states[ix].id == response) {

					trace() << "Adding default state for '" << icmp_states[ix].name << "'" << endl;

#ifdef GETTEXT_PACKAGE
					Udjat::String summary{dgettext(GETTEXT_PACKAGE,icmp_states[ix].summary)};
					Udjat::String body{dgettext(GETTEXT_PACKAGE,icmp_states[ix].body)};
#else
					Udjat::String summary{icmp_states[ix].summary};
					Udjat::String summary{icmp_states[ix].body};
#endif // GETTEXT_PACKAGE

					summary.expand(*this);
					body.expand(*this);

					auto state =
						make_shared<ICMPResponseState>(
								icmp_states[ix].name,
								icmp_states[ix].level,
								Quark{summary}.c_str(),
								Quark{body}.c_str(),
								icmp_states[ix].id
							);

					states.available.push_back(state);
					detected = state;
					break;

				}

			}

		}

		if(detected) {
			states.icmp = detected;
		} else {
			states.icmp = Abstract::Agent::computeState();
		}

		if(!states.addr || states.icmp->level() > states.addr->level()) {
			super::set(states.icmp);
		}

	}
	*/

 }


