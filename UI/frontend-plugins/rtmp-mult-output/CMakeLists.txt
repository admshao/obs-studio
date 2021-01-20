project(rtmp-mult-output)


set(rtmp-mult-output_HEADERS
	${rtmp-mult-output_HEADERS}
	edit-widget.h
	push-widget.h
	obs-multi-rtmp.h
	)
set(rtmp-mult-output_SOURCES
	${rtmp-mult-output_SOURCES}
	edit-widget.cpp
	push-widget.cpp
	obs-multi-rtmp.cpp
	)

add_library(rtmp-mult-output MODULE
	${rtmp-mult-output_HEADERS}
	${rtmp-mult-output_SOURCES}
	)
target_link_libraries(rtmp-mult-output
	${frontend-tools_PLATFORM_LIBS}
	obs-frontend-api
	Qt5::Widgets
	libobs)

set_target_properties(rtmp-mult-output PROPERTIES FOLDER "frontend")

install_obs_plugin_with_data(rtmp-mult-output data)