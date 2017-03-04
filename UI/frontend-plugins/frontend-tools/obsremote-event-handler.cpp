#include "obsremote-event-handler.hpp"

OBSRemoteEventHandler::OBSRemoteEventHandler(OBSRemote *obsr)
{
	this->obsr = obsr;
	obs_frontend_add_event_callback(EventHandler, this);
}

void OBSRemoteEventHandler::EventHandler(enum obs_frontend_event event,
                                         void *private_data)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) private_data;
	//if (event == OBS_FRONTEND_EVENT_EXIT)
		//onExit();
}
