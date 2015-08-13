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
#include <glib.h>

#include <Elementary.h>
#include <vconf.h>
#include <ui-gadget-module.h>
#include <wifi-direct.h>
#include <efl_extension.h>

#if defined(X)
#include "utilX.h"
#endif
#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

static void delete_popup(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;
	__FUNC_EXIT__;
}

static void pushbutton_accept_connect_cb(void *data, Evas_Object * obj,
		void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}
	int result = -1;
	result = wifi_direct_accept_connection(ugd->mac_addr_req);
	if (result != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_INFO,"Failed to connect");
	}
	delete_popup(ugd);

	__FUNC_EXIT__;
}

static void pushbutton_reject_connect_cb(void *data, Evas_Object * obj,
		void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}
	int result = -1;
	if (ugd->mac_addr_req[0] != '\0') {
		result = wifi_direct_reject_connection(ugd->mac_addr_req);
		if (result != WIFI_DIRECT_ERROR_NONE)
			DBG(LOG_INFO,"Failed to reject connection");
	} else {
		DBG(LOG_INFO,"No Peer mac address");
	}
	delete_popup(ugd);

	__FUNC_EXIT__;
}
#ifndef MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE
static void _mouseup_wifi_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;
	if (ev->button == 3) {
		struct ug_data *ugd = (struct ug_data *) data;

		evas_object_del(ugd->act_popup);
		ugd->act_popup = NULL;
	}
}

static void _keydown_wifi_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{

#if defined(KEY)
	Evas_Event_Key_Down *ev = event_info;
	if (!strcmp(ev->keyname, KEY_BACK)) {
		struct ug_data *ugd = (struct ug_data *) data;

		ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
		wfd_ug_view_refresh_glitem(ugd->head);

		evas_object_del(ugd->act_popup);
		ugd->act_popup = NULL;
	}
#endif
}
#endif /* MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE */


static void _mouseup_hotspot_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;
	if (ev->button == 3) {
		struct ug_data *ugd = (struct ug_data *) data;

#ifdef WFD_ON_OFF_GENLIST
		wfd_ug_refresh_on_off_check(ugd);
#endif
		evas_object_del(ugd->act_popup);
		ugd->act_popup = NULL;
	}
}
static void _keydown_hotspot_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
#if defined(KEY)
	Evas_Event_Key_Down *ev = event_info;
	if (!strcmp(ev->keyname, KEY_BACK)) {
		struct ug_data *ugd = (struct ug_data *) data;

		ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
		wfd_ug_view_refresh_glitem(ugd->head);

		evas_object_del(ugd->act_popup);
		ugd->act_popup = NULL;
	}
#endif
}

static void _mouseup_disconnect_all_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;
	if (ev->button == 3) {
		struct ug_data *ugd = (struct ug_data *) data;
		if (NULL == ugd) {
			DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
			return;
		}

		evas_object_del(ugd->act_popup);
		ugd->act_popup = NULL;
	}
}

/*static void _keydown_disconnect_all_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Key_Down *ev = event_info;
	if (!strcmp(ev->keyname, KEY_BACK)) {
		struct ug_data *ugd = (struct ug_data *) data;
		if (NULL == ugd) {
			DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
			return;
		}

		evas_object_del(ugd->act_popup);
		ugd->act_popup = NULL;
	}
}*/


/**
 *	This function let the ug call it when click 'ok' button in hotspot action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_act_popup_hotspot_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	int result = -1;
	struct ug_data *ugd = (struct ug_data *) data;

	result = wfd_mobile_ap_off(ugd);
	if (0 == result) {
#ifdef WFD_ON_OFF_GENLIST
		/* refresh the header */
		if (ugd->on_off_check) {
			elm_check_state_set(ugd->on_off_check, TRUE);
			elm_object_disabled_set(ugd->on_off_check, TRUE);
		}
#endif

		/* while activating/deactivating, disable the buttons */
		if (ugd->scan_toolbar) {
			wfd_ug_view_refresh_button(ugd->scan_toolbar, "IDS_WIFI_SK4_SCAN", FALSE);
		}

		if (ugd->multiconn_scan_stop_btn) {
			wfd_ug_view_refresh_button(ugd->multiconn_scan_stop_btn, "IDS_WIFI_SK4_SCAN", FALSE);
		}

		if (ugd->back_btn) {
			elm_object_disabled_set(ugd->back_btn, TRUE);
		}
		ugd->is_hotspot_locally_disabled = TRUE;
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;
	__FUNC_EXIT__;
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
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

