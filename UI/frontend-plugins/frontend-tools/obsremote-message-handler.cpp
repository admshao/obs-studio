#include <obs-frontend-api.h>
#include "obsremote-message-handler.hpp"

obs_data_t *SendOkResponse(obs_data_t *ret = NULL)
{
	if (!ret)
		ret = obs_data_create();
	obs_data_set_string(ret, "status", "ok");
	return ret;
}

obs_data_t *GetErrorResponse(const char *error)
{
	obs_data_t *ret = obs_data_create();
	obs_data_set_string(ret, "status", "error");
	obs_data_set_string(ret, "error", error);
	return ret;
}

static void
obsremote_get_source_filter(obs_source_t *, obs_source_t *filter, void *param)
{
	obs_data_array_t *filters = (obs_data_array_t *) param;
	obs_data_t *filter_data = obs_data_create();

	obs_data_set_string(filter_data, "name", obs_source_get_name(filter));
	obs_data_set_int(filter_data, "type", obs_source_get_type(filter));
	obs_data_array_push_back(filters, filter_data);

	obs_data_release(filter_data);
}

static obs_data_array_t *obsremote_get_filters(obs_source_t *source)
{
	if (!source)
		return NULL;

	obs_data_array_t *filters = obs_data_array_create();
	obs_source_enum_filters(source, obsremote_get_source_filter,
	                        (void *) filters);

	return filters;
}

static bool
obsremote_get_scene_item(obs_scene_t *, obs_sceneitem_t *item, void *param)
{
	obs_data_array_t *sources = (obs_data_array_t *) param;
	obs_source_t *source = obs_sceneitem_get_source(item);

	obs_data_t *source_data = obs_data_create();
	obs_data_set_string(source_data, "name", obs_source_get_name(source));
	obs_data_set_int(source_data, "type", obs_source_get_type(source));
	obs_data_array_t *filters = obsremote_get_filters(source);
	obs_data_set_array(source_data, "filters", filters);
	obs_data_array_push_back(sources, source_data);

	obs_data_array_release(filters);
	obs_data_release(source_data);
	return true;
}

static obs_data_array_t *obsremote_get_sources(obs_source_t *source)
{
	if (!source)
		return NULL;

	obs_scene_t *scene = obs_scene_from_source(source);
	if (!scene)
		return NULL;

	obs_data_array_t *sources = obs_data_array_create();
	obs_scene_enum_items(scene, obsremote_get_scene_item, (void *)
		sources);

	return sources;
}

static obs_data_t *obsremote_get_scene_data(obs_source_t *source)
{
	obs_data_t *data = obs_data_create();
	if (!source)
		return data;

	obs_data_set_string(data, "name", obs_source_get_name(source));
	obs_data_array_t *sources = obsremote_get_sources(source);
	obs_data_set_array(data, "sources", sources);
	obs_data_array_release(sources);
	obs_data_array_t *filters = obsremote_get_filters(source);
	obs_data_set_array(data, "filters", filters);
	obs_data_array_release(filters);

	return data;
}

obs_data_t *
OBSAPIMessageHandler::HandleGetVersion(OBSAPIMessageHandler *handler,
                                       obs_data_t *message)
{
	obs_data_t *resp = obs_data_create();
	obs_data_set_double(resp, "version", OBS_REMOTE_VERSION);
	return SendOkResponse(resp);
}

obs_data_t *
OBSAPIMessageHandler::HandleGetAuthRequired(OBSAPIMessageHandler *handler,
                                            obs_data_t *message)
{
	OBSRemoteConfig *config = OBSRemoteConfig::GetInstance();
	obs_data_t *resp = obs_data_create();
	obs_data_set_bool(resp, "authRequired", config->UseAuth());

	if (config->UseAuth()) {
		obs_data_set_string(resp, "challenge",
		                    config->challenge.c_str());
		obs_data_set_string(resp, "salt", config->salt.c_str());
	}
	return SendOkResponse(resp);
}

