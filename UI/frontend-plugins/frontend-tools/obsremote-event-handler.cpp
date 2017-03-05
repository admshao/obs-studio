#include "obsremote-event-handler.hpp"

OBSRemoteEventHandler::OBSRemoteEventHandler()
{
	obs_frontend_add_event_callback(EventHandler, this);
}

OBSRemoteEventHandler::~OBSRemoteEventHandler()
{
	obs_frontend_remove_event_callback(EventHandler, this);
}

void OBSRemoteEventHandler::EventHandler(enum obs_frontend_event event,
                                         void *me)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) me;

	switch (event) {
	case OBS_FRONTEND_EVENT_SCENE_CHANGED:
		handler->onSceneChange();
		break;
	case OBS_FRONTEND_EVENT_EXIT:
		handler->onExit();
		break;
	default:
		break;

	}
}

void OBSRemoteEventHandler::sendUpdate(const char *type, obs_data_t *msg = NULL)
{
	if (!msg)
		msg = obs_data_create();
	obs_data_set_string(msg, "update-type", type);

	updatesToSend.push_back(msg);
}

void OBSRemoteEventHandler::onSceneChange()
{
	obs_data_t *data = obs_data_create();

	obs_source_t *scene = obs_frontend_get_current_scene();
	obs_data_set_string(data, "scene-name", obs_source_get_name(scene));
	sendUpdate("SwitchScenes", data);

	obs_source_release(scene);
}

void OBSRemoteEventHandler::onExit()
{
	sendUpdate("Exit");
}
