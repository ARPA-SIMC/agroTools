# Note: define srcarchivename in Travis build only.
%{!?srcarchivename: %global srcarchivename agroTools-%{version}}

Name:           agroTools
Version:        1.0.5
Release:        1%{?dist}
Summary:        One-dimensional soil water balance

URL:            https://github.com/ARPA-SIMC/agroTools
Source0:        https://github.com/ARPA-SIMC/agroTools/archive/v%{version}.tar.gz#/%{srcarchivename}.tar.gz
License:        GPL

BuildRequires:  qt5-qtbase
BuildRequires:  qt5-devel
BuildRequires:  gdal-libs
BuildRequires:  gdal-devel
BuildRequires:  geos
BuildRequires:  geos-devel

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

pushd bin/Makeall_criteriaOutputTool
qmake-qt5 Makeall_criteriaOutputTool.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_csv2dbGrid
qmake-qt5 Makeall_csv2dbGrid.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p %{buildroot}/%{_bindir}/
cp -a bin/csv2dbMeteo/release/Csv2dbMeteo %{buildroot}/%{_bindir}/
cp -a bin/criteriaOutputTool/release/CriteriaOutput %{buildroot}/%{_bindir}/
cp -a bin/csv2dbGrid/release/Csv2dbGrid %{buildroot}/%{_bindir}/
mkdir -p %{buildroot}/%{_datadir}/
cp -a deploy/appimage/usr/share/agroTools %{buildroot}/%{_datadir}/

%files
%{_bindir}/Csv2dbMeteo
%{_bindir}/CriteriaOutput
%{_bindir}/Csv2dbGrid
%{_datadir}/agroTools/*

%changelog

