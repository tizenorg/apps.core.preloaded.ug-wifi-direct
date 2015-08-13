CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED appcore-efl appsvc ui-gadget-1 dlog
	elementary
	capi-network-tethering
	network
	capi-network-wifi-direct
	notification
	capi-appfw-application
	feedback
	dbus-1
	deviced efl-assist
	capi-ui-efl-util
	efl-extension
	)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

ADD_SUBDIRECTORY(popup-wifidirect)
ADD_SUBDIRECTORY(ug-wifidirect)