obs_data_t *
OBSAPIMessageHandler::HandleAuthenticate(OBSAPIMessageHandler *handler,
                                         obs_data_t *message)
{
	const char *auth = obs_data_get_string(message, "auth");
	if (!auth || strlen(auth) < 1) {
		return GetErrorResponse("auth not specified!");
	}

	if (OBSRemoteConfig::GetInstance()->Auth(auth)) {
		handler->authenticated = true;
		return SendOkResponse();
	} else {
		return GetErrorResponse("Authentication Failed.");
	}
}

obs_data_t *
OBSAPIMessageHandler::HandleGetCurrentScene(OBSAPIMessageHandler *handler,
                                            obs_data_t *message)
{
	obs_source_t *current_scene = obs_frontend_get_current_scene();
	obs_data_t *resp = obsremote_get_scene_data(current_scene);
	obs_source_release(current_scene);
	return SendOkResponse(resp);
}

obs_data_t *
OBSAPIMessageHandler::HandleGetSceneList(OBSAPIMessageHandler *handler,
                                         obs_data_t *message)
{
	struct obs_frontend_source_list scenes = {};
	obs_frontend_get_scenes(&scenes);

	obs_source_t *current_scene = obs_frontend_get_current_scene();

	obs_data_t *resp = obs_data_create();
	obs_data_set_string(resp, "current-scene", obs_source_get_name
		(current_scene));

	obs_data_array_t *scenes_data = obs_data_array_create();
	for (size_t i = 0; i < scenes.sources.num; i++) {
		obs_source_t *scene = scenes.sources.array[i];
		obs_data_t *data = obsremote_get_scene_data(scene);
		obs_data_array_push_back(scenes_data, data);
		obs_data_release(data);
	}

	obs_data_set_array(resp, "scenes", scenes_data);

	obs_data_array_release(scenes_data);
	obs_source_release(current_scene);
	obs_frontend_source_list_free(&scenes);
	return resp;
}

obs_data_t *
OBSAPIMessageHandler::HandleSetCurrentScene(OBSAPIMessageHandler *handler,
                                            obs_data_t *message)
{
	obs_data_t *ret = NULL;
	const char *name = obs_data_get_string(message, "scene-name");
	obs_source_t *source = obs_get_source_by_name(name);
	if (source) {
		obs_frontend_set_current_scene(source);
		ret = SendOkResponse(ret);
	} else {
		ret = GetErrorResponse("Invalid Scene");
	}
	obs_source_release(source);
	return ret;
}

obs_data_t *
OBSAPIMessageHandler::HandleGetSceneNames(OBSAPIMessageHandler *handler,
                                          obs_data_t *message)
{
	char **names = obs_frontend_get_scene_names();
	char **op_names = names;

	obs_data_t *resp = obs_data_create();
	obs_data_array_t *names_array = obs_data_array_create();

	while (*op_names) {
		obs_data_t *data = obs_data_create();
		obs_data_set_string(data, "name", *op_names);
		obs_data_array_push_back(names_array, data);
		obs_data_release(data);
		op_names++;
	}
	obs_data_set_array(resp, "scenes", names_array);

	obs_data_array_release(names_array);
	bfree(names);
	return resp;
}

obs_data_t *
OBSAPIMessageHandler::HandleStartRecording(OBSAPIMessageHandler *handler,
                                           obs_data_t *message)
{
	obs_frontend_recording_start();
	return SendOkResponse();
}

obs_data_t *
OBSAPIMessageHandler::HandleStopRecording(OBSAPIMessageHandler *handler,
                                          obs_data_t *message)
{
	obs_frontend_recording_stop();
	return SendOkResponse();
}

obs_data_t *
OBSAPIMessageHandler::HandleRecordingActive(OBSAPIMessageHandler *handler,
                                            obs_data_t *message)
{
	obs_data_t *resp = obs_data_create();
	obs_data_set_bool(resp, "recording", obs_frontend_recording_active());
	return SendOkResponse(resp);
}

obs_data_t *
OBSAPIMessageHandler::HandleStartStreaming(OBSAPIMessageHandler *handler,
                                           obs_data_t *message)
{
	obs_frontend_streaming_start();
	return SendOkResponse();
}

