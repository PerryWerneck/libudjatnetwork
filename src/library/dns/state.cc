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
 #include <udjat/net/dns.h>
 #include <udjat/net/ip/address.h>
 #include <iostream>
 #include <private/module.h>
 #include <udjat/tools/string.h>
 #include <udjat/agent/level.h>

 using namespace std;

 namespace Udjat {

	static const struct dns_state {
		const char *name;
		const DNS::Response id;
		const Level level;
		const char *label;
		const char *summary;
		const char *body;
	} dns_states[] = {
		{
			"invalid",
			DNS::invalid,
			Level::critical,
			N_("Invalid DNS"),
			N_("Invalid DNS response"),
			N_("The DNS resolver state is invalid.")
		},
		{
			"no-server",
			DNS::cant_resolve_server_address,
			Level::critical,
			N_("No server"),
			N_("Cant resolve DNS Server"),
			N_("Unable to resolve the name of the defined DNS server.")
		},
		{
			"no-name",
			DNS::cant_resolve_address,
			Level::error,
			N_("cant-resolve"),
			N_("Cant resolve name"),
			N_("The DNS doesnt known the defined hostname.")
		},
		{
			"resolved",
			DNS::dns_ok,
			Level::ready,
			N_("resolved"),
			N_("The hostname was resolved"),
			N_("The DNS was able to resolve hostname.")
		},

	};

	std::shared_ptr<DNS::State> DNS::State::Factory(const pugi::xml_node &node) {

		class State : public DNS::State {
		private:
			Udjat::String summary;
			Udjat::String body;

		public:
			State(const pugi::xml_node &node, const dns_state &state) : DNS::State{node,state.id} {

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

		DNS::Response id{ResponseFactory(node.attribute("dns-response").as_string())};

		for(const dns_state &st : dns_states) {

			if(st.id == id) {
				return make_shared<State>(node,st);
			}

		}

		throw runtime_error("The required attribute 'dns-response' is missing or invalid");

	}

	std::shared_ptr<DNS::State> DNS::State::Factory(const DNS::Response id) {

		class State : public DNS::State {
		public:
#ifdef GETTEXT_PACKAGE
			State(const dns_state &state) : DNS::State{
				dgettext(GETTEXT_PACKAGE,state.name),
				state.level,
				dgettext(GETTEXT_PACKAGE,state.summary),
				dgettext(GETTEXT_PACKAGE,state.body),
				state.id
			} {
				Object::properties.label = dgettext(GETTEXT_PACKAGE,state.label);
			}
#else
			State(const icmp_state &state) : DNS::State{
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

		for(const dns_state &st : dns_states) {

			if(st.id == id) {
				return make_shared<State>(st);
			}

		}

		throw runtime_error("The required attribute 'icmp-response' is missing or invalid");

	}


 }


