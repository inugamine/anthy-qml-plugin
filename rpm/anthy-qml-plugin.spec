Name:       anthy-qml-plugin
Summary:    Anthy QML Plugin for Japanese Input
Version:    0.1.0
Release:    1
License:    LGPL-2.1+
URL:        https://github.com/user/anthy-qml-plugin
Source0:    %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
Requires:       anthy-unicode

%description
QML plugin that provides Anthy Japanese input method engine
for Sailfish OS applications and keyboards.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
make %{?_smp_mflags}

%install
%qmake5_install

%files
%defattr(-,root,root,-)
%{_libdir}/qt5/qml/jp/anthy/
