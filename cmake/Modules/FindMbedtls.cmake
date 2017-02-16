# Once done these will be defined:
#
#  MBEDTLS_FOUND
#  MBEDTLS_INCLUDE_DIRS
#  MBEDTLS_LIBRARIES
#  MBEDTLS_VERSION

find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
	pkg_check_modules(_MBEDTLS QUIET mbedtls)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_lib_suffix 64)
else()
	set(_lib_suffix 32)
endif()

find_path(Mbedtls_INCLUDE_DIR
	NAMES mbedtls/sha256.h mbedtls/base64.h
	HINTS
		ENV MbedtlsPath${_lib_suffix}
		ENV MbedtlsPath
		ENV DepsPath${_lib_suffix}
		ENV DepsPath
		${MbedtlsPath${_lib_suffix}}
		${MbedtlsPath}
		${DepsPath${_lib_suffix}}
		${DepsPath}
		${_MBEDTLS_INCLUDE_DIRS}
	PATHS
		/usr/include /usr/local/include /opt/local/include /sw/include)

find_library(Mbedtls_LIB
	NAMES ${_MBEDTLS_LIBRARIES} mbedcrypto
	HINTS
		ENV MbedtlsPath${_lib_suffix}
		ENV MbedtlsPath
		ENV DepsPath${_lib_suffix}
		ENV DepsPath
		${MbedtlsPath${_lib_suffix}}
		${MbedtlsPath}
		${DepsPath${_lib_suffix}}
		${DepsPath}
		${_MBEDTLS_LIBRARY_DIRS}
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
find_package_handle_standard_args(Mbedtls
	FOUND_VAR MBEDTLS_FOUND
	REQUIRED_VARS Mbedtls_LIB Mbedtls_INCLUDE_DIR
	VERSION_VAR _MBEDTLS_VERSION_STRING)
mark_as_advanced(Mbedtls_INCLUDE_DIR Mbedtls_LIB)

if(MBEDTLS_FOUND)
	set(MBEDTLS_INCLUDE_DIRS ${Mbedtls_INCLUDE_DIR})
	set(MBEDTLS_LIBRARIES ${Mbedtls_LIB})
	set(MBEDTLS_VERSION ${_MBEDTLS_VERSION_STRING})
else()
	set(MBEDTLS_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/deps/mbedtls/include/)
endif()
