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
#include <app_control.h>
#include <wifi-direct.h>
#include <efl_extension.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

void scan_button_create(struct ug_data *ugd)
{
	__FUNC_ENTER__;

	Evas_Object *btn;
	btn = elm_button_add(ugd->layout);
	/* Use "bottom" style button */
	elm_object_style_set(btn, "bottom");
	elm_object_domain_translatable_text_set(btn, PACKAGE, "IDS_WIFI_SK4_SCAN");
	ugd->scan_toolbar = btn;
	if (ugd->wfd_status <= WIFI_DIRECT_STATE_DEACTIVATING) {
		wfd_ug_view_refresh_button(ugd->scan_toolbar, "IDS_WIFI_SK4_SCAN",
			FALSE);
	}
	evas_object_smart_callback_add(btn, "clicked",_scan_btn_cb, (void *)ugd);
	/* Set button into "toolbar" swallow part */
	elm_object_part_content_set(ugd->layout, "button.big", btn);
	evas_object_show(ugd->scan_toolbar);

	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'back' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
Eina_Bool _back_btn_cb(void *data, Elm_Object_Item *it)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	WFD_RETV_IF(ugd == NULL, FALSE, "The param is NULL\n");
	int ret = -1;
	bool owner = FALSE;
	app_control_h control = NULL;

#ifdef WFD_DBUS_LAUNCH
	if (ugd->dbus_cancellable != NULL) {
		g_cancellable_cancel(ugd->dbus_cancellable);
		g_object_unref(ugd->dbus_cancellable);
		ugd->dbus_cancellable = NULL;
		if (ugd->conn) {
			g_object_unref(ugd->conn);
			ugd->conn = NULL;
		}
		DBG(LOG_INFO, "Cancel dbus call");
	}
#endif

	wfd_refresh_wifi_direct_state(ugd);
	if (ugd->wfd_status <= WIFI_DIRECT_STATE_DEACTIVATING) {
		DBG(LOG_INFO, "WiFi direct is already deactivated\n");
		goto cleanup;
	}

	if (NULL != ugd->mac_addr_connecting) {
		if (ugd->is_conn_incoming) {
			DBG(LOG_INFO, "Reject the incoming connection before client deregister \n");
			ret = wifi_direct_reject_connection(ugd->mac_addr_connecting);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				DBG(LOG_ERROR, "Failed to send reject request [%d]\n", ret);
			}
		} else {
			DBG(LOG_INFO, "Cancel the outgoing connection before client deregister \n");
			ret = wifi_direct_cancel_connection(ugd->mac_addr_connecting);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				DBG(LOG_ERROR, "Failed to send cancel request [%d]\n", ret);
			}
		}
		ugd->mac_addr_connecting = NULL;
	}

	if (ugd->raw_connected_peer_cnt == 0) {
		ret = wifi_direct_is_group_owner(&owner);
		if (ret == WIFI_DIRECT_ERROR_NONE) {
			if (owner) {
				wifi_direct_destroy_group();
			}
		}
	}

cleanup:
	wfd_ug_view_free_peers(ugd);
	ret = app_control_create(&control);
	if (ret) {
		DBG(LOG_ERROR, "Failed to create control");
	} else {
		if (ugd->wfd_status > WIFI_DIRECT_STATE_CONNECTING) {
			app_control_add_extra_data(control, "Connection", "TRUE");
		} else {
			app_control_add_extra_data(control, "Connection", "FALSE");
		}

		ug_send_result(ugd->ug, control);
		app_control_destroy(control);
	}

	ug_destroy_me(ugd->ug);
	__FUNC_EXIT__;
	return FALSE;
}

void wfd_cancel_progressbar_stop_timer(struct ug_data *ugd)
{
	__FUNC_ENTER__;

	if(ugd->timer_stop_progress_bar > 0) {
		g_source_remove(ugd->timer_stop_progress_bar);
	}
	ugd->timer_stop_progress_bar = 0;

	__FUNC_EXIT__;
}

void wfd_cancel_not_alive_delete_timer(struct ug_data *ugd)
{
	__FUNC_ENTER__;

	if(ugd->timer_delete_not_alive_peer > 0) {
		g_source_remove(ugd->timer_delete_not_alive_peer);
	}
	ugd->timer_delete_not_alive_peer = 0;

	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click failed devices item
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _gl_failed_peer_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = wfd_get_ug_data();
	int ret = -1;

	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	if (ugd->display_timer != NULL) {
		ecore_timer_del(ugd->display_timer);
		ugd->display_timer = NULL;
	}

	wfd_refresh_wifi_direct_state(ugd);
	DBG(LOG_INFO, "Start discovery again, status: %d\n", ugd->wfd_status);

	/* if connected, show the popup*/
	if (ugd->wfd_status >= WIFI_DIRECT_STATE_CONNECTED) {
		wfd_ug_act_popup(ugd, _("IDS_WIFI_BODY_CURRENT_CONNECTION_WILL_BE_DISCONNECTED_SO_THAT_SCANNING_CAN_START_CONTINUE_Q"), POP_TYPE_SCAN_AGAIN);
	} else {
		ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
		ret = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
		if (ret != WIFI_DIRECT_ERROR_NONE) {
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
			DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", ret);
			wifi_direct_cancel_discovery();
		}
	}

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug call it when click 'scan' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _scan_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}
	int ret = -1;
	const char *btn_text = NULL;
	btn_text = elm_object_part_text_get(ugd->scan_toolbar, "default");
	DBG(LOG_INFO, "Button text=%s",btn_text);

	if (!g_strcmp0(elm_object_text_get(obj), _("IDS_WIFI_SK4_SCAN"))) {
		wfd_refresh_wifi_direct_state(ugd);
		DBG(LOG_INFO, "Start discovery again, status: %d\n", ugd->wfd_status);
		/* if connected, show the popup*/
		if (ugd->wfd_status >= WIFI_DIRECT_STATE_CONNECTED || ugd->raw_connected_peer_cnt > 0) {
			wfd_ug_act_popup(ugd, _("IDS_WIFI_BODY_CURRENT_CONNECTION_WILL_BE_DISCONNECTED_SO_THAT_SCANNING_CAN_START_CONTINUE_Q"), POP_TYPE_SCAN_AGAIN);
		} else if (WIFI_DIRECT_STATE_DEACTIVATED == ugd->wfd_status) {
			wfd_client_switch_on(ugd);
			__FUNC_EXIT__;
			return;
		} else {
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
			ret = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
				DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", ret);
				wifi_direct_cancel_discovery();
			}
		}
	} else if (!g_strcmp0(elm_object_text_get(obj), _("IDS_WIFI_SK_STOP"))) {
		DBG(LOG_INFO, "Stop pressed.\n");
		ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_STOPPED;
		wfd_cancel_progressbar_stop_timer(ugd);
		wfd_delete_progressbar_cb(ugd);
		wfd_cancel_not_alive_delete_timer(ugd);
	} else if (0 == strcmp(_("IDS_WIFI_SK2_CANCEL_CONNECTION"), btn_text)) {
		DBG(LOG_INFO, "Cancel Connection");
		wfd_ug_act_popup(ugd, _("IDS_WIFI_POP_THIS_WI_FI_DIRECT_CONNECTION_WILL_BE_CANCELLED"), POP_TYPE_CANCEL_CONNECT);
	} else {
		DBG(LOG_INFO, "Invalid Case\n");
	}
	__FUNC_EXIT__;
	return;
}

void wfd_check_gl_busy_peers(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	if (ugd->gl_busy_peer_cnt == 0) {
		WFD_IF_DEL_ITEM(ugd->busy_wfd_item);
	}
	__FUNC_EXIT__;
}

void wfd_check_gl_available_peers(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	if (ugd->gl_available_peer_cnt == 0 && ugd->avlbl_wfd_item != NULL) {
		WFD_IF_DEL_ITEM(ugd->avlbl_wfd_item);
	}
	__FUNC_EXIT__;
}