#ifdef WFD_ON_OFF_GENLIST
	wfd_ug_refresh_on_off_check(ugd);
#endif

	wfd_client_destroy_tethering(ugd);

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;
	__FUNC_EXIT__;
}

#ifndef MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE
/**
 *	This function let the ug call it when click 'ok' button in wifi action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_act_popup_wifi_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	int result = -1;
	struct ug_data *ugd = (struct ug_data *) data;

	result = wfd_wifi_off(ugd);
	if (0 == result) {
#ifdef WFD_ON_OFF_GENLIST
		/* refresh the header */
		if (ugd->on_off_check) {
			elm_check_state_set(ugd->on_off_check, TRUE);
			elm_object_disabled_set(ugd->on_off_check, TRUE);
		}
#endif
		/* while activating/deactivating, disable the buttons */
		if (ugd->scan_toolbar) {
			wfd_ug_view_refresh_button(ugd->scan_toolbar, "IDS_WIFI_SK4_SCAN", FALSE);
		}

		if (ugd->multiconn_scan_stop_btn) {
			wfd_ug_view_refresh_button(ugd->multiconn_scan_stop_btn, "IDS_WIFI_SK4_SCAN", FALSE);
		}

		if (ugd->back_btn) {
			elm_object_disabled_set(ugd->back_btn, TRUE);
		}
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;
	__FUNC_EXIT__;
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
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

#ifdef WFD_ON_OFF_GENLIST
	wfd_ug_refresh_on_off_check(ugd);
#endif
	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;
	__FUNC_EXIT__;
}

#endif /* MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE */

gboolean _wfd_disconnect_idle_cb(gpointer user_data)
{
	__FUNC_ENTER__;
	GList *iterator = NULL;

	struct ug_data *ugd = (struct ug_data *) user_data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return FALSE;
	}
	Evas_Object *content;
	content = elm_object_part_content_unset(ugd->layout, "button.next");
	WFD_IF_DEL_OBJ(content);

	wfd_ug_view_init_genlist(ugd, true);

	if (0 != wfd_client_disconnect(NULL)) {
		DBG(LOG_ERROR, "Disconnection Failed !!!\n");
	}

	ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
	discover_cb(WIFI_DIRECT_ERROR_NONE, WIFI_DIRECT_DISCOVERY_STARTED, ugd);

	if (ugd->multi_connect_mode != WFD_MULTI_CONNECT_MODE_NONE) {
		wfd_free_multi_selected_peers(ugd);
	} else {
		/* update the connecting icon */
		for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
			((device_type_s *)iterator->data)->conn_status = PEER_CONN_STATUS_DISCONNECTED;
			wfd_ug_view_refresh_glitem(((device_type_s *)iterator->data)->gl_item);
		}
	}

	__FUNC_EXIT__;
	return FALSE;
}

gboolean _wfd_cancel_connect_idle_cb(gpointer user_data)
{
	__FUNC_ENTER__;

	int res;
	GList *iterator = NULL;
	struct ug_data *ugd = (struct ug_data *) user_data;
	if (NULL == ugd || NULL == ugd->mac_addr_connecting) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return FALSE;
	}

	res = wifi_direct_cancel_connection(ugd->mac_addr_connecting);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed wifi_direct_cancel_connection() [%d]\n", res);
		return FALSE;
	}

	ugd->mac_addr_connecting = NULL;
	if (ugd->multi_connect_mode != WFD_MULTI_CONNECT_MODE_NONE) {
		if (ugd->raw_connected_peer_cnt > 0) {
			res = wifi_direct_disconnect_all();
			if (res != WIFI_DIRECT_ERROR_NONE) {
				DBG(LOG_ERROR, "Failed wifi_direct_disconnect_all() [%d]\n", res);
				return FALSE;
			}
		}

		wfd_free_multi_selected_peers(ugd);
		wfd_ug_view_init_genlist(ugd, true);
	} else {
		/* update the connecting icon */
		for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
			((device_type_s *)iterator->data)->conn_status = PEER_CONN_STATUS_DISCONNECTED;
		}
		wfd_ug_view_init_genlist(ugd, false);
	}

	__FUNC_EXIT__;
	return FALSE;
}

