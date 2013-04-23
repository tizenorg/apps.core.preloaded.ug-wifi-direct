/*
*  WiFi-Direct UG
*
* Copyright 2012  Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://floralicense.org/license

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
#include <appcore-efl.h>
#include <appsvc.h>
#include <app_service.h>

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
	int w, h;

	/* eo = elm_win_add(parent, name, ELM_WIN_BASIC); */
	eo = elm_win_add(NULL, name, ELM_WIN_NOTIFICATION);
	if (eo) {
		elm_win_title_set(eo, name);
		elm_win_borderless_set(eo, EINA_TRUE);
		elm_win_alpha_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request", _win_del, NULL);
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
		evas_object_resize(eo, w, h);
		evas_object_raise(eo);
	}

	return eo;
}

static int _app_create(void *data)
{
	__WDPOP_LOG_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();

	if (data == NULL) {
		WDPOP_LOGD( "Incorrect parameter\n");
		return -1;
	}

	bindtextdomain(LOCALE_FILE_NAME, LOCALEDIR);

	ad->popup_data = (wfd_popup_t *) malloc(sizeof(wfd_popup_t));
	if (!ad->popup_data) {
		WDPOP_LOGE("malloc failed\n");
		return -1;
	}

	memset(ad->popup_data, 0x0, sizeof(wfd_popup_t));
	ad->win = _create_win(NULL, PACKAGE);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);

	int r;

	if (!ecore_x_display_get()) {
		return -1;
	}

	r = appcore_set_i18n(PACKAGE, NULL);
	if (r != 0) {
		WDPOP_LOGD( "appcore_set_i18n error\n");
		return -1;
	}

	if (init_wfd_popup_client(ad) == FALSE) {
		WDPOP_LOGE("init_wfd_popup_client error\n");
		wfd_prepare_popup(WFD_POP_FAIL_INIT, NULL);
	}

	__WDPOP_LOG_FUNC_EXIT__;
	return 0;
}

static int _app_terminate(void *data)
{
	__WDPOP_LOG_FUNC_ENTER__;

	if (data == NULL) {
		WDPOP_LOGE("Incorrect parameter\n");
		return -1;
	}

	wfd_appdata_t *ad = (wfd_appdata_t *) data;

	if (deinit_wfd_popup_client(ad) == FALSE) {
		WDPOP_LOGE("deinit_wfd_popup_client error\n");
	} else {
		if (ad->popup) {
			evas_object_del(ad->popup);
			ad->popup = NULL;
		}
		if (ad->win) {
			evas_object_del(ad->win);
			ad->win = NULL;
		}
		if (ad->discovered_peers) {
			free(ad->discovered_peers);
			ad->discovered_peers = NULL;
		}
	}

	__WDPOP_LOG_FUNC_EXIT__;
	return 0;
}

static int _app_pause(void *data)
{
	__WDPOP_LOG_FUNC_ENTER__;
	__WDPOP_LOG_FUNC_EXIT__;
	return 0;
}

static int _app_resume(void *data)
{
	__WDPOP_LOG_FUNC_ENTER__;
	__WDPOP_LOG_FUNC_EXIT__;
	return 0;
}

static int _app_reset(bundle *b, void *data)
{
	__WDPOP_LOG_FUNC_ENTER__;

	if (b == NULL) {
		WDPOP_LOGD( "Bundle is NULL");
		return -1;
	}

	// From Notification
	char *noti_type = NULL;
	noti_type = (char *)appsvc_get_data(b, NOTIFICATION_BUNDLE_PARAM);

	if (noti_type == NULL) {
		WDPOP_LOGD( "Notification type is wrong.");
		return -1;
	}

	WDPOP_LOGD( "Notification type is [%s]", noti_type);
	if (strncmp(noti_type, NOTIFICATION_BUNDLE_VALUE, strlen(NOTIFICATION_BUNDLE_PARAM)) == 0) {
		WDPOP_LOGD( "Launch wifidirect-ugapp");
		service_h service;
		service_create(&service);
		service_set_operation(service, SERVICE_OPERATION_DEFAULT);
		service_set_package(service, "org.tizen.wifi-direct-ugapp");
		service_send_launch_request(service, NULL, NULL);
		service_destroy(service);
	}

	__WDPOP_LOG_FUNC_EXIT__;
	return 0;
}

int main(int argc, char *argv[])
{
	wfd_appdata_t ad;
	struct appcore_ops ops = {
		.create = _app_create,
		.terminate = _app_terminate,
		.pause = _app_pause,
		.resume = _app_resume,
		.reset = _app_reset,
	};

	memset(&ad, 0x0, sizeof(wfd_appdata_t));
	ops.data = &ad;
	g_wfd_ad = &ad;

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
