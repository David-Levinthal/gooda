Name:		gooda
Version:	1.0.0
Release:	1%{?dist}
Summary:	PMU-based performance analysis tool
Group:		perf
License:	Apache
URL:		http://code.google.com/p/gooda/
Source0:	http://code.google.com/p/gooda/downloads/list/%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root

%description
Gooda is a performance tool to analyze samples obtained via the perf tool
on Linux. Gooda provides a fine grain cycle breakdown analysis of apps.
Gooda processes the raw data from the perf.data file into a series of
spreadsheets which can then be visualized with a web browser using
the Gooda visualizer http://code.google.com/p/gooda-visualizer/.
Gooda uses perf with actual hardware monitoring events (PMU events).
Gooda comes with a series collection scripts which make it easy to
invoke perf correctly for each CPU model.

%prep
%setup

%build
echo build
echo making
make DESTDIR=/usr

%install
make DESTDIR=$RPM_BUILD_ROOT PREFIX=/usr GOODA_DIR=%{_libexecdir}/gooda install

%clean
rm -fr $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/*
%{_libexecdir}/gooda
%doc README

%changelog
* Fri Jun 8 2012 Stephane Eranian <eranian@google.com> 1.1.0
- initial packaging
