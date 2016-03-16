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

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

Elm_Gen_Item_Class device_itc;

/**
 *	This function let the ug call it when click 'back' button in multi connect view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
Eina_Bool _multiconnect_view_pop_cb(void *data, Elm_Object_Item *it)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int ret = 0;
	device_type_s *peer_for_free = NULL;
	device_type_s *peer = NULL;

	if (!ugd) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return EINA_FALSE;
	}

	peer = ugd->multi_conn_dev_list_start;

	while (peer != NULL) {
		DBG(LOG_INFO, "Free peer, ssid:%s\n", peer->ssid);
		peer_for_free = peer;
		peer = peer->next;
		free(peer_for_free);
		peer_for_free = NULL;
	}

	ugd->multi_conn_dev_list_start = NULL;
	ugd->multiconn_view_genlist = NULL;
	ugd->mcview_title_item = NULL;
	ugd->mcview_nodevice_item = NULL;
	ugd->multi_navi_item = NULL;
	ugd->multiconn_conn_btn = NULL;
	ugd->multiconn_layout = NULL;
	ugd->multiconn_scan_stop_btn = NULL;

	DBG(LOG_INFO, "MultiConnectMode: %d\n", ugd->multi_connect_mode);

	/* if pressing connect for multi-connecting, it should not start discovery again */
	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_NONE) {
		if (ugd->view_type == NULL) {
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
			DBG(LOG_INFO, "Discovery started\n");
			ret = wifi_direct_start_discovery_specific_channel(false, 1,
				WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
				DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", ret);
				wifi_direct_cancel_discovery();
			}
		} else if (g_strcmp0(D_(ugd->view_type),
			D_("IDS_WIFI_BUTTON_MULTI_CONNECT")) == 0) {
			DBG(LOG_INFO, "Discovery not started\n");
			ug_destroy_me(ugd->ug);
		}
	}

	__FUNC_EXIT__;
	return EINA_TRUE;
}

/**
 *	This function let the ug reset the connected failed peers
 *	@return   false
 *	@param[in] event_info the pointer to the event information
 */
gboolean __wfd_multi_connect_reset_cb(void *data)
{
	__FUNC_ENTER__;

	if (!data) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return false;
	}

	device_type_s *peer = NULL;
	struct ug_data *ugd = (struct ug_data *)data;

	peer = ugd->gl_mul_conn_peers_start;
	while (peer != NULL) {
		elm_object_item_del(peer->gl_item);
		peer->gl_item = NULL;
		peer = peer->next;
	}

	WFD_IF_DEL_ITEM(ugd->multi_connect_wfd_item);
	WFD_IF_DEL_ITEM(ugd->multi_connect_sep_item);

	wfd_ug_update_available_peers(ugd);

	__FUNC_EXIT__;
	return false;
}

/**
 *	This function let the ug free the selected peers in multi connect view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_free_multi_selected_peers(void *data)
{
	__FUNC_ENTER__;

	int i = 0;
	struct ug_data *ugd = (struct ug_data *)data;

	/* destroy the created group */
	wifi_direct_destroy_group();

	/* release the selected peers */
	for (i = 0; i < ugd->raw_multi_selected_peer_cnt; i++) {
		memset(&ugd->raw_multi_selected_peers[i], 0x00, sizeof(device_type_s));
	}

	ugd->raw_multi_selected_peer_cnt = 0;
	ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_NONE;

	__FUNC_EXIT__;

}

