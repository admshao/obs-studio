#include <obs-module.h>
#include "frontend-tools-config.h"
#include <libwebsockets.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("frontend-tools", "en-US")

#if defined(_WIN32) || defined(__APPLE__)
void InitSceneSwitcher();
void FreeSceneSwitcher();
#endif

#if defined(_WIN32) && BUILD_CAPTIONS
void InitCaptions();
void FreeCaptions();
#endif

void InitOutputTimer();
void FreeOutputTimer();

bool obs_module_load(void)
{
	struct lws_context *context;
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);
	info.port = 4444;
	context = lws_create_context(&info);
	if (context == NULL) {
		lwsl_err("libwebsocket init failed\n");
		return -1;
	}

	lws_context_destroy(context);

	lwsl_notice("libwebsockets-test-server exited cleanly\n");
#if defined(_WIN32) || defined(__APPLE__)
	InitSceneSwitcher();
#endif
#if defined(_WIN32) && BUILD_CAPTIONS
	InitCaptions();
#endif
	InitOutputTimer();
	return true;
}

void obs_module_unload(void)
{
#if defined(_WIN32) || defined(__APPLE__)
	FreeSceneSwitcher();
#endif
#if defined(_WIN32) && BUILD_CAPTIONS
	FreeCaptions();
#endif
	FreeOutputTimer();
}
