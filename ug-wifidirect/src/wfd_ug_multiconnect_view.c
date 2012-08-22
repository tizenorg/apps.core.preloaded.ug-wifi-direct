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

Elm_Gen_Item_Class select_all_itc;
Elm_Gen_Item_Class device_itc;


void _multiconnect_view_back_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    if(!ugd)
    {
        DBG(LOG_ERROR, "The param is NULL\n");
        return;
    }

    ugd->multiconn_view_genlist = NULL;
    elm_naviframe_item_pop(ugd->naviframe);

    __FUNC_EXIT__;
    return;
}

void reset_multi_conn_dev_list(void *data)
{
	struct ug_data *ugd = (struct ug_data*) data;
	int i;
    for (i = 0; i < MAX_PEER_NUM; i++)
    {
    	ugd->multi_conn_dev_list[i].dev_sel_state = FALSE;
    	ugd->multi_conn_dev_list[i].peer.gl_item = NULL;
    }
    ugd->gl_available_dev_cnt_at_multiconn_view = 0;
}

gboolean __wfd_multi_connect_reset_cb(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_NONE;
    ugd->raw_multi_selected_peer_cnt = 0;
	wfd_ug_view_update_peers(ugd);

	__FUNC_EXIT__;
	return false;
}

int wfd_stop_multi_connect(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_COMPLETED;
    wfd_client_set_p2p_group_owner_intent(7);

    g_timeout_add(1000 /*ms*/, __wfd_multi_connect_reset_cb, ugd);

    __FUNC_EXIT__;
	return 0;
}

int wfd_start_multi_connect(void* data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
	int i;
	int res;

	if (ugd->raw_multi_selected_peer_cnt > 0)
	{
		ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_IN_PROGRESS;
		if (wfd_client_set_p2p_group_owner_intent(15) == WIFI_DIRECT_ERROR_NONE)
		{
			for (i=0;i<ugd->raw_multi_selected_peer_cnt; i++)
			{
				res = wfd_client_connect(ugd->raw_multi_selected_peers[i].mac_addr);
				if (res == -1)
				{
					DBG(LOG_VERBOSE, "Failed to connect [%s].\n", ugd->raw_multi_selected_peers[i].ssid);
					ugd->raw_multi_selected_peers[i].conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
				}
				else
				{
					ugd->raw_multi_selected_peers[i].conn_status = PEER_CONN_STATUS_CONNECTING;
					break;
				}
			}

			if (i >= ugd->raw_multi_selected_peer_cnt)
			{
				wfd_client_set_p2p_group_owner_intent(7);

				DBG(LOG_VERBOSE, "All connect trails are failed.\n");
				return -1;
			}
		}
		else
		{
			// error popup...
			DBG(LOG_VERBOSE, "Setting GO intent is failed.\n");
			return -1;
		}

	}
	else
	{
		DBG(LOG_VERBOSE, "No selected peers.\n");
		return -1;
	}

	__FUNC_EXIT__;
	return 0;
}

gboolean wfd_multi_connect_next_cb(void* data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
	int i;
	int res;

	// Reset g_source handler..
	ugd->g_source_multi_connect_next = 0;

	if (ugd->raw_multi_selected_peer_cnt > 0)
	{
		ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_IN_PROGRESS;
		for (i=0;i<ugd->raw_multi_selected_peer_cnt; i++)
		{
			if (ugd->raw_multi_selected_peers[i].conn_status == PEER_CONN_STATUS_WAIT_FOR_CONNECT)
			{
				res = wfd_client_connect(ugd->raw_multi_selected_peers[i].mac_addr);
				if (res == -1)
				{
					DBG(LOG_VERBOSE, "Failed to connect [%s].\n", ugd->raw_multi_selected_peers[i].ssid);
					ugd->raw_multi_selected_peers[i].conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
				}
				else
				{
					ugd->raw_multi_selected_peers[i].conn_status = PEER_CONN_STATUS_CONNECTING;
					break;
				}
			}
		}

		if (i >= ugd->raw_multi_selected_peer_cnt)
		{
			// All selected peers are touched.
			DBG(LOG_VERBOSE, "Stop Multi Connect...\n");
			wfd_stop_multi_connect(ugd);
		}
	}
	else
	{
		DBG(LOG_VERBOSE, "No selected peers.\n");
		return -1;
	}

	__FUNC_EXIT__;
	return false;
}