/**
 *	This function let the ug free some peer in genlist
 *	@return  void
 *	@param[in] start_pos the start position of peers list
 *	@param[in] mac_addr the mac_addr of peer for free
 *	@param[in] cnt the count of gl peers in list
 */
void free_gl_peer(device_type_s **start_pos, const char *mac_addr, int *cnt)
{
	__FUNC_ENTER__;
	device_type_s *peer = *start_pos;
	device_type_s *peer_tmp = peer;

	if (peer == NULL) {
		DBG(LOG_INFO, "no peer in genlist");
		return;
	}

	while (peer) {
		if(strcmp(peer->mac_addr, mac_addr)) {
			peer_tmp = peer;
			peer = peer->next;
		} else {
			if(peer->next != NULL) {
				peer_tmp->next = peer->next;
				break;
			} else {
				peer_tmp->next = NULL;
				break;
			}
		}
	}

	if (peer == *start_pos) {
		DBG(LOG_INFO, "the head is free");
		*start_pos = peer->next;
	}

	(*cnt)--;
	if (peer) {
		WFD_IF_DEL_ITEM(peer->gl_item);
		peer->next = NULL;
		free(peer);
	}
	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click avaliable peer to connect
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _gl_peer_sel(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	int res = -1;
	char txt[MAX_POPUP_TEXT_SIZE] = {0,};
	char popup_text[MAX_POPUP_TEXT_SIZE] = {0, };
	bool is_peer_alive = false;
	struct ug_data *ugd = wfd_get_ug_data();
	device_type_s *peer = (device_type_s *)data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (!ugd || !peer) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return;
	}

	wfd_ug_get_connected_peers(ugd);
	DBG(LOG_INFO, "No of connected peers= %d",ugd->raw_connected_peer_cnt);

	if (ugd->raw_connected_peer_cnt >= MAX_CONNECTED_PEER_NUM) {
		snprintf(popup_text, MAX_POPUP_TEXT_SIZE, _("IDS_ST_POP_YOU_CAN_CONNECT_UP_TO_PD_DEVICES_AT_THE_SAME_TIME"), MAX_CONNECTED_PEER_NUM);
		wfd_ug_warn_popup(ugd, popup_text, POP_TYPE_MULTI_CONNECT_POPUP);
		if (item) {
			elm_genlist_item_selected_set(item, EINA_FALSE);
		}
		return;
	}

	if (ugd->disconnect_btn) {
		Evas_Object *content;
		content = elm_object_part_content_unset(ugd->layout, "button.next");
		WFD_IF_DEL_OBJ(content);
		ugd->disconnect_btn = NULL;
		elm_object_part_content_set(ugd->layout, "button.big",
			ugd->scan_toolbar);
	}

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	GList *iterator = NULL;

	for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
		if (!strncmp(peer->mac_addr, (const char *)((device_type_s *)iterator->data)->mac_addr, MAC_LENGTH)) {
			/* peer is found in last discovery */
			is_peer_alive = true;
			break;
		}
	}

	if (!is_peer_alive) {
		/* peer exists only in genlist, waiting to be deleted */
		device_type_s *peer_start = ugd->gl_avlb_peers_start;
		for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
			if(!strncmp(peer_start->mac_addr, peer->mac_addr, MAC_LENGTH)) {
				DBG(LOG_INFO, "Device [%s] found in genlist, but it is already lost", ((device_type_s *)iterator->data)->ssid);
				sprintf(txt, "Cannot find device %s", ((device_type_s *)iterator->data)->ssid);
				free_gl_peer(&ugd->gl_avlb_peers_start, ((device_type_s *)iterator->data)->mac_addr, &ugd->gl_available_peer_cnt);
				wfd_check_gl_available_peers(ugd);
				wfd_ug_warn_popup(ugd, txt, POPUP_TYPE_INFO);
				return;
			}
		}
	}

	wfd_cancel_not_alive_delete_timer(ugd);

	/* get WFD status */
	wfd_refresh_wifi_direct_state(ugd);

	if (PEER_CONN_STATUS_DISCONNECTED == peer->conn_status ||
		peer->is_group_owner == TRUE) {
		DBG_SECURE(LOG_DEBUG, "Connect with peer ["MACSECSTR"]\n",
			MAC2SECSTR(peer->mac_addr));

		if (WIFI_DIRECT_STATE_CONNECTING == ugd->wfd_status) {
			DBG(LOG_DEBUG, "It's in connecting status now.\n");
			return;
		}

		ugd->mac_addr_connecting = peer->mac_addr;
		res = wfd_client_connect((const char *)peer->mac_addr);
		if (res != 0) {
			DBG(LOG_ERROR, "Failed to send connection request. [%d]\n", res);
			return;
		}
	}

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug call it when click busy peer
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _gl_busy_peer_sel(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)wfd_get_ug_data();
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	if (NULL == ugd) {
		DBG(LOG_ERROR, "Data is NULL\n");
		return;
	}

	wfd_ug_warn_popup(ugd, _("IDS_ST_POP_DEVICE_CONNECTED_TO_ANOTHER_DEVICE"), POP_TYPE_BUSY_DEVICE_POPUP);

	__FUNC_EXIT__;
}

void ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ugd = (struct ug_data *) data;

	if (!ugd) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	if (ugd->ctxpopup) {
		evas_object_del(ugd->ctxpopup);
		ugd->ctxpopup = NULL;
	}
}

void _ctxpopup_move()
{
	__FUNC_ENTER__;

	int win_w = 0, win_h = 0;
	int move_x = 0, move_y = 0;
	int changed_ang = 0;
	struct ug_data *ugd = wfd_get_ug_data();

	if (!ugd || !ugd->win) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return;
	}

	if (!ugd->ctxpopup) {
		DBG(LOG_INFO, "NULL parameters.\n");
		return;
	}

	elm_win_screen_size_get(ugd->win, NULL, NULL, &win_w, &win_h);
	changed_ang = elm_win_rotation_get(ugd->win);

	switch (changed_ang) {
		case 0:
		case 180:
			move_x = win_w/2;
			move_y = win_h;
			break;

		case 90:
			move_x = win_h/2;
			move_y = win_w;
			break;

		case 270:
			move_x = win_h/2;
			move_y = win_w;
			break;

		default:
			move_x = 0;
			move_y = 0;
			break;
	}

	evas_object_move(ugd->ctxpopup, move_x, move_y);

	__FUNC_EXIT__;
}

void _create_mluti_connect_view(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	WFD_RET_IF(ugd == NULL, "The param is NULL\n");
	WFD_IF_DEL_OBJ(ugd->ctxpopup);
	int ret = 0;

	wfd_client_free_raw_discovered_peers(ugd);
	ugd->raw_discovered_peer_cnt = 0;
	wfd_create_multiconnect_view(ugd);
	ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
	ret = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
		DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", ret);
		wifi_direct_cancel_discovery();
	}

	__FUNC_EXIT__;
}

