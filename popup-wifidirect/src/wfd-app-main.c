/*
*  WiFi-Direct UG
*
* Copyright 2012  Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.tizenopensource.org/license

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

/**
 * This file implements wifi direct application main functions.
 *
 * @file    wfd-app-main.c
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */

#include <libintl.h>
#include <Elementary.h>
#include <notification.h>
#include <ui-gadget-module.h>
#include <app_control_internal.h>
#include <feedback.h>
#include <wifi-direct.h>
#include <efl_util.h>
#include <linux/unistd.h>
#include <vconf.h>

#include "wfd-app.h"
#include "wfd-app-util.h"

wfd_appdata_t *g_wfd_ad;


wfd_appdata_t *wfd_get_appdata()
{
	return g_wfd_ad;
}

static void _win_del(void *data, Evas_Object *obj, void *event)
{
	elm_exit();
}

static Evas_Object *_create_win(Evas_Object *parent, const char *name)
{
	Evas_Object *eo;

	/* eo = elm_win_add(parent, name, ELM_WIN_BASIC); */
	eo = elm_win_add(NULL, name, ELM_WIN_NOTIFICATION);
	if (eo) {
		elm_win_title_set(eo, name);
		elm_win_borderless_set(eo, EINA_TRUE);
		elm_win_alpha_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request", _win_del, NULL);
		efl_util_set_notification_window_level(eo, EFL_UTIL_NOTIFICATION_LEVEL_1);
		evas_object_raise(eo);
	}

	return eo;
}

static bool _app_create(void *data)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();
	int ret = 0;

	if (data == NULL) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Incorrect parameter\n");
		return FALSE;
	}

	bindtextdomain(LOCALE_FILE_NAME, LOCALEDIR);

	ad->popup_data = (wfd_popup_t *) malloc(sizeof(wfd_popup_t));
	if (!ad->popup_data) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "malloc failed\n");
		return FALSE;
	}

	memset(ad->popup_data, 0x0, sizeof(wfd_popup_t));
	ad->win = _create_win(NULL, PACKAGE);

	/* set rotation */
	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = {0, 90, 180, 270};
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	ad->conformant = elm_conformant_add(ad->win);
	assertm_if(NULL == ad->conformant, "conformant is NULL!!");
	elm_win_conformant_set(ad->win, EINA_TRUE);
	elm_win_resize_object_add(ad->win, ad->conformant);
	evas_object_size_hint_weight_set(ad->conformant, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(ad->conformant, EVAS_HINT_FILL, 0.0);
	evas_object_show(ad->conformant);


	ad->back_grnd = elm_bg_add(ad->conformant);
	if (NULL == ad->back_grnd) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Create background failed\n");
		return FALSE;
	}
	elm_object_signal_emit(ad->conformant, "elm,state,indicator,nooverlap", "elm");
	elm_object_style_set(ad->back_grnd, "indicator/headerbg");
	elm_object_part_content_set(ad->conformant, "elm.swallow.indicator_bg", ad->back_grnd);
	evas_object_size_hint_weight_set(ad->back_grnd,
		EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ad->back_grnd);

	ad->layout = elm_layout_add(ad->conformant);
	elm_object_content_set(ad->conformant, ad->layout);

	ret = init_wfd_client(ad);
	if (!ret) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "init_wfd_popup_client error\n");
		wfd_prepare_popup(WFD_POP_FAIL_INIT, NULL);
		__WFD_APP_FUNC_EXIT__;
		return FALSE;
	}

	ret = wfd_app_util_register_vconf_callbacks(ad);
	if (ret < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to register vconf notification");
		return FALSE;
	}

	/* Register Hard Key Press CB */
	wfd_app_util_register_hard_key_down_cb(ad);

	/* Initializes feedback API */
	ret = feedback_initialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "feedback_initialize error : %d\n", ret);
		return FALSE;
	}
	__WFD_APP_FUNC_EXIT__;
	return TRUE;
}

