Summary: E-Cell is a generic software package for cellular modeling and simulation.
Name: ecell
Version: 3.1.102
Release: 1
URL: http://www.e-cell.org
Source0: %{name}-%{version}.tar.gz
#License: GPL
Group: Applications
Copyright: E-Cell Project
Packager: Takeshi Sakurada
BuildRoot: %{_tmppath}/%{name}-root

Requires: Numeric
Requires: gsl
Requires: gsl-devel
Requires: boost-python
Requires: boost-python-devel
Requires: boost
Requires: boost-devel

%description
E-Cell Simulation Environment (E-Cell SE) is a software package for cellular and biochemical modeling and simulation. E-Cell attempts to provide a framework not only for analyzing metabolism, but also for higher-order cellular phenomena such as gen regulation networks, DNA replication and other occurences in the cell cycle.

%prep
%setup 
#%setup -a 1

CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{_prefix}

%build
make

%install
#rm -rf $RPM_BUILD_ROOT

make strip 
make prefix="$RPM_BUILD_ROOT/usr" install
chmod 666 $RPM_BUILD_ROOT/usr/lib/toollauncher/launcher.ini
make prefix="$RPM_BUILD_ROOT/usr" doc-install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/bin/*
/usr/lib/*
/usr/share/*
/usr/include/*

%changelog
* Sun Oct 13 2002 Takeshi Sakurada <sakurada@e-cell.org>
- Initial build.