/**
 *	This function let the ug stop to connect to selected peer
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_stop_multi_connect(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;

	if (ugd->act_popup) {
		evas_object_del(ugd->act_popup);
		ugd->act_popup = NULL;
	}

	/* change the title of failed peers */
	ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_COMPLETED;
	wfd_ug_view_refresh_glitem(ugd->multi_connect_wfd_item);
	wfd_ug_update_toolbar(ugd);

	if (ugd->gl_connected_peer_cnt > 0) {
		wfd_client_set_p2p_group_owner_intent(7);
	} else {
		wifi_direct_destroy_group();
	}

	if(ugd->timer_multi_reset > 0) {
		g_source_remove(ugd->timer_multi_reset);
	}
	/* after 30s, remove the failed peers */
	ugd->timer_multi_reset = g_timeout_add(30000 /*ms*/,
		__wfd_multi_connect_reset_cb, ugd);

	if (ugd->g_source_multi_connect_next > 0) {
		g_source_remove(ugd->g_source_multi_connect_next);
	}
	ugd->g_source_multi_connect_next = 0;

	/*when multi-connect end and auto_exit is true, exit UG*/
	if (ugd->is_auto_exit) {
		_wfd_ug_auto_exit(ugd);
	}

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug start to connect the selected peers
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_start_multi_connect(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;

	if (ugd->raw_multi_selected_peer_cnt > 0) {
		ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_IN_PROGRESS;

		res = wfd_client_group_add();
		if (res == -1) {
			DBG(LOG_ERROR, "Failed to add group");
			wfd_free_multi_selected_peers(ugd);

			__FUNC_EXIT__;
			return -1;
		}
	} else {
		DBG(LOG_INFO, "No selected peers.\n");
		return -1;
	}

	__FUNC_EXIT__;
	return 0;
}

void wfd_sort_multi_selected_peers(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	int i = 0;
	int j = 0;
	device_type_s peer;

	for(i = 0; i < ugd->raw_multi_selected_peer_cnt; i++) {
		for(j = 0; j < ugd->raw_multi_selected_peer_cnt-i-1; j++) {
			if (strcasecmp(ugd->raw_multi_selected_peers[j].ssid, ugd->raw_multi_selected_peers[j + 1].ssid) > 0) {
				peer = ugd->raw_multi_selected_peers[j];
				ugd->raw_multi_selected_peers[j] = ugd->raw_multi_selected_peers[j + 1];
				ugd->raw_multi_selected_peers[j + 1] = peer;
			}
		}
	}
	__FUNC_EXIT__;
}

/**
 *	This function let the ug connect to the next selected peer automatically
 *	@return   If stop the timer, return false, else return true
 *	@param[in] data the pointer to the main data structure
 */
gboolean wfd_multi_connect_next_cb(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int i;
	int res;

	// Reset g_source handler..
	if (ugd->g_source_multi_connect_next > 0) {
		g_source_remove(ugd->g_source_multi_connect_next);
	}
	ugd->g_source_multi_connect_next = 0;

	if (ugd->raw_multi_selected_peer_cnt > 0) {
		ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_IN_PROGRESS;
		for (i = 0; i < ugd->raw_multi_selected_peer_cnt; i++) {
			if (ugd->raw_multi_selected_peers[i].conn_status == PEER_CONN_STATUS_WAIT_FOR_CONNECT) {
				ugd->mac_addr_connecting = ugd->raw_multi_selected_peers[i].mac_addr;
				res = wfd_client_connect(ugd->raw_multi_selected_peers[i].mac_addr);
				if (res == -1) {
					ugd->raw_multi_selected_peers[i].conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
				} else {
					ugd->raw_multi_selected_peers[i].conn_status = PEER_CONN_STATUS_CONNECTING;
					break;
				}
			}
		}

		if (i >= ugd->raw_multi_selected_peer_cnt) {
			// All selected peers are touched.
			DBG(LOG_INFO, "Stop Multi Connect...\n");
			wfd_stop_multi_connect(ugd);
		}
	} else {
		DBG(LOG_INFO, "No selected peers.\n");
	}

	__FUNC_EXIT__;
	return false;
}

/**
 *	This function let the ug call it when click 'connect' button in multi connect view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _connect_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int count = 0;
	char popup_text[MAX_POPUP_TEXT_SIZE] = {0};
	device_type_s *peer = NULL;
	char *format_str = NULL;

	peer = ugd->multi_conn_dev_list_start;
	while (peer != NULL) {
		if(peer->dev_sel_state) {
			count++;
		}
		peer = peer->next;
	}
	DBG(LOG_INFO, "MultiSelected Peer Count: %d", count);

	/* if more than 7 device selected, show the popup */
	if (count > MAX_CONNECTED_PEER_NUM) {
		format_str = D_("IDS_ST_POP_YOU_CAN_CONNECT_UP_TO_PD_DEVICES_AT_THE_SAME_TIME");
		snprintf(popup_text, MAX_POPUP_TEXT_SIZE, format_str, MAX_CONNECTED_PEER_NUM);
		wfd_ug_warn_popup(ugd, popup_text, POP_TYPE_MULTI_CONNECT_POPUP);
		__FUNC_EXIT__;
		return;
	}

	peer = ugd->multi_conn_dev_list_start;
	count = 0;
	while (peer != NULL) {
		if(peer->dev_sel_state) {
			DBG(LOG_INFO, "peer_name[%s] select state[%d]", peer->ssid, peer->dev_sel_state);
			memcpy(&ugd->raw_multi_selected_peers[count], peer, sizeof(device_type_s));
			ugd->raw_multi_selected_peers[count].conn_status = PEER_CONN_STATUS_WAIT_FOR_CONNECT;
			count++;
		}
		peer = peer->next;
	}

	DBG(LOG_INFO, "MultiSelected Peer Count2: %d", count);
	ugd->raw_multi_selected_peer_cnt = count;
	wfd_sort_multi_selected_peers(ugd);
	wfd_cancel_progressbar_stop_timer(ugd);
	wfd_cancel_not_alive_delete_timer(ugd);

	/* start multi connection */
	wfd_start_multi_connect(ugd);

	elm_naviframe_item_pop(ugd->naviframe);

	__FUNC_EXIT__;
	return;
}

void wfd_naviframe_title_set(struct ug_data *ugd, const char *title)
{
	if (!ugd || !ugd->multi_navi_item) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	elm_object_item_part_text_set(ugd->multi_navi_item, "default", title);
}

/**
 *	This function let the ug call it when click the peer in multi connect view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_gl_multi_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	int sel_count = 0;
	bool is_sel = FALSE;

	Eina_Bool state = EINA_FALSE;
	Evas_Object *chk_box = NULL;
	const char *object_type = NULL;
	struct ug_data *ugd = (struct ug_data *)wfd_get_ug_data();
	device_type_s *peer = (device_type_s *)data;
	char *format_str = NULL;

	if (NULL == ugd) {
		DBG(LOG_ERROR, "ugd is NULL\n");
		return;
	}

	ugd->is_multi_check_all_selected = TRUE;

	if(event_info != NULL) {
		Evas_Object *content = elm_object_item_part_content_get(event_info, "elm.icon.2");
		chk_box = elm_object_part_content_get(content, "elm.swallow.content");
		state = elm_check_state_get(chk_box);
		DBG(LOG_INFO, "state = %d \n", state);
		if (chk_box ==  NULL) {
			DBG(LOG_INFO, "Check box is null\n");
		}
	}

	object_type = evas_object_type_get(obj);
	 if (g_strcmp0(object_type, "elm_genlist") == 0) {
		Elm_Object_Item *item = (Elm_Object_Item *)event_info;
		if (item) {
			elm_genlist_item_selected_set(item, EINA_FALSE);
		}
		DBG(LOG_INFO, "State elm_genlist %d\n", state);
		elm_check_state_set(chk_box, !state);
		peer->dev_sel_state = !state;
	} else if (g_strcmp0(object_type, "elm_check") == 0){
		DBG(LOG_INFO, "elm_check state; %d\n", peer->dev_sel_state);
		peer->dev_sel_state = !peer->dev_sel_state;
	}

	DBG(LOG_INFO, "ptr->dev_sel_state = %d \n", peer->dev_sel_state);
	DBG_SECURE(LOG_INFO, "ptr->peer.mac_addr = ["MACSECSTR"]\n",
		MAC2SECSTR(peer->mac_addr));

	peer = ugd->multi_conn_dev_list_start;
	while (peer != NULL) {
		if(peer->dev_sel_state) {
			is_sel = TRUE;
			sel_count++;
		} else {
			ugd->is_multi_check_all_selected = FALSE;
		}
		peer = peer->next;
	}

	if (is_sel) {
		char title[MAX_POPUP_TEXT_SIZE] = {0};
		format_str = D_("IDS_ST_HEADER_PD_SELECTED");
		snprintf(title, MAX_POPUP_TEXT_SIZE, format_str, sel_count);
		wfd_naviframe_title_set(ugd, title);
	} else {
		wfd_naviframe_title_set(ugd, D_("IDS_DLNA_HEADER_SELECT_DEVICES_ABB"));
	}

	state = elm_check_state_get(ugd->select_all_icon);
	if (ugd->multiconn_layout) {
		if (state != EINA_TRUE && ugd->is_multi_check_all_selected == TRUE) {
			elm_check_state_set(ugd->select_all_icon, TRUE);
			ugd->is_select_all_checked = TRUE;
		} else if (ugd->is_multi_check_all_selected == FALSE) {
			elm_check_state_set(ugd->select_all_icon, FALSE);
			ugd->is_select_all_checked = FALSE;
		}
		wfd_ug_view_refresh_button(ugd->multiconn_conn_btn,
			"IDS_WIFI_SK_CONNECT", is_sel);
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug get the label of peer
 *	@return   the label of peer
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_wfd_gl_device_label_get(void *data, Evas_Object *obj, const char *part)
{
	__FUNC_ENTER__;
	DBG(LOG_INFO, "part %s", part);
	device_type_s *peer = (device_type_s *)data;
	char *ssid;

	if (NULL == peer) {
		return NULL;
	}

	if (!strcmp("elm.text", part)) {
		if (strlen(peer->ssid) != 0) {
			ssid = elm_entry_utf8_to_markup(peer->ssid);
			if (NULL == ssid) {
				DBG(LOG_ERROR, "elm_entry_utf8_to_markup failed.\n");
				__FUNC_EXIT__;
				return NULL;
			}
			__FUNC_EXIT__;
			return ssid;
		}
	}
	return NULL;
}

/**
 *	This function let the ug get the icon path of peer
 *	@return   the  icon path of titile
 *	@param[in] peer the pointer to the peer
 */
