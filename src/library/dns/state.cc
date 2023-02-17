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
			"cant-resolve-server",
			DNS::cant_resolve_server_address,
			Level::critical,
			N_("No server"),
			N_("Cant resolve DNS Server"),
			N_("Unable to resolve the name of the defined DNS server.")
		},
		{
			"cant-resolve-host",
			DNS::cant_resolve_address,
			Level::error,
			N_("Cant resolve"),
			N_("Cant resolve name"),
			N_("Unable to resolve ${hostname}.")
		},
		{
			"host-not-found",
			DNS::host_not_found,
			Level::error,
			N_("Not found"),
			N_("Host not found"),
			N_("Cant find ${hostname}.")
		},
		{
			"resolved",
			DNS::dns_ok,
			Level::ready,
			N_("Ok"),
			N_("The hostname was resolved"),
			N_("The DNS was able to resolve ${hostname}.")
		},

	};

	UDJAT_API DNS::Response DNS::ResponseFactory(const char *name) {

		for(size_t ix = 0; ix < N_ELEMENTS(dns_states); ix++) {
			if(!strcasecmp(name,dns_states[ix].name)) {
				return (DNS::Response) ix;
			}
		}

		throw runtime_error(string{"Invalid DNS response id: "} + name);

	}

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

		DNS::Response id{ResponseFactory(node.attribute("dns-state").as_string())};

		for(const dns_state &st : dns_states) {

			if(st.id == id) {
				return make_shared<State>(node,st);
			}

		}

		throw runtime_error("The required attribute 'dns-state' is missing or invalid");

	}

	std::shared_ptr<DNS::State> DNS::State::Factory(const Udjat::Abstract::Object &object, const DNS::Response id) {

		class State : public DNS::State {
		private:
			Udjat::String summary;
			Udjat::String body;

		public:
			State(const Udjat::Abstract::Object &object, const dns_state &state) : DNS::State{state.name,state.level,state.id} {
#ifdef GETTEXT_PACKAGE
				summary = dgettext(GETTEXT_PACKAGE,state.summary);
				body = dgettext(GETTEXT_PACKAGE,state.body);
				Object::properties.label = dgettext(GETTEXT_PACKAGE,state.label);
#else
				summary = state.summary);
				body = state.body;
				Object::properties.label = state.label;
#endif // GETTEXT_PACKAGE

				properties.body = body.expand(object).c_str();
				Object::properties.summary = summary.expand(object).c_str();
			}

		};

		for(const dns_state &st : dns_states) {

			if(st.id == id) {
				return make_shared<State>(object,st);
			}

		}

		throw runtime_error("Invalid DNS state");

	}


 }