void _more_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	Evas_Object *naviframe = (Evas_Object *)data;
	Elm_Object_Item *multi_connect_item = NULL;
	Elm_Object_Item *rename_item = NULL;
	struct ug_data *ugd = wfd_get_ug_data();

	if (!naviframe || !ugd) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return;
	}

	ugd->more_btn_multiconnect_item = NULL;

	if (ugd->ctxpopup) {
		evas_object_del(ugd->ctxpopup);
	}

	ugd->ctxpopup = elm_ctxpopup_add(naviframe);
	elm_object_style_set(ugd->ctxpopup, "more/default");
	eext_object_event_callback_add(ugd->ctxpopup, EA_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(ugd->ctxpopup, EA_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ugd->ctxpopup, "dismissed", ctxpopup_dismissed_cb, ugd);
	elm_ctxpopup_auto_hide_disabled_set(ugd->ctxpopup, EINA_TRUE);

	elm_ctxpopup_direction_priority_set(ugd->ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
					ELM_CTXPOPUP_DIRECTION_LEFT,
					ELM_CTXPOPUP_DIRECTION_RIGHT,
					ELM_CTXPOPUP_DIRECTION_DOWN);

	_ctxpopup_move();

	multi_connect_item = elm_ctxpopup_item_append(ugd->ctxpopup, "IDS_WIFI_BUTTON_MULTI_CONNECT", NULL, _create_mluti_connect_view, ugd);
	elm_object_item_domain_text_translatable_set(multi_connect_item, PACKAGE, EINA_TRUE);
	ugd->more_btn_multiconnect_item = multi_connect_item;

	wfd_refresh_wifi_direct_state(ugd);
	if (WIFI_DIRECT_STATE_CONNECTING == ugd->wfd_status ||
		WIFI_DIRECT_STATE_CONNECTED == ugd->wfd_status ||
		WIFI_DIRECT_STATE_DEACTIVATED == ugd->wfd_status ||
		ugd->raw_connected_peer_cnt > 0) {
		elm_object_item_disabled_set(multi_connect_item, TRUE);
	}

	rename_item = elm_ctxpopup_item_append(ugd->ctxpopup, "IDS_ST_BODY_RENAME_DEVICE_ABB", NULL, _gl_rename_device_sel, ugd);
	elm_object_item_domain_text_translatable_set(rename_item, PACKAGE, EINA_TRUE);
	evas_object_show(ugd->ctxpopup);

	__FUNC_EXIT__;
}