static void _app_terminate(void *data)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *) data;
	int ret = 0;

	if (data == NULL) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Incorrect parameter\n");
		return;
	}

	wfd_app_util_del_notification(ad);

	ret = wfd_app_util_deregister_vconf_callbacks(ad);
	if (ret < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to register vconf notification");
	}

	/* Deregister Hardkey CB */
	wfd_app_util_deregister_hard_key_down_cb(ad);

	/* Deinitializes feedback API */
	ret = feedback_deinitialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "feedback_deinitialize error : %d\n", ret);
	}
	if (ad->transmit_timer) {
		ecore_timer_del(ad->transmit_timer);
		ad->transmit_timer = NULL;
	}

	wfd_destroy_popup();

	ret = deinit_wfd_client(ad);
	if (ret < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "deinit_wfd_client error\n");
	}

	if (ad->back_grnd) {
		evas_object_del(ad->back_grnd);
		ad->back_grnd = NULL;
	}

	if (ad->win) {
		evas_object_del(ad->win);
		ad->win = NULL;
	}

	if (ad->popup_data) {
		free(ad->popup_data);
		ad->popup_data = NULL;
	}

	__WFD_APP_FUNC_EXIT__;
	return;
}

static void _app_pause(void *data)
{
	__WFD_APP_FUNC_ENTER__;
	__WFD_APP_FUNC_EXIT__;
	return;
}

static void _app_resume(void *data)
{
	__WFD_APP_FUNC_ENTER__;
	__WFD_APP_FUNC_EXIT__;
	return;
}

static void _app_reset(app_control_h control, void *data)
{
	__WFD_APP_FUNC_ENTER__;

	int ret;
	wfd_appdata_t *ad = (wfd_appdata_t *) data;
	if (ad == NULL) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Incorrect parameter\n");
		return;
	}
	if (control == NULL) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Service is NULL");
		return;
	}

	// From Notification
	char *noti_type = NULL;
	app_control_get_extra_data(control, NOTIFICATION_BUNDLE_PARAM, &noti_type);

	if (noti_type == NULL) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Notification type is wrong.");
		return;
	}

	WFD_APP_LOG(WFD_APP_LOG_LOW, "Notification type is [%s]", noti_type);
	if (strncmp(noti_type, NOTIFICATION_BUNDLE_VALUE, strlen(NOTIFICATION_BUNDLE_PARAM)) == 0) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Launch wifidirect-ug");
		wifi_direct_get_state(&ad->wfd_status);
		WFD_APP_LOG(WFD_APP_LOG_LOW, "State: %d", ad->wfd_status);
		if (ad->wfd_status == WIFI_DIRECT_STATE_CONNECTED) {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "Connected");
			if (ad->transmit_timer) {
				ecore_timer_del(ad->transmit_timer);
				ad->transmit_timer = NULL;
			}
			WFD_APP_LOG(WFD_APP_LOG_LOW, "start the transmit timer again\n");
			ad->last_wfd_transmit_time = time(NULL);
			ad->transmit_timer = ecore_timer_add(5.0,
				(Ecore_Task_Cb)wfd_automatic_deactivated_for_connection_cb, ad);
		}
		app_control_h ug_control;
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Launching Settings EFL from notification\n");
		app_control_create(&ug_control);
		app_control_set_operation(ug_control, APP_CONTROL_OPERATION_DEFAULT);
		app_control_set_launch_mode(ug_control, APP_CONTROL_LAUNCH_MODE_GROUP);
		app_control_set_app_id(ug_control, "setting-wifidirect-efl");

		ret = app_control_send_launch_request(ug_control, NULL, NULL);
		if(ret == APP_CONTROL_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "Launch Wi-Fi Direct successful");
		} else {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Fail to launch Wi-Fi Direct");
		}
		app_control_destroy(ug_control);

	}
	WFD_IF_FREE_MEM(noti_type);
	__WFD_APP_FUNC_EXIT__;
	return;
}

int main(int argc, char *argv[])
{
	wfd_appdata_t ad;
	ui_app_lifecycle_callback_s event_callback;
	memset(&event_callback, 0x0, sizeof(ui_app_lifecycle_callback_s));

	event_callback.create = _app_create;
	event_callback.terminate = _app_terminate;
	event_callback.pause = _app_pause;
	event_callback.resume = _app_resume;
	event_callback.app_control = _app_reset;

	memset(&ad, 0x0, sizeof(wfd_appdata_t));
	g_wfd_ad = &ad;

	return ui_app_main(argc, argv, &event_callback, &ad);
}
