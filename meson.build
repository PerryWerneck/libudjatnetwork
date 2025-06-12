# Maintainer: Perry Werneck <perry.werneck@gmail.com>

# SPDX-License-Identifier: LGPL-3.0-or-later 

# Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

project(
	'libudjatnetwork', 
	['cpp'],
	version: '1.2.0',
	default_options : ['c_std=c11', 'cpp_std=c++17'],
	license: 'GPL-3.0-or-later',
)

project_description = 'Network module and library for udjat'

#
# Versioning
#
pkg_version = meson.project_version()
version_array = pkg_version.split('.')
pkg_major_version = version_array[0].to_int()
pkg_minor_version = version_array[1].to_int()
pkg_micro_version = version_array[2].to_int()

libversion = '@0@.@1@'.format(pkg_major_version,pkg_minor_version)

#
# i18n
#
#subdir('po')

#
# Required libraries
#
# libresolv, resolv.h?
libudjat = dependency('libudjat')
lib_deps = [
  libudjat,
]

#
# Compiler flags
#
cxx = meson.get_compiler('cpp')
enable_debug = get_option('debug') or get_option('buildtype').contains('debug')

time_epoch = run_command('sh', '-c', 'echo "$SOURCE_DATE_EPOCH"', check : true).stdout().strip()
if time_epoch == ''
  time_epoch = run_command('date','+%Y%m%d', check: true).stdout().strip()
endif

compiler_flags = [
  '-ggdb3',
  '-fvisibility=hidden',
  '-fPIC',
  '-DHAVE_CONFIG_H=1',
  '-DBUILD_DATE='+time_epoch,
]

if enable_debug
  compiler_flags += [
	  '-DDEBUG=1'
  ]
else
  compiler_flags += [
	'-DNDEBUG=1'
  ]
endif

add_project_arguments(cxx.get_supported_arguments(compiler_flags), language: 'cpp')

#
# Configuration
#
app_conf = configuration_data()
app_conf.set_quoted('PACKAGE_NAME', meson.project_name())
app_conf.set_quoted('PACKAGE_VERSION', meson.project_version())
app_conf.set_quoted('PACKAGE_DESCRIPTION', project_description)
app_conf.set('PACKAGE_VERSION_MAJOR', pkg_major_version)
app_conf.set('PACKAGE_VERSION_MINOR', pkg_minor_version)
app_conf.set('PACKAGE_VERSION_MICRO', pkg_micro_version)

app_conf.set_quoted('LOG_DOMAIN', 'http')

app_conf.set('PRODUCT_NAME', libudjat.get_variable('product_name'))
app_conf.set('PRODUCT_VERSION', libudjat.version())

if cxx.compiles('#include <unistd.h>', name : 'unistd.h')
  app_conf.set('HAVE_UNISTD_H', 1)
endif

includes_dir = include_directories('src/include')

#
# Sources
#
lib_src = [
  'src/library/icmp/response.cc',
  'src/library/icmp/state.cc',
  'src/library/icmp/worker.cc',
  'src/library/defaultgateway.cc',
  'src/library/dns/agent.cc',
  'src/library/dns/linux/record.cc',
  'src/library/dns/linux/resolver.cc',
  'src/library/dns/linux/state.cc',
  'src/library/ip/agent.cc',
  'src/library/ip/factory.cc',
  'src/library/ip/state.cc',
  'src/library/ip/subnet.cc',
  'src/library/nic/agent.cc',
  'src/library/nic/factory.cc',
  'src/library/nic/list.cc',
  'src/library/range.cc',
]

module_src = [
  'src/module/init.cc',
]

test_src = [
  'src/testprogram/testprogram.cc'
]

#
# SDK
#
# https://mesonbuild.com/Pkgconfig-module.html
#
pkg = import('pkgconfig')

#
# OS dependencies
#
if host_machine.system() == 'windows'

  app_conf.set('HAVE_WINHTTP', 1)

  pkg.generate(
    name: 'lib' + meson.project_name(),
    description: project_description,
    requires: [ 'libudjat' ],
    libraries: [ '-l' + meson.project_name() + '.dll' ]
  )

  pkg.generate(
    name: 'lib' + meson.project_name() + '-static',
    description: project_description,
    requires: [ json_c ],
    libraries: [ 
      '-l' + meson.project_name(),
      '-lws2_32',
  ]
  )

  lib_src += [
    'src/library/os/windows/defaultgateway.cc',
    'src/library/os/windows/icmp_controller.cc',
    'src/library/os/windows/ipaddress.cc',
    'src/library/os/windows/subnet.cc',
  ]

