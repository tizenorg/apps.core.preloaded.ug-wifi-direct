#sbs-git:slp/apps/u/ug-wifi-direct libug-setting-wifidirect-efl 0.3.4 82f07b22ef73127a446c49e00b8dca37010b3ee2
%define PREFIX /usr/ug

Name:       libug-setting-wifidirect-efl
Summary:    Wi-Fi Direct setting UI gadget 
Version:    1.0.21
Release:    1
Group:      Network & Connectivity/Wireless
License:    Flora
Source0:    %{name}-%{version}.tar.gz
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
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(wifi-direct)
BuildRequires:  pkgconfig(network)
BuildRequires:  pkgconfig(capi-network-tethering)
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
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(syspopup-caller)
BuildRequires:  cmake
BuildRequires:  gettext-devel

%description
wifi direct client library (Shared Library)


%package -n org.tizen.wifi-direct-popup  
Summary:    Wifi-Wirect system popup   
Requires:   %{name} = %{version}-%{release}   

%package -n org.tizen.wifi-direct-ugapp 
Summary:    Wifi-Wirect application launching UG   
Requires:   %{name} = %{version}-%{release}   
  
%description -n org.tizen.wifi-direct-popup   
Wi-Fi Direct system popup.   
  

%description -n org.tizen.wifi-direct-ugapp  
Wi-Fi Direct application launching UG.   

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=$PREFIX
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post
mkdir -p /usr/ug/bin/
ln -sf /usr/bin/ug-client /usr/ug/bin/ug-setting-wifidirect-efl
/sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest libug-setting-wifidirect-efl.manifest
%defattr(-,root,root,-)
/usr/ug/lib/*
/usr/ug/res/images/*
/usr/ug/res/edje/*
/usr/ug/res/locale/*/*/*

%files -n org.tizen.wifi-direct-popup
%manifest org.tizen.wifi-direct-popup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.wifi-direct-popup/bin/*
/usr/apps/org.tizen.wifi-direct-popup/res/images/*
/usr/apps/org.tizen.wifi-direct-popup/res/locale/*/*/*
#/usr/share/applications/org.tizen.wifi-direct-popup.desktop
#for appfw new manifest
/usr/share/packages/org.tizen.wifi-direct-popup.xml
%{_sysconfdir}/smack/accesses.d/org.tizen.wifi-direct-popup.rule

%files -n org.tizen.wifi-direct-ugapp
%manifest org.tizen.wifi-direct-ugapp.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.wifi-direct-ugapp/bin/*
#/usr/share/applications/org.tizen.wifi-direct-ugapp.desktop
#for appfw new manifest
/usr/share/packages/org.tizen.wifi-direct-ugapp.xml
%{_sysconfdir}/smack/accesses.d/org.tizen.wifi-direct-ugapp.rule
