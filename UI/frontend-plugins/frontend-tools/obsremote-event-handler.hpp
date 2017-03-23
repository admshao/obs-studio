#pragma once

#include "obsremote-functions.hpp"
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

	void handleGlobalAudioSignals();
	void handleSceneSignals(obs_source_t *scene);

	static void
	EventHandler(enum obs_frontend_event event, void *me);

private:
	signal_handler_t *scene_signal = nullptr;

	static void onSceneItemVisible(void *me, calldata_t *data);
	static void onSceneItemAdd(void *me, calldata_t *data);
	static void onSceneItemRemove(void *me, calldata_t *data);
	static void onSceneItemReorder(void *me, calldata_t *data);
	static void onSceneItemSelect(void *me, calldata_t *data);
	static void onSceneItemDeselect(void *me, calldata_t *data);
	static void onSourceRename(void *me, calldata_t *data);
	static void onSourceVolume(void *me, calldata_t *data);
	static void onSourceMute(void *me, calldata_t *data);

	void sendUpdate(const char *type, obs_data_t *msg);

	void onRecordingStarting();
	void onRecordingStarted();
	void onRecordingStopping();
	void onRecordingStopped();
	void onStreamingStarting();
	void onStreamingStarted();
	void onStreamingStopping();
	void onStreamingStopped();
	void onProfileChanged();
	void onProfileListChanged();
	void onSceneCollectionChanged();
	void onSceneCollectionListChanged();
	void onSceneListChanged();
	void onSceneChanged();
	void onGlobalAudioSourcesChanged();
	void onExit();
};
