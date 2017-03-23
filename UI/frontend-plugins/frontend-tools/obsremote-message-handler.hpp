#pragma once

#include <deque>
#include <string>
#include <map>
#include <set>

#include "obsremote-functions.hpp"
#include "obsremote-config.hpp"

using namespace std;

struct OBSAPIMessageHandler
{
	OBSAPIMessageHandler();

	string ip;
	string name;

	bool authenticated;
	map<string, obs_data_t *(*)(OBSAPIMessageHandler *, obs_data_t *)>
		messageMap;
	set<string> messagesNotRequiringAuth;
	deque<obs_data_t *> messagesToSend;

	static obs_data_t *
	HandleGetVersion(OBSAPIMessageHandler *handler, obs_data_t *message);
	static obs_data_t *
	HandleGetAuthRequired(OBSAPIMessageHandler *handler,
	                      obs_data_t *message);
	static obs_data_t *
	HandleAuthenticate(OBSAPIMessageHandler *handler,
	                   obs_data_t *message);
	static obs_data_t *
	HandleGetCurrentScene(OBSAPIMessageHandler *handler,
	                      obs_data_t *message);
	static obs_data_t *
	HandleGetSceneList(OBSAPIMessageHandler *handler,
	                   obs_data_t *message);
	static obs_data_t *
	HandleSetCurrentScene(OBSAPIMessageHandler *handler,
	                      obs_data_t *message);
	static obs_data_t *
	HandleGetSceneNames(OBSAPIMessageHandler *handler, obs_data_t *message);

	static obs_data_t *
	HandleStartRecording(OBSAPIMessageHandler *handler,
	                     obs_data_t *message);

	static obs_data_t *
	HandleStopRecording(OBSAPIMessageHandler *handler, obs_data_t *message);

	static obs_data_t *
	HandleRecordingActive(OBSAPIMessageHandler *handler,
	                      obs_data_t *message);

	static obs_data_t *
	HandleStartStreaming(OBSAPIMessageHandler *handler,
	                     obs_data_t *message);

	static obs_data_t *
	HandleStopStreaming(OBSAPIMessageHandler *handler, obs_data_t *message);

	static obs_data_t *
	HandleStreamingActive(OBSAPIMessageHandler *handler,
	                      obs_data_t *message);

	static obs_data_t *
	HandleListSceneCollections(OBSAPIMessageHandler *handler,
	                           obs_data_t *message);

	static obs_data_t *
	HandleSetCurrentSceneCollection(OBSAPIMessageHandler *handler,
	                                obs_data_t *message);

	static obs_data_t *
	HandleGetCurrentSceneCollection(OBSAPIMessageHandler *handler,
	                                obs_data_t *message);

	static obs_data_t *
	HandleListProfiles(OBSAPIMessageHandler *handler,
	                   obs_data_t *message);

	static obs_data_t *
	HandleSetCurrentProfile(OBSAPIMessageHandler *handler,
	                        obs_data_t *message);

	static obs_data_t *
	HandleGetCurrentProfile(OBSAPIMessageHandler *handler,
	                        obs_data_t *message);

	static obs_data_t *
	HandleGetGlobalAudioList(OBSAPIMessageHandler *handler,
	                         obs_data_t *message);

	static obs_data_t *
	HandleSetVolume(OBSAPIMessageHandler *handler,
	                obs_data_t *message);

	static obs_data_t *
	HandleGetVolume(OBSAPIMessageHandler *handler,
	                obs_data_t *message);

	static obs_data_t *
	HandleSetMuted(OBSAPIMessageHandler *handler,
	               obs_data_t *message);

	bool HandleReceivedMessage(void *in, size_t len);

};