#include "obsremote-functions.hpp"

obs_data_t *SendOkResponse(obs_data_t *ret)
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

void
obsremote_get_source_filter(obs_source_t *, obs_source_t *filter, void *param)
{
	obs_data_array_t *filters = (obs_data_array_t *) param;
	obs_data_t *filter_data = obs_data_create();

	obs_data_set_string(filter_data, "name", obs_source_get_name(filter));
	obs_data_set_int(filter_data, "type", obs_source_get_type(filter));
	obs_data_array_push_back(filters, filter_data);

	obs_data_release(filter_data);
}

obs_data_array_t *obsremote_get_filters(obs_source_t *source)
{
	if (!source)
		return NULL;

	obs_data_array_t *filters = obs_data_array_create();
	obs_source_enum_filters(source, obsremote_get_source_filter,
	                        (void *) filters);

	return filters;
}

obs_data_t *
obsremote_get_scene_item_data(obs_sceneitem_t *item)
{
	obs_source_t *source = obs_sceneitem_get_source(item);
	obs_data_t *source_data = obs_data_create();
	obs_data_set_string(source_data, "name", obs_source_get_name(source));
	obs_data_set_int(source_data, "type", obs_source_get_type(source));
	obs_data_set_bool(source_data, "visible", obs_sceneitem_visible(item));
	uint32_t flags = obs_source_get_output_flags(source);
	bool audio = (flags & OBS_SOURCE_AUDIO) != 0;
	obs_data_set_bool(source_data, "audio", audio);
	if (audio) {
		obs_data_set_double(source_data, "volume",
		                    obs_source_get_volume(source));
	}
	obs_data_array_t *filters = obsremote_get_filters(source);
	obs_data_set_array(source_data, "filters", filters);
	obs_data_array_release(filters);
	return source_data;
}

bool
obsremote_get_scene_item(obs_scene_t *, obs_sceneitem_t *item, void *param)
{
	obs_data_array_t *sources = (obs_data_array_t *) param;
	obs_data_t *source_data = obsremote_get_scene_item_data(item);
	obs_data_array_push_back(sources, source_data);
	obs_data_release(source_data);
	return true;
}

obs_data_array_t *obsremote_get_sources(obs_source_t *source)
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

obs_data_t *obsremote_get_scene_data(obs_source_t *source)
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
