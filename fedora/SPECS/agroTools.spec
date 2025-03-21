# Note: define srcarchivename in Travis build only.
%{!?srcarchivename: %global srcarchivename agroTools-%{version}}

Name:           agroTools
Version:        1.8.7
Release:        2%{?dist}
Summary:        Agro tools

URL:            https://github.com/ARPA-SIMC/agroTools
Source0:        https://github.com/ARPA-SIMC/agroTools/archive/v%{version}.tar.gz#/%{srcarchivename}.tar.gz
License:        GPL

BuildRequires:  qt5-qtbase
BuildRequires:  qt5-devel
BuildRequires:  gdal-libs
BuildRequires:  gdal-devel
BuildRequires:  geos
BuildRequires:  geos-devel
BuildRequires:  netcdf
BuildRequires:  netcdf-devel

Requires:       qt5-qtbase-mysql

%description
agroTools

%prep
%autosetup -n %{srcarchivename}

%build

pushd bin/csv2dbMeteo
qmake-qt5 csv2dbMeteo.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_csv2dbGrid
qmake-qt5 Makeall_csv2dbGrid.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_criteriaOutputTool
qmake-qt5 Makeall_criteriaOutputTool.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_frostForecast
qmake-qt5 Makeall_frostForecast.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p %{buildroot}/%{_bindir}/
cp -a bin/csv2dbMeteo/release/Csv2dbMeteo %{buildroot}/%{_bindir}/
cp -a bin/csv2dbGrid/release/Csv2dbGrid %{buildroot}/%{_bindir}/
cp -a bin/criteriaOutputTool/release/CriteriaOutput %{buildroot}/%{_bindir}/
cp -a bin/frostForecast/release/FrostForecast %{buildroot}/%{_bindir}/
mkdir -p %{buildroot}/%{_datadir}/
cp -a deploy/appimage/usr/share/agroTools %{buildroot}/%{_datadir}/

%files
%{_bindir}/Csv2dbMeteo
%{_bindir}/Csv2dbGrid
%{_bindir}/CriteriaOutput
%{_bindir}/FrostForecast
%{_datadir}/agroTools/*

%changelog
* Wed Feb 26 2025 Fausto Tomei <ftomei@arpae.it> - 1.8.7-2
- Release 1.8.7

* Mon May 27 2024 Laura Costantini <laura.costantini0@gmail.com> - 1.8.4-1
- Release 1.8.4

* Thu Apr 4 2024 Laura Costantini <laura.costantini0@gmail.com> - 1.8.3-1
- Release 1.8.3

* Thu Mar 28 2024 Fausto Tomei <ftomei@arpae.it> - 1.8.2-1
- Release 1.8.2

* Tue Feb 20 2024 Gabriele Antolini <gantolini@arpae.it> - 1.8.1-1
- Release 1.8.1

* Tue Feb 20 2024 Gabriele Antolini <gantolini@arpae.it> - 1.8.0-1
- Release 1.8.0

* Mon Feb 12 2024 Fausto Tomei <ftomei@arpae.it> - 1.7.4-1
- Release 1.7.4

* Mon Feb 05 2024 Fausto Tomei <ftomei@arpae.it> - 1.7.3-1
- Release 1.7.3

* Thu Jan 18 2024 Laura Costantini <laura.costantini0@gmail.com> - 1.7.2-1
- Release 1.7.2

* Fri Jan 5 2024 Fausto Tomei <ftomei@arpae.it> - 1.7.1-1
- Release 1.7.1

* Fri Dec 15 2023 Fausto Tomei <ftomei@arpae.it> - 1.7.0-1
- Release 1.7.0

* Tue Apr 18 2023 Fausto Tomei <ftomei@arpae.it> - 1.6.0-2
- Release 1.6.0

* Mon Apr 17 2023 Fausto Tomei <ftomei@arpae.it> - 1.6.0-1
- Release 1.6.0

* Mon Jun  6 2022 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.5.1-2
- Fixed spec file

* Tue May 31 2022 Fausto Tomei <ftomei@arpae.it> - 1.5.1-1
- Release 1.5.1

* Tue May 24 2022 Fausto Tomei <ftomei@arpae.it> - 1.5.0-1
- Release 1.5.0

* Mon May 23 2022 Fausto Tomei <ftomei@arpae.it> - 1.4.2-1
- Release 1.4.2

* Fri Feb 25 2022 Daniele Branchini <dbranchini@arpae.it> - 1.4.1-1
- Release 1.4.1

* Mon Feb 21 2022 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.4.0-1
- Release 1.4.0

* Mon Oct 18 2021 Fausto Tomei <ftomei@arpae.it> - 1.2.0-1
- Release 1.2.0

* Thu May 13 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.1.0-1
- Release 1.1.0

* Tue Apr 20 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.0.0-1
- Release 1.0.0
