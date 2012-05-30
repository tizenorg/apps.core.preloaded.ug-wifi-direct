%define PREFIX /opt/ug

Name:       libug-setting-wifidirect-efl
Summary:    Wi-Fi Direct setting UI gadget 
Version:    0.4.2
Release:    1
Group:      TO_BE_FILLED
License:    Samsung Proprietary License
Source0:    %{name}-%{version}.tar.gz
Source1001: packaging/ug-wifi-direct.manifest 
Requires(post): /sbin/ldconfig   
Requires(post): /usr/bin/sqlite3   
Requires(postun): /sbin/ldconfig   
BuildRequires:  cmake   
BuildRequires:  edje-tools   
BuildRequires:  gettext-tools
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(ui-gadget)
BuildRequires:  pkgconfig(wifi-direct)
BuildRequires:  pkgconfig(network)
BuildRequires:  pkgconfig(utilX)   
BuildRequires:  pkgconfig(elementary)   
BuildRequires:  pkgconfig(edje)   
BuildRequires:  pkgconfig(evas)   
BuildRequires:  pkgconfig(ecore)   
BuildRequires:  pkgconfig(ethumb)   
BuildRequires:  pkgconfig(glib-2.0)   
BuildRequires:  pkgconfig(efreet)   
BuildRequires:  pkgconfig(dbus-1)   
BuildRequires:  pkgconfig(dbus-glib-1)   
BuildRequires:  pkgconfig(edbus)   
BuildRequires:  pkgconfig(aul)   
BuildRequires:  pkgconfig(devman)   
BuildRequires:  pkgconfig(appsvc)   
BuildRequires:  pkgconfig(pmapi)   
BuildRequires:  pkgconfig(capi-appfw-application)   
BuildRequires:  pkgconfig(capi-system-runtime-info)   
BuildRequires:  pkgconfig(capi-system-device) 

BuildRequires:  cmake
BuildRequires:  gettext-devel

%define debug_package %{nil}  

%description
wifi direct client library (Shared Library)


%package -n org.tizen.wifi-direct-popup  
Summary:    Wifi-Wirect system popup   
Requires:   %{name} = %{version}-%{release}   
  
%description -n org.tizen.wifi-direct-popup   
Wi-Fi Direct system popup.   
  

%prep
%setup -q

%build
cp %{SOURCE1001} .
cmake . -DCMAKE_INSTALL_PREFIX=$PREFIX
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

%make_install

%post
vconftool set -t int db/wifi_direct/onoff 0 -u apps -i

%postun


%files
%manifest ug-wifi-direct.manifest
%defattr(-,root,root,-)
/opt/ug/lib/*
/opt/ug/res/images/*
/opt/ug/res/edje/*
/opt/ug/res/locale/*/*/*

%files -n org.tizen.wifi-direct-popup
%manifest ug-wifi-direct.manifest
%defattr(-,root,root,-)
/opt/apps/org.tizen.wifi-direct-popup/bin/*
/opt/apps/org.tizen.wifi-direct-popup/res/locale/*/*/*
/opt/share/applications/*
