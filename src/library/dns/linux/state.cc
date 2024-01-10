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
 #include <udjat/defs.h>
 #include <udjat/net/dns.h>
 #include <udjat/tools/string.h>
 #include <netdb.h>
 #include <udjat/tools/intl.h>
 #include <memory>

 using namespace std;

 namespace Udjat {

	/// @brief Translate message.
	static const char * translate(const char *from_xml, const char *message) {
		if(*from_xml) {
			return from_xml;
		}
#ifdef GETTEXT_PACKAGE
		return dgettext(GETTEXT_PACKAGE,message);
#else
		return message;
#endif // GETTEXT_PACKAGE
	}

	DNS::State::State(const pugi::xml_node &node, const int code) : Udjat::State<int>(node,code) {
	}

	static const struct DNSState {
		const int code;
		const Level level;
		const char *name;
		const char *label;
		const char *summary;
		const char *body;
	} dnsstates[] = {

		{
			-1,
			Level::critical,
			"invalid",
			N_("Invalid DNS"),
			N_("Invalid DNS response"),
			N_("The DNS resolver state is invalid.")
		},

		{
			HOST_NOT_FOUND,
			Level::error,
			"host-not-found",
			N_("Not found"),
			N_("Cant find ${hostname}"),
			N_("Authoritative Answer 'Host not found' response on query for ${hostname}")
		},

		{
			TRY_AGAIN,
			Level::warning,
			"try-again",
			N_("Try again"),
			N_("Host not found/Server Fail"),
			N_("Non-Authoritative Host not found, or SERVERFAIL on query for ${hostname}")
		},

		{
			NO_RECOVERY,
			Level::error,
			"failed",
			N_("Server failed"),
			N_("Server has failed"),
			N_("Non recoverable errors, FORMERR, REFUSED, NOTIMP on query for ${hostname}")
		},

		{
			NO_DATA,
			Level::error,
			"no-data",
			N_("No data"),
			N_("No data for ${hostname}"),
			""
		},

		{
			NETDB_SUCCESS,
			Level::ready,
			"resolved",
			N_("DNS Ok"),
			N_("Got ${hostname}"),
			N_("The server was able to resolve ${hostname}")
		},
	};


	/// @brief The state for known codes.
	class KnownState : public DNS::State {
	private:
		String summary;
		String body;

	public:
		KnownState(const Udjat::Abstract::Object &object, const DNSState &state) : DNS::State(state.code) {

			Object::properties.label = translate(Object::properties.label,state.label);

			summary = translate(Object::properties.summary,state.summary);
			summary.expand(object,true,true);
			Object::properties.summary = summary.c_str();

			body = translate(properties.body,state.body);
			body.expand(object,true,true);
			properties.body = body.c_str();

			debug("----> ",body);

		}

		KnownState(const Udjat::Abstract::Object &object, const pugi::xml_node &node, const DNSState &state) : DNS::State(node, state.code) {

			Object::properties.label = translate(Object::properties.label,state.label);

			summary = translate(Object::properties.summary,state.summary);
			summary.expand(node);
			summary.expand(object);
			Object::properties.summary = summary.c_str();

			body = translate(properties.body,state.body);
			body.expand(node);
			body.expand(object);
			properties.body = body.c_str();

		}
	};

	/// @brief The state for unknown codes.
	class UnKnownState : public DNS::State {
	public:
		UnKnownState(const pugi::xml_node &node,const int code) : DNS::State(node, code) {
			if(!Object::properties.summary[0]) {
				Object::properties.summary = hstrerror(code);
			}
		}
	};

	DNS::State::State(const int code) : Udjat::State<int>{"nssdb",code,(code == NETDB_SUCCESS ? Level::ready : Level::error)} {
		for(const DNSState &state : dnsstates) {
			if(state.code == code) {
				rename(state.name);
				Object::properties.label = translate(Object::properties.label,state.label);
				Object::properties.summary = translate(Object::properties.summary,state.summary);
				properties.body = translate(properties.body,state.body);
				return;
			}
		}
		Object::properties.summary = hstrerror(code);
	}

	std::shared_ptr<DNS::State> DNS::State::Factory(const Udjat::Abstract::Object &object, const pugi::xml_node &node) {

		const char *state_name = node.attribute("dns-state").as_string();
		if(!*state_name) {
			throw std::system_error(EINVAL, std::system_category(), "Required attribute 'dns-state' is missing");
		}

		for(const DNSState &state : dnsstates) {

			if(!strcasecmp(state_name,state.name)) {
				return make_shared<KnownState>(object,node,state);
			}

		}

		throw std::system_error(EINVAL, std::system_category(),state_name);

	}

	std::shared_ptr<DNS::State> DNS::State::Factory(const Udjat::Abstract::Object &object, const int code) {

		for(const DNSState &state : dnsstates) {
			if(state.code == code) {
				debug("Got internal state for code '",code,"'");
				return make_shared<KnownState>(object,state);
			}
		}

		debug("No internal state for code '",code,"'");
		return make_shared<DNS::State>(code);

	}

 }
