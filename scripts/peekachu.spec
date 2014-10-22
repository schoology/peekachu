Summary: Utility for peeking at key values in memcached
Name: peekachu
Version: 0.1.0
Release: 1

URL: https://github.com/schoology/peekachu
Source0: %{name}-%{version}.tar.gz
License: MIT
Group: Development/Tools
Packager: Dave Newman <dnewman@schoology.com>
BuildRoot: %{_tmppath}/%{name}-root

BuildRequires: autoconf
BuildRequires: automake
BuildRequires: libtool

%description
peekachu is a command line utility for peeking at key values in memcache. It supports looking at raw values or decompressed
values using the FastLZ library. peekachu is also able to set keys in memcache either in raw form or compressed using FastLZ.

%prep
%setup -q
autoreconf -fvi

%build

%configure
%__make

%install
[ %{buildroot} != "/" ] && rm -rf %{buildroot}

%makeinstall PREFIX=%{buildroot}

%clean
[ %{buildroot} != "/" ] && rm -rf %{buildroot}

%files
/usr/bin/peekachu

%changelog
* Tue Oct 21 2014 Dave Newman <dnewman@schoology.com>
- peekachu: version 0.1.0 release
- initial release
