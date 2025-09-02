#
# spec file for package udjat-module-network
#
# Copyright (c) 2015 SUSE LINUX GmbH, Nuernberg, Germany.
# Copyright (C) 2023 Perry Werneck perry.werneck@gmail.com
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://github.com/PerryWerneck/udjat-module-network
#

%define product_name %(pkg-config --variable=product_name libudjat)
%define module_path %(pkg-config --variable=module_path libudjat)

Summary:		Network module for %{product_name} 
Name:			libudjatnetwork
Version: 1.2.0
Release:		0
License:		LGPL-3.0
Source:			%{name}-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/libudjatnetwork

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

BuildRequires:	autoconf >= 2.61
BuildRequires:	automake
BuildRequires:	libtool
BuildRequires:	binutils
BuildRequires:	coreutils
BuildRequires:	gcc-c++

BuildRequires:	pkgconfig(libudjat) >= 2.0.0

%description
Network module for udjat.

Add factory for %{product_name} network validation and check agents.

#---[ Library ]-------------------------------------------------------------------------------------------------------

%package -n libudjatnetwork%{_libvrs}
Summary:	UDJat network library

%description -n libudjatnetwork%{_libvrs}
Network abstraction library for %{product_name}

#---[ Development ]---------------------------------------------------------------------------------------------------

%package -n libudjatnetwork-devel
Summary:	Development files for %{name}
Requires:	pkgconfig(libudjat)
Requires:	libudjatnetw%{_libvrs} = %{version}

%description -n libudjatnetwork-devel

Development files for Udjat's network abstraction library.

%lang_package -n libudjatnetwork%{_libvrs}

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup

NOCONFIGURE=1 \
	./autogen.sh

%configure

%build
make all

%install
%makeinstall
%find_lang libudjatnetw-%{MAJOR_VERSION}.%{MINOR_VERSION} langfiles

%files
%defattr(-,root,root)
%{module_path}/*.so

%files -n libudjatnetwork%{_libvrs}
%defattr(-,root,root)
%{_libdir}/libudjatnetwork.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%files -n libudjatnetwork%{_libvrs}-lang -f langfiles

%files -n libudjatnetwork-devel
%defattr(-,root,root)

%dir %{_includedir}/udjat/net
%dir %{_includedir}/udjat/net/dns
%dir %{_includedir}/udjat/net/ip
%dir %{_includedir}/udjat/net/nic
%dir %{_includedir}/udjat/net/linux

%{_includedir}/udjat/net/*.h
%{_includedir}/udjat/net/dns/*.h
%{_includedir}/udjat/net/ip/*.h
%{_includedir}/udjat/net/nic/*.h
%{_includedir}/udjat/net/linux/*.h

%{_libdir}/*.so
%exclude %{_libdir}/*.a
%{_libdir}/pkgconfig/*.pc

%pre -n libudjatnetwork%{_libvrs} -p /sbin/ldconfig

%post -n libudjatnetwork%{_libvrs} -p /sbin/ldconfig

%postun -n libudjatnetwork%{_libvrs} -p /sbin/ldconfig

%changelog

