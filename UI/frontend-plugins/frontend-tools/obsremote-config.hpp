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
#define REQ_GET_CURRENT_SCENE "GetCurrentScene"
#define REQ_GET_SCENE_LIST "GetSceneList"
#define REQ_GET_SCENE_NAMES "GetSceneNames"
#define REQ_SET_CURRENT_SCENE "SetCurrentScene"

using namespace std;

class OBSRemoteConfig
{
public:
	const char *getChallenge();

	bool UseAuth();

	bool Auth(const char *response);

	void SetPassword(string newpass);

	static OBSRemoteConfig *GetInstance();

	OBSRemoteConfig();

	~OBSRemoteConfig();

	string challenge = "";
	string secret = "";
	string salt = "";
	string password = "";

	bool enabled = false;
	int port = DEFAULT_PORT;

	bool listenToChanges = false;

private:
	static OBSRemoteConfig *_instance;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context rng;
};
