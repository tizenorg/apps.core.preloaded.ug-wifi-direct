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
#include <vconf.h>
#include <ui-gadget-module.h>
#include <wifi-direct.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

Elm_Gen_Item_Class select_all_itc;
Elm_Gen_Item_Class device_itc;

/**
 *	This function let the ug call it when click 'back' button in multi connect view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _multiconnect_view_back_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;

	if (!ugd) {
		WDUG_LOGE("The param is NULL\n");
		return;
	}

	ugd->multiconn_view_genlist = NULL;
	elm_naviframe_item_pop(ugd->naviframe);

	__WDUG_LOG_FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug reset the connected failed peers
 *	@return   false
 *	@param[in] event_info the pointer to the event information
 */
gboolean __wfd_multi_connect_reset_cb(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	int i = 0;
	struct ug_data *ugd = (struct ug_data *)data;

	/* remove the failed peers*/
	for (i = 0; i < ugd->raw_multi_selected_peer_cnt; i++) {
		if (ugd->raw_multi_selected_peers[i].conn_status == PEER_CONN_STATUS_FAILED_TO_CONNECT) {
			memset(&ugd->raw_multi_selected_peers[i], 0x00, sizeof(device_type_s));
			ugd->raw_multi_selected_peer_cnt--;
		}
	}

	wfd_ug_view_update_peers(ugd);

	__WDUG_LOG_FUNC_EXIT__;
	return false;
}

