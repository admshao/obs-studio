#include "obsremote-event-handler.hpp"

OBSRemoteEventHandler::OBSRemoteEventHandler()
{
	obs_frontend_add_event_callback(EventHandler, this);

	handleGlobalAudioSignals();

	obs_source_t *scene = obs_frontend_get_current_scene();
	handleSceneSignals(scene);
	obs_source_release(scene);
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
	case OBS_FRONTEND_EVENT_RECORDING_STARTING:
		handler->onRecordingStarting();
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STARTED:
		handler->onRecordingStarted();
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STOPPING:
		handler->onRecordingStopping();
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
		handler->onRecordingStopped();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STARTING:
		handler->onStreamingStarting();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STARTED:
		handler->onStreamingStarted();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
		handler->onStreamingStopping();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
		handler->onStreamingStopped();
		break;
	case OBS_FRONTEND_EVENT_PROFILE_CHANGED:
		handler->onProfileChanged();
		break;
	case OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED:
		handler->onProfileListChanged();
		break;
	case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:
		handler->onSceneCollectionChanged();
		break;
	case OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED:
		handler->onSceneCollectionListChanged();
		break;
	case OBS_FRONTEND_EVENT_SCENE_CHANGED:
		handler->onSceneChanged();
		break;
	case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
		handler->onSceneListChanged();
		break;
	case OBS_FRONTEND_EVENT_GLOBAL_AUDIO_SOURCES_CHANGED:
		handler->onGlobalAudioSourcesChanged();
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

void OBSRemoteEventHandler::onRecordingStarting()
{
	sendUpdate("RecordingStarting");
}

void OBSRemoteEventHandler::onRecordingStarted()
{
	sendUpdate("RecordingStarted");
}

void OBSRemoteEventHandler::onRecordingStopping()
{
	sendUpdate("RecordingStopping");
}

void OBSRemoteEventHandler::onRecordingStopped()
{
	sendUpdate("RecordingStopped");
}

void OBSRemoteEventHandler::onStreamingStarting()
{
	sendUpdate("StreamingStarting");
}

void OBSRemoteEventHandler::onStreamingStarted()
{
	sendUpdate("StreamingStarted");
}

void OBSRemoteEventHandler::onStreamingStopping()
{
	sendUpdate("StreamingStopping");
}

void OBSRemoteEventHandler::onStreamingStopped()
{
	sendUpdate("StreamingStopped");
}

void OBSRemoteEventHandler::onProfileChanged()
{
	sendUpdate("ProfileChanged");
}

void OBSRemoteEventHandler::onProfileListChanged()
{
	sendUpdate("ProfileListChanged");
}

void OBSRemoteEventHandler::onSceneCollectionChanged()
{
	sendUpdate("SceneCollectionChanged");

	scene_signal = nullptr;

	onSceneListChanged();
	onSceneChanged();
}

void OBSRemoteEventHandler::onSceneCollectionListChanged()
{
	sendUpdate("SceneCollectionListChanged");
}

void OBSRemoteEventHandler::onSceneListChanged()
{
	sendUpdate("ScenesChanged");
}

void OBSRemoteEventHandler::onSceneChanged()
{
	obs_data_t *data = obs_data_create();

	obs_source_t *scene = obs_frontend_get_current_scene();
	obs_data_set_string(data, "scene-name", obs_source_get_name(scene));
	sendUpdate("SwitchScenes", data);


	handleSceneSignals(scene);
	obs_source_release(scene);
}

void OBSRemoteEventHandler::onGlobalAudioSourcesChanged()
{
	sendUpdate("GlobalAudioSourcesChanged");
	handleGlobalAudioSignals();
}

void OBSRemoteEventHandler::onExit()
{
	sendUpdate("Exit");
}

void OBSRemoteEventHandler::handleGlobalAudioSignals()
{
	struct obs_frontend_source_list audio_sources = {};
	obs_frontend_get_global_audio_sources(&audio_sources);

	for (size_t i = 0; i < audio_sources.sources.num; i++) {
		obs_source_t *source = audio_sources.sources.array[i];
		signal_handler_t *signal = obs_source_get_signal_handler
			(source);
		if (signal) {
			signal_handler_disconnect(signal, "volume",
			                          onSourceVolume, this);
			signal_handler_disconnect(signal, "mute", onSourceMute,
			                          this);
		}
		signal_handler_connect(signal, "volume", onSourceVolume, this);
		signal_handler_connect(signal, "mute", onSourceMute, this);
	}

	obs_frontend_source_list_free(&audio_sources);
}

void OBSRemoteEventHandler::handleSceneSignals(obs_source_t *source)
{
	obs_scene_t *scene = obs_scene_from_source(source);
	if (scene_signal) {
		signal_handler_disconnect(scene_signal, "item_visible",
		                          onSceneItemVisible, this);
		signal_handler_disconnect(scene_signal, "item_add",
		                          onSceneItemAdd, this);
		signal_handler_disconnect(scene_signal, "item_remove",
		                          onSceneItemRemove,
		                          this);
		signal_handler_disconnect(scene_signal, "item_select",
		                          onSceneItemSelect,
		                          this);
		signal_handler_disconnect(scene_signal, "item_deselect",
		                          onSceneItemDeselect,
		                          this);
		signal_handler_disconnect(scene_signal, "reorder",
		                          onSceneItemReorder,
		                          this);

		obs_scene_enum_items(scene, [](obs_scene_t *scene,
		                               obs_sceneitem_t *item, void *p)
		{
			OBSRemoteEventHandler *obsreh =
				(OBSRemoteEventHandler *) p;
			signal_handler_disconnect
				(obs_source_get_signal_handler
					 (obs_sceneitem_get_source(item)),
				 "rename", onSourceRename, obsreh);
			signal_handler_disconnect
				(obs_source_get_signal_handler
					 (obs_sceneitem_get_source(item)),
				 "volume", onSourceVolume, obsreh);
			signal_handler_disconnect
				(obs_source_get_signal_handler
					 (obs_sceneitem_get_source(item)),
				 "mute", onSourceMute, obsreh);
			return true;
		}, this);

	}

	scene_signal = obs_source_get_signal_handler(source);
	signal_handler_connect(scene_signal, "item_visible", onSceneItemVisible,
	                       this);
	signal_handler_connect(scene_signal, "item_add", onSceneItemAdd, this);
	signal_handler_connect(scene_signal, "item_remove", onSceneItemRemove,
	                       this);
	signal_handler_connect(scene_signal, "item_select",
	                       onSceneItemSelect,
	                       this);
	signal_handler_connect(scene_signal, "item_deselect",
	                       onSceneItemDeselect,
	                       this);
	signal_handler_connect(scene_signal, "reorder", onSceneItemReorder,
	                       this);
	obs_scene_enum_items(scene, [](obs_scene_t *scene,
	                               obs_sceneitem_t *item, void *p)
	{
		OBSRemoteEventHandler *obsreh = (OBSRemoteEventHandler *) p;
		signal_handler_connect(obs_source_get_signal_handler(
			obs_sceneitem_get_source(item)), "rename",
		                       onSourceRename, obsreh);
		signal_handler_connect(obs_source_get_signal_handler(
			obs_sceneitem_get_source(item)), "volume",
		                       onSourceVolume, obsreh);
		signal_handler_connect(obs_source_get_signal_handler(
			obs_sceneitem_get_source(item)), "mute",
		                       onSourceMute, obsreh);
		return true;
	}, this);
}

void OBSRemoteEventHandler::onSceneItemReorder(void *me, calldata_t *data)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) me;

	obs_scene_t *scene = nullptr;
	calldata_get_ptr(data, "scene", &scene);

	obs_source_t *source = obs_scene_get_source(scene);

	obs_data_t *ret = obs_data_create();
	obs_data_array_t *sources = obsremote_get_sources(source);
	obs_data_set_array(ret, "sources", sources);
	obs_data_array_release(sources);

	handler->sendUpdate("ItemReorder", ret);
}

