CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-setting-wifidirect-efl C)
SET(PACKAGE_NAME ug-setting-wifidirect-efl)

SET(PREFIX "/opt/ug")
SET(LIBDIR "${PREFIX}/lib")
SET(RESDIR "${PREFIX}/res")
SET(EDJDIR "${RESDIR}/edje/${PROJECT_NAME}") 
SET(IMGDIR "${RESDIR}/images/${PROJECT_NAME}") 

SET(SRCS 
	src/wfd_ug.c
	src/wfd_client.c
	src/wfd_ug_popup.c
	src/wfd_ug_genlist.c
	src/wfd_ug_main_view.c
	src/wfd_ug_multiconnect_view.c
	src/wfd_ug_about_view.c
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)


SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -fvisibility=hidden -Wall")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "-O2")

ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DRESDIR=\"${RESDIR}\"")

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 0.1.0)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

ADD_CUSTOM_TARGET(wfd_ug.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/resources/images
                ${CMAKE_CURRENT_SOURCE_DIR}/resources/wfd_ug.edc ${CMAKE_BINARY_DIR}/wfd_ug.edj
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/wfd_ug.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} wfd_ug.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/wfd_ug.edj DESTINATION ${EDJDIR})
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/images/ DESTINATION ${IMGDIR} FILES_MATCHING PATTERN "*.png")

ADD_SUBDIRECTORY(po)