static char *__wfd_get_device_icon_path(device_type_s *peer)
{
	__FUNC_ENTER__;
	char *img_name = NULL;

	switch (peer->category) {
	case WFD_DEVICE_TYPE_COMPUTER:
		img_name = WFD_ICON_DEVICE_COMPUTER;
		break;
	case WFD_DEVICE_TYPE_INPUT_DEVICE:
		img_name = WFD_ICON_DEVICE_INPUT_DEVICE;
		break;
	case WFD_DEVICE_TYPE_PRINTER:
		img_name = WFD_ICON_DEVICE_PRINTER;
		break;
	case WFD_DEVICE_TYPE_CAMERA:
		img_name = WFD_ICON_DEVICE_CAMERA;
		break;
	case WFD_DEVICE_TYPE_STORAGE:
		img_name = WFD_ICON_DEVICE_STORAGE;
		break;
	case WFD_DEVICE_TYPE_NW_INFRA:
		img_name = WFD_ICON_DEVICE_NETWORK_INFRA;
		break;
	case WFD_DEVICE_TYPE_DISPLAYS:
		img_name = WFD_ICON_DEVICE_DISPLAY;
		break;
	case WFD_DEVICE_TYPE_MM_DEVICES:
		img_name = WFD_ICON_DEVICE_MULTIMEDIA;
		break;
	case WFD_DEVICE_TYPE_GAME_DEVICES:
		img_name = WFD_ICON_DEVICE_GAMING;
		break;
	case WFD_DEVICE_TYPE_TELEPHONE:
		img_name = WFD_ICON_DEVICE_TELEPHONE;
		break;
	case WFD_DEVICE_TYPE_AUDIO:
		img_name = WFD_ICON_DEVICE_HEADSET;
		break;
	default:
		img_name = WFD_ICON_DEVICE_UNKNOWN;
		break;
	}
	return img_name;
}

