Summary:		Synchronisation framework.
Name:			opensync
Version:		0.14
Release:		1
License:		LGPL
Group:			Development/Libraries
Source:			%{name}-%{version}.tar.gz
BuildRoot:		%{_tmppath}/%{name}-%{version}
Requires:		libxml2 glib2 sqlite >= 3.0.0
BuildRequires:	libxml2-devel glib2-devel sqlite-devel >= 3.0.0

%description
OpenSync is a synchronization framework that is platform and distribution independent.
It consists of several plugins that can be used to connect to devices, a powerfull sync-engine and the framework itself.
OpenSync is platform and distribution independent has no dependencies on X related libraries.

%package devel
Summary:		Header files, libraries and development documentation for %{name}
Group:			Development/Libraries
Requires:		%{name} = %{version}

%description devel
This package contains the header files, static libraries and development
documentation for %{name}. If you like to develop programs using %{name},
you will need to install %{name}-devel.

%package tools
Summary:		Tools for %{name}
Group:			Development/Tools
Requires:		%{name}

%description tools
Tools to test and debug %{name}.

%prep
%setup -q

%build
%configure --enable-engine --disable-debug --disable-tracing --disable-profiling --enable-tools --disable-unit-tests --disable-python

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}
rm -f %{buildroot}%{_libdir}/*.la
rm -f %{buildroot}%{_libdir}/opensync/formats/*.la

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_libdir}/libopensync-xml.so*
%{_libdir}/libopensync.so*
%{_libdir}/libosengine.so*
%{_libdir}/opensync

%files devel
%defattr(-,root,root)
%{_includedir}/opensync-1.0
%{_libdir}/pkgconfig/*.pc

%files tools
%defattr(-,root,root)
%{_bindir}/osyncdump
%{_bindir}/osyncplugin
%{_bindir}/osyncstress

%changelog
* Fri Mar 18 2005 Pierre Ossman <drzeus@drzeus.cx> 0.13-1
- Initial package
