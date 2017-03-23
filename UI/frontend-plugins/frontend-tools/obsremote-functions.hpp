#pragma once

#include <obs-frontend-api.h>

obs_data_t *SendOkResponse(obs_data_t *ret = NULL);

obs_data_t *GetErrorResponse(const char *error);

void
obsremote_get_source_filter(obs_source_t *, obs_source_t *filter, void *param);

obs_data_array_t *obsremote_get_filters(obs_source_t *source);

obs_data_t *
obsremote_get_scene_item_data(obs_sceneitem_t *item);

bool
obsremote_get_scene_item(obs_scene_t *, obs_sceneitem_t *item, void *param);

obs_data_array_t *obsremote_get_sources(obs_source_t *source);

obs_data_t *obsremote_get_scene_data(obs_source_t *source);