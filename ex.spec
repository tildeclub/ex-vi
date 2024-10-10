#
# Sccsid @(#)ex.spec    1.7 (gritter) 1/22/05
#
Summary: A port of the traditional ex/vi editors
Name: ex-vi
Version: 10102024
Release: 1
License: BSD
Source: %{name}-%{version}.tar.gz
Group: System Environment/Base
Vendor: Gunnar Ritter <Gunnar.Ritter@pluto.uni-freiburg.de>
URL: <http://ex-vi.sourceforge.net>
BuildRoot: %{_tmppath}/%{name}-root

# prefix applies to bindir, libexecdir, and mandir.
%global debug_package %{nil}
%define prefix          /usr
%define bindir          %{prefix}/5bin
%define libexecdir      %{prefix}/5lib
%define mandir          %{prefix}/share/man/5man

%define preservedir     /var/preserve

# install command
%define ucbinstall      install

%define cflags          -Os -fomit-frame-pointer

%define makeflags       PREFIX=%{prefix} BINDIR=%{bindir} LIBEXECDIR=%{libexecdir} MANDIR=%{mandir} PRESERVEDIR=%{preservedir} INSTALL=%{ucbinstall} RPMCFLAGS="%{cflags}"

%description
This is a port of the traditional ex and vi editor implementation as
found on 2BSD and 4BSD. It was enhanced to support most of the additions
in System V and POSIX.2, and international character sets like UTF-8 and
many East Asian encodings.

%changelog
* Thu Oct 10 2024 Your Name <you@example.com> - 10102024-1
- Initial build of ex-vi package.

%prep
rm -rf %{buildroot}
%setup

%build
make %{makeflags}

%install
make DESTDIR=%{buildroot} %{makeflags} install

%clean
cd ..; rm -rf %{_builddir}/%{name}-%{version}
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc Changes LICENSE README TODO
%{bindir}/*
%{libexecdir}/*
%{mandir}/man1/*
%{preservedir}
