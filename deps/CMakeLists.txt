
if(NOT MINGW)
add_subdirectory(w32-pthreads)
endif()

add_subdirectory(glad)
add_subdirectory(ipc-util)
add_subdirectory(libff)
add_subdirectory(file-updater)

if(BUILD_CAPTIONS)
	add_subdirectory(libcaption)
endif()

find_package(Jansson 2.5 QUIET)

if(NOT JANSSON_FOUND)
	message(STATUS "Jansson >=2.5 not found, building bundled version")

	add_subdirectory(jansson)

	set(OBS_JANSSON_IMPORT "jansson" CACHE INTERNAL "Internal var")
	set(OBS_JANSSON_INCLUDE_DIRS "" CACHE INTERNAL "Internal var")
else()
	message(STATUS "Using system Jansson library")

	set(OBS_JANSSON_IMPORT "${JANSSON_LIBRARIES}" CACHE INTERNAL "Internal var")
	set(OBS_JANSSON_INCLUDE_DIRS "${JANSSON_INCLUDE_DIRS}" CACHE INTERNAL "Internal var")
endif()

find_package(Mbedtls 2.0 QUIET)

if(NOT MBEDTLS_FOUND)
	message(STATUS "mbedtls >=2.0 not found, building bundled version")

	#add_subdirectory(mbedtls)

	#set(OBS_MBEDTLS_IMPORT "mbedtls" CACHE INTERNAL "Internal var")
	#set(OBS_MBEDTLS_INCLUDE_DIRS "" CACHE INTERNAL "Internal var")
else()
	message(STATUS "Using system mbedtls library")

	set(OBS_MBEDTLS_IMPORT "${MBEDTLS_LIBRARIES}" CACHE INTERNAL "Internal var")
	set(OBS_MBEDTLS_INCLUDE_DIRS "${MBEDTLS_INCLUDE_DIRS}" CACHE INTERNAL "Internal var")
endif()
