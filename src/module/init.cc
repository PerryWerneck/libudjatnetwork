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
 #include <private/module.h>
 #include <udjat/module.h>
 #include <udjat/network/agents/host.h>
 #include <udjat/network/agents/nic.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <linux/capability.h>
 #include <sys/syscall.h>
 #include <udjat/moduleinfo.h>

 using namespace Udjat;

 const ModuleInfo moduleinfo{ "Network monitor" };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {

	class Module : public Udjat::Module {
	private:
		Network::HostAgent::Factory hostFactory;
		Network::NICAgent::Factory 	nicFactory;

	public:

		Module() : Udjat::Module("network",moduleinfo) {
		};

		~Module() {
		};

	};

	if(getuid()) {

		// Non root, do we have CAP_NET_RAW
		struct __user_cap_header_struct caphdr = {
			.version=_LINUX_CAPABILITY_VERSION_3,
			.pid=0,
		};

		struct __user_cap_data_struct cap[_LINUX_CAPABILITY_U32S_3];

		if (!syscall(SYS_capget,&caphdr,cap)) {

			if (cap[CAP_TO_INDEX(CAP_NET_RAW)].effective&CAP_TO_MASK(CAP_NET_RAW)) {

				cout << "network\tRunning as user with CAP_NET_RAW capability." << endl;

			} else {

#ifdef DEBUG
				cerr << "module\tThis module requires root privileges or CAP_NET_RAW capability." << endl;
#else
				throw runtime_error("This module requires root privileges or CAP_NET_RAW capability.");
#endif // DEBUG
			}

		}

	}

	return new Module();
 }