void OBSRemoteEventHandler::onSceneItemRemove(void *me, calldata_t *data)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) me;

	obs_sceneitem_t *item = nullptr;
	calldata_get_ptr(data, "item", &item);

	obs_data_t *source = obs_data_create();
	obs_data_set_string(source, "name", obs_source_get_name(
		obs_sceneitem_get_source(item)));

	handler->sendUpdate("ItemRemove", source);
}

void OBSRemoteEventHandler::onSceneItemAdd(void *me, calldata_t *data)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) me;

	obs_sceneitem_t *item = nullptr;
	calldata_get_ptr(data, "item", &item);

	obs_data_t *source = obsremote_get_scene_item_data(item);

	handler->sendUpdate("ItemAdd", source);
}

void OBSRemoteEventHandler::onSceneItemVisible(void *me, calldata_t *data)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) me;

	obs_sceneitem_t *item = nullptr;
	calldata_get_ptr(data, "item", &item);

	bool visible = false;
	calldata_get_bool(data, "visible", &visible);

	obs_data_t *source = obs_data_create();
	obs_data_set_string(source, "source", obs_source_get_name(
		obs_sceneitem_get_source(item)));
	obs_data_set_bool(source, "visible", visible);

	handler->sendUpdate("ItemVisible", source);
}