void _connect_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    int i = 0;
    int count = 0;
    char popup_text[MAX_POPUP_TEXT_SIZE] = {0};
    DBG(LOG_VERBOSE, "_connect_btn_cb \n");

    for (i = 0; i < ugd->gl_available_peer_cnt ; i++)
    {
    	if (TRUE == ugd->multi_conn_dev_list[i].dev_sel_state)
    	{
			DBG(LOG_VERBOSE, "ugd->peers[i].mac_addr = %s, i = %d\n", ugd->multi_conn_dev_list[i].peer.mac_addr, i);

			memcpy(&ugd->raw_multi_selected_peers[count], &ugd->multi_conn_dev_list[i].peer,sizeof(device_type_s));
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

    if (wfd_start_multi_connect(ugd) != -1)
    {
    	wfd_ug_view_update_peers(ugd);
    }

	elm_naviframe_item_pop(ugd->naviframe);

	//ToDo: Do we need to free multiconn_view_genlist?
	ugd->multiconn_view_genlist = NULL;
	_change_multi_button_title(ugd);

	__FUNC_EXIT__;
	return;
}



static void _wfd_gl_multi_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	int i = 0;
	int index = 0;
	int sel_count = 0;
	bool is_sel = FALSE;
	bool is_selct_all = TRUE;
	Eina_Bool state = 0;
	Evas_Object *chk_box = NULL;
	char msg[MAX_POPUP_TEXT_SIZE] = {0};
	struct ug_data *ugd = (struct ug_data*) data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (NULL == ugd || NULL == item) {
	    DBG(LOG_ERROR, "The param is NULL\n");
	    return;
	}

	elm_genlist_item_selected_set(item, EINA_FALSE);
	index = elm_genlist_item_index_get(item) - 3; /* subtract the previous items */
	DBG(LOG_VERBOSE, "selected index = %d \n", index);
	if (index < 0) {
	    DBG(LOG_ERROR, "The index is invalid.\n");
	    return;
	}

	chk_box = elm_object_item_part_content_get((Elm_Object_Item *)event_info, "elm.icon.1");
	state = elm_check_state_get(chk_box);
	DBG(LOG_VERBOSE, "state = %d \n", state);
	elm_check_state_set(chk_box, !state);

	ugd->multi_conn_dev_list[index].dev_sel_state = !state;
	DBG(LOG_VERBOSE, "ptr->dev_sel_state = %d \n", ugd->multi_conn_dev_list[index].dev_sel_state);
	DBG(LOG_VERBOSE, "ptr->peer.mac_addr = %s \n", ugd->multi_conn_dev_list[index].peer.mac_addr);

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
		wfd_ug_tickernoti_popup(msg);
	}

	__FUNC_EXIT__;
}

static void _wfd_gl_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	int sel_count = 0;
	char msg[MAX_POPUP_TEXT_SIZE] = {0};
	struct ug_data *ugd = (struct ug_data*) data;

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (NULL == ugd || NULL == obj) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return;
	}

	Evas_Object *sel_chkbox = elm_object_item_part_content_get(ugd->mcview_select_all_item, "elm.icon");
	if (sel_chkbox==NULL)
	{
		DBG(LOG_VERBOSE, "select-all chkbox is NULL\n");
		return;
	}
	Eina_Bool state = elm_check_state_get(sel_chkbox);

	if (state==TRUE)
		state = FALSE;
	else
		state = TRUE;
	elm_check_state_set(sel_chkbox, state);

	DBG(LOG_VERBOSE, "state = %d \n", state);

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
		wfd_ug_tickernoti_popup(msg);
	}

    //elm_check_state_set(ugd->mcview_select_all_icon, EINA_FALSE);
}


static char *_wfd_gl_device_label_get(void *data, Evas_Object *obj, const char *part)
{
	DBG(LOG_VERBOSE, "part %s", part);
	device_type_s *peer = (device_type_s*) data;

	if (NULL == peer)
		return NULL;

	if (!strcmp(part, "elm.text")) {
		return strdup(peer->ssid);
	}
	return NULL;
}