obs_data_t *
OBSAPIMessageHandler::HandleStopStreaming(OBSAPIMessageHandler *handler,
                                          obs_data_t *message)
{
	obs_frontend_streaming_stop();
	return SendOkResponse();
}

obs_data_t *
OBSAPIMessageHandler::HandleStreamingActive(OBSAPIMessageHandler *handler,
                                            obs_data_t *message)
{
	obs_data_t *resp = obs_data_create();
	obs_data_set_bool(resp, "streaming", obs_frontend_streaming_active());
	return SendOkResponse(resp);
}

obs_data_t *
OBSAPIMessageHandler::HandleListSceneCollections(OBSAPIMessageHandler *handler,
                                                 obs_data_t *message)
{
	char **scene_collections = obs_frontend_get_scene_collections();
	char **collection = scene_collections;

	obs_data_t *resp = obs_data_create();
	obs_data_array_t *scene_collections_array = obs_data_array_create();

	while (*collection) {
		obs_data_t *data = obs_data_create();
		obs_data_set_string(data, "name", *collection);
		obs_data_array_push_back(scene_collections_array, data);
		obs_data_release(data);
		collection++;
	}
	obs_data_set_array(resp, "scene-collections", scene_collections_array);

	obs_data_array_release(scene_collections_array);
	bfree(scene_collections);
	return resp;
}

obs_data_t *OBSAPIMessageHandler::HandleSetCurrentSceneCollection(
	OBSAPIMessageHandler *handler, obs_data_t *message)
{
	obs_data_t *ret = NULL;
	const char *name = obs_data_get_string(message,
	                                       "scene-collection-name");
	if (name) {
		obs_frontend_set_current_scene_collection(name);
		ret = SendOkResponse(ret);
	} else {
		ret = GetErrorResponse("Invalid Scene Collection Name");
	}
	return ret;
}

obs_data_t *OBSAPIMessageHandler::HandleGetCurrentSceneCollection(
	OBSAPIMessageHandler *handler, obs_data_t *message)
{
	obs_data_t *ret = obs_data_create();
	char * name = obs_frontend_get_current_scene_collection();
	obs_data_set_string(ret, "scene-collection-name", name);
	bfree(name);
	return ret;
}

obs_data_t *
OBSAPIMessageHandler::HandleListProfiles(OBSAPIMessageHandler *handler,
                                         obs_data_t *message)
{
	char **profiles = obs_frontend_get_profiles();
	char **profile = profiles;

	obs_data_t *resp = obs_data_create();
	obs_data_array_t *profile_array = obs_data_array_create();

	while (*profile) {
		obs_data_t *data = obs_data_create();
		obs_data_set_string(data, "name", *profile);
		obs_data_array_push_back(profile_array, data);
		obs_data_release(data);
		profile++;
	}
	obs_data_set_array(resp, "profiles", profile_array);

	obs_data_array_release(profile_array);
	bfree(profiles);
	return resp;
}

obs_data_t *
OBSAPIMessageHandler::HandleSetCurrentProfile(OBSAPIMessageHandler *handler,
                                              obs_data_t *message)
{
	obs_data_t *ret = NULL;
	const char *name = obs_data_get_string(message, "profile-name");
	if (name) {
		obs_frontend_set_current_profile(name);
		ret = SendOkResponse(ret);
	} else {
		ret = GetErrorResponse("Invalid Profile Name");
	}
	return ret;
}

obs_data_t *
OBSAPIMessageHandler::HandleGetCurrentProfile(OBSAPIMessageHandler *handler,
                                              obs_data_t *message)
{
	obs_data_t *ret = obs_data_create();
	char * name = obs_frontend_get_current_profile();
	obs_data_set_string(ret, "profile-name", name);
	bfree(name);
	return ret;
}

