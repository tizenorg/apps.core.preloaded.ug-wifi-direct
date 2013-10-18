/*
*  WiFi-Direct UG
*
* Copyright 2012 Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.1 (the "License");
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

#include <libintl.h>

#include <assert.h>
#include <glib.h>

#include <Elementary.h>
#include <efl_assist.h>
#include <vconf.h>
#include <ui-gadget-module.h>
#include <wifi-direct.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

/**
 *	This function let the ug call it when click 'ok' button in hotspot action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_act_popup_hotspot_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;
	int result = -1;
	struct ug_data *ugd = (struct ug_data *) data;

	result = wfd_mobile_ap_off(ugd);
	if (0 == result) {
		/* refresh the header */
		ugd->head_text_mode = HEAD_TEXT_TYPE_ACTIVATING;
		wfd_ug_view_refresh_glitem(ugd->head);

		/* while activating/deactivating, disable the buttons */
		if (ugd->scan_btn) {
			wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
		}

		if (ugd->multi_scan_btn) {
			wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
		}

		if (ugd->back_btn) {
			elm_object_disabled_set(ugd->back_btn, TRUE);
		}
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;
	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'cancel' button in hotspot action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_act_popup_hotspot_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
	wfd_ug_view_refresh_glitem(ugd->head);

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;
	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'ok' button in wifi action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_act_popup_wifi_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;
	int result = -1;
	struct ug_data *ugd = (struct ug_data *) data;

	result = wfd_wifi_off(ugd);
	if (0 == result) {
		/* refresh the header */
		ugd->head_text_mode = HEAD_TEXT_TYPE_ACTIVATING;
		wfd_ug_view_refresh_glitem(ugd->head);

		/* while activating/deactivating, disable the buttons */
		if (ugd->scan_btn) {
			wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
		}

		if (ugd->multi_scan_btn) {
			wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
		}

		if (ugd->back_btn) {
			elm_object_disabled_set(ugd->back_btn, TRUE);
		}
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;
	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'cancel' button in wifi action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_act_popup_wifi_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
	wfd_ug_view_refresh_glitem(ugd->head);

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;
	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'ok' button in disconnect all popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_act_popup_disconnect_all_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;
	int i = 0;
	struct ug_data *ugd = (struct ug_data *) data;
	if (NULL == ugd) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return;
	}

	wfd_client_disconnect(NULL);

	if (ugd->multi_connect_mode != WFD_MULTI_CONNECT_MODE_NONE) {
		wfd_free_multi_selected_peers(ugd);
	} else {
		/* update the connecting icon */
		for (i = 0; i < ugd->raw_discovered_peer_cnt; i++) {
			ugd->raw_discovered_peers[i].conn_status = PEER_CONN_STATUS_DISCONNECTED;
			wfd_ug_view_refresh_glitem(ugd->raw_discovered_peers[i].gl_item);
		}
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'cancel' button in disconnect all popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_act_popup_disconnect_all_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	if (NULL == ugd) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return;
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug create a action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] message the pointer to the text of popup
 *	@param[in] popup_type the message type
 */
void wfd_ug_act_popup(void *data, const char *message, int popup_type)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
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
	if (popup_type == POPUP_TYPE_WIFI_OFF || popup_type == POPUP_TYPE_HOTSPOT_OFF) {
		elm_object_text_set(btn1, S_("IDS_COM_SK_NO"));
		elm_object_text_set(btn2, S_("IDS_COM_SK_YES"));
	} else {
		elm_object_text_set(btn1, S_("IDS_COM_SK_CANCEL"));
		elm_object_text_set(btn2, S_("IDS_COM_SK_OK"));
	}

	elm_object_part_content_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button2", btn2);

	/* set the different callback by type */
	if (popup_type == POPUP_TYPE_WIFI_OFF) {
		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_wifi_cancel_cb, (void*) ugd);
		ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _wfd_ug_act_popup_wifi_cancel_cb, (void*) ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_wifi_ok_cb, (void*) ugd);
	} else if (popup_type == POPUP_TYPE_HOTSPOT_OFF) {
		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_hotspot_cancel_cb, (void*) ugd);
		ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _wfd_ug_act_popup_hotspot_cancel_cb, (void*) ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_hotspot_ok_cb, (void*) ugd);
	} else if (popup_type == POP_TYPE_DISCONNECT || popup_type == POP_TYPE_DISCONNECT_ALL ||
				popup_type == POP_TYPE_SCAN_AGAIN) {
		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_disconnect_all_cancel_cb, (void*) ugd);
		ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _wfd_ug_act_popup_disconnect_all_cancel_cb, (void*) ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_disconnect_all_ok_cb, (void*) ugd);
	}

	evas_object_show(popup);
	ugd->act_popup = popup;
	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug remove the action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfg_ug_act_popup_remove(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	if (ugd->act_popup) {
		evas_object_del(ugd->act_popup);
		ugd->act_popup = NULL;
	}
	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'ok' button in warning popup of terminated problem
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_terminate_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	evas_object_del(ugd->warn_popup);
	ugd->warn_popup = NULL;

	wfd_ug_view_free_peers(ugd);

	ug_destroy_me(ugd->ug);
	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'ok' button in warning popup of turning off WFD automatically
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_automatic_turn_off_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	evas_object_del(ugd->warn_popup);
	ugd->warn_popup = NULL;

	/* turn off the Wi-Fi Direct */
	wfd_client_switch_off(ugd);

	/* antomaticlly turn on tethering mode */
	if (TRUE == ugd->is_hotspot_off) {
		wfd_mobile_ap_on(ugd);
	}

	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'ok' button in warning popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_warn_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	evas_object_del(ugd->warn_popup);
	ugd->warn_popup = NULL;

	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug create a warning popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] message the pointer to the text of popup
 *	@param[in] popup_type the message type
 */
void wfd_ug_warn_popup(void *data, const char *message, int popup_type)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	Evas_Object *popup = NULL;
	Evas_Object *btn = NULL;

	popup = elm_popup_add(ugd->base);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "title,text", _("IDS_WFD_POP_TITILE_CONNECTION"));
	elm_object_text_set(popup, message);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup_button/default");
	elm_object_text_set(btn, S_("IDS_COM_SK_OK"));
	elm_object_part_content_set(popup, "button1", btn);
	if (popup_type == POPUP_TYPE_TERMINATE) {
		evas_object_smart_callback_add(btn, "clicked", _wfd_ug_terminate_popup_cb, (void *)ugd);
	} else if (popup_type == POP_TYPE_AUTOMATIC_TURN_OFF) {
		evas_object_smart_callback_add(btn, "clicked", _wfd_ug_automatic_turn_off_popup_cb, (void *)ugd);
	} else {
		evas_object_smart_callback_add(btn, "clicked", _wfd_ug_warn_popup_cb, (void *)ugd);
	}

	evas_object_show(popup);
	ugd->warn_popup = popup;
	__WDUG_LOG_FUNC_EXIT__;
}

