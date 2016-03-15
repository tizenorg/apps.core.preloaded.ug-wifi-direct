/*
 * Wi-Fi direct
 *
 * Copyright 2012 Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <vconf.h>
#include <sensor_internal.h>
#include <vconf-keys.h>

#include <libintl.h>
#include <glib.h>

#include <Elementary.h>
#include <vconf.h>
#include <ui-gadget-module.h>

#include "wfd_motion_control.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"


static int motion_handle = -1;

static TARGET_VIEW_FOCUS __motion_target_view_focus_get(void *data)
{
	struct ug_data *ugd = (struct ug_data *)data;

	if ((ugd == NULL) || (ugd->naviframe == NULL)) {
		return MOTION_TARGET_VIEW_FOCUS_OFF;
	}

	if (elm_object_focus_get(ugd->naviframe)) {
		return MOTION_TARGET_VIEW_FOCUS_ON;
	} else {
		return MOTION_TARGET_VIEW_FOCUS_OFF;
	}
}


static void __motion_shake_cb(unsigned int event_type, sensor_event_data_t *event_data, void *data)
{
	__FUNC_ENTER__;

	gboolean motion_activated = FALSE;
	struct ug_data *ugd = (struct ug_data *) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR,"NULL pointer!");
		return;
	}
	TARGET_VIEW_FOCUS focus_state = __motion_target_view_focus_get(ugd);

	DBG(LOG_DEBUG, "event type: %d, focus state: %d\n", event_type, focus_state);

	if (focus_state != MOTION_TARGET_VIEW_FOCUS_ON) {
		return;
	}

	if (vconf_get_bool(VCONFKEY_SETAPPL_MOTION_ACTIVATION,
			(void *)&motion_activated)) {
		DBG(LOG_ERROR,"Get motion activation status fail");
		return;
	}

	if (FALSE == motion_activated) {
		DBG(LOG_INFO,"Motion value is false");
		return;
	}

	if (vconf_get_bool(VCONFKEY_SETAPPL_USE_SHAKE,
			(void *)&motion_activated)) {
		DBG(LOG_ERROR, "Get use shake status fail");
		return;
	}

	if (FALSE == motion_activated) {
		DBG(LOG_INFO,"Shake status is false");
		return;
	}

	int ret = -1;
	const char *btn_text = NULL;

	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	wfd_refresh_wifi_direct_state(ugd);

	if (NULL == ugd->scan_toolbar) {
		DBG(LOG_ERROR, "NULL == ugd->scan_toolbar\n");
		return;
	}

	btn_text = elm_object_part_text_get(ugd->scan_toolbar, "default");
	if (NULL == btn_text) {
		DBG(LOG_ERROR, "Incorrect button text(NULL)\n");
		return;
	}

	GList *iterator = NULL;

	if (0 == strcmp(btn_text, D_("IDS_WIFI_SK4_SCAN"))) {
		if (WIFI_DIRECT_STATE_CONNECTED == ugd->wfd_status) {
			ret = wfd_client_disconnect(NULL);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				DBG(LOG_ERROR, "Failed wfd_client_disconnect() [%d]\n", ret);
				__FUNC_EXIT__;
				return;
			}

			if (ugd->multi_connect_mode != WFD_MULTI_CONNECT_MODE_NONE) {
				wfd_free_multi_selected_peers(ugd);
			} else {
				/* update the connecting icon */
				for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
					((device_type_s *)iterator->data)->conn_status = PEER_CONN_STATUS_DISCONNECTED;
					wfd_ug_view_refresh_glitem(((device_type_s *)iterator->data)->gl_item);
				}
			}
		} else if (WIFI_DIRECT_STATE_DEACTIVATED == ugd->wfd_status) {
			wfd_client_switch_on(ugd);
		} else if ((WIFI_DIRECT_STATE_DISCOVERING == ugd->wfd_status) || (WIFI_DIRECT_STATE_ACTIVATED == ugd->wfd_status)) {
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
			ret = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
				DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", ret);
				wifi_direct_cancel_discovery();
			}
		}
	}

	__FUNC_EXIT__;
}

void motion_create(struct ug_data *ugd)
{
	int ret = -1;

	motion_handle = sf_connect(MOTION_SENSOR);
	if (motion_handle < 0) {
		DBG(LOG_ERROR, "Failed Operation sf_connect.\n");
		return;
	}
	ret = sf_register_event(motion_handle, MOTION_ENGINE_EVENT_SHAKE, NULL,
			__motion_shake_cb, ugd);
	if (ret < 0) {
		DBG(LOG_ERROR, "Failed Operation sf_register_event. [%d]\n", ret);
		goto fail;
	}
	ret = sf_start(motion_handle, 0);
	if (ret < 0) {
		DBG(LOG_ERROR, "Failed Operation sf_start. [%d]\n", ret);
		goto fail;
	}

	DBG(LOG_INFO, "Succesfully, Init Sensor Handle\n");
	return;

fail:
	ret = sf_disconnect(motion_handle);
	if (ret < 0) {
		DBG(LOG_ERROR, "Failed Operation sf_disconnect. [%d]\n", ret);
	}
}

void motion_destroy(void)
{
	int ret = -1;

	if (motion_handle < 0) {
		DBG(LOG_ERROR, "Motion Handle Not valid !!!");
		return;
	}

	ret = sf_stop(motion_handle);
	if (ret < 0) {
		DBG(LOG_ERROR, "Failed Operation sf_stop. [%d]\n", ret);
	}
	ret = sf_unregister_event(motion_handle, MOTION_ENGINE_EVENT_SHAKE);
	if (ret < 0) {
		DBG(LOG_ERROR, "Failed Operation sf_unregister_event. [%d]\n", ret);
	}
	ret = sf_disconnect(motion_handle);
	if (ret < 0) {
		DBG(LOG_ERROR, "Failed Operation sf_disconnect. [%d]\n", ret);
	}
}
