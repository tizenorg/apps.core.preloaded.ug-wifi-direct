CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED appcore-efl appsvc ui-gadget x11 dlog
	elementary ecore-x
	network pmapi
	wifi-direct
	)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

ADD_SUBDIRECTORY(popup-wifidirect)
ADD_SUBDIRECTORY(ug-wifidirect)