bool OBSAPIMessageHandler::HandleReceivedMessage(void *in, size_t len)
{
	obs_data_t *message = obs_data_create_from_json((char *) in);
	if (!message) {
		return false;
	}
	const char *type = obs_data_get_string(message, "request-type");
	const char *id = obs_data_get_string(message, "message-id");
	if (!type) {
		this->messagesToSend.push_back(
			GetErrorResponse("message type not specified"));
		obs_data_release(message);
		return true;
	}

	obs_data_t *(*messageFunc)(OBSAPIMessageHandler *, obs_data_t *) =
	(messageMap[type]);
	obs_data_t *ret = NULL;

	if (messageFunc) {
		if (!OBSRemoteConfig::GetInstance()->UseAuth() ||
		    this->authenticated ||
		    messagesNotRequiringAuth.find(type) !=
		    messagesNotRequiringAuth.end()) {
			ret = messageFunc(this, message);
		} else {
			ret = GetErrorResponse("Not Authenticated");
		}
	} else {
		this->messagesToSend.push_back(
			GetErrorResponse("message type not recognized"));
		obs_data_release(message);
		return true;
	}

	if (ret) {
		if (id) {
			obs_data_set_string(ret, "message-id", id);
		}

		this->messagesToSend.push_back(ret);
	} else {
		this->messagesToSend.push_back(
			GetErrorResponse("no response given"));
	}

	obs_data_release(message);
	return true;
}

OBSAPIMessageHandler::OBSAPIMessageHandler()
{
	messageMap[REQ_GET_VERSION] = OBSAPIMessageHandler::HandleGetVersion;
	messageMap[REQ_GET_AUTH_REQUIRED] =
		OBSAPIMessageHandler::HandleGetAuthRequired;
	messageMap[REQ_AUTHENTICATE] = OBSAPIMessageHandler::HandleAuthenticate;

	messageMap[REQ_GET_CURRENT_SCENE] =
		OBSAPIMessageHandler::HandleGetCurrentScene;
	messageMap[REQ_GET_SCENE_LIST] =
		OBSAPIMessageHandler::HandleGetSceneList;
	messageMap[REQ_GET_SCENE_NAMES] =
		OBSAPIMessageHandler::HandleGetSceneNames;
	messageMap[REQ_SET_CURRENT_SCENE] =
		OBSAPIMessageHandler::HandleSetCurrentScene;

	messageMap[REQ_START_RECORDING] =
		OBSAPIMessageHandler::HandleStartRecording;
	messageMap[REQ_STOP_RECORDING] =
		OBSAPIMessageHandler::HandleStopRecording;
	messageMap[REQ_RECORDING_ACTIVE] =
		OBSAPIMessageHandler::HandleRecordingActive;
	messageMap[REQ_START_STREAMING] =
		OBSAPIMessageHandler::HandleStartStreaming;
	messageMap[REQ_STOP_STREAMING] =
		OBSAPIMessageHandler::HandleStopStreaming;
	messageMap[REQ_STREAMING_ACTIVE] =
		OBSAPIMessageHandler::HandleStreamingActive;

	messageMap[REQ_LIST_PROFILES] =
		OBSAPIMessageHandler::HandleListProfiles;
	messageMap[REQ_SET_CURRENT_PROFILE] =
		OBSAPIMessageHandler::HandleSetCurrentProfile;
	messageMap[REQ_GET_CURRENT_PROFILE] =
		OBSAPIMessageHandler::HandleGetCurrentProfile;

	messageMap[REQ_LIST_SCENE_COLLECTIONS] =
		OBSAPIMessageHandler::HandleListSceneCollections;
	messageMap[REQ_SET_CURRENT_SCENE_COLLECTION] =
		OBSAPIMessageHandler::HandleSetCurrentSceneCollection;
	messageMap[REQ_GET_CURRENT_SCENE_COLLECTION] =
		OBSAPIMessageHandler::HandleGetCurrentSceneCollection;

	messagesNotRequiringAuth.insert(REQ_GET_VERSION);
	messagesNotRequiringAuth.insert(REQ_GET_AUTH_REQUIRED);
	messagesNotRequiringAuth.insert(REQ_AUTHENTICATE);

	authenticated = false;
}
