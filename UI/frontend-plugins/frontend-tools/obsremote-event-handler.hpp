#pragma once

#include "obsremote-config.hpp"
#include "obsremote.hpp"

class OBSRemoteEventHandler
{
public:
	OBSRemoteEventHandler(OBSRemote *obsr);

	static void
	EventHandler(enum obs_frontend_event event, void *private_data);

private:
	OBSRemote *obsr;
};