/**
 *	This function let the ug call it when click 'ok' button in cancel connection popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_act_popup_cancel_connect_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	g_idle_add(_wfd_cancel_connect_idle_cb, ugd);

	__FUNC_EXIT__;
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
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	g_idle_add(_wfd_disconnect_idle_cb, ugd);

	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'cancel' button in disconnect all popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_act_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	evas_object_del(ugd->act_popup);
	ugd->act_popup = NULL;

	__FUNC_EXIT__;
}

void act_popup_language_changed(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	int popup_type = (int)data;
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	struct ug_data *ugd = wfd_get_ug_data();

	if (!ugd || !ugd->act_popup) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return;
	}

	btn1 = elm_object_part_content_get(ugd->act_popup, "button1");
	btn2 = elm_object_part_content_get(ugd->act_popup, "button2");
	elm_object_domain_translatable_text_set(btn1, PACKAGE,
			"IDS_BR_SK_CANCEL");
	elm_object_domain_translatable_text_set(btn2, PACKAGE,
			"IDS_BR_SK_OK");

	switch(popup_type) {

	case POPUP_TYPE_HOTSPOT_OFF:
		elm_object_domain_translatable_text_set(btn1, PACKAGE,
				"IDS_BR_SK_NO");
		elm_object_domain_translatable_text_set(btn2, PACKAGE,
				"IDS_BR_SK_YES");
		elm_object_domain_translatable_text_set(ugd->act_popup, PACKAGE,
				"IDS_WIFI_BODY_USING_WI_FI_DIRECT_WILL_DISCONNECT_CURRENT_WI_FI_TETHERING_CONTINUE_Q");
		break;

	case POP_TYPE_DISCONNECT:
		elm_object_domain_translatable_text_set(ugd->act_popup, PACKAGE,
				"IDS_WIFI_POP_CURRENT_CONNECTION_WILL_BE_DISCONNECTED_CONTINUE_Q");
		break;
	case POP_TYPE_SCAN_AGAIN:
		elm_object_domain_translatable_text_set(ugd->act_popup, PACKAGE,
				"IDS_WIFI_BODY_CURRENT_CONNECTION_WILL_BE_DISCONNECTED_SO_THAT_SCANNING_CAN_START_CONTINUE_Q");
		break;
	case POP_TYPE_CANCEL_CONNECT:
		elm_object_domain_translatable_text_set(ugd->act_popup, PACKAGE,
				"IDS_WIFI_POP_THIS_WI_FI_DIRECT_CONNECTION_WILL_BE_CANCELLED");
		break;
	case POP_TYPE_ACCEPT_CONNECTION:
		elm_object_domain_translatable_text_set(ugd->act_popup, PACKAGE,
				"IDS_WIFI_BODY_PS_IS_REQUESTING_A_WI_FI_DIRECT_CONNECTION_ALLOW_Q");
		break;
	default:
		break;
	}

	__FUNC_EXIT__;
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
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	Evas_Object *popup = NULL;
	Evas_Object *btn1 = NULL, *btn2 = NULL;

	WFD_IF_DEL_OBJ(ugd->act_popup);

	popup = elm_popup_add(ugd->base);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(popup, EA_CALLBACK_BACK, eext_popup_back_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_domain_translatable_part_text_set(popup, "title,text",
			 PACKAGE, _("IDS_WIFI_HEADER_WI_FI_DIRECT_CONNECTION_ABB"));
	elm_object_domain_translatable_text_set(popup, PACKAGE, message);
	evas_object_smart_callback_add(popup, "language,changed", act_popup_language_changed, (void *)popup_type);

	btn1 = elm_button_add(popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_style_set(btn2, "popup");

	/* set the different text by type */
	if (
#ifndef MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE
		popup_type == POPUP_TYPE_WIFI_OFF ||
#endif

		popup_type == POPUP_TYPE_HOTSPOT_OFF ||

		FALSE) {
		elm_object_domain_translatable_part_text_set(popup, "title,text",
				 PACKAGE, "IDS_WIFI_BODY_WI_FI_DIRECT_ABB");
		elm_object_domain_translatable_text_set(btn1, PACKAGE,
				"IDS_BR_SK_CANCEL");
		elm_object_domain_translatable_text_set(btn2, PACKAGE,
				"IDS_ST_BUTTON_ENABLE");
	} else if (popup_type == POP_TYPE_CANCEL_CONNECT) {
		elm_object_domain_translatable_text_set(btn1, PACKAGE,
				"IDS_BR_SK_CANCEL");
		elm_object_domain_translatable_text_set(btn2, PACKAGE,
				"IDS_WIFI_SK_STOP");
	} else if (popup_type == POP_TYPE_DISCONNECT) {
		elm_object_domain_translatable_part_text_set(popup, "title,text",
				 PACKAGE, "IDS_WIFI_SK_DISCONNECT");
		elm_object_domain_translatable_text_set(btn1, PACKAGE,
				"IDS_BR_SK_CANCEL");
		elm_object_domain_translatable_text_set(btn2, PACKAGE,
				"IDS_BR_SK_OK");

	} else if (popup_type == POP_TYPE_SCAN_AGAIN) {
		elm_object_domain_translatable_part_text_set(popup, "title,text",
				 PACKAGE, "IDS_WIFI_SK4_SCAN");
		elm_object_domain_translatable_text_set(btn1, PACKAGE,
				"IDS_BR_SK_CANCEL");
		elm_object_domain_translatable_text_set(btn2, PACKAGE,
				"IDS_BR_SK_OK");
	} else if (popup_type == POP_TYPE_ACCEPT_CONNECTION) {
		elm_object_domain_translatable_part_text_set(popup, "title,text",
				 PACKAGE, "IDS_WIFI_BODY_WI_FI_DIRECT_ABB");
		elm_object_domain_translatable_text_set(btn1, PACKAGE,
				"IDS_BR_SK_CANCEL");
		elm_object_domain_translatable_text_set(btn2, PACKAGE,
				"IDS_WIFI_SK_CONNECT");
	} else {
		elm_object_domain_translatable_text_set(btn1, PACKAGE,
				"IDS_BR_SK_CANCEL");
		elm_object_domain_translatable_text_set(btn2, PACKAGE,
				"IDS_BR_SK_OK");
	}

	elm_object_part_content_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button2", btn2);

	/* set the different callback by type */