/**
 *	This function let the ug free the selected peers in multi connect view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_free_multi_selected_peers(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
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

	__WDUG_LOG_FUNC_EXIT__;

}

/**
 *	This function let the ug stop to connect to selected peer
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_stop_multi_connect(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;

	/* change the title of failed peers */
	ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_COMPLETED;
	wfd_ug_view_refresh_glitem(ugd->multi_connect_wfd_item);

	wfd_client_set_p2p_group_owner_intent(7);

	/* after 30s, remove the failed peers */
	g_timeout_add(30000 /*ms*/, __wfd_multi_connect_reset_cb, ugd);

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug start to connect the selected peers
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_start_multi_connect(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;

	if (ugd->raw_multi_selected_peer_cnt > 0) {
		ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_IN_PROGRESS;

		res = wfd_client_group_add();
		if (res == -1) {
			WDUG_LOGE("Failed to add group");
			wfd_free_multi_selected_peers(ugd);

			__WDUG_LOG_FUNC_EXIT__;
			return -1;
		}

	} else {
		WDUG_LOGD("No selected peers.\n");
		return -1;
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug connect to the next selected peer automatically
 *	@return   If stop the timer, return false, else return true
 *	@param[in] data the pointer to the main data structure
 */
gboolean wfd_multi_connect_next_cb(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int i;
	int res;

	// Reset g_source handler..
	ugd->g_source_multi_connect_next = 0;

	if (ugd->raw_multi_selected_peer_cnt > 0) {
		ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_IN_PROGRESS;
		for (i = 0; i < ugd->raw_multi_selected_peer_cnt; i++) {
			if (ugd->raw_multi_selected_peers[i].conn_status == PEER_CONN_STATUS_WAIT_FOR_CONNECT) {
				res = wfd_client_connect(ugd->raw_multi_selected_peers[i].mac_addr);
				if (res == -1) {
					WDUG_LOGD("Failed to connect [%s].\n", ugd->raw_multi_selected_peers[i].ssid);
					ugd->raw_multi_selected_peers[i].conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
				} else {
					ugd->raw_multi_selected_peers[i].conn_status = PEER_CONN_STATUS_CONNECTING;
					break;
				}
			}
		}

		if (i >= ugd->raw_multi_selected_peer_cnt) {
			// All selected peers are touched.
			WDUG_LOGD("Stop Multi Connect...\n");
			wfd_stop_multi_connect(ugd);
		}
	} else {
		WDUG_LOGD("No selected peers.\n");
		return -1;
	}

	__WDUG_LOG_FUNC_EXIT__;
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
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int i = 0;
	int count = 0;
	char popup_text[MAX_POPUP_TEXT_SIZE] = {0};
	WDUG_LOGD("_connect_btn_cb \n");

	for (i = 0; i < ugd->gl_available_peer_cnt ; i++) {
		if (TRUE == ugd->multi_conn_dev_list[i].dev_sel_state) {
			WDUG_LOGD("ugd->peers[i].mac_addr = %s, i = %d\n", ugd->multi_conn_dev_list[i].peer.mac_addr, i);

			memcpy(&ugd->raw_multi_selected_peers[count], &ugd->multi_conn_dev_list[i].peer, sizeof(device_type_s));
			ugd->raw_multi_selected_peers[count].conn_status = PEER_CONN_STATUS_WAIT_FOR_CONNECT;
			count++;
		}
	}

	ugd->raw_multi_selected_peer_cnt = count;

	/* if more than 7 device selected, show the popup */
	if (count > MAX_POPUP_PEER_NUM) {
		snprintf(popup_text, MAX_POPUP_TEXT_SIZE, _("IDS_WFD_POP_MULTI_CONNECT"), count);
		wfd_ug_warn_popup(ugd, popup_text, POP_TYPE_MULTI_CONNECT_POPUP);
	}

	/* start multi connection */
	wfd_start_multi_connect(ugd);

	elm_naviframe_item_pop(ugd->naviframe);

	//ToDo: Do we need to free multiconn_view_genlist?
	ugd->multiconn_view_genlist = NULL;
	_change_multi_button_title(ugd);

	__WDUG_LOG_FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug delete 'select(n)' notify
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
static void _wfd_multi_del_select_info_label(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;

	if (NULL == ugd) {
		WDUG_LOGE("The param is NULL\n");
		return;
	}

	if (ugd->notify) {
		evas_object_del(ugd->notify);
		ugd->notify = NULL;
	}

	if (ugd->notify_layout) {
		evas_object_del(ugd->notify_layout);
		ugd->notify_layout = NULL;
	}

	__WDUG_LOG_FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug add 'select(n)' notify
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] count the number of selected peers
 */
static void _wfd_multi_add_select_info_label(void *data, int count)
{
	__WDUG_LOG_FUNC_ENTER__;

	char select_lablel[MAX_POPUP_TEXT_SIZE] = {0};
	struct ug_data *ugd = (struct ug_data *)data;

	if (NULL == ugd || count <= 0) {
		WDUG_LOGE("The param is NULL\n");
		return;
	}

	/* delete previous notify */
	_wfd_multi_del_select_info_label(ugd);

	/* add notify */
	ugd->notify = elm_notify_add(ugd->base);
	if (NULL == ugd->notify) {
		WDUG_LOGE("Add notify failed\n");
		return;
	}

	/* set the align to center of bottom */
	elm_notify_align_set(ugd->notify, ELM_NOTIFY_ALIGN_FILL, 1.0);

	ugd->notify_layout = elm_layout_add(ugd->notify);
	if (NULL == ugd->notify_layout) {
		evas_object_del(ugd->notify);
		ugd->notify = NULL;
		return;
	}

	elm_layout_theme_set(ugd->notify_layout, "standard", "selectioninfo", "vertical/bottom_64");
	elm_object_content_set(ugd->notify, ugd->notify_layout);

	snprintf(select_lablel, MAX_POPUP_TEXT_SIZE, _("IDS_WFD_POP_SELECTED_DEVICE_NUM"), count);
	elm_object_part_text_set(ugd->notify_layout, "elm.text", select_lablel);
	elm_notify_timeout_set(ugd->notify, 3);
	evas_object_show(ugd->notify);

	__WDUG_LOG_FUNC_EXIT__;
	return;
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
	__WDUG_LOG_FUNC_ENTER__;

	int i = 0;
	int index = 0;
	int sel_count = 0;
	bool is_sel = FALSE;
	bool is_selct_all = TRUE;
	Eina_Bool state = 0;
	Evas_Object *chk_box = NULL;
	char msg[MAX_POPUP_TEXT_SIZE] = {0};
	struct ug_data *ugd = (struct ug_data *)data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (NULL == ugd || NULL == item) {
		WDUG_LOGE("The param is NULL\n");
		return;
	}

	elm_genlist_item_selected_set(item, EINA_FALSE);
	index = elm_genlist_item_index_get(item) - 2; /* subtract the previous items */
	WDUG_LOGD("selected index = %d \n", index);
	if (index < 0) {
		WDUG_LOGE("The index is invalid.\n");
		return;
	}

	chk_box = elm_object_item_part_content_get((Elm_Object_Item *)event_info, "elm.icon.1");
	state = elm_check_state_get(chk_box);
	WDUG_LOGD("state = %d \n", state);
	elm_check_state_set(chk_box, !state);

	ugd->multi_conn_dev_list[index].dev_sel_state = !state;
	WDUG_LOGD("ptr->dev_sel_state = %d \n", ugd->multi_conn_dev_list[index].dev_sel_state);
	WDUG_LOGD("ptr->peer.mac_addr = %s \n", ugd->multi_conn_dev_list[index].peer.mac_addr);

	/* update the checkbox and button */
	for (; i < ugd->gl_available_dev_cnt_at_multiconn_view; i++) {
		if (ugd->multi_conn_dev_list[i].dev_sel_state) {
			is_sel = TRUE;
			sel_count++;
		} else {
			is_selct_all = FALSE;
		}
	}

	chk_box = elm_object_item_part_content_get(ugd->mcview_select_all_item, "elm.icon");
	elm_check_state_set(chk_box, is_selct_all);

	if (ugd->multi_connect_btn) {
		wfd_ug_view_refresh_button(ugd->multi_connect_btn, _("IDS_WFD_BUTTON_CONNECT"), is_sel);
	}

	if (sel_count > 0) {
		snprintf(msg, MAX_POPUP_TEXT_SIZE, _("IDS_WFD_POP_SELECTED_DEVICE_NUM"), sel_count);
		_wfd_multi_add_select_info_label(ugd, sel_count);
	} else {
		_wfd_multi_del_select_info_label(ugd);
	}

	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click the 'select all' item in multi connect view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_gl_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	int sel_count = 0;
	char msg[MAX_POPUP_TEXT_SIZE] = {0};
	struct ug_data *ugd = (struct ug_data *)data;

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (NULL == ugd || NULL == obj) {
		WDUG_LOGE("NULL parameters.\n");
		return;
	}

	Evas_Object *sel_chkbox = elm_object_item_part_content_get(ugd->mcview_select_all_item, "elm.icon");
	if (sel_chkbox == NULL) {
		WDUG_LOGD("select-all chkbox is NULL\n");
		return;
	}

	Eina_Bool state = elm_check_state_get(sel_chkbox);
	if (state == TRUE) {
		state = FALSE;
	} else {
		state = TRUE;
	}

	elm_check_state_set(sel_chkbox, state);
	WDUG_LOGD("state = %d \n", state);

	int i = 0;
	bool is_sel = FALSE;
	Elm_Object_Item *item = NULL;
	Evas_Object *chk_box = NULL;

	/* set the state of all the available devices */
	for (i = 0; i < ugd->gl_available_dev_cnt_at_multiconn_view; i++) {
		is_sel = state;
		ugd->multi_conn_dev_list[i].dev_sel_state = state;
		item = ugd->multi_conn_dev_list[i].peer.gl_item;
		chk_box = elm_object_item_part_content_get(item, "elm.icon.1");
		elm_check_state_set(chk_box, state);

		if (state) {
			sel_count++;
		}
	}

	/* update the connect button */
	if (ugd->multi_connect_btn) {
		wfd_ug_view_refresh_button(ugd->multi_connect_btn, _("IDS_WFD_BUTTON_CONNECT"), is_sel);
	}

	/* tickernoti popup */
	if (sel_count > 0) {
		snprintf(msg, MAX_POPUP_TEXT_SIZE, _("IDS_WFD_POP_SELECTED_DEVICE_NUM"), sel_count);
		_wfd_multi_add_select_info_label(ugd, sel_count);
	} else {
		_wfd_multi_del_select_info_label(ugd);
	}
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
	WDUG_LOGD("part %s", part);
	device_type_s *peer = (device_type_s *)data;

	if (NULL == peer) {
		return NULL;
	}

	if (!strcmp(part, "elm.text")) {
		return strdup(peer->ssid);
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
	char *img_path = NULL;

	switch (peer->category) {
	case WFD_DEVICE_TYPE_COMPUTER:
		img_path = WFD_ICON_DEVICE_COMPUTER;
		break;
	case WFD_DEVICE_TYPE_INPUT_DEVICE:
		img_path = WFD_ICON_DEVICE_INPUT_DEVICE;
		break;
	case WFD_DEVICE_TYPE_PRINTER:
		img_path = WFD_ICON_DEVICE_PRINTER;
		break;
	case WFD_DEVICE_TYPE_CAMERA:
		img_path = WFD_ICON_DEVICE_CAMERA;
		break;
	case WFD_DEVICE_TYPE_STORAGE:
		img_path = WFD_ICON_DEVICE_STORAGE;
		break;
	case WFD_DEVICE_TYPE_NW_INFRA:
		img_path = WFD_ICON_DEVICE_NETWORK_INFRA;
		break;
	case WFD_DEVICE_TYPE_DISPLAYS:
		img_path = WFD_ICON_DEVICE_DISPLAY;
		break;
	case WFD_DEVICE_TYPE_MM_DEVICES:
		img_path = WFD_ICON_DEVICE_MULTIMEDIA_DEVICE;
		break;
	case WFD_DEVICE_TYPE_GAME_DEVICES:
		img_path = WFD_ICON_DEVICE_GAMING_DEVICE;
		break;
	case WFD_DEVICE_TYPE_TELEPHONE:
		img_path = WFD_ICON_DEVICE_TELEPHONE;
		break;
	case WFD_DEVICE_TYPE_AUDIO:
		img_path = WFD_ICON_DEVICE_AUDIO_DEVICE;
		break;
	default:
		img_path = WFD_ICON_DEVICE_COMPUTER;
		break;
	}

	return img_path;
}


/**
 *	This function let the ug call it when click the check box
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (NULL == obj) {
		WDUG_LOGE("NULL parameters.\n");
		return;
	}

	Eina_Bool state = elm_check_state_get(obj);
	elm_check_state_set(obj, !state);
	WDUG_LOGD("state = %d \n", state);
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
	char *img_path = NULL;
	device_type_s *peer = (device_type_s *) data;
	Evas_Object *icon = NULL;

	WDUG_LOGD("Part %s", part);

	if (!strcmp(part, "elm.icon.1")) {
		WDUG_LOGD("Part %s", part);
		icon = elm_check_add(obj);
		elm_check_state_set(icon, EINA_FALSE);
		evas_object_smart_callback_add(icon, "changed", _wfd_check_clicked_cb, (void *)data);
	} else if (!strcmp(part, "elm.icon.2")) {
		img_path = __wfd_get_device_icon_path(peer);
		icon = elm_icon_add(obj);
		elm_icon_file_set(icon, img_path, NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_icon_resizable_set(icon, 1, 1);
		evas_object_show(icon);
	}

	return icon;
}

/**
 *	This function let the ug get the label of select all
 *	@return   the label of select all
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_wfd_gl_select_all_label_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text")) {
		WDUG_LOGD("Adding text %s", part);
		return strdup("Select all");
	}
	return NULL;
}

/**
 *	This function let the ug get the icon of select all
 *	@return   the icon of select all
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static Evas_Object *_wfd_gl_select_all_icon_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = NULL;

	if (!strcmp(part, "elm.icon")) {
		WDUG_LOGD("Part %s", part);
		icon = elm_check_add(obj);
		elm_check_state_set(icon, EINA_FALSE);
		evas_object_smart_callback_add(icon, "changed", _wfd_check_clicked_cb, (void *)data);
	}

	return icon;
}

/**
 *	This function let the ug fee the multi connect devices
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int wfd_free_multiconnect_device(struct ug_data *ugd)
{
	__WDUG_LOG_FUNC_ENTER__;

	int i = 0;

	if (ugd->multiconn_view_genlist == NULL) {
		return 0;
	}

	if (ugd->mcview_title_item != NULL) {
		elm_object_item_del(ugd->mcview_title_item);
		ugd->mcview_title_item = NULL;
	}

	if (ugd->mcview_select_all_item != NULL) {
		elm_object_item_del(ugd->mcview_select_all_item);
		ugd->mcview_select_all_item = NULL;
	}

	if (ugd->mcview_nodevice_item != NULL) {
		elm_object_item_del(ugd->mcview_nodevice_item);
		ugd->mcview_nodevice_item = NULL;
	}

	for (i = 0; i < ugd->gl_available_dev_cnt_at_multiconn_view;  i++) {
		if (ugd->multi_conn_dev_list[i].peer.gl_item != NULL) {
			elm_object_item_del(ugd->multi_conn_dev_list[i].peer.gl_item);
			ugd->multi_conn_dev_list[i].peer.gl_item = NULL;
		}
	}
	ugd->gl_available_dev_cnt_at_multiconn_view = 0;

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug update the multi connect devices
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int wfd_update_multiconnect_device(struct ug_data *ugd)
{
	__WDUG_LOG_FUNC_ENTER__;

	int count = 0;
	device_type_s *device = NULL;
	Evas_Object *genlist = NULL;
	int i = 0;

	genlist = ugd->multiconn_view_genlist;
	if (ugd->multiconn_view_genlist == NULL) {
		return 0;
	}

	wfd_free_multiconnect_device(ugd);

	count = 0;
	for (i = 0; i < ugd->raw_discovered_peer_cnt; i++) {
		device = &ugd->raw_discovered_peers[i];
		if (device->is_connected == FALSE) {
			count++;
		}
	}
	ugd->gl_available_dev_cnt_at_multiconn_view = count;

	if (ugd->gl_available_dev_cnt_at_multiconn_view == 0) {
		WDUG_LOGE("There are No peers\n");
		ugd->mcview_title_item = elm_genlist_item_append(genlist, &title_itc, ugd, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(ugd->mcview_title_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		ugd->mcview_nodevice_item = elm_genlist_item_append(genlist, &noitem_itc, (void *)ugd, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(ugd->mcview_nodevice_item , ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	} else {
		ugd->mcview_title_item = elm_genlist_item_append(genlist, &title_itc, ugd, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(ugd->mcview_title_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		ugd->mcview_select_all_item = elm_genlist_item_append(genlist, &select_all_itc, ugd, NULL, ELM_GENLIST_ITEM_NONE, _wfd_gl_sel_cb, ugd);

		count = 0;
		for (i = 0; i < ugd->raw_discovered_peer_cnt; i++) {
			device = &ugd->raw_discovered_peers[i];
			if (device->is_connected == FALSE) {
				WDUG_LOGD("%dth peer being added on genlist\n", i);

				if (ugd->multi_conn_dev_list[count].peer.gl_item != NULL) {
					elm_object_item_del(ugd->multi_conn_dev_list[count].peer.gl_item);
				}

				ugd->multi_conn_dev_list[count].peer.gl_item = NULL;
				memcpy(&ugd->multi_conn_dev_list[count].peer, device, sizeof(device_type_s));
				ugd->multi_conn_dev_list[count].dev_sel_state = FALSE;
				ugd->multi_conn_dev_list[count].peer.gl_item = elm_genlist_item_append(genlist, &device_itc,
					(void *)&ugd->multi_conn_dev_list[count].peer, NULL, ELM_GENLIST_ITEM_NONE, _wfd_gl_multi_sel_cb, ugd);
				count++;
			}
		}
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug create the view for multi connection
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 */
void wfd_create_multiconnect_view(struct ug_data *ugd)
{
	__WDUG_LOG_FUNC_ENTER__;

	Evas_Object *back_btn = NULL;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *navi_item = NULL;

	if (ugd == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)");
		return;
	}

	select_all_itc.item_style = "1text.1icon.3";
	select_all_itc.func.text_get = _wfd_gl_select_all_label_get;
	select_all_itc.func.content_get = _wfd_gl_select_all_icon_get;
	select_all_itc.func.state_get = NULL;
	select_all_itc.func.del = NULL;

	device_itc.item_style = "1text.2icon.2";
	device_itc.func.text_get = _wfd_gl_device_label_get;
	device_itc.func.content_get = _wfd_gl_device_icon_get;
	device_itc.func.state_get = NULL;
	device_itc.func.del = NULL;

	WDUG_LOGD("_wifid_create_multiconnect_view");
	back_btn = elm_button_add(ugd->naviframe);
	elm_object_style_set(back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(back_btn, "clicked", _multiconnect_view_back_btn_cb, (void *)ugd);
	elm_object_focus_allow_set(back_btn, EINA_FALSE);

	genlist = elm_genlist_add(ugd->naviframe);
	ugd->multiconn_view_genlist = genlist;
	ugd->mcview_title_item = NULL;

	wfd_update_multiconnect_device(ugd);

	evas_object_show(genlist);

	navi_item = elm_naviframe_item_push(ugd->naviframe, _("Multi connect"), back_btn, NULL, genlist, NULL);

	/* create scan button */
	ugd->multi_scan_btn = elm_button_add(ugd->naviframe);
	elm_object_style_set(ugd->multi_scan_btn, "naviframe/toolbar/default");
	elm_object_text_set(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"));
	evas_object_smart_callback_add(ugd->multi_scan_btn, "clicked", _scan_btn_cb, (void *)ugd);
	elm_object_item_part_content_set(navi_item, "toolbar_button1", ugd->multi_scan_btn);

	/* create connect button */
	ugd->multi_connect_btn = elm_button_add(ugd->naviframe);
	elm_object_style_set(ugd->multi_connect_btn, "naviframe/toolbar/default");
	elm_object_text_set(ugd->multi_connect_btn, _("IDS_WFD_BUTTON_CONNECT"));
	evas_object_smart_callback_add(ugd->multi_connect_btn, "clicked", _connect_btn_cb, (void *)ugd);
	elm_object_disabled_set(ugd->multi_connect_btn, EINA_TRUE);
	elm_object_item_part_content_set(navi_item, "toolbar_button2", ugd->multi_connect_btn);

	__WDUG_LOG_FUNC_EXIT__;
}