/**
 *	This function make items into group
*/
void _wfd_realize_item(Elm_Object_Item *pre_item, int count)
{
	__FUNC_ENTER__;
	int i = 0;
	if (count < 1 || pre_item == NULL) {
		return;
	}

	Elm_Object_Item *item = elm_genlist_item_next_get(pre_item);
	if (item == NULL) {
		return;
	}

	if (count == 1) {
		elm_object_item_signal_emit(item, "elm,state,normal", "");
		return;
	}

	for (i = 0; i < count; i++) {
		if (i == 0) {
			elm_object_item_signal_emit(item, "elm,state,top", "");
		} else if (i == count - 1) {
			elm_object_item_signal_emit(item, "elm,state,bottom", "");
		} else {
			elm_object_item_signal_emit(item, "elm,state,center", "");
		}

		item = elm_genlist_item_next_get(item);
	}
	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when unresized event is received
*/
static void _gl_unrealized(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;

	_wfd_realize_item(ugd->avlbl_wfd_item, ugd->gl_available_peer_cnt);
	_wfd_realize_item(ugd->conn_wfd_item, ugd->gl_connected_peer_cnt);
	_wfd_realize_item(ugd->multi_connect_wfd_item, ugd->gl_multi_connect_peer_cnt);
	_wfd_realize_item(ugd->busy_wfd_item, ugd->gl_busy_peer_cnt);
	_wfd_realize_item(ugd->conn_failed_wfd_item, ugd->gl_connected_failed_peer_cnt);
	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when resized event is received
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	if (!data || !event_info) {
		DBG(LOG_ERROR, "Invalid parameters");
		return;
	}

	struct ug_data *ugd = (struct ug_data *)data;
#ifdef ACCESSIBILITY_FEATURE
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	int index = elm_genlist_item_index_get(item);
	char *sr_msg = NULL;
#endif

	_wfd_realize_item(ugd->avlbl_wfd_item, ugd->gl_available_peer_cnt);
	_wfd_realize_item(ugd->conn_wfd_item, ugd->gl_connected_peer_cnt);
	_wfd_realize_item(ugd->multi_connect_wfd_item, ugd->gl_multi_connect_peer_cnt);
	_wfd_realize_item(ugd->busy_wfd_item, ugd->gl_busy_peer_cnt);
	_wfd_realize_item(ugd->conn_failed_wfd_item, ugd->gl_connected_failed_peer_cnt);

#ifdef ACCESSIBILITY_FEATURE
	/* screen reader */
	if (GENLIST_HEADER_POS == index && item != NULL) {
		Evas_Object *check = elm_object_item_part_content_get(item, "elm.icon");
		if (check) {
			Eina_Bool state = elm_check_state_get(check);
			if (state) {
				sr_msg = strdup(SR_CHECKBOX_ON_MSG);
			} else {
				sr_msg = strdup(SR_CHECKBOX_OFF_MSG);
			}

			if (sr_msg) {
				Evas_Object *ao = NULL;
				ao = elm_object_item_access_object_get(item);
				elm_access_info_set(ao, ELM_ACCESS_CONTEXT_INFO, sr_msg);
				free(sr_msg);
			} else {
				DBG(LOG_ERROR, "index = %d, screen reader message create fail!", index);
			}
		} else {
			DBG(LOG_ERROR, "index = %d, get check box fail!", index);
		}
	}
#endif

	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'disconnect' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _wfd_ug_disconnect_button_cb(void *data, Evas_Object * obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	WFD_RET_IF(ugd == NULL, "Incorrect parameter(NULL)\n");

	wfd_ug_act_popup(ugd, _("IDS_WIFI_POP_CURRENT_CONNECTION_WILL_BE_DISCONNECTED_CONTINUE_Q"), POP_TYPE_DISCONNECT);

	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click "cancel connection" button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _wfd_ug_cancel_connection_button_cb(void *data, Evas_Object * obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	WFD_RET_IF(ugd == NULL, "Incorrect parameter(NULL)\n");

	wfd_ug_act_popup(ugd, _("IDS_WIFI_POP_THIS_WI_FI_DIRECT_CONNECTION_WILL_BE_CANCELLED"), POP_TYPE_CANCEL_CONNECT);

	__FUNC_EXIT__;
}


/**
 *	This function let the ug update the genlist item
 *	@return   void
 *	@param[in] gl_item the pointer to genlist item
 */
void wfd_ug_view_refresh_glitem(Elm_Object_Item *gl_item)
{
	__FUNC_ENTER__;
	if (gl_item != NULL) {
		elm_genlist_item_update(gl_item);
	}
	__FUNC_EXIT__;
}

/**
 *	This function let the ug refresh the attributes of button
 *	@return   void
 *	@param[in] tb_item the pointer to the toolbar button
 *	@param[in] text the pointer to the text of button
 *	@param[in] enable whether the button is disabled
 */
void wfd_ug_view_refresh_button(Evas_Object *tb_item, const char *text,
		int enable)
{
	__FUNC_ENTER__;

	if (NULL == tb_item || NULL == text) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	DBG(LOG_INFO, "Set the attributes of button: text[%s], enabled[%d]\n", text, enable);
	elm_object_domain_translatable_part_text_set(tb_item, "default",
			 PACKAGE, text);
	elm_object_disabled_set(tb_item, !enable);

	__FUNC_EXIT__;
}

/**
 *	This function let the ug know whether current device is connected by me
 *	@return   If connected, return TRUE, else return FALSE
 *	@param[in] ugd the pointer to the main data structure
 *	@param[in] dev the pointer to the device
 */
static bool __wfd_is_device_connected_with_me(struct ug_data *ugd, device_type_s *dev)
{
	__FUNC_ENTER__;
	int i = 0;

	for (i = 0; i < ugd->raw_connected_peer_cnt; i++) {
		if (strncmp(ugd->raw_connected_peers[i].mac_addr,
			dev->mac_addr, strlen(ugd->raw_connected_peers[i].mac_addr)) == 0) {
			return TRUE;
		}
	}

	__FUNC_EXIT__;
	return FALSE;
}

/**
 *	This function let the ug know whether current device is connected by other peer
 *	@return   If connected, return TRUE, else return FALSE
 *	@param[in] ugd the pointer to the main data structure
 *	@param[in] dev the pointer to the device
 */
static bool __wfd_is_device_busy(struct ug_data *ugd, device_type_s *dev)
{
	__FUNC_ENTER__;

	if (ugd->I_am_group_owner == TRUE) {
		if (dev->is_connected || dev->is_group_owner) {
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		if (dev->is_connected == TRUE && dev->is_group_owner == TRUE) {
			return FALSE;
		}

		if (dev->is_connected == TRUE && dev->is_group_owner == FALSE) {
			return TRUE;
		}

		if (dev->is_connected == FALSE) {
			return FALSE;
		}
	}

	__FUNC_EXIT__;
	return FALSE;
}

/**
 *	This function let the ug calculate how many devices are avaliable
 *	@return   TRUE
 *	@param[in] ugd the pointer to the main data structure
 *	@param[in] dev the pointer to the number of avaliable devices
 */
static bool __wfd_is_any_device_available(struct ug_data *ugd, int* no_of_available_dev)
{
	__FUNC_ENTER__;
	GList *iterator = NULL;

	for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
		/* Not include the device which is connected with me */
		if (__wfd_is_device_connected_with_me(ugd, (device_type_s *)iterator->data)) {
			continue;
		}
		if (!__wfd_is_device_busy(ugd, (device_type_s *)iterator->data) &&
			((device_type_s *)iterator->data)->conn_status != PEER_CONN_STATUS_FAILED_TO_CONNECT) {
			(*no_of_available_dev)++;
		}
	}

	__FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the ug calculate how many devices are busy
 *	@return   TRUE
 *	@param[in] ugd the pointer to the main data structure
 *	@param[in] dev the pointer to the number of busy devices
 */
static bool __wfd_is_any_device_busy(struct ug_data *ugd, int* no_of_busy_dev)
{
	__FUNC_ENTER__;
	GList *iterator = NULL;

	for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
		/* Not include the device which is connected with me */
		if (__wfd_is_device_connected_with_me(ugd, (device_type_s *)iterator->data)) {
			continue;
		}
		if (__wfd_is_device_busy(ugd, (device_type_s *)iterator->data)) {
			(*no_of_busy_dev)++;
		}
	}

	__FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the ug calculate how many devices are connected failed
 *	@return   TRUE
 *	@param[in] ugd the pointer to the main data structure
 *	@param[in] dev the pointer to the number of connected failed devices
 */
static bool __wfd_is_any_device_connect_failed(struct ug_data *ugd, int* no_of_connect_failed_dev)
{
	__FUNC_ENTER__;
	GList *iterator = NULL;

	for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
		/* Not include the device which is connected with me */
		if (__wfd_is_device_connected_with_me(ugd, (device_type_s *)iterator->data)) {
			continue;
		}
		if (!__wfd_is_device_busy(ugd, (device_type_s *)iterator->data) &&
			((device_type_s *)iterator->data)->conn_status == PEER_CONN_STATUS_FAILED_TO_CONNECT) {
			(*no_of_connect_failed_dev)++;
		}
	}

	__FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the ug create the main genlist
 *	@return   the main genlist
 *	@param[in] data the pointer to the main data structure
 */
static Evas_Object *_create_basic_genlist(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	Evas_Object *genlist;

	genlist = elm_genlist_add(ugd->layout);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* Wifi ON/OFF toggle button */
#ifdef WFD_ON_OFF_GENLIST
	ugd->item_wifi_onoff = elm_genlist_item_append(genlist, &wfd_onoff_itc, ugd,
			NULL, ELM_GENLIST_ITEM_NONE,_onoff_changed_cb, ugd);
	elm_genlist_item_selected_set(ugd->item_wifi_onoff,
					EINA_FALSE);
#endif
	evas_object_smart_callback_add(genlist, "realized", _gl_realized, ugd);
	evas_object_smart_callback_add(genlist, "unrealized", _gl_unrealized, ugd);
	ugd->device_name_item = elm_genlist_item_append(genlist, &device_name_itc, ugd, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if(ugd->device_name_item != NULL)
		elm_genlist_item_select_mode_set(ugd->device_name_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	__FUNC_EXIT__;
	return genlist;
}

/**
 *	This function let the ug create no device item to append the genlist
 *	@return   the main item
 *	@param[in] data the pointer to the main data structure
 */
Evas_Object *_create_no_device_genlist(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	Elm_Object_Item *last_item = NULL;
	int i = 0;

	WFD_IF_DEL_ITEM(ugd->avlbl_wfd_item);

	if (ugd->conn_wfd_item != NULL) {
		last_item = ugd->conn_wfd_item;
		for (i = 0; i < ugd->gl_connected_peer_cnt; i++) {
			last_item = elm_genlist_item_next_get(last_item);
		}
	} else {
		last_item = ugd->device_name_item;
	}

	ugd->nodevice_title_item = elm_genlist_item_insert_after(ugd->genlist,
		&title_no_device_itc, (void *)ugd, NULL, last_item,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if(ugd->nodevice_title_item != NULL)
		elm_genlist_item_select_mode_set(ugd->nodevice_title_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	ugd->nodevice_item = elm_genlist_item_insert_after(ugd->genlist, &noitem_itc, (void *)ugd, NULL,
		ugd->nodevice_title_item, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if(ugd->nodevice_item != NULL)
		elm_genlist_item_select_mode_set(ugd->nodevice_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	__FUNC_EXIT__;
	return ugd->genlist;
}

/**
 *	This function let the ug create busy device list
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int _create_busy_dev_list(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	ugd->busy_wfd_item = elm_genlist_item_append(ugd->genlist, &title_busy_itc, (void *)ugd,
		NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if(ugd->busy_wfd_item != NULL)
		elm_genlist_item_select_mode_set(ugd->busy_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	__FUNC_EXIT__;
	return 0;
}

void wfd_free_nodivice_item(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	WFD_IF_DEL_ITEM(ugd->nodevice_title_item);
	WFD_IF_DEL_ITEM(ugd->nodevice_item);
	__FUNC_EXIT__;
}

/**
 *	This function let the ug create avaliable device list
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int _create_available_dev_genlist(void *data)
{
	__FUNC_ENTER__;
	int i = 0;
	struct ug_data *ugd = (struct ug_data *) data;
	Elm_Object_Item *last_item = NULL;

	wfd_free_nodivice_item(ugd);

	if (ugd->conn_wfd_item != NULL) {
		last_item = ugd->conn_wfd_item;
		for (i=0; i<ugd->gl_connected_peer_cnt; i++) {
			last_item = elm_genlist_item_next_get(last_item);
		}
	} else {
		last_item = ugd->device_name_item;
	}

	ugd->avlbl_wfd_item = elm_genlist_item_insert_after(ugd->genlist, &title_available_itc, (void *)ugd, NULL,
		last_item, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if(ugd->avlbl_wfd_item != NULL) {
		elm_genlist_item_select_mode_set(ugd->avlbl_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		elm_genlist_item_update(ugd->avlbl_wfd_item);
	}
	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug create multi connect device list
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
static int _create_multi_connect_dev_genlist(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	ugd->multi_connect_wfd_item = elm_genlist_item_insert_after(ugd->genlist, &title_multi_connect_itc, (void *)ugd,
								NULL, ugd->device_name_item, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if(ugd->multi_connect_wfd_item != NULL)
		elm_genlist_item_select_mode_set(ugd->multi_connect_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug create connected device list
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int _create_connected_dev_genlist(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	ugd->conn_wfd_item = elm_genlist_item_insert_after(ugd->genlist, &title_conn_itc, (void *)ugd, NULL,
		ugd->device_name_item, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if(ugd->conn_wfd_item != NULL) {
		elm_genlist_item_select_mode_set(ugd->conn_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		elm_genlist_item_update(ugd->conn_wfd_item);
	}

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug create connected falied device list
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int _create_connected_failed_dev_genlist(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	Elm_Object_Item *last_item = NULL;
	int i = 0;

	if (ugd->avlbl_wfd_item != NULL) {
		last_item = ugd->avlbl_wfd_item;
		for (i=0; i<ugd->gl_available_peer_cnt; i++) {
			last_item = elm_genlist_item_next_get(last_item);
		}
	} else if (ugd->conn_wfd_item != NULL) {
		last_item = ugd->conn_wfd_item;
		for (i=0; i<ugd->gl_connected_peer_cnt; i++) {
			last_item = elm_genlist_item_next_get(last_item);
		}
	} else {
		last_item = ugd->device_name_item;
	}

	ugd->conn_failed_wfd_item = elm_genlist_item_insert_after(ugd->genlist, &title_conn_failed_itc, (void *)ugd,
								NULL, last_item, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if(ugd->conn_failed_wfd_item != NULL)
		elm_genlist_item_select_mode_set(ugd->conn_failed_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug make the display callback for connect failed peers
 *	@return   if stop the timer, return ECORE_CALLBACK_CANCEL, else return ECORE_CALLBACK_RENEW
 *	@param[in] data the pointer to the user data
 */
static Eina_Bool _connect_failed_peers_display_cb(void *user_data)
{
	__FUNC_ENTER__;
	int interval = 0;
	int res = -1;
	struct ug_data *ugd = (struct ug_data *) user_data;

	if (NULL == ugd) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	if (ugd->avlbl_wfd_item == NULL) {
		_create_available_dev_genlist(ugd);
	}

	if (ugd->wfd_discovery_status == WIFI_DIRECT_DISCOVERY_BACKGROUND) {
		DBG(LOG_INFO, "Background mode\n");
		ugd->display_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	/* check the timeout, if not timeout, keep the cb */
	interval = time(NULL) - ugd->last_display_time;
	if (interval < MAX_DISPLAY_TIME_OUT) {
		return ECORE_CALLBACK_RENEW;
	}

	if (ugd->is_paused == false) {
		DBG(LOG_INFO, "Not Paused");
		/* start discovery again */
		ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
		res = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
		if (res != WIFI_DIRECT_ERROR_NONE) {
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
			DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", res);
			wifi_direct_cancel_discovery();
		}
	}

	ugd->display_timer = NULL;
	__FUNC_EXIT__;
	return ECORE_CALLBACK_CANCEL;
}

void wfd_ug_view_free_peer(device_type_s *gl_peers_start)
{
	__FUNC_ENTER__;
	device_type_s *peer_for_free = NULL;
	device_type_s *peer = gl_peers_start;

	while (peer != NULL && peer->gl_item != NULL) {
		DBG(LOG_INFO, "Deleted item, ssid:%s\n", peer->ssid);
		elm_object_item_del(peer->gl_item);
		peer->gl_item = NULL;
		peer_for_free = peer;
		peer = peer->next;
		free(peer_for_free);
	}
	__FUNC_EXIT__;
}

void wfd_check_gl_conn_peers(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	if (ugd->gl_connected_peer_cnt == 0) {
		WFD_IF_DEL_ITEM(ugd->conn_wfd_item);
	}
	__FUNC_EXIT__;
}

/**
 *	This function let the ug free the peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_view_free_peers(void *data)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;

	ugd->gl_connected_peer_cnt = 0;

	if (ugd->gl_conn_peers_start != NULL) {
		wfd_ug_view_free_peer(ugd->gl_conn_peers_start);
		ugd->gl_conn_peers_start = NULL;
	}

	ugd->gl_available_peer_cnt = 0;

	if (ugd->gl_avlb_peers_start != NULL) {
		wfd_ug_view_free_peer(ugd->gl_avlb_peers_start);
		ugd->gl_avlb_peers_start = NULL;
	}

	ugd->gl_busy_peer_cnt = 0;

	if (ugd->gl_busy_peers_start != NULL) {
		wfd_ug_view_free_peer(ugd->gl_busy_peers_start);
		ugd->gl_busy_peers_start = NULL;
	}

	ugd->gl_multi_connect_peer_cnt = 0;

	if (ugd->gl_mul_conn_peers_start != NULL) {
		wfd_ug_view_free_peer(ugd->gl_mul_conn_peers_start);
		ugd->gl_mul_conn_peers_start = NULL;
	}

	if (ugd->gl_connected_peer_cnt == 0) {
		WFD_IF_DEL_ITEM(ugd->conn_wfd_item);
	}

	if (ugd->display_timer != NULL) {
		ecore_timer_del(ugd->display_timer);
		ugd->display_timer = NULL;
	}

	WFD_IF_DEL_ITEM(ugd->multi_connect_wfd_item);
	WFD_IF_DEL_ITEM(ugd->multi_connect_sep_item);
	WFD_IF_DEL_ITEM(ugd->avlbl_wfd_item);
	WFD_IF_DEL_ITEM(ugd->busy_wfd_item);

	__FUNC_EXIT__;
}

void wfd_ug_update_toolbar(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	int no_of_conn_dev = ugd->raw_connected_peer_cnt;
	Evas_Object *btn;

	wfd_refresh_wifi_direct_state(ugd);
		if (ugd->wfd_status == WIFI_DIRECT_STATE_CONNECTING) {
			DBG(LOG_INFO, "WIFI_DIRECT_STATE_CONNECTING\n");
			if( ugd->multi_connect_wfd_item != NULL) {
				DBG(LOG_INFO, "multi_connect_toolbar_item\n");
				btn = elm_button_add(ugd->layout);
				/* Use "bottom" style button */
				elm_object_style_set(btn, "bottom");
				elm_object_domain_translatable_text_set(btn, PACKAGE,
						"IDS_WIFI_SK2_CANCEL_CONNECTION");
				evas_object_smart_callback_add(btn, "clicked",
					_wfd_ug_cancel_connection_button_cb, (void *)ugd);
				/* Set button into "toolbar" swallow part */
				elm_object_part_content_set(ugd->layout, "button.next", btn);
				ugd->disconnect_btn = btn;
				evas_object_show(ugd->disconnect_btn);
				elm_object_part_content_set(ugd->layout, "button.prev",
					ugd->scan_toolbar);
				wfd_ug_view_refresh_button(ugd->scan_toolbar,
					"IDS_WIFI_SK4_SCAN", FALSE);
				evas_object_data_set(ugd->disconnect_btn, "multi", "disconnect");
				DBG(LOG_INFO, "button: disconnect button added\n");
			} else {
				DBG(LOG_INFO, "scan_toolbar\n");
				WFD_IF_DEL_ITEM(ugd->multi_connect_toolbar_item);
				wfd_ug_view_refresh_button(ugd->scan_toolbar,
					"IDS_WIFI_SK2_CANCEL_CONNECTION", TRUE);
				evas_object_data_set(ugd->scan_btn, "multi", "cancel");
				DBG(LOG_INFO, "button: stop connect button added\n");
			}
		} else if (no_of_conn_dev > 0) {
		if (!ugd->multi_connect_toolbar_item) {
			btn = elm_button_add(ugd->layout);
			/* Use "bottom" style button */
			elm_object_style_set(btn, "bottom");
			elm_object_domain_translatable_text_set(btn, PACKAGE,
					"IDS_WIFI_SK_DISCONNECT");
			evas_object_smart_callback_add(btn, "clicked",
				_wfd_ug_disconnect_button_cb, (void *)ugd);
			/* Set button into "toolbar" swallow part */
			elm_object_part_content_set(ugd->layout, "button.next", btn);
			ugd->disconnect_btn = btn;
			evas_object_show(ugd->disconnect_btn);
		}
		elm_object_part_content_set(ugd->layout, "button.prev",
			ugd->scan_toolbar);
		wfd_ug_view_refresh_button(ugd->scan_toolbar,
			"IDS_WIFI_SK4_SCAN", TRUE);
		evas_object_data_set(ugd->disconnect_btn, "multi", "disconnect");
		DBG(LOG_INFO, "button: disconnect button added\n");
	}else {
		if (no_of_conn_dev == 0 && ugd->disconnect_btn != NULL) {
			DBG(LOG_INFO, "disconnect btn removed when conn failed\n");
			Evas_Object *content;
			content = elm_object_part_content_unset(ugd->layout, "button.next");
			WFD_IF_DEL_OBJ(content);
			ugd->disconnect_btn = NULL;
			elm_object_part_content_set(ugd->layout, "button.big",
				ugd->scan_toolbar);
		}
		wfd_ug_view_refresh_button(ugd->scan_toolbar,
			"IDS_WIFI_SK4_SCAN", TRUE);
		evas_object_data_set(ugd->scan_toolbar, "multi", "connect");
		DBG(LOG_INFO, "button: scan button added\n");
	}
	__FUNC_EXIT__;
}

/**
 *	This function let the ug init the genlist
 */
void wfd_ug_view_init_genlist(void *data, bool is_free_all_peers)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	int no_of_busy_dev = 0;
	int no_of_available_dev = 0;
	int no_of_conn_failed_dev = 0;

	if (is_free_all_peers) {
		wfd_ug_view_free_peers(ugd);
	}

	if (ugd->gl_failed_peers_start != NULL) {
		DBG(LOG_INFO, "These are failed peers, must clear them");
		ugd->gl_connected_failed_peer_cnt = 0;
		wfd_ug_view_free_peer(ugd->gl_failed_peers_start);
		ugd->gl_failed_peers_start = NULL;
		WFD_IF_DEL_ITEM(ugd->conn_failed_wfd_item);
	}

	if (ugd->avlbl_wfd_item != NULL) {
		DBG(LOG_INFO, "There are available peers in genlist");
		wfd_ug_view_refresh_glitem(ugd->avlbl_wfd_item);
		return;
	}

	__wfd_is_any_device_busy(ugd, &no_of_busy_dev);
	__wfd_is_any_device_available(ugd, &no_of_available_dev);
	__wfd_is_any_device_connect_failed(ugd, &no_of_conn_failed_dev);

	__FUNC_EXIT__;
}

/**
 *	This function let the ug find a peer in genlist
 */
device_type_s *find_peer_in_glist(device_type_s *start_pos, const char *mac_addr)
{
	__FUNC_ENTER__;

	if (start_pos == NULL) {
		DBG(LOG_INFO, "no peer in genlist");
		return NULL;
	}

	device_type_s *peer = start_pos;

	while (peer != NULL) {
		if (!strncmp(peer->mac_addr, mac_addr, MAC_LENGTH - 1)) {
			peer->is_alive = true;
			DBG(LOG_INFO, "device [%s] found in genlist", peer->ssid);
			__FUNC_EXIT__;
			return peer;
		}
		peer = peer->next;
	}

	__FUNC_EXIT__;
	return NULL;
}

void delete_not_alive_peers(struct ug_data *ugd, device_type_s **start_pos, int *cnt)
{
	__FUNC_ENTER__;
	if (*start_pos == NULL) {
		DBG(LOG_INFO, "no peer in genlist");
		return;
	}

	device_type_s *peer = *start_pos;
	device_type_s *peer_tmp = NULL;
	while (peer != NULL) {
		peer_tmp = peer->next;
		if(peer->is_alive == false) {
			 free_gl_peer(start_pos, peer->mac_addr, cnt);
		}
		peer = peer_tmp;
	}

//	wfd_check_gl_available_peers(ugd);
	if (ugd->gl_busy_peer_cnt == 0) {
		WFD_IF_DEL_ITEM(ugd->busy_wfd_item);
	}
	__FUNC_EXIT__;
	return;
}

void set_not_alive_peers(device_type_s *start_pos)
{
	__FUNC_ENTER__;
	if (start_pos == NULL) {
		DBG(LOG_INFO, "no peer in genlist");
		return;
	}

	device_type_s *peer = start_pos;
	while (peer != NULL) {
		peer->is_alive = false;
		peer = peer->next;
	}

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug get the insert position for next item
 */
Elm_Object_Item *get_insert_postion(device_type_s *peer, Elm_Object_Item *pre_item, int peer_cnt)
{
	__FUNC_ENTER__;
	int i = 0;
	device_type_s *peer_ite = NULL;
	Elm_Object_Item *head = elm_genlist_item_next_get(pre_item);
	Elm_Object_Item *item = NULL;

	if(peer_cnt == 0) {
		DBG(LOG_INFO, "first peer [%s] would be added", peer->ssid);
		return pre_item;
	}

	peer_ite = (device_type_s *)elm_object_item_data_get(head);
	if(peer_ite->gl_item != NULL) {
		for(i=0; i < peer_cnt; i++) {
			if (strcasecmp(peer_ite->ssid, peer->ssid) > 0) {
				/* if peer_ite is greater than peer, return previous item */
				__FUNC_EXIT__;
				return  elm_genlist_item_prev_get(head);
			} else {
				item = elm_genlist_item_next_get(head);
				if (item == NULL) {
					/* return the last item */
					return head;
				} else {
					head = item;
					peer_ite = (device_type_s *)elm_object_item_data_get(head);
				}
			}
		}
	}
	__FUNC_EXIT__;
	return elm_genlist_item_prev_get(head);
}

/**
 *	This function let the ug insert peer item to genlist
 */
int insert_gl_item(Evas_Object *genlist, Elm_Object_Item *item, Elm_Gen_Item_Class *itc, device_type_s **start_pos,
				device_type_s *peer_for_insert, void *callback)
{
	__FUNC_ENTER__;
	WFD_RETV_IF(item == NULL, -1, "Item is NULL\n");
	device_type_s *peer = NULL;
	device_type_s *peer_ite = NULL;

	peer = (device_type_s *)malloc(sizeof(device_type_s));
	WFD_RETV_IF(peer == NULL, -1, "malloc failed\n");

	memcpy(peer, peer_for_insert, sizeof(device_type_s));
	peer->next = NULL;

	if(*start_pos == NULL) {
		*start_pos = peer;
	} else {
		peer_ite = *start_pos;
		while(peer_ite->next != NULL) {
			/* move pointer to the last peer */
			peer_ite = peer_ite->next;
		}
		peer_ite->next = peer;
	}

	peer->is_alive = true;
	peer->gl_item = elm_genlist_item_insert_after(genlist, itc, (void *)peer, NULL, item,
		ELM_GENLIST_ITEM_NONE, callback, (void *)peer);
	if (callback == NULL && peer->gl_item != NULL) {
		elm_genlist_item_select_mode_set(peer->gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}
	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug update connected peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_update_connected_peers(void *data)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	int i = 0 ;
	int res;
	bool is_group_owner = FALSE;
	Elm_Object_Item *item = NULL;

	res = wifi_direct_is_group_owner(&is_group_owner);
	if (res == WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_INFO, "is_group_owner=[%d]", is_group_owner);
	}

	if (!ugd->conn_wfd_item) {
		_create_connected_dev_genlist(ugd);
	}

	for (i = 0; i < ugd->raw_connected_peer_cnt; i++) {
		if (find_peer_in_glist(ugd->gl_conn_peers_start, ugd->raw_connected_peers[i].mac_addr) == NULL) {
			if (find_peer_in_glist(ugd->gl_avlb_peers_start, ugd->raw_connected_peers[i].mac_addr) != NULL) {
				free_gl_peer(&ugd->gl_avlb_peers_start, ugd->raw_connected_peers[i].mac_addr,
					&ugd->gl_available_peer_cnt);
				wfd_check_gl_available_peers(ugd);
			}

			if (find_peer_in_glist(ugd->gl_mul_conn_peers_start, ugd->raw_connected_peers[i].mac_addr) != NULL) {
				free_gl_peer(&ugd->gl_mul_conn_peers_start, ugd->raw_connected_peers[i].mac_addr,
					&ugd->gl_multi_connect_peer_cnt);
				if (ugd->gl_multi_connect_peer_cnt == 0) {
					WFD_IF_DEL_ITEM(ugd->multi_connect_wfd_item);
					WFD_IF_DEL_ITEM(ugd->multi_connect_sep_item);
				}
			}

			item = get_insert_postion(&(ugd->raw_connected_peers[i]), ugd->conn_wfd_item,
				ugd->gl_connected_peer_cnt);
			res = insert_gl_item(ugd->genlist, item, &peer_conn_itc, &ugd->gl_conn_peers_start,
					&ugd->raw_connected_peers[i], NULL);
			if (res != 0) {
				break;
			}
			ugd->gl_connected_peer_cnt++;
		}
	}

	/* if is not GO, free all available peers */
	if (is_group_owner == FALSE) {
		ugd->gl_available_peer_cnt = 0;
		WFD_IF_DEL_ITEM(ugd->avlbl_wfd_item);

		if (ugd->gl_avlb_peers_start != NULL) {
			wfd_ug_view_free_peer(ugd->gl_avlb_peers_start);
			ugd->gl_avlb_peers_start = NULL;
		}
	}

	/* free busy peers */
	if (ugd->gl_busy_peers_start != NULL) {
		ugd->gl_busy_peer_cnt = 0;
		WFD_IF_DEL_ITEM(ugd->busy_wfd_item);

		wfd_ug_view_free_peer(ugd->gl_busy_peers_start);
		ugd->gl_busy_peers_start = NULL;

	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug update the multi-connect peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_view_update_multiconn_peers(void *data)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	int i;
	int res;
	Elm_Object_Item *item = NULL;

	if (ugd->raw_multi_selected_peer_cnt > 0) {
		if (ugd->raw_connected_peer_cnt < ugd->raw_multi_selected_peer_cnt &&
			ugd->multi_connect_wfd_item == NULL) {
			_create_multi_connect_dev_genlist(ugd);
		}

		for (i = 0; i < ugd->raw_multi_selected_peer_cnt; i++) {
			if (ugd->raw_multi_selected_peers[i].conn_status != PEER_CONN_STATUS_CONNECTED) {
				item = get_insert_postion(&(ugd->raw_multi_selected_peers[i]),
					ugd->multi_connect_wfd_item, ugd->gl_multi_connect_peer_cnt);
				res = insert_gl_item(ugd->genlist, item, &peer_itc, &ugd->gl_mul_conn_peers_start,
						&ugd->raw_multi_selected_peers[i], NULL);
				if (res != 0) {
					break;
				}
				ugd->gl_multi_connect_peer_cnt++;
			}
		}
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug update the available and busy peers
 */
void wfd_ug_update_available_peers(void *data)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	int no_of_busy_dev = 0;
	int no_of_available_dev = 0;
	int no_of_conn_dev = 0;
	bool is_group_owner = FALSE;
	int res = 0;
	Elm_Object_Item *item = NULL;
	device_type_s *peer = NULL;
	GList *iterator = NULL;

	__wfd_is_any_device_busy(ugd, &no_of_busy_dev);
	__wfd_is_any_device_available(ugd, &no_of_available_dev);
	no_of_conn_dev = ugd->raw_connected_peer_cnt;

	res = wifi_direct_is_group_owner(&is_group_owner);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_INFO, "Fail to get group_owner_state. ret=[%d]", res);
		ugd->I_am_group_owner = FALSE;
	} else {
		ugd->I_am_group_owner = is_group_owner;
	}

	DBG(LOG_INFO, "avail_dev=[%d], busy_dev=[%d], GO=[%d]\n", no_of_available_dev, no_of_busy_dev, is_group_owner);
	if (no_of_available_dev != 0 || no_of_busy_dev != 0) {
		DBG(LOG_INFO, "There are available or busy peers\n");
		wfd_free_nodivice_item(ugd);
	}

	if (no_of_conn_dev == 0 || is_group_owner == TRUE) {
		if (ugd->avlbl_wfd_item == NULL) {
			_create_available_dev_genlist(ugd);
		}

		for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
			/* Not include the device which is connected with me */
			if (__wfd_is_device_connected_with_me(ugd, (device_type_s *)iterator->data)) {
				continue;
			}
			if (!__wfd_is_device_busy(ugd, (device_type_s *)iterator->data) &&
				((device_type_s *)iterator->data)->conn_status != PEER_CONN_STATUS_FAILED_TO_CONNECT &&
				((device_type_s *)iterator->data)->conn_status != PEER_CONN_STATUS_CONNECTED) {
				/* free disconnected gl peer */
				if (find_peer_in_glist(ugd->gl_conn_peers_start, ((device_type_s *)iterator->data)->mac_addr) != NULL) {
					free_gl_peer(&ugd->gl_conn_peers_start, ((device_type_s *)iterator->data)->mac_addr,
						&ugd->gl_connected_peer_cnt);
				}

				/* free busy gl peer, which is available now */
				if (find_peer_in_glist(ugd->gl_busy_peers_start, ((device_type_s *)iterator->data)->mac_addr) != NULL) {
					free_gl_peer(&ugd->gl_busy_peers_start, ((device_type_s *)iterator->data)->mac_addr, &ugd->gl_busy_peer_cnt);
					if (ugd->gl_busy_peer_cnt == 0) {
						WFD_IF_DEL_ITEM(ugd->busy_wfd_item);
					}
				}

				if (find_peer_in_glist(ugd->gl_failed_peers_start, (const char *)((device_type_s *)iterator->data)->mac_addr) != NULL) {
					continue;
				}

				peer = find_peer_in_glist(ugd->gl_avlb_peers_start, (const char *)((device_type_s *)iterator->data)->mac_addr);
				if (peer == NULL) {
					item = get_insert_postion((device_type_s *)iterator->data,
						ugd->avlbl_wfd_item, ugd->gl_available_peer_cnt);
					res = insert_gl_item(ugd->genlist, item, &peer_itc, &ugd->gl_avlb_peers_start,
							(device_type_s *)iterator->data, _gl_peer_sel);
					if (res != 0) {
						break;
					}
					ugd->gl_available_peer_cnt++;
				} else if (no_of_conn_dev > 0 && ((device_type_s *)iterator->data)->is_group_owner == TRUE) {
					/* if peer is GO, free it */
					free_gl_peer(&ugd->gl_avlb_peers_start, ((device_type_s *)iterator->data)->mac_addr,
						&ugd->gl_available_peer_cnt);
				}
			}
		}
	}

	wfd_check_gl_available_peers(ugd);
	wfd_check_gl_conn_peers(ugd);

	if (no_of_conn_dev == 0 && no_of_busy_dev > 0) {
		if (ugd->busy_wfd_item == NULL) {
			_create_busy_dev_list(ugd);
		}

		for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
			/* Not include the device which is connected with me */
			if (__wfd_is_device_connected_with_me(ugd, (device_type_s *)iterator->data)) {
				continue;
			}
			if (__wfd_is_device_busy(ugd, (device_type_s *)iterator->data) == TRUE) {
				if (find_peer_in_glist(ugd->gl_busy_peers_start, ((device_type_s *)iterator->data)->mac_addr) == NULL) {
					if (find_peer_in_glist(ugd->gl_avlb_peers_start,
						((device_type_s *)iterator->data)->mac_addr) != NULL) {
						free_gl_peer(&ugd->gl_avlb_peers_start, ((device_type_s *)iterator->data)->mac_addr,
							&ugd->gl_available_peer_cnt);
						wfd_check_gl_available_peers(ugd);
					}
					item = get_insert_postion((device_type_s *)iterator->data, ugd->busy_wfd_item,
						ugd->gl_busy_peer_cnt);
					res = insert_gl_item(ugd->genlist, item, &peer_busy_itc, &ugd->gl_busy_peers_start,
							(device_type_s *)iterator->data, _gl_busy_peer_sel);
					if (res != 0) {
						break;
					}
					ugd->gl_busy_peer_cnt++;
				}
			}
		}
	}

	wfd_check_gl_busy_peers(ugd);

	__FUNC_EXIT__;
}

/**
 *	This function let the ug update the failed peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_update_failed_peers(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	int no_of_conn_failed_dev = 0;
	GList *iterator = NULL;

	__wfd_is_any_device_connect_failed(ugd, &no_of_conn_failed_dev);
	DBG(LOG_INFO, "conn_failed_dev=[%d]", no_of_conn_failed_dev);

	if (no_of_conn_failed_dev == 0) {
		return;
	}

	/* add timer for disappearing failed peers after N secs */
	if (NULL == ugd->display_timer) {
		ugd->last_display_time = time(NULL);
		ugd->display_timer = ecore_timer_add(0.05, (Ecore_Task_Cb)_connect_failed_peers_display_cb, ugd);
	}

	for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
		if (!__wfd_is_device_busy(ugd, (device_type_s *)iterator->data) &&
			((device_type_s *)iterator->data)->conn_status == PEER_CONN_STATUS_FAILED_TO_CONNECT) {
			if (find_peer_in_glist(ugd->gl_failed_peers_start, ((device_type_s *)iterator->data)->mac_addr) == NULL) {
				if (find_peer_in_glist(ugd->gl_avlb_peers_start, ((device_type_s *)iterator->data)->mac_addr) != NULL) {
					free_gl_peer(&ugd->gl_avlb_peers_start, ((device_type_s *)iterator->data)->mac_addr,
						&ugd->gl_available_peer_cnt);
					wfd_check_gl_available_peers(ugd);
				}
			}
		}
	}

	__FUNC_EXIT__;
}

#ifdef WFD_ON_OFF_GENLIST
/**
 *	This function is called when user swipes on/off button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _onoff_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	WFD_RET_IF(ugd == NULL, "Incorrect parameter(NULL)\n");
	WFD_RET_IF(ugd->on_off_check == NULL, "on_off_check(NULL)\n");
	wfd_refresh_wifi_direct_state(ugd);
	if(ugd->device_name_item != NULL)
		elm_genlist_item_update(ugd->device_name_item);

	elm_object_disabled_set(ugd->on_off_check, TRUE);
	if(ugd->disconnect_btn) {
		Evas_Object *content;
		content = elm_object_part_content_unset(ugd->layout, "button.next");
		WFD_IF_DEL_OBJ(content);
		ugd->disconnect_btn = NULL;
	}
	elm_object_part_content_set(ugd->layout, "button.big", ugd->scan_toolbar);

	/* turn on/off wfd */
	if (!ugd->wfd_onoff) {
		if (ugd->wfd_status <= WIFI_DIRECT_STATE_DEACTIVATING) {
			DBG(LOG_INFO, "wifi-direct switch on\n");
			elm_genlist_item_selected_set(ugd->item_wifi_onoff,
								EINA_FALSE);
			wfd_client_switch_on(ugd);
		}
	} else {
		if (ugd->wfd_status >= WIFI_DIRECT_STATE_ACTIVATING) {
			DBG(LOG_INFO, "wifi-direct switch off\n");
			elm_genlist_item_selected_set(ugd->item_wifi_onoff,
								EINA_FALSE);
			wfd_client_switch_off(ugd);
		}
	}

	__FUNC_EXIT__;
}

void wfd_ug_refresh_on_off_check(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	WFD_RET_IF(ugd == NULL, "Incorrect parameter(NULL)\n");
	WFD_RET_IF(ugd->on_off_check == NULL, "on_off_check(NULL)\n");

	wfd_refresh_wifi_direct_state(ugd);
	if (ugd->wfd_status == WIFI_DIRECT_STATE_DEACTIVATING ||
		ugd->wfd_status == WIFI_DIRECT_STATE_ACTIVATING ) {
		elm_object_disabled_set(ugd->on_off_check, TRUE);
	} else {
		elm_object_disabled_set(ugd->on_off_check, FALSE);
	}
	if (ugd->wfd_status > WIFI_DIRECT_STATE_ACTIVATING) {
		elm_check_state_set(ugd->on_off_check, TRUE);
	} else {
		elm_check_state_set(ugd->on_off_check, FALSE);
	}

	__FUNC_EXIT__;
}

void wfd_ug_create_on_off_check(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	WFD_RET_IF(ugd == NULL, "Incorrect parameter(NULL)\n");
	WFD_RET_IF(ugd->naviframe == NULL, "naviframe NULL\n");

	Evas_Object *check = elm_check_add(ugd->naviframe);
	elm_object_style_set(check, "naviframe/title_on&off");
	elm_check_state_set(check, ugd->wfd_onoff);
	evas_object_propagate_events_set(check, EINA_FALSE);
	evas_object_smart_callback_add(check, "changed", _onoff_changed_cb, ugd);
	elm_object_focus_allow_set(check, EINA_TRUE);
	elm_object_item_part_content_set(ugd->navi_item, "title_right_btn", check);
	evas_object_show(check);
	ugd->on_off_check = check;

	__FUNC_EXIT__;
}
#endif

/**
 *	This function let the ug create the main view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void create_wfd_ug_view(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	WFD_RET_IF(ugd == NULL, "Incorrect parameter(NULL)\n");
#ifdef TIZEN_WIFIDIRECT_MORE_BTN
	Evas_Object *more_btn;
#endif
	Evas_Object *layout;

	ugd->naviframe = elm_naviframe_add(ugd->base);
	elm_naviframe_prev_btn_auto_pushed_set(ugd->naviframe, EINA_FALSE);
	eext_object_event_callback_add(ugd->naviframe, EA_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ugd->naviframe, EA_CALLBACK_MORE, eext_naviframe_more_cb, NULL);
	elm_object_part_content_set(ugd->base, "elm.swallow.content", ugd->naviframe);

	ugd->back_btn = elm_button_add(ugd->naviframe);
	elm_object_style_set(ugd->back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(ugd->back_btn, "clicked", _back_btn_cb, (void *)ugd);
	elm_object_focus_allow_set(ugd->back_btn, EINA_FALSE);

	/* Create layout */
	layout = elm_layout_add(ugd->naviframe);
	elm_layout_file_set(layout, WFD_UG_EDJ_PATH, "main_layout");
	ugd->layout = layout;

	ugd->genlist = _create_basic_genlist(ugd);
	if (ugd->genlist == NULL) {
		DBG(LOG_ERROR, "Failed to create basic genlist");
		return;
	}
	elm_object_part_content_set(layout, "elm.swallow.content", ugd->genlist);

	elm_genlist_fx_mode_set(ugd->genlist, EINA_FALSE);
	evas_object_show(ugd->base);
	elm_object_focus_set(ugd->base, EINA_TRUE);

	ugd->navi_item = elm_naviframe_item_push(ugd->naviframe, ugd->title,
			ugd->back_btn, NULL, layout, NULL);
	elm_naviframe_item_pop_cb_set(ugd->navi_item, _back_btn_cb, ugd);

#ifdef TIZEN_WIFIDIRECT_MORE_BTN
	more_btn = elm_button_add(ugd->naviframe);
	elm_object_style_set(more_btn, "naviframe/more/default");
	evas_object_smart_callback_add(more_btn, "clicked",
			_more_button_cb, ugd->win);
	elm_object_item_part_content_set(ugd->navi_item, "toolbar_more_btn",
			more_btn);
#endif

	wifi_direct_initialize();
	wifi_direct_get_state(&ugd->wfd_status);
	if (ugd->wfd_status > WIFI_DIRECT_STATE_DEACTIVATING) {
		scan_button_create(ugd);
	}

	if (ugd->view_type && g_strcmp0(_(ugd->view_type), _("IDS_WIFI_BUTTON_MULTI_CONNECT")) == 0) {
		int ret = 0;
		ugd->raw_discovered_peer_cnt = 0;
		wfd_create_multiconnect_view(ugd);
		ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
		ret = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
		if (ret != WIFI_DIRECT_ERROR_NONE) {
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
			DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", ret);
			wifi_direct_cancel_discovery();
		}
		return;
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug destroy the main view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void destroy_wfd_ug_view(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	WFD_IF_DEL_ITEM(ugd->device_name_item);
	WFD_IF_DEL_ITEM(ugd->multi_connect_toolbar_item);
	WFD_IF_DEL_ITEM(ugd->conn_wfd_item);

	WFD_IF_DEL_OBJ(ugd->scan_toolbar);
	WFD_IF_DEL_OBJ(ugd->back_btn);
	WFD_IF_DEL_OBJ(ugd->toolbar);
	WFD_IF_DEL_OBJ(ugd->genlist);
	WFD_IF_DEL_OBJ(ugd->naviframe);
	__FUNC_EXIT__;
}
