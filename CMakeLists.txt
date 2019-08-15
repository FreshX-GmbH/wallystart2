CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(nwallyd)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH on)
set(MACOSX_RPATH on)

set (CMAKE_INSTALL_PREFIX "" CACHE PATH "default install path" FORCE )

# Compile video support (requires libffmpeg)
set(WITH_VIDEO off)

# We want to gently wait for events or poll agressivly (more performant)
set(WITH_KMS_DEMO off)

# Set number of compile cpu's
set(MULTITHREADED_BUILD 1)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DLINUX -DWALLYSTART")

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(PkgConfig)
pkg_check_modules(LIBSDL sdl2>=2.0)
pkg_check_modules(LIBSDLTTF SDL2_ttf>=2.0)
pkg_check_modules(LIBSDLIMG SDL2_image>=2.0)
set(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j${MULTITHREADED_BUILD}")

string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\1" DARWIN_MAJOR_VERSION "${CMAKE_SYSTEM_VERSION}")
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\2" DARWIN_MINOR_VERSION "${CMAKE_SYSTEM_VERSION}")
message("OS Version : " ${CMAKE_SYSTEM_VERSION})
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
 if("${DARWIN_MAJOR_VERSION}" LESS 16)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DWITHOUT_CLOCK_GETTIME")
 endif()
endif()

include(CheckIncludeFile)
CHECK_INCLUDE_FILE(bcm_host.h RASPBERRY)


if("${RASPBERRY}" STREQUAL "1")
    set(RPI_C_FLAGS  "-DRASPBERRY")
    set(RPI_LD_FLAGS  "-lbcm_host -lvchostif -lvcos -lvchiq_arm -lpthread -lm")
else()
    set(RASPBERRY 0)
endif()

message("RPI : " ${RASPBERRY})

if(WITH_VIDEO STREQUAL on)
	set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DWITH_VIDEO")
	pkg_check_modules(LIBSWRE libswresample)
	pkg_check_modules(LIBAVUTIL libavutil)
	pkg_check_modules(LIBAVCODEC libavcodec)
	pkg_check_modules(LIBAVFORMAT libavformat)
	pkg_check_modules(LIBSWSCALE libswscale)
	set(FFMPEG_LIBRARIES ${LIBSWRE_LIBRARIES} ${LIBAVUTIL_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBSWSCALE_LIBRARIES})
	include_directories(${LIBSWRE_INCLUDE_DIRS} ${LIBAVUTIL_INCLUDE_DIRS} ${LIBAVCODEC_INCLUDE_DIRS} ${LIBAVFORMAT_INCLUDE_DIRS} ${LIBSWSCALE_INCLUDE_DIRS})
endif()

include_directories(${LIBSDL_INCLUDE_DIRS} src)
link_directories(${LIBSDL_LIBRARY_DIRS})

# TODO : Build Versioning
file (STRINGS "version" CURR_BUILD_NUMBER)
execute_process(COMMAND bin/incrementVersion ${CURR_BUILD_NUMBER} RESULT_VARIABLE CMAKE_BUILD_NUMBER OUTPUT_FILE version)
file (STRINGS "version" CMAKE_BUILD_NUMBER)

string(TIMESTAMP CMAKE_BUILD_DATE "%Y-%m-%d")

set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE STRING "Directory where lib will install")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Directory where lib will install")
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/etc" CACHE PATH "The directory the configs are installed in")

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD") 
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DDARWIN")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DDARWIN")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux") 
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DLINUX")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DWINDOWS")
endif()

add_subdirectory(src)

set(PROJECT_DESCRIPTION "Wally Daemon")

set(SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc" CACHE PATH "System configuration directory")
set(PREFSDIR ${SYSCONFDIR}/${CMAKE_PROJECT_NAME}/wallyd.d)

configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/etc/wallyd.conf.in" "${CMAKE_CURRENT_SOURCE_DIR}/etc/wallyd.conf")
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/src/wally-release.in" "${CMAKE_CURRENT_SOURCE_DIR}/etc/wally-release")
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/src/autoversion.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/autoversion.h" )
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/etc/wallyd.conf" DESTINATION ${SYSCONFDIR})

if("${RASPBERRY}" STREQUAL "1")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/etc/wallystart.conf.pi" DESTINATION "${SYSCONFDIR}")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/etc/wallystart.conf.recovery.pi" DESTINATION "${SYSCONFDIR}")
  execute_process(COMMAND "mv" "wallystart.conf.pi wallystart.conf" WORKING_DIRECTORY "${SYSCONFDIR}/etc")
  execute_process(COMMAND "mv" "wallystart.conf.recovery.pi wallystart.conf.recovery" WORKING_DIRECTORY "${SYSCONFDIR}/etc")
else()
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/etc/wallystart.conf" DESTINATION ${SYSCONFDIR})
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/etc/wallystart.conf.recovery" DESTINATION ${SYSCONFDIR})
endif()
install(DIRECTORY etc/wallyd.d DESTINATION ${SYSCONFDIR})

