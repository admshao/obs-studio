# Once done these will be defined:
#
#  LIBWEBSOCKETS_FOUND
#  LIBWEBSOCKETS_INCLUDE_DIRS
#  LIBWEBSOCKETS_LIBRARIES
#  LIBWEBSOCKETS_VERSION

find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
	pkg_check_modules(_LIBWEBSOCKETS QUIET libwebsockets)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_lib_suffix 64)
else()
	set(_lib_suffix 32)
endif()

find_path(Libwebsockets_INCLUDE_DIR
	NAMES libwebsockets.h
	HINTS
		ENV LibwebsocketsPath${_lib_suffix}
		ENV LibwebsocketsPath
		ENV DepsPath${_lib_suffix}
		ENV DepsPath
		${LibwebsocketsPath${_lib_suffix}}
		${LibwebsocketsPath}
		${DepsPath${_lib_suffix}}
		${DepsPath}
		${_LIBWEBSOCKETS_INCLUDE_DIRS}
	PATHS
		/usr/include /usr/local/include /opt/local/include /sw/include)

find_library(Libwebsockets_LIB
	NAMES ${_LIBWEBSOCKETS_LIBRARIES} libwebsockets
	HINTS
		ENV LibwebsocketsPath${_lib_suffix}
		ENV LibwebsocketsPath
		ENV DepsPath${_lib_suffix}
		ENV DepsPath
		${LibwebsocketsPath${_lib_suffix}}
		${LibwebsocketsPath}
		${DepsPath${_lib_suffix}}
		${DepsPath}
		${_LIBWEBSOCKETS_LIBRARY_DIRS}
	PATHS
		/usr/lib /usr/local/lib /opt/local/lib /sw/lib
	PATH_SUFFIXES
		lib${_lib_suffix} lib
		libs${_lib_suffix} libs
		bin${_lib_suffix} bin
		../lib${_lib_suffix} ../lib
		../libs${_lib_suffix} ../libs
		../bin${_lib_suffix} ../bin)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libwebsockets
	FOUND_VAR LIBWEBSOCKETS_FOUND
	REQUIRED_VARS Libwebsockets_LIB Libwebsockets_INCLUDE_DIR
	VERSION_VAR _LIBWEBSOCKETS_VERSION_STRING)
mark_as_advanced(Libwebsockets_INCLUDE_DIR Libwebsockets_LIB)

if(LIBWEBSOCKETS_FOUND)
	set(LIBWEBSOCKETS_INCLUDE_DIRS ${Libwebsockets_INCLUDE_DIR})
	set(LIBWEBSOCKETS_LIBRARIES ${Libwebsockets_LIB})
	set(LIBWEBSOCKETS_VERSION ${_LIBWEBSOCKETS_VERSION_STRING})
endif()