else

  pkg.generate(
    name: 'lib' + meson.project_name(),
    description: project_description,
    requires: [ 'libudjat' ],
    libraries: [ '-l' + meson.project_name() ]
  )

  pkg.generate(
    name: 'lib' + meson.project_name() + '-static',
    description: project_description,
    libraries: [ '-l:lib' + meson.project_name() + '.a' ]
  )

  lib_src += [
    'src/library/os/linux/defaultgateway.cc',
    'src/library/os/linux/icmp_controller.cc',
    'src/library/os/linux/icmphost.cc',
    'src/library/os/linux/netlink.cc',
    'src/library/os/linux/nicagent.cc',
    'src/library/os/linux/nicdetect.cc',
    'src/library/os/linux/niclist.cc',
    'src/library/os/linux/nicstate.cc',
    'src/library/os/linux/subnet.cc',
  ]

endif

#
# Targets
#
config_src = [
  configure_file(
      output : 'config.h', 
      configuration : app_conf
  )
]

if host_machine.system() == 'windows'

  # https://mesonbuild.com/Windows-module.html
  windows = import('windows')
  resources = windows.compile_resources(
    configure_file(
      input : 'src/library/os/windows/resources.rc.in',
      output : 'resources.rc',
      configuration : app_conf
    )
  )

  dynamic = shared_library(
    meson.project_name(), 
    config_src + lib_src + resources,
    install: true,
    version : libversion,
    soversion : libversion,
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps,
    link_args: [ 
      '-lws2_32',
    ],
    include_directories: includes_dir
  )

  # https://mesonbuild.com/Reference-manual_functions.html#shared_library
  module = shared_library(
    'http', 
    config_src + module_src + resources,
    name_prefix: 'udjat-module-',
    install: true,
    install_tag: 'module',
    link_with: dynamic,
    install_dir: libudjat.get_variable('module_path'),
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps,
    include_directories: includes_dir
  )

else

  dynamic = shared_library(
    meson.project_name(), 
    config_src + lib_src,
    install: true,
    version : libversion,
    soversion : libversion,
    cpp_args: [ '-ffat-lto-objects' ],    
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps,
    include_directories: includes_dir
  )

  # https://mesonbuild.com/Reference-manual_functions.html#shared_library
  module = shared_library(
    'http', 
    config_src + module_src,
    name_prefix: 'udjat-module-',
    install: true,
    link_with: dynamic,
    install_dir: libudjat.get_variable('module_path'),
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps,
    include_directories: includes_dir
  )

endif

static_library(
  meson.project_name(), 
  config_src + lib_src,
  install: true,
  dependencies: lib_deps,
  cpp_args: [ '-DSTATIC_LIBRARY', '-fno-lto' ],
  include_directories: includes_dir
)

executable(
  meson.project_name(),
  config_src + test_src,
  install: false,
  link_with : [ dynamic, module ],
  link_args: [ '-l:libudjat-loader.a' ],
  dependencies: [ libudjat ],
  include_directories: includes_dir
)

install_headers(
  'src/include/udjat/net/dns.h',
  'src/include/udjat/net/gateway.h',
  'src/include/udjat/net/icmp.h',
  'src/include/udjat/net/' + host_machine.system() + '/dns.h',
  'src/include/udjat/net/' + host_machine.system() + '/icmp.h',
  subdir: 'udjat/net'  
)

install_headers(
  'src/include/udjat/net/' + host_machine.system() + '/dns.h',
  'src/include/udjat/net/' + host_machine.system() + '/icmp.h',
  subdir: 'udjat/net/' + host_machine.system()
)

install_headers(
  'src/include/udjat/net/dns/agent.h',
  'src/include/udjat/net/dns/response.h',
  'src/include/udjat/net/dns/state.h',
  subdir: 'udjat/net/dns'  
)

install_headers(
  'src/include/udjat/net/ip/agent.h',
  'src/include/udjat/net/ip/state.h',
  'src/include/udjat/net/ip/subnet.h',
  subdir: 'udjat/net'  
)



