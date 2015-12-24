#sbs-git:slp/apps/u/ug-wifi-di/rect libug-setting-wifidirect-efl 0.3.4 82f07b22ef73127a446c49e00b8dca37010b3ee2
%define PREFIX /usr/apps/setting-wifidirect-efl

Name:       libug-setting-wifidirect-efl
Summary:    Wi-Fi Direct setting UI gadget
Version:    1.11.65
Release:    1
Group:      App/Network
License:    Flora-1.1
Source0:    %{name}-%{version}.tar.gz

%if "%{profile}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

Requires(post): /sbin/ldconfig
Requires(post): /usr/bin/sqlite3
Requires(post): sys-assert
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext-tools
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(capi-network-wifi-direct)
BuildRequires:  pkgconfig(network)
BuildRequires:  pkgconfig(capi-network-tethering)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(ethumb)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(efreet)
#BuildRequires:  pkgconfig(dbus-1)
#BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-system-runtime-info)
BuildRequires:  pkgconfig(capi-system-device)
#BuildRequires:  pkgconfig(capi-system-sensor)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(feedback)
BuildRequires:  pkgconfig(deviced)
BuildRequires:  pkgconfig(efl-assist)
BuildRequires:  pkgconfig(capi-ui-efl-util)
BuildRequires:  pkgconfig(efl-extension)
#BuildRequires:  model-build-features
BuildRequires:  cmake
BuildRequires:  gettext-devel
BuildRequires:  hash-signer

%description
wifi direct client library (Shared Library)


%package -n org.tizen.wifi-direct-popup
Summary:    Wifi-Wirect system popup
License:    Flora-1.1
Requires:   %{name} = %{version}-%{release}


%description -n org.tizen.wifi-direct-popup
Wi-Fi Direct system popup.



%prep
%setup -q

%build
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%cmake . -DCMAKE_INSTALL_PREFIX=$PREFIX \
	 -DMODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE=1 \

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
%define tizen_sign 1
%define tizen_sign_base /usr/apps/org.tizen.wifi-direct-popup
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1
#%__strip %{buildroot}/usr/ug/lib/libug-setting-wifidirect-efl.so.0.1.0
#%__strip %{buildroot}/usr/apps/org.tizen.wifi-direct-popup/bin/wifi-direct-popup

mkdir -p %{buildroot}/usr/share/license
cp %{_builddir}/%{buildsubdir}/LICENSE %{buildroot}/usr/share/license/%{name}
cp %{_builddir}/%{buildsubdir}/LICENSE %{buildroot}/usr/share/license/org.tizen.wifi-direct-popup

%post
mkdir -p /usr/apps/setting-wifidirect-efl/bin/ -m 777
chown -R 5000:5000 /usr/apps/setting-wifidirect-efl/bin/
chsmack -a "_" /usr/apps/setting-wifidirect-efl/bin/

ln -sf /usr/bin/ug-client /usr/apps/setting-wifidirect-efl/bin/ug-setting-wifidirect-efl
ln -sf /usr/apps/setting-wifidirect-efl/lib/libug-setting-wifidirect-efl.so /usr/apps/setting-wifidirect-efl/lib/libug-ug-setting-wifidirect-efl.so

%post -n org.tizen.wifi-direct-popup


%postun


%files
%manifest setting-wifidirect-efl.manifest
%defattr(-,root,root,-)
/usr/apps/setting-wifidirect-efl/lib/ug/*
/usr/apps/setting-wifidirect-efl/res/edje/*
#/usr/ug/res/locale/*/*/*
#/usr/apps/setting-wifidirect-efl/data/locale/*/*/*
%{_datadir}/locale/*/LC_MESSAGES/*.mo
/usr/share/license/%{name}
/usr/share/packages/setting-wifidirect-efl.xml
/usr/apps/setting-wifidirect-efl/shared/res/tables/setting-wifidirect-efl_ChangeableColorTable.xml
/usr/apps/setting-wifidirect-efl/shared/res/tables/setting-wifidirect-efl_FontInfoTable.xml
/usr/apps/setting-wifidirect-efl/shared/icons/*

%files -n org.tizen.wifi-direct-popup
%manifest org.tizen.wifi-direct-popup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.wifi-direct-popup/bin/*
/usr/apps/org.tizen.wifi-direct-popup/res/images/*
/usr/apps/org.tizen.wifi-direct-popup/res/edje/*
/usr/apps/org.tizen.wifi-direct-popup/res/locale/*/*/*
#%{_datadir}/locale/*/LC_MESSAGES/*.mo
/usr/apps/org.tizen.wifi-direct-popup/author-signature.xml
/usr/apps/org.tizen.wifi-direct-popup/signature1.xml
#/usr/share/applications/org.tizen.wifi-direct-popup.desktop
#for appfw new manifest
/usr/share/packages/org.tizen.wifi-direct-popup.xml
/usr/share/license/org.tizen.wifi-direct-popup