#ifndef MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE
	if (popup_type == POPUP_TYPE_WIFI_OFF) {
		evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_wifi_cb, (void *)ugd);
//		evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, _keydown_wifi_cb, (void *)ugd);

		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_wifi_cancel_cb, (void *)ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_wifi_ok_cb, (void *)ugd);
	} else
#endif /* MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE */

	if (popup_type == POPUP_TYPE_HOTSPOT_OFF) {
		evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_hotspot_cb, (void *)ugd);
//		evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, _keydown_hotspot_cb, (void *)ugd);

		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_hotspot_cancel_cb, (void *)ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_hotspot_ok_cb, (void *)ugd);
	} else

	if (popup_type == POP_TYPE_DISCONNECT ||
		popup_type == POP_TYPE_SCAN_AGAIN) {
		evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_disconnect_all_cb, (void *)ugd);
//		evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, _keydown_disconnect_all_cb, (void *)ugd);

		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_cancel_cb, (void *)ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_disconnect_all_ok_cb, (void *)ugd);
	} else if (popup_type == POP_TYPE_CANCEL_CONNECT) {
		evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_disconnect_all_cb, (void *)ugd);

		evas_object_smart_callback_add(btn1, "clicked", _wfd_ug_act_popup_cancel_cb, (void *)ugd);
		evas_object_smart_callback_add(btn2, "clicked", _wfd_ug_act_popup_cancel_connect_ok_cb, (void *)ugd);
	} else if (popup_type == POP_TYPE_ACCEPT_CONNECTION ) {
		evas_object_smart_callback_add(btn1, "clicked",
				pushbutton_reject_connect_cb, (void *)ugd);
		evas_object_smart_callback_add(btn2, "clicked",
				pushbutton_accept_connect_cb, (void *)ugd);
	}

	evas_object_show(popup);
	ugd->act_popup = popup;
	__FUNC_EXIT__;
}

/**
 *	This function let the ug remove the action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfg_ug_act_popup_remove(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	if (ugd->act_popup) {
		evas_object_del(ugd->act_popup);
		ugd->act_popup = NULL;
	}
	__FUNC_EXIT__;
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
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	evas_object_del(ugd->warn_popup);
	ugd->warn_popup = NULL;

	wfd_ug_view_free_peers(ugd);

	ug_destroy_me(ugd->ug);
	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'ok' button in warning popup of turning off WFD automatically
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
/*static void _wfd_ug_automatic_turn_off_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	evas_object_del(ugd->warn_popup);
	ugd->warn_popup = NULL;

	__FUNC_EXIT__;
}*/