static char *__wfd_get_device_icon_path(device_type_s *peer)
{
 	char *img_path = NULL;

        switch(peer->category)
        {
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



static void _wfd_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ugd = (struct ug_data*) data;
	if (NULL == ugd || NULL == obj) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return;
	}

	int i = 0;
	bool is_sel = FALSE;
	Elm_Object_Item *item = NULL;
	Evas_Object *chk_box = NULL;
	Eina_Bool state = elm_check_state_get(obj);
	elm_check_state_set(obj, !state);

	DBG(LOG_VERBOSE, "state = %d \n", state);

#if 0
	/* set the state of all the available devices */
	for (i = 0; i < ugd->gl_available_dev_cnt_at_multiconn_view; i++) {
		is_sel = state;
		ugd->multi_conn_dev_list[i].dev_sel_state = state;
		item = ugd->multi_conn_dev_list[i].peer.gl_item;
		chk_box = elm_object_item_part_content_get(item, "elm.icon.1");
		elm_check_state_set(chk_box, state);
	}

	/* update the connect button */
	wfd_ug_view_refresh_button(ugd->multi_connect_btn, _("IDS_WFD_BUTTON_CONNECT"), is_sel);
#endif
}


static Evas_Object *_wfd_gl_device_icon_get(void *data, Evas_Object *obj, const char *part)
{
	char *img_path = NULL;
	device_type_s *peer = (device_type_s*) data;
	Evas_Object* icon = NULL;

	DBG(LOG_VERBOSE, "Part %s", part);

	if (!strcmp(part, "elm.icon.1")) {
		DBG(LOG_VERBOSE, "Part %s", part);
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


static char *_wfd_gl_select_all_label_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text")) {
		DBG(LOG_VERBOSE, "Adding text %s", part);
		return strdup("Select all");
	}
	return NULL;
}

static Evas_Object *_wfd_gl_select_all_icon_get(void *data, Evas_Object *obj, const char *part)
{
	struct ug_data *ugd = (struct ug_data*) data;
	Evas_Object* icon = NULL;

	if (!strcmp(part, "elm.icon")) {
		DBG(LOG_VERBOSE, "Part %s", part);
		icon = elm_check_add(obj);
		elm_check_state_set(icon, EINA_FALSE);
		evas_object_smart_callback_add(icon, "changed", _wfd_check_clicked_cb, (void *)data);
	}

	return icon;
}




int _wfd_free_multiconnect_device(struct ug_data *ugd)
{
    __FUNC_ENTER__;

    int count = 0;
    int i = 0;

    if (ugd->multiconn_view_genlist == NULL)
    {
    	return 0;
    }

    if (ugd->mcview_title_item != NULL)
    {
        elm_object_item_del(ugd->mcview_title_item);
        ugd->mcview_title_item = NULL;
    }

    if (ugd->mcview_select_all_item != NULL)
    {
        elm_object_item_del(ugd->mcview_select_all_item);
        ugd->mcview_select_all_item = NULL;
    }

    if (ugd->mcview_nodevice_item != NULL)
    {
        elm_object_item_del(ugd->mcview_nodevice_item);
        ugd->mcview_nodevice_item = NULL;
    }

    for(i = 0; i < ugd->gl_available_dev_cnt_at_multiconn_view;  i++)
    {
        if (ugd->multi_conn_dev_list[i].peer.gl_item != NULL)
        {
            elm_object_item_del(ugd->multi_conn_dev_list[i].peer.gl_item);
            ugd->multi_conn_dev_list[i].peer.gl_item = NULL;
        }
    }
    ugd->gl_available_dev_cnt_at_multiconn_view = 0;

    __FUNC_EXIT__;
    return 0;
}

