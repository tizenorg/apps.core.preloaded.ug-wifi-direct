CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED appcore-efl appsvc ui-gadget-1 x11 dlog
	elementary ecore-x
	capi-network-tethering network pmapi
	wifi-direct
	notification
	capi-appfw-application
	syspopup-caller
	)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

ADD_SUBDIRECTORY(popup-wifidirect)
ADD_SUBDIRECTORY(ug-wifidirect)
ADD_SUBDIRECTORY(ugapp-wifidirect)

