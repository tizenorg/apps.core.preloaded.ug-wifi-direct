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

#include <libintl.h>

#include <assert.h>
#include <glib.h>

#include <Elementary.h>
#include <vconf.h>
#include <ui-gadget-module.h>
#include <wifi-direct.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

static void _wfd_ug_act_popup_wifi_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    // TODO: Turn off WiFi
    ugd->wfd_status = WFD_LINK_STATUS_DEACTIVATED;
    wfd_wifi_off(ugd);

     evas_object_del(ugd->act_popup);
     ugd->act_popup = NULL;
    __FUNC_EXIT__;
}

static void _wfd_ug_act_popup_wifi_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    // TODO: set genlist head item as "WiFi Direct"
    ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
    wfd_ug_view_refresh_glitem(ugd->head);

     evas_object_del(ugd->act_popup);
     ugd->act_popup = NULL;
    __FUNC_EXIT__;
}

static void _wfd_ug_act_popup_disconnect_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	char *mac_addr = NULL;
	struct ug_data *ugd = (struct ug_data*) data;

	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	if (ugd->gl_connected_peer_cnt < 1) {
		DBG(LOG_ERROR, "No connected peer\n");
		evas_object_del(ugd->act_popup);
		ugd->act_popup = NULL;
		return;
	}

	/* just one peer */
	mac_addr = ugd->gl_connected_peers[0].mac_addr;
	wfd_client_disconnect(mac_addr);
	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS) {
		wfd_stop_multi_connect(ugd);
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	__FUNC_EXIT__;
}

static void _wfd_ug_act_popup_disconnect_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data*) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	__FUNC_EXIT__;
}

static void _wfd_ug_act_popup_disconnect_all_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data*) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	wfd_client_disconnect(NULL);
	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS) {
		wfd_stop_multi_connect(ugd);
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	__FUNC_EXIT__;
}

static void _wfd_ug_act_popup_disconnect_all_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data*) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	__FUNC_EXIT__;
}

static void _wfd_ug_act_popup_scan_again_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data*) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	if (ugd->conn_wfd_item != NULL) {
		elm_object_item_del(ugd->conn_wfd_item);
		ugd->conn_wfd_item = NULL;
	}

	/* cancel the current connection */
	wfd_client_disconnect(NULL);
	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS) {
		wfd_stop_multi_connect(ugd);
	}

	/* start discovery again */
	wfd_client_start_discovery(ugd);
	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	__FUNC_EXIT__;
}

static void _wfd_ug_act_popup_scan_again_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data*) data;

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	__FUNC_EXIT__;
}

void wfd_ug_act_popup(void *data, const char *message, int popup_type)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data*) data;
	Evas_Object *popup = NULL;
	Evas_Object *btn1 = NULL, *btn2 = NULL;

	popup = elm_popup_add(ugd->base);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, message);

	btn1 = elm_button_add(popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup_button/default");
	elm_object_style_set(btn2, "popup_button/default");

	/* set the different text by type */
	if (popup_type == POPUP_TYPE_WIFI_OFF) {
		elm_object_text_set(btn1, S_("IDS_COM_SK_YES"));
		elm_object_text_set(btn2, S_("IDS_COM_SK_NO"));
	} else {
		elm_object_text_set(btn1, S_("IDS_COM_SK_OK"));
		elm_object_text_set(btn2, S_("IDS_COM_SK_CANCEL"));
	}

	elm_object_part_content_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button2", btn2);

	/* set the different callback by type */
	if (popup_type == POPUP_TYPE_WIFI_OFF) {
		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_wifi_ok_cb, (void*) ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_wifi_cancel_cb, (void*) ugd);
	} else if (popup_type == POP_TYPE_DISCONNECT) {
		//evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_disconnect_ok_cb, (void*) ugd);
		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_disconnect_all_ok_cb, (void*) ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_disconnect_cancel_cb, (void*) ugd);
	} else if (popup_type == POP_TYPE_DISCONNECT_ALL) {
		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_disconnect_all_ok_cb, (void*) ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_disconnect_all_cancel_cb, (void*) ugd);
	} else if (popup_type == POP_TYPE_SCAN_AGAIN) {
		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_scan_again_ok_cb, (void*) ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_scan_again_cancel_cb, (void*) ugd);
	}

	evas_object_show(popup);
	ugd->act_popup = popup;
	__FUNC_EXIT__;
}

void wfg_ug_act_popup_remove(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    if(ugd->act_popup)
    {
        evas_object_del(ugd->act_popup);
        ugd->act_popup = NULL;
    }
    __FUNC_EXIT__;
}

static void _wfd_ug_terminate_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    evas_object_del(ugd->warn_popup);
    ugd->warn_popup = NULL;

    wfd_ug_view_free_peers(ugd);

    ug_destroy_me(ugd->ug);
    __FUNC_EXIT__;
}

static void _wfd_ug_warn_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    evas_object_del(ugd->warn_popup);
    ugd->warn_popup = NULL;

    __FUNC_EXIT__;
}

void wfd_ug_warn_popup(void *data, const char *message, int popup_type)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    Evas_Object *popup = NULL;
    Evas_Object *btn = NULL;

    popup = elm_popup_add(ugd->base);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, message);

    btn = elm_button_add(popup);
    elm_object_style_set(btn, "popup_button/default");
    elm_object_text_set(btn, S_("IDS_COM_SK_OK"));
    elm_object_part_content_set(popup, "button1", btn);
    if(popup_type == POPUP_TYPE_TERMINATE)
        evas_object_smart_callback_add(btn, "clicked", _wfd_ug_terminate_popup_cb, (void*) ugd);
    else
        evas_object_smart_callback_add(btn, "clicked", _wfd_ug_warn_popup_cb, (void*) ugd);

    evas_object_show(popup);
    ugd->warn_popup = popup;
    __FUNC_EXIT__;
}

void wfg_ug_warn_popup_remove(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    if(ugd->warn_popup)
    {
        evas_object_del(ugd->warn_popup);
        ugd->warn_popup = NULL;
    }
    __FUNC_EXIT__;
}


void wfd_ug_tickernoti_popup(char *msg)
{
	__FUNC_ENTER__;

	int ret = -1;
	bundle *b = NULL;

	b = bundle_create();
	if (!b) {
		DBG(LOG_ERROR, "FAIL: bundle_create()\n");
		return;
	}

	/* tickernoti style */
	ret = bundle_add(b, "0", "info");
	if (ret) {
		DBG(LOG_ERROR, "Fail to add tickernoti style\n");
		bundle_free(b);
		return;
	}

	/* popup text */
	ret = bundle_add(b, "1", msg);
	if (ret) {
		DBG(LOG_ERROR, "Fail to add popup text\n");
		bundle_free(b);
		return;
	}

	/* orientation of tickernoti */
	ret = bundle_add(b, "2", "1");
	if (ret) {
		DBG(LOG_ERROR, "Fail to add orientation of tickernoti\n");
		bundle_free(b);
		return;
	}

	/* timeout(second) of tickernoti */
	ret = bundle_add(b, "3", "3");
	if (ret) {
		DBG(LOG_ERROR, "Fail to add timeout of tickernoti\n");
		bundle_free(b);
		return;
	}

	/* launch tickernoti */
	ret = syspopup_launch(TICKERNOTI_SYSPOPUP, b);
	if (ret) {
		DBG(LOG_ERROR, "Fail to launch syspopup\n");
	}

	bundle_free(b);
	__FUNC_EXIT__;
}