int _wfd_update_multiconnect_device(struct ug_data *ugd)
{
    __FUNC_ENTER__;

    int count = 0;
    device_type_s *device = NULL;
    Evas_Object *genlist = NULL;
    int i = 0;

    genlist = ugd->multiconn_view_genlist;
    if (ugd->multiconn_view_genlist == NULL)
    {
    	return 0;
    }

    _wfd_free_multiconnect_device(ugd);

    count = 0;
    for(i = 0; i < ugd->raw_discovered_peer_cnt;  i++)
    {
    	device = &ugd->raw_discovered_peers[i];
    	if (device->is_connected == FALSE)
    	{
			count++;
    	}
    }
    ugd->gl_available_dev_cnt_at_multiconn_view = count;

    if (ugd->gl_available_dev_cnt_at_multiconn_view == 0)
    {
		DBG(LOG_ERROR, "There are No peers\n");
        ugd->mcview_title_item = elm_genlist_item_append(genlist, &title_itc, ugd, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
    	elm_genlist_item_select_mode_set(ugd->mcview_title_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
        ugd->mcview_nodevice_item = elm_genlist_item_append(genlist, &noitem_itc, (void*)ugd, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
        elm_genlist_item_select_mode_set(ugd->mcview_nodevice_item , ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
    }
    else
    {
        ugd->mcview_title_item = elm_genlist_item_append(genlist, &title_itc, ugd, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
    	elm_genlist_item_select_mode_set(ugd->mcview_title_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
    	ugd->mcview_select_all_item = elm_genlist_item_append(genlist, &select_all_itc, ugd, NULL, ELM_GENLIST_ITEM_NONE, _wfd_gl_sel_cb, ugd);

		count = 0;
		for(i = 0; i < ugd->raw_discovered_peer_cnt;  i++)
		{
			device = &ugd->raw_discovered_peers[i];
			if (device->is_connected == FALSE)
			{
				DBG(LOG_VERBOSE, "%dth peer being added on genlist\n", i);

				if (ugd->multi_conn_dev_list[count].peer.gl_item != NULL)
					elm_object_item_del(ugd->multi_conn_dev_list[count].peer.gl_item);
				ugd->multi_conn_dev_list[count].peer.gl_item = NULL;

				memcpy(&ugd->multi_conn_dev_list[count].peer, device, sizeof(device_type_s));

				ugd->multi_conn_dev_list[count].dev_sel_state = FALSE;
				ugd->multi_conn_dev_list[count].peer.gl_item =
						elm_genlist_item_append(genlist, &device_itc, (void*) &ugd->multi_conn_dev_list[count].peer,
								NULL, ELM_GENLIST_ITEM_NONE, _wfd_gl_multi_sel_cb, ugd);
				count++;
			}
		}
    }

    __FUNC_EXIT__;
    return 0;
}

void _wifid_create_multiconnect_view(struct ug_data *ugd)
{
	__FUNC_ENTER__;

    Evas_Object *back_btn = NULL;
    Evas_Object *control_bar = NULL;
    Evas_Object *genlist = NULL;

    Elm_Object_Item *navi_item = NULL;
	Elm_Object_Item *item = NULL;

    if(ugd == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)");
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

    DBG(LOG_VERBOSE, "_wifid_create_multiconnect_view");
    back_btn = elm_button_add(ugd->naviframe);
    elm_object_style_set(back_btn, "naviframe/back_btn/default");
    evas_object_smart_callback_add(back_btn, "clicked", _multiconnect_view_back_btn_cb, (void*) ugd);
    elm_object_focus_allow_set(back_btn, EINA_FALSE);

    genlist = elm_genlist_add(ugd->naviframe);
    ugd->multiconn_view_genlist = genlist;
    elm_object_style_set(ugd->multiconn_view_genlist, "dialogue");

    ugd->mcview_title_item = NULL;

    _wfd_update_multiconnect_device(ugd);

    evas_object_show(genlist);

    navi_item = elm_naviframe_item_push(ugd->naviframe, _("Multi connect"), back_btn, NULL, genlist, NULL);

    control_bar = elm_toolbar_add(ugd->naviframe);
    elm_toolbar_shrink_mode_set(control_bar, ELM_TOOLBAR_SHRINK_EXPAND);
    evas_object_show(control_bar);

    ugd->multi_scan_btn = elm_toolbar_item_append(control_bar, NULL, _("IDS_WFD_BUTTON_SCAN"), _scan_btn_cb, (void*) ugd);
    item = elm_toolbar_item_append(control_bar, NULL, NULL, NULL, NULL);
    elm_object_item_disabled_set(item, EINA_TRUE);
    ugd->multi_connect_btn = elm_toolbar_item_append(control_bar, NULL, _("IDS_WFD_BUTTON_CONNECT"), _connect_btn_cb, (void*) ugd);

    if (ugd->multi_connect_btn) {
    	wfd_ug_view_refresh_button(ugd->multi_connect_btn, _("IDS_WFD_BUTTON_CONNECT"), FALSE);
    }

    item = elm_toolbar_item_append(control_bar, NULL, NULL, NULL, NULL);
    elm_object_item_disabled_set(item, EINA_TRUE);
    elm_object_item_part_content_set(navi_item, "controlbar", control_bar);

    __FUNC_EXIT__;
}