/**
 *	This function let the ug get the icon of peer
 *	@return   the icon of peer
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static Evas_Object *_wfd_gl_device_icon_get(void *data, Evas_Object *obj, const char *part)
{
	__FUNC_ENTER__;
	char *img_name = NULL;
	device_type_s *peer = (device_type_s *) data;
	Evas_Object *icon = NULL;
	Evas_Object *icon_layout = NULL;

	DBG(LOG_INFO, "Part %s", part);

	if (!strcmp("elm.swallow.icon", part)) {
		DBG(LOG_INFO, "Part %s", part);
		icon_layout = elm_layout_add(obj);
		elm_layout_theme_set(icon_layout, "layout", "list/B/type.3", "default");
		img_name = __wfd_get_device_icon_path(peer);
		icon = elm_image_add(icon_layout);
		elm_image_file_set(icon, WFD_UG_EDJ_PATH, img_name);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_color_set(icon, 2, 61, 132, 204);
		evas_object_show(icon);
		evas_object_propagate_events_set(icon, EINA_FALSE);
		elm_layout_content_set(icon_layout, "elm.swallow.content", icon);
	} else if (!strcmp("elm.swallow.end", part)) {
		icon_layout = elm_layout_add(obj);
		elm_layout_theme_set(icon_layout, "layout", "list/C/type.2", "default");
		DBG(LOG_INFO, "Part %s", part);
		icon = elm_check_add(icon_layout);
		elm_object_style_set(icon, "default/genlist");
		evas_object_propagate_events_set(icon, EINA_FALSE);
		if (peer->dev_sel_state == EINA_TRUE) {
			elm_check_state_set(icon, EINA_TRUE);
		}
		evas_object_smart_callback_add(icon,
			"changed", _wfd_gl_multi_sel_cb, (void *)data);
		elm_layout_content_set(icon_layout, "elm.swallow.content", icon);
	}

	if (icon_layout)
		evas_object_show(icon_layout);

	return icon_layout;
}

/**
 *	This function let the ug call it when unresized event is received
*/
static void _gl_unrealized(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int sel_count = 0;
	bool is_sel = FALSE;
	device_type_s *peer = NULL;
	char *format_str = NULL;

	if (!ugd->multiconn_conn_btn) {
		DBG(LOG_INFO, "popup naviframe, no need to update UI\n");
		return;
	}

	if (ugd->gl_available_dev_cnt_at_multiconn_view > 0) {
		peer = ugd->multi_conn_dev_list_start;
		while (peer != NULL) {
			if(peer->dev_sel_state) {
				is_sel = TRUE;
				sel_count++;
			}
			peer = peer->next;
		}
	}

	if (is_sel) {
		char title[MAX_POPUP_TEXT_SIZE] = {0};
		format_str = D_("IDS_ST_HEADER_PD_SELECTED");
		snprintf(title, MAX_POPUP_TEXT_SIZE, format_str, sel_count);
		wfd_naviframe_title_set(ugd, title);
	} else {
		wfd_naviframe_title_set(ugd, D_("IDS_DLNA_HEADER_SELECT_DEVICES_ABB"));
	}

	if (ugd->multiconn_conn_btn) {
		wfd_ug_view_refresh_button(ugd->multiconn_conn_btn,
			"IDS_WIFI_SK_CONNECT", is_sel);
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug free the multi connect devices
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int wfd_free_multiconnect_device(struct ug_data *ugd)
{
	__FUNC_ENTER__;

	if (ugd->multiconn_view_genlist == NULL) {
		return 0;
	}

	ugd->gl_available_dev_cnt_at_multiconn_view = 0;
	if (ugd->multi_conn_dev_list_start != NULL) {
		wfd_ug_view_free_peer(ugd->multi_conn_dev_list_start);
		ugd->multi_conn_dev_list_start = NULL;
	}

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug create "no device found" item
*/
Evas_Object *_create_no_device_multiconnect_genlist(struct ug_data *ugd)
{
	__FUNC_ENTER__;

	if (NULL == ugd) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return NULL;
	}

	if (!ugd->mcview_nodevice_item) {
		ugd->mcview_nodevice_item = elm_genlist_item_append(ugd->multiconn_view_genlist, &noitem_itc, (void *)ugd, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		if(ugd->mcview_nodevice_item != NULL)
			elm_genlist_item_select_mode_set(ugd->mcview_nodevice_item , ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	__FUNC_EXIT__;
	return ugd->multiconn_view_genlist;
}

/**
 *	This function let the ug update the multi connect devices
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int wfd_update_multiconnect_device(struct ug_data *ugd, bool is_free_all_peers)
{
	__FUNC_ENTER__;

	int count = 0;
	device_type_s *device = NULL;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *item = NULL;
	int res = 0;

	genlist = ugd->multiconn_view_genlist;
	if (ugd->multiconn_view_genlist == NULL) {
		return 0;
	}

	if (is_free_all_peers) {
		wfd_free_multiconnect_device(ugd);
	}

	GList *iterator = NULL;
	for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
		device = (device_type_s *)iterator->data;
		if (device->is_connected == FALSE && device->is_group_owner == FALSE) {
			count++;
		}
	}

	if (count == 0) {
		DBG(LOG_INFO, "There are No peers\n");
		if (!ugd->mcview_title_item) {
			ugd->mcview_title_item = elm_genlist_item_append(genlist, &multi_view_title_itc, ugd, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			if(ugd->mcview_title_item != NULL)
				elm_genlist_item_select_mode_set(ugd->mcview_title_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else {
			wfd_ug_view_refresh_glitem(ugd->mcview_title_item);
		}
		elm_check_state_set(ugd->select_all_icon, EINA_FALSE);
		ugd->is_select_all_checked = FALSE;
		wfd_free_multiconnect_device(ugd);
		wfd_naviframe_title_set(ugd, D_("IDS_DLNA_HEADER_SELECT_DEVICES_ABB"));
	} else {
		WFD_IF_DEL_ITEM(ugd->mcview_nodevice_item);
		wfd_ug_view_refresh_glitem(ugd->mcview_title_item);

		for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
			device = (device_type_s *)iterator->data;
			if (device->is_connected == FALSE && device->is_group_owner == FALSE) {
				if (!find_peer_in_glist(ugd->multi_conn_dev_list_start, device->mac_addr)) {
					item = get_insert_postion(device, ugd->mcview_title_item, ugd->gl_available_dev_cnt_at_multiconn_view);
					res = insert_gl_item(genlist, item, &device_itc, &ugd->multi_conn_dev_list_start,
								device, _wfd_gl_multi_sel_cb);
					if (res != 0) {
						break;
					}
					elm_check_state_set(ugd->select_all_icon, EINA_FALSE);
					ugd->is_select_all_checked = FALSE;

					ugd->gl_available_dev_cnt_at_multiconn_view++;
				}
			}
		}
	}
	ugd->is_multi_check_all_selected = FALSE;

	__FUNC_EXIT__;
	return 0;
}

void wfd_genlist_select_all_check_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	__FUNC_ENTER__;
	Elm_Object_Item *item = NULL;
	Evas_Object *chk_box = NULL;
	Evas_Object *content = NULL;
	const char *object_type;
	Eina_Bool state;
	char *format_str = NULL;


	struct ug_data *ugd = (struct ug_data *)data;
	if (NULL == ugd || NULL == obj) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return;
	}

	if (ugd->multi_conn_dev_list_start == NULL) {
		elm_genlist_item_selected_set(ugd->select_all_view_genlist, EINA_FALSE);
		elm_check_state_set(ugd->select_all_icon, EINA_FALSE);
		ugd->is_select_all_checked = FALSE;

		DBG(LOG_INFO, "No devices in multi-connect view.\n");
		return;
	}

	device_type_s *peer = ugd->multi_conn_dev_list_start;
	elm_genlist_item_selected_set(ugd->select_all_view_genlist, EINA_FALSE);


	object_type = evas_object_type_get(obj);
	 if (g_strcmp0(object_type, "elm_genlist") == 0) {
		state = elm_check_state_get(ugd->select_all_icon);
		elm_check_state_set(ugd->select_all_icon, !state);
		ugd->is_select_all_checked = !state;
	}

	state = elm_check_state_get(ugd->select_all_icon);
	DBG(LOG_INFO, "state = %d",state);
	if (state == EINA_TRUE) {
		if (ugd->is_multi_check_all_selected == FALSE) {
			int sel_count = 0;
			while (peer != NULL) {
				peer->dev_sel_state = TRUE;
				item = peer->gl_item;
				content = elm_object_item_part_content_get(item, "elm.icon.2");
				chk_box = elm_object_part_content_get(content, "elm.swallow.content");
				elm_check_state_set(chk_box, TRUE);
				sel_count++;
				peer = peer->next;
			}
			ugd->is_multi_check_all_selected = TRUE;

			char title[MAX_POPUP_TEXT_SIZE] = {0};
			format_str = D_("IDS_ST_HEADER_PD_SELECTED");
			snprintf(title, MAX_POPUP_TEXT_SIZE, format_str, sel_count);
			wfd_naviframe_title_set(ugd, title);

			if (ugd->multiconn_layout) {
				wfd_ug_view_refresh_button(ugd->multiconn_conn_btn,
					"IDS_WIFI_SK_CONNECT", TRUE);
			}
		}
	} else {
		while (peer != NULL) {
			peer->dev_sel_state = FALSE;
			item = peer->gl_item;
			content = elm_object_item_part_content_get(item, "elm.icon.2");
			chk_box = elm_object_part_content_get(content, "elm.swallow.content");
			elm_check_state_set(chk_box, FALSE);
			peer = peer->next;
		}
		ugd->is_multi_check_all_selected = FALSE;
		wfd_naviframe_title_set(ugd,
			D_("IDS_DLNA_HEADER_SELECT_DEVICES_ABB"));

		if (ugd->multiconn_layout) {
			wfd_ug_view_refresh_button(ugd->multiconn_conn_btn,
				"IDS_WIFI_SK_CONNECT", FALSE);
		}
	}
}

/**
 *	This function let the ug call it when click 'scan' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _multi_scan_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	int ret = -1;
	const char *btn_text = NULL;
	device_type_s *peer = NULL;
	struct ug_data *ugd = (struct ug_data *) data;

	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	if (ugd->multiconn_conn_btn) {
		peer = ugd->multi_conn_dev_list_start;
		while (peer) {
			if(peer->dev_sel_state) {
				wfd_ug_view_refresh_button(ugd->multiconn_conn_btn, "IDS_WIFI_SK_CONNECT", TRUE);
				break;
			}
			peer = peer->next;
		}
		if (!peer) {
			wfd_ug_view_refresh_button(ugd->multiconn_conn_btn, "IDS_WIFI_SK_CONNECT", FALSE);
		}
	}

	btn_text = elm_object_text_get(ugd->multiconn_scan_stop_btn);
	if (NULL == btn_text) {
		DBG(LOG_ERROR, "Incorrect button text(NULL)\n");
		return;
	}

	if (0 == strcmp(btn_text, D_("IDS_WIFI_SK4_SCAN"))) {
		wfd_refresh_wifi_direct_state(ugd);
		DBG(LOG_INFO, "Start discovery again, status: %d\n", ugd->wfd_status);

		/* if connected, show the popup*/
		if (ugd->wfd_status >= WIFI_DIRECT_STATE_CONNECTED) {
			wfd_ug_act_popup(ugd, D_("IDS_WIFI_BODY_CURRENT_CONNECTION_WILL_BE_DISCONNECTED_SO_THAT_SCANNING_CAN_START_CONTINUE_Q"), POP_TYPE_SCAN_AGAIN);
		} else if (WIFI_DIRECT_STATE_DEACTIVATED == ugd->wfd_status) {
			wfd_client_switch_on(ugd);
			__FUNC_EXIT__;
			return;
		} else {
			WFD_IF_DEL_ITEM(ugd->mcview_nodevice_item);
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
			ret = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
				DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", ret);
				wifi_direct_cancel_discovery();
			}
		}
		elm_object_domain_translatable_text_set(ugd->multiconn_scan_stop_btn,
				PACKAGE, "IDS_WIFI_SK_STOP");
	} else if (0 == strcmp(btn_text, D_("IDS_WIFI_SK_STOP"))) {
		DBG(LOG_INFO, "Stop pressed.\n");
		ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_STOPPED;
		wfd_cancel_progressbar_stop_timer(ugd);
		wfd_delete_progressbar_cb(ugd);
		wfd_cancel_not_alive_delete_timer(ugd);
	}

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug create the view for multi connection
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 */
void wfd_create_multiconnect_view(struct ug_data *ugd)
{
	__FUNC_ENTER__;

	Evas_Object *genlist = NULL;
	Elm_Object_Item *navi_item = NULL;
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	Evas_Object *layout = NULL;

	if (ugd == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)");
		return;
	}

	device_itc.item_style = WFD_GENLIST_1LINE_TEXT_ICON_STYLE;
	device_itc.func.text_get = _wfd_gl_device_label_get;
	device_itc.func.content_get = _wfd_gl_device_icon_get;
	device_itc.func.state_get = NULL;
	device_itc.func.del = NULL;

	/* Create layout */
	layout = elm_layout_add(ugd->naviframe);
	elm_layout_file_set(layout, WFD_UG_EDJ_PATH, "bottom_btn");
	ugd->multiconn_layout = layout;


	genlist = elm_genlist_add(ugd->multiconn_layout);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "unrealized", _gl_unrealized, ugd);

	elm_object_part_content_set(layout, "elm.swallow.content", genlist);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
#if defined(GENLIST_REALIZATION_MOTE_SET)
	elm_genlist_realization_mode_set(genlist, TRUE);
#endif

	ugd->multiconn_view_genlist = genlist;
	ugd->mcview_title_item = NULL;
	ugd->mcview_nodevice_item = NULL;
	ugd->gl_available_dev_cnt_at_multiconn_view = 0;
	evas_object_show(genlist);

	navi_item = elm_naviframe_item_push(ugd->naviframe,
		"IDS_DLNA_HEADER_SELECT_DEVICES_ABB", NULL, NULL, layout, NULL);
	elm_object_item_domain_text_translatable_set(navi_item, PACKAGE, EINA_TRUE);

	elm_naviframe_item_pop_cb_set(navi_item, _multiconnect_view_pop_cb, (void *)ugd);
	ugd->select_all_view_genlist = elm_genlist_item_append(genlist,
			&select_all_multi_connect_itc, ugd, NULL, ELM_GENLIST_ITEM_NONE,
			wfd_genlist_select_all_check_changed_cb, (void *)ugd );


	btn1 = elm_button_add(ugd->multiconn_layout);
	elm_object_style_set(btn1, "bottom");
	if (ugd->view_type && g_strcmp0(D_(ugd->view_type),
		D_("IDS_WIFI_BUTTON_MULTI_CONNECT")) == 0) {
		elm_object_domain_translatable_text_set(btn1, PACKAGE,
				"IDS_WIFI_SK4_SCAN");
	} else {
		elm_object_domain_translatable_text_set(btn1, PACKAGE,
				"IDS_WIFI_SK_STOP");
	}
	elm_object_part_content_set(ugd->multiconn_layout, "button.prev", btn1);
	evas_object_smart_callback_add(btn1, "clicked",_multi_scan_btn_cb, (void *)ugd);
	evas_object_show(btn1);
	ugd->multiconn_scan_stop_btn = btn1;

	btn2 = elm_button_add(ugd->multiconn_layout);
	elm_object_style_set(btn2, "bottom");
	elm_object_domain_translatable_text_set(btn2, PACKAGE,
			"IDS_WIFI_SK_CONNECT");
	elm_object_part_content_set(ugd->multiconn_layout, "button.next", btn2);
	evas_object_smart_callback_add(btn2, "clicked",_connect_btn_cb, (void *)ugd);
	evas_object_show(btn2);
	ugd->multiconn_conn_btn = btn2;
	elm_object_disabled_set(ugd->multiconn_conn_btn, EINA_TRUE);

	ugd->multi_navi_item = navi_item;

	wfd_update_multiconnect_device(ugd, true);

	__FUNC_EXIT__;
}
