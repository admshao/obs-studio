#pragma once

#include <obs-frontend-api.h>

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>
#include <string>

#define OBS_REMOTE_VERSION 1.2

#define CONFIG_NAME "obsremote"
#define PARAM_ENABLED "enabled"
#define PARAM_PORT "port"
#define PARAM_PASSWORD "password"
#define PARAM_SECRET "secret"
#define PARAM_SALT "salt"

#define DEFAULT_PORT 4444

#define REQ_GET_VERSION "GetVersion"
#define REQ_GET_AUTH_REQUIRED "GetAuthRequired"
#define REQ_AUTHENTICATE "Authenticate"

#define REQ_GET_SCENE_LIST "GetSceneList"
#define REQ_GET_SCENE_NAMES "GetSceneNames"
#define REQ_GET_CURRENT_SCENE "GetCurrentScene"
#define REQ_SET_CURRENT_SCENE "SetCurrentScene"

#define REQ_START_RECORDING "StartRecording"
#define REQ_STOP_RECORDING "StopRecording"
#define REQ_RECORDING_ACTIVE "RecordingActive"
#define REQ_START_STREAMING "StartStreaming"
#define REQ_STOP_STREAMING "StopStreaming"
#define REQ_STREAMING_ACTIVE "StreamingActive"

#define REQ_LIST_PROFILES "ListProfiles"
#define REQ_SET_CURRENT_PROFILE "SetCurrentProfile"
#define REQ_GET_CURRENT_PROFILE "GetCurrentProfile"

#define REQ_LIST_SCENE_COLLECTIONS "ListSceneCollections"
#define REQ_SET_CURRENT_SCENE_COLLECTION "SetCurrentSceneCollection"
#define REQ_GET_CURRENT_SCENE_COLLECTION "GetCurrentSceneCollection"

#define REQ_GET_GLOBAL_AUDIO_LIST "GetGlobalAudioList"
#define REQ_SET_VOLUME "SetVolume"
#define REQ_GET_VOLUME "GetVolume"
#define REQ_SET_MUTED "SetMuted"

using namespace std;

class OBSRemoteConfig
{
public:
	OBSRemoteConfig();
	~OBSRemoteConfig();

	const char *getChallenge();
	bool UseAuth();
	bool Auth(const char *response);
	void SetPassword(string newpass);
	static OBSRemoteConfig *GetInstance();

	string challenge;
	string secret;
	string salt;
	string password;

	bool listenToChanges = false;
	bool enabled = false;
	int port = DEFAULT_PORT;

private:
	static OBSRemoteConfig *_instance;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context rng;
};
