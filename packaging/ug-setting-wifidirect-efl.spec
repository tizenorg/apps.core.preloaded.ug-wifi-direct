Name:       ug-setting-wifidirect-efl
Summary:    Wi-Fi Direct setting UI gadget
Version:    1.11.74
Release:    1
Group:      Applications/Network
License:    Flora-1.1
Source0:    %{name}-%{version}.tar.gz

%if "%{profile}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

Requires(post): /sbin/ldconfig
Requires(post): /usr/bin/sqlite3
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
BuildRequires:  pkgconfig(capi-ui-efl-util)
BuildRequires:  pkgconfig(efl-extension)

BuildRequires: pkgconfig(libtzplatform-config)

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

%cmake . -DCMAKE_INSTALL_PREFIX=${_prefix} \
	-DMODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE=1 \
	-DTZ_SYS_RO_APP=%TZ_SYS_RO_APP \
	-DTZ_SYS_RO_UG=%TZ_SYS_RO_UG \

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
%define tizen_sign 1
%define tizen_sign_base %{TZ_SYS_RO_APP}/org.tizen.wifi-direct-popup
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1

%post
mkdir -p %{TZ_SYS_RO_APP}/%{name}/bin/
ln -sf %{TZ_SYS_BIN}/ug-client %{TZ_SYS_RO_APP}/%{name}/bin/%{name}
ln -sf %{TZ_SYS_RO_APP}/%{name}/lib/lib%{name}.so %{TZ_SYS_RO_APP}/%{name}/lib/lib%{name}.so

%post -n org.tizen.wifi-direct-popup


%postun


%files
%manifest %{name}.manifest
%license LICENSE
%defattr(-,root,root,-)
%{TZ_SYS_RO_APP}/%{name}/lib/ug/*
%{TZ_SYS_RO_APP}/%{name}/res/edje/*
%{TZ_SYS_RO_UG}/res/locale/*/*/*
%{TZ_SYS_RO_APP}/%{name}/shared/res/tables/setting-wifidirect-efl_ChangeableColorTable.xml
%{TZ_SYS_RO_APP}/%{name}/shared/res/tables/setting-wifidirect-efl_FontInfoTable.xml
%{TZ_SYS_RO_APP}/%{name}/shared/icons/*
%{TZ_SYS_RO_PACKAGES}/%{name}.xml

%files -n org.tizen.wifi-direct-popup
%manifest org.tizen.wifi-direct-popup.manifest
%license LICENSE
%defattr(-,root,root,-)
%{TZ_SYS_RO_APP}/org.tizen.wifi-direct-popup/bin/*
%{TZ_SYS_RO_APP}/org.tizen.wifi-direct-popup/res/images/*
%{TZ_SYS_RO_APP}/org.tizen.wifi-direct-popup/res/edje/*
%{TZ_SYS_RO_UG}/res/locale/*/*/*
%{TZ_SYS_RO_APP}/org.tizen.wifi-direct-popup/author-signature.xml
%{TZ_SYS_RO_APP}/org.tizen.wifi-direct-popup/signature1.xml
%{TZ_SYS_RO_PACKAGES}/org.tizen.wifi-direct-popup.xml