/**
 *	This function let the ug call it when click 'ok' button in warning popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_ug_warn_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	evas_object_del(ugd->warn_popup);
	ugd->warn_popup = NULL;

	if (ugd->rename_entry) {
		elm_object_focus_set(ugd->rename_entry, EINA_TRUE);
	}

	__FUNC_EXIT__;
}

void warn_popup_language_changed(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	int popup_type = (int)data;
	struct ug_data *ugd = wfd_get_ug_data();
	char popup_text[MAX_POPUP_TEXT_SIZE] = {0};
	Evas_Object *btn = NULL;

	if (!ugd || !ugd->warn_popup) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return;
	}

	btn = elm_object_part_content_get(ugd->warn_popup, "button1");
	elm_object_domain_translatable_text_set(btn, PACKAGE,
			"IDS_BR_SK_OK");

	switch(popup_type) {
	case POPUP_TYPE_ACTIVATE_FAIL_POLICY_RESTRICTS:
		elm_object_domain_translatable_text_set(ugd->warn_popup, PACKAGE,
				"IDS_COM_POP_SECURITY_POLICY_RESTRICTS_USE_OF_WI_FI");
		break;
	case POPUP_TYPE_TERMINATE:
		elm_object_domain_translatable_text_set(ugd->warn_popup, PACKAGE,
				"IDS_COM_POP_FAILED");
		break;
	case POPUP_TYPE_TERMINATE_DEACTIVATE_FAIL:
		elm_object_domain_translatable_text_set(ugd->warn_popup, PACKAGE,
				"IDS_WIFI_POP_DEACTIVATION_FAILED");
		break;
	case POPUP_TYPE_TERMINATE_NOT_SUPPORT:
		elm_object_domain_translatable_text_set(ugd->warn_popup, PACKAGE,
				"IDS_ST_POP_NOT_SUPPORTED");
		break;
	case POP_TYPE_BUSY_DEVICE_POPUP:
		elm_object_domain_translatable_text_set(ugd->warn_popup, PACKAGE,
				"IDS_ST_POP_DEVICE_CONNECTED_TO_ANOTHER_DEVICE");
		break;
	case POP_TYPE_MULTI_CONNECT_POPUP:
		snprintf(popup_text, MAX_POPUP_TEXT_SIZE, _("IDS_ST_POP_YOU_CAN_CONNECT_UP_TO_PD_DEVICES_AT_THE_SAME_TIME"), MAX_CONNECTED_PEER_NUM);
		elm_object_domain_translatable_text_set(ugd->warn_popup, PACKAGE,
				popup_text);
		break;
	default:
		break;
	}

	__FUNC_EXIT__;
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
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	Evas_Object *popup = NULL;
	Evas_Object *btn = NULL;

	popup = elm_popup_add(ugd->base);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(popup, EA_CALLBACK_BACK, eext_popup_back_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	if (popup_type == POP_TYPE_MULTI_CONNECT_POPUP ||
		popup_type == POP_TYPE_BUSY_DEVICE_POPUP) {
		elm_object_domain_translatable_part_text_set(popup, "title,text",
				 PACKAGE, "IDS_WIFI_HEADER_UNABLE_TO_CONNECT_ABB2");
	} else {
		elm_object_domain_translatable_part_text_set(popup, "title,text",
				 PACKAGE, _("IDS_WIFI_HEADER_WI_FI_DIRECT_CONNECTION_ABB"));
	}
	elm_object_domain_translatable_text_set(popup, PACKAGE, message);
	evas_object_smart_callback_add(popup, "language,changed", warn_popup_language_changed, (void *)popup_type);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup");
	elm_object_domain_translatable_text_set(btn, PACKAGE, "IDS_BR_SK_OK");
	elm_object_part_content_set(popup, "button1", btn);
	if (popup_type == POPUP_TYPE_TERMINATE ||
		popup_type == POPUP_TYPE_TERMINATE_DEACTIVATE_FAIL ||
		popup_type == POPUP_TYPE_TERMINATE_NOT_SUPPORT) {
		evas_object_smart_callback_add(btn, "clicked", _wfd_ug_terminate_popup_cb, (void *)ugd);
	} else {
		evas_object_smart_callback_add(btn, "clicked", _wfd_ug_warn_popup_cb, (void *)ugd);
	}

	evas_object_show(popup);
	ugd->warn_popup = popup;
	__FUNC_EXIT__;
}

