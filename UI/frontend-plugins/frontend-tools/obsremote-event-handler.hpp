#pragma once

#include "obsremote-config.hpp"
#include "obsremote.hpp"

#include <deque>

using namespace std;

class OBSRemoteEventHandler
{
public:
	OBSRemoteEventHandler();
	~OBSRemoteEventHandler();

	deque<obs_data_t *> updatesToSend;

	static void
	EventHandler(enum obs_frontend_event event, void *me);

private:
	void sendUpdate(const char *type, obs_data_t *msg);

	void onRecordingStarting();
	void onRecordingStarted();
	void onRecordingStopping();
	void onRecordingStopped();
	void onSceneChange();
	void onExit();
};