void OBSRemoteEventHandler::onSourceRename(void *me, calldata_t *data)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) me;

	obs_data_t *source = obs_data_create();
	obs_data_set_string(source, "prev_name",
	                    calldata_string(data, "prev_name"));
	obs_data_set_string(source, "new_name",
	                    calldata_string(data, "new_name"));

	handler->sendUpdate("ItemRename", source);
}

void OBSRemoteEventHandler::onSceneItemSelect(void *me, calldata_t *data)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) me;

	obs_sceneitem_t *item = nullptr;
	calldata_get_ptr(data, "item", &item);

	obs_data_t *source = obs_data_create();
	obs_data_set_string(source, "name", obs_source_get_name(
		obs_sceneitem_get_source(item)));

	handler->sendUpdate("ItemSelect", source);
}

void OBSRemoteEventHandler::onSceneItemDeselect(void *me, calldata_t *data)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) me;

	obs_sceneitem_t *item = nullptr;
	calldata_get_ptr(data, "item", &item);

	obs_data_t *source = obs_data_create();
	obs_data_set_string(source, "name", obs_source_get_name(
		obs_sceneitem_get_source(item)));

	handler->sendUpdate("ItemDeselect", source);
}

void OBSRemoteEventHandler::onSourceVolume(void *me, calldata_t *data)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) me;

	obs_source_t *source = nullptr;
	calldata_get_ptr(data, "source", &source);

	double volume = 0;
	calldata_get_float(data, "volume", &volume);

	obs_data_t *ret = obs_data_create();
	obs_data_set_string(ret, "name", obs_source_get_name(source));
	obs_data_set_double(ret, "volume", volume);

	handler->sendUpdate("SourceVolume", ret);
}

void OBSRemoteEventHandler::onSourceMute(void *me, calldata_t *data)
{
	OBSRemoteEventHandler *handler = (OBSRemoteEventHandler *) me;


	obs_source_t *source = nullptr;
	calldata_get_ptr(data, "source", &source);

	bool muted = false;
	calldata_get_bool(data, "muted", &muted);

	obs_data_t *ret = obs_data_create();
	obs_data_set_string(ret, "name", obs_source_get_name(source));
	obs_data_set_bool(ret, "muted", muted);

	handler->sendUpdate("SourceMute", ret);
}
