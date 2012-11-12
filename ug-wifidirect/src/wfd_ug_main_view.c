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

void _wfd_onoff_btn_cb(void *data, Evas_Object *obj, void *event_info);


void _back_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    if(!ugd)
    {
        DBG(LOG_ERROR, "The param is NULL\n");
        return;
    }

    wfd_ug_view_free_peers(ugd);
#if 0
    bundle *b;
    b = bundle_create();
    if(!b)
    {
        DBG(LOG_ERROR, "Failed to create bundle");
        return;
    }

    wfd_refresh_wifi_direct_state(ugd);
    if (ugd->wfd_status > WIFI_DIRECT_STATE_CONNECTING)
        bundle_add(b, "Connection", "TRUE");
    else
        bundle_add(b, "Connection", "FALSE");

    ug_send_result(ugd->ug, b);

    bundle_free(b);
#else
	int ret = -1;
	service_h service = NULL;
	ret = service_create(&service);
	if(ret)
	{
		DBG(LOG_ERROR, "Failed to create service");
		return;
	}

	wfd_refresh_wifi_direct_state(ugd);
	if (ugd->wfd_status > WIFI_DIRECT_STATE_CONNECTING)
		service_add_extra_data(service, "Connection", "TRUE");
	else
		service_add_extra_data(service, "Connection", "FALSE");

	ug_send_result(ugd->ug, service);
	service_destroy(service);
#endif
    ug_destroy_me(ugd->ug);

    __FUNC_EXIT__;
    return;
}

void _scan_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data*) data;
    Elm_Object_Item *btn = event_info;
    char *btn_text = NULL;

    if (NULL == ugd || NULL == btn) {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return;
    }

    btn_text = elm_object_item_text_get(btn);
    if (0 == strcmp(btn_text, _("IDS_WFD_BUTTON_SCAN"))) {
	wfd_refresh_wifi_direct_state(ugd);
	DBG(LOG_VERBOSE, "Start discovery again, status: %d\n", ugd->wfd_status);

	/* if connected, show the popup*/
	if (ugd->wfd_status >= WIFI_DIRECT_STATE_CONNECTED) {
		wfd_ug_act_popup(ugd, IDS_WFD_POP_SCAN_AGAIN, POP_TYPE_SCAN_AGAIN);
	} else if (WIFI_DIRECT_STATE_ACTIVATED == ugd->wfd_status) {
		wfd_client_start_discovery(ugd);
	} else if (WIFI_DIRECT_STATE_DEACTIVATED == ugd->wfd_status) {
		_wfd_onoff_btn_cb(ugd, NULL, NULL);
		__FUNC_EXIT__;
		return;
	}

	if (ugd->scan_btn) {
		wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_STOPSCAN"), TRUE);
	}

	if (ugd->multi_scan_btn) {
		wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_STOPSCAN"), TRUE);
	}
    } else if (0 == strcmp(btn_text, _("IDS_WFD_BUTTON_STOPSCAN"))) {
    	DBG(LOG_VERBOSE, "Stop discoverying.\n");
	ugd->wfd_status = WIFI_DIRECT_STATE_ACTIVATED;
	wfd_refresh_wifi_direct_state(ugd);
	ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
	wfd_ug_view_refresh_glitem(ugd->head);
	wifi_direct_cancel_discovery();
    }

    __FUNC_EXIT__;
    return;
}

Eina_Bool _is_wifi_on()
{
    __FUNC_ENTER__;
    int wifi_state;

    vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);

    if (wifi_state >= VCONFKEY_WIFI_CONNECTED)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    __FUNC_EXIT__;
}

void _wfd_onoff_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    wfd_refresh_wifi_direct_state(ugd);

    if(!ugd->wfd_onoff)
    {
        if(ugd->wfd_status < 0)
        {
            DBG(LOG_VERBOSE, "bad wfd status\n");
            wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_ACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

            ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
            wfd_ug_view_refresh_glitem(ugd->head);
            return;
        }
        DBG(LOG_VERBOSE, "wifi-direct switch on\n");
        ugd->head_text_mode = HEAD_TEXT_TYPE_ACTIVATING;
        wfd_client_switch_on(ugd);
    }
    else
    {
        if(ugd->wfd_status < 0)
        {
            DBG(LOG_VERBOSE, "bad wfd status\n");
            wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_DEACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

            ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
            wfd_ug_view_refresh_glitem(ugd->head);
            return;
        }
        DBG(LOG_VERBOSE, "wifi-direct switch off\n");
        ugd->head_text_mode = HEAD_TEXT_TYPE_DEACTIVATING;
        wfd_client_switch_off(ugd);
    }

    wfd_ug_view_refresh_glitem(ugd->head);

    if (ugd->scan_btn) {
	wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
    }

    if (ugd->multi_scan_btn) {
	wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
    }


    __FUNC_EXIT__;
}


static void _gl_header_sel(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    Elm_Object_Item *item = (Elm_Object_Item *)event_info;

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return;
    }

    if(item != NULL)
        elm_genlist_item_selected_set(item, EINA_FALSE);

    _wfd_onoff_btn_cb(ugd, NULL, NULL);

    __FUNC_EXIT__;
}

#if 0
static Evas_Object *_gl_noitem_icon_get(void *data, Evas_Object * obj,
					  const char *part)
{
    __FUNC_ENTER__;
    Evas_Object *nocontent;

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

#if 1   // use nocontent
    nocontent = elm_layout_add(obj);
    if(nocontent == NULL)
    {
        DBG(LOG_ERROR, "Failed to add nocontent");
        return NULL;
    }
    elm_layout_theme_set(nocontent, "layout", "nocontents", "unnamed");
    elm_object_part_text_set(nocontent, "elm.text", _("IDS_WFD_BODY_NO_DEVICES"));
    evas_object_size_hint_min_set(nocontent, 400, 200);
    evas_object_size_hint_max_set(nocontent, 400, 200);
    evas_object_resize(nocontent, 400, 200);

    __FUNC_EXIT__;

    return nocontent;
#else   // use image
    Evas_Object *icon;
    icon = elm_icon_add(obj);
    elm_icon_file_set(icon, "/usr/ug/res/images/ug-wifi-direct/A09_NoDevice.png", NULL);
    evas_object_size_hint_min_set(icon, 400, 200);
    evas_object_size_hint_max_set(icon, 400, 200);
    evas_object_resize(icon, 400, 200);

    return icon;
#endif
}
#endif

static void _gl_peer_sel(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;
    assertm_if(NULL == obj, "NULL!!");
    assertm_if(NULL == data, "NULL!!");
    device_type_s *peer = (device_type_s*) data;
    Elm_Object_Item *item = (Elm_Object_Item *)event_info;
    struct ug_data* ugd = wfd_get_ug_data();
    int res;

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return;
    }

    if(item != NULL)
        elm_genlist_item_selected_set(item, EINA_FALSE);

    if(peer->conn_status == PEER_CONN_STATUS_DISCONNECTED)
    {
#if 0   // for new connection during link_status is CONNECTING
    	wfd_refresh_wifi_direct_state(ugd);
        if(ugd->wfd_status == WFD_LINK_STATUS_CONNECTING)
        {
            res = wfd_client_disconnect(NULL);
            if(res != 0)
            {
                DBG(LOG_ERROR, "Failed to send disconnection request. [%d]\n", res);
                return;
            }
        }
#endif
        DBG(LOG_VERBOSE, "Connect with peer [%s]\n", peer->mac_addr);
        res = wfd_client_connect((const char*) peer->mac_addr);
        if(res != 0)
        {
            DBG(LOG_ERROR, "Failed to send connection request. [%d]\n", res);
            return;
        }
        peer->conn_status = PEER_CONN_STATUS_CONNECTING;
    }
    else    // PEER_CONN_STATUS_CONNECTED or PEER_CONN_STATUS_CONNECTING)
    {
        res = wfd_client_disconnect((const char*) peer->mac_addr);
        if(res != 0)
        {
            DBG(LOG_ERROR, "Failed to send disconnection request. [%d]\n", res);
            return;
        }
        peer->conn_status = PEER_CONN_STATUS_DISCONNECTED;
    }

    wfd_ug_view_refresh_glitem(peer->gl_item);
    wfd_ug_view_refresh_glitem(ugd->multi_button_item);

    __FUNC_EXIT__;
    return;
}

static void _gl_busy_peer_sel(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data*) data;

    elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

    DBG(LOG_VERBOSE, "Busy device is clicked");

    wfd_ug_warn_popup(ugd, IDS_WFD_POP_WARN_BUSY_DEVICE, POP_TYPE_BUSY_DEVICE_POPUP);

    __FUNC_EXIT__;
}

static void _gl_about_wifi_sel(void *data, Evas_Object *obj, void *event_info)
{
    struct ug_data *ugd = (struct ug_data*) data;

    DBG(LOG_VERBOSE, "About wifi clicked");

    _wifid_create_about_view(ugd);
    elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
}

void _wifid_create_multibutton_cb(void *data, Evas_Object * obj, void *event_info)
{
    struct ug_data *ugd = (struct ug_data*) data;
    const char *text_lbl = NULL;

    text_lbl = elm_object_text_get(ugd->multi_btn);
    DBG(LOG_VERBOSE, "text_lbl = %s", text_lbl);

    if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS)
    {
    	ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_NONE;
		if (0 == strcmp(_("IDS_WFD_BUTTON_CANCEL"), text_lbl))
		{
			wfd_ug_act_popup(ugd, _("IDS_WFD_POP_CANCEL_CONNECT"), POP_TYPE_DISCONNECT_ALL);
		}
		else
		{
			DBG(LOG_VERBOSE, "Invalid Case\n");
		}
    }
    else
    {
		if (0 == strcmp(_("IDS_WFD_BUTTON_MULTI"), text_lbl))
		{
			_wifid_create_multiconnect_view(ugd);
		}
		else if (0 == strcmp(_("IDS_WFD_BUTTON_CANCEL"), text_lbl))
		{
			wfd_ug_act_popup(ugd, _("IDS_WFD_POP_CANCEL_CONNECT"), POP_TYPE_DISCONNECT_ALL);
		}
		else if (0 == strcmp(_("IDS_WFD_BUTTON_DISCONNECT_ALL"), text_lbl))
		{
			wfd_ug_act_popup(ugd, _("IDS_WFD_POP_DISCONNECT"), POP_TYPE_DISCONNECT_ALL);
		}
		else if (0 == strcmp(_("IDS_WFD_BUTTON_DISCONNECT"), text_lbl))
		{
			wfd_ug_act_popup(ugd, _("IDS_WFD_POP_DISCONNECT"), POP_TYPE_DISCONNECT);
		}
		else
		{
			DBG(LOG_VERBOSE, "Invalid Case\n");
		}
    }
}


int _change_multi_button_title(void *data)
{
    struct ug_data *ugd = (struct ug_data*) data;

    if(ugd->multi_button_item == NULL) /*Needs to be check as the peer count is not getting updated*/
    {
    	return -1;
    }

	wfd_refresh_wifi_direct_state(ugd);
	if (ugd->wfd_status == WFD_LINK_STATUS_CONNECTING)
	{
		//if (conn_prog_count > 0)
		elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_CANCEL"));
	}
	else if (ugd->wfd_status > WFD_LINK_STATUS_CONNECTING)
	{
		if (ugd->gl_connected_peer_cnt > 1)
			elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_DISCONNECT_ALL"));
		else
			elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_DISCONNECT"));
	}
	else
	{
		elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_MULTI"));
	}

    evas_object_show(ugd->multi_btn);

    return 0;
}


void wfd_ug_view_refresh_glitem(void *obj)
{
    __FUNC_ENTER__;
    elm_genlist_item_update(obj);
    __FUNC_EXIT__;
}

void wfd_ug_view_refresh_button(void *obj, const char *text, int enable)
{
	__FUNC_ENTER__;

	if (NULL == obj || NULL == text) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	DBG(LOG_VERBOSE, "Set the attributes of button: text[%s], enabled[%d]\n", text, enable);
	elm_object_item_text_set(obj, text);
	elm_object_item_disabled_set(obj, !enable);

	__FUNC_EXIT__;
}

static bool __wfd_is_device_connected_with_me(struct ug_data *ugd, device_type_s *dev)
{
	int i;

	for(i=0; i<ugd->raw_connected_peer_cnt; i++)
	{
		if (strncmp(ugd->raw_connected_peers[i].mac_addr, dev->mac_addr, strlen(ugd->raw_connected_peers[i].mac_addr)) == 0)
			return TRUE;
	}
	return FALSE;
}

static bool __wfd_is_device_busy(struct ug_data *ugd, device_type_s *dev)
{
	if (__wfd_is_device_connected_with_me(ugd, dev) == TRUE)
		return FALSE;

	if (ugd->I_am_group_owner == TRUE)
	{
		if (dev->is_connected || dev->is_group_owner)
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		if (dev->is_connected == TRUE && dev->is_group_owner == TRUE)
			return FALSE;
		if (dev->is_connected == TRUE && dev->is_group_owner == FALSE)
			return TRUE;
		if (dev->is_connected == FALSE)
			return FALSE;
	}

	return FALSE;
}

static bool __wfd_is_any_device_available(struct ug_data *ugd, int* no_of_available_dev)
{
    int i =0 ;
    for (i = 0; i < ugd->raw_discovered_peer_cnt; i++)
    {
        if (!__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]) &&
		ugd->raw_discovered_peers[i].conn_status == PEER_CONN_STATUS_DISCONNECTED)
		(*no_of_available_dev)++;
    }
    return TRUE;
}

static bool __wfd_is_any_device_busy(struct ug_data *ugd, int* no_of_busy_dev)
{
    int i =0 ;
    for (i = 0; i < ugd->raw_discovered_peer_cnt; i++)
    {
        if (__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]))
            (*no_of_busy_dev)++;
    }
    return TRUE;
}

static bool __wfd_is_any_device_connect_failed(struct ug_data *ugd, int* no_of_connect_failed_dev)
{
    int i =0 ;
    for (i = 0; i < ugd->raw_discovered_peer_cnt; i++)
    {
        if (!__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]) &&
		ugd->raw_discovered_peers[i].conn_status == PEER_CONN_STATUS_FAILED_TO_CONNECT)
		(*no_of_connect_failed_dev)++;

    }
    return TRUE;
}

static Evas_Object *_create_basic_genlist(void *data)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data*) data;
	Evas_Object *genlist;
	Elm_Object_Item *separator_item;


	genlist = elm_genlist_add(ugd->naviframe);


	separator_item = elm_genlist_item_append(genlist, &sep_itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(separator_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	ugd->head = elm_genlist_item_append(genlist, &head_itc, ugd, NULL, ELM_GENLIST_ITEM_NONE, _gl_header_sel, (void*) ugd);

	//elm_genlist_item_select_mode_set(ugd->head, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	__FUNC_EXIT__;

	return genlist;
}

static Evas_Object *_create_about_genlist(void *data)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data*) data;

    ugd->about_wfd_item = elm_genlist_item_append(ugd->genlist, &name_itc, ugd, NULL, ELM_GENLIST_ITEM_NONE, _gl_about_wifi_sel, (void*) ugd);

    /* add end separator */
    ugd->about_wfdsp_sep_end_item = elm_genlist_item_append(ugd->genlist, &sep_itc_end, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
    elm_genlist_item_select_mode_set(ugd->about_wfdsp_sep_end_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    __FUNC_EXIT__;

    return ugd->genlist;
}

static Evas_Object *_create_no_device_genlist(void *data)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data*) data;

    ugd->nodevice_title_item = elm_genlist_item_append(ugd->genlist, &title_itc, (void*)ugd, NULL,
									ELM_GENLIST_ITEM_NONE, NULL, NULL);

    ugd->nodevice_item = elm_genlist_item_append(ugd->genlist, &noitem_itc, (void*)ugd, NULL,
									ELM_GENLIST_ITEM_NONE, NULL, NULL);

	ugd->nodevice_sep_low_item = elm_genlist_item_append(ugd->genlist, &sep_itc, NULL, NULL,
											ELM_GENLIST_ITEM_NONE, NULL, NULL);

    elm_genlist_item_select_mode_set(ugd->nodevice_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    __FUNC_EXIT__;
    return ugd->genlist;
}

int _create_multi_button_genlist(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

 	ugd->multi_button_sep_high_item = elm_genlist_item_append(ugd->genlist, &sep_itc, NULL, NULL,
											ELM_GENLIST_ITEM_NONE, NULL, NULL);

	/* if not connected and number of devices is less than 2, don't show the button */
	if (ugd->raw_multi_selected_peer_cnt > 1 ||
		ugd->gl_available_peer_cnt > 1 ||
		ugd->gl_connected_peer_cnt > 0) {
		ugd->multi_button_item = elm_genlist_item_append(ugd->genlist, &button_itc, ugd, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
		ugd->multi_button_sep_low_item = elm_genlist_item_append(ugd->genlist, &sep_itc, NULL, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}

    evas_object_show(ugd->multi_btn);
    __FUNC_EXIT__;
    return 0;
}


int _create_busy_dev_list(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

	ugd->busy_wfd_item = elm_genlist_item_append(ugd->genlist, &title_busy_itc, (void*)ugd, NULL,
									ELM_GENLIST_ITEM_NONE, NULL, NULL);

    elm_genlist_item_select_mode_set(ugd->busy_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);


    __FUNC_EXIT__;
    return 0;
}

static int _create_available_dev_genlist(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    ugd->avlbl_wfd_item = elm_genlist_item_append(ugd->genlist, &title_itc, (void*)ugd, NULL,
									ELM_GENLIST_ITEM_NONE, NULL, NULL);

    // elm_genlist_item_select_mode_set(ugd->avlbl_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    __FUNC_EXIT__;
    return 0;
}

static int _create_multi_connect_dev_genlist(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    ugd->multi_connect_wfd_item = elm_genlist_item_append(ugd->genlist, &title_multi_connect_itc, (void*)ugd, NULL,
									ELM_GENLIST_ITEM_NONE, NULL, NULL);

    // elm_genlist_item_select_mode_set(ugd->avlbl_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    __FUNC_EXIT__;
    return 0;
}

int _create_connected_dev_genlist(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    ugd->conn_wfd_item = elm_genlist_item_append(ugd->genlist, &title_conn_itc, (void*)ugd, NULL,
									ELM_GENLIST_ITEM_NONE, NULL, NULL);

    elm_genlist_item_select_mode_set(ugd->conn_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
    __FUNC_EXIT__;
    return 0;
}

int _create_connected_failed_dev_genlist(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    ugd->conn_failed_wfd_item = elm_genlist_item_append(ugd->genlist, &title_conn_failed_itc, (void*)ugd, NULL,
									ELM_GENLIST_ITEM_NONE, NULL, NULL);

    elm_genlist_item_select_mode_set(ugd->conn_failed_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
    __FUNC_EXIT__;
    return 0;
}

static Eina_Bool _connect_failed_peers_display_cb(void *user_data)
{
	int interval = 0;
	struct ug_data *ugd = (struct ug_data*) user_data;

	if (NULL == ugd) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	/* check the timeout, if not timeout, keep the cb */
	interval = time(NULL) - ugd->last_display_time;
	if (interval < MAX_DISPLAY_TIME_OUT) {
		return ECORE_CALLBACK_RENEW;
	}

	/* re-discovery */
	wfd_client_start_discovery(ugd);

	/* get peers and update the view */
	wfd_ug_get_discovered_peers(ugd);
	wfd_ug_get_connected_peers(ugd);
	wfd_ug_view_update_peers(ugd);

	return ECORE_CALLBACK_CANCEL;
}

void wfd_ug_view_free_peers(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    int i;

    for(i = 0; i < ugd->gl_connected_peer_cnt; i++)
    {
        DBG(LOG_VERBOSE, "%dth connected peer = %x is deleted\n", i, ugd->gl_connected_peers[i]);
        if (ugd->gl_connected_peers[i].gl_item != NULL)
        {
            elm_object_item_del(ugd->gl_connected_peers[i].gl_item);
            ugd->gl_connected_peers[i].gl_item = NULL;
            DBG(LOG_VERBOSE, "Deleted item\n");
        }
    }
	ugd->gl_connected_peer_cnt = 0;

    for(i = 0; i < ugd->gl_connected_failed_peer_cnt; i++)
    {
        DBG(LOG_VERBOSE, "%dth connected failed peer = %x is deleted\n", i, ugd->gl_connected_failed_peers[i]);
	if (ugd->gl_connected_failed_peers[i].gl_item != NULL)
	{
	    elm_object_item_del(ugd->gl_connected_failed_peers[i].gl_item);
	    ugd->gl_connected_failed_peers[i].gl_item = NULL;
	    DBG(LOG_VERBOSE, "Deleted item\n");
	}
    }

    ugd->gl_connected_failed_peer_cnt = 0;

    for(i = 0; i < ugd->gl_available_peer_cnt; i++)
    {
        DBG(LOG_VERBOSE, "%dth discovered peer = %x is deleted\n", i, ugd->gl_available_peers[i]);
        if (ugd->gl_available_peers[i].gl_item != NULL)
        {
            elm_object_item_del(ugd->gl_available_peers[i].gl_item);
            ugd->gl_available_peers[i].gl_item = NULL;
            DBG(LOG_VERBOSE, "Deleted item\n");
        }
    }
	ugd->gl_available_peer_cnt = 0;

    for(i = 0; i < ugd->gl_busy_peer_cnt; i++)
    {
        DBG(LOG_VERBOSE, "%dth busy peer = %x is deleted\n", i, ugd->gl_busy_peers[i]);
        if (ugd->gl_busy_peers[i].gl_item != NULL)
        {
            elm_object_item_del(ugd->gl_busy_peers[i].gl_item);
            ugd->gl_busy_peers[i].gl_item = NULL;
            DBG(LOG_VERBOSE, "Deleted item\n");
        }
    }
	ugd->gl_busy_peer_cnt = 0;

    for(i = 0; i < ugd->gl_multi_connect_peer_cnt; i++)
    {
        DBG(LOG_VERBOSE, "%dth busy peer = %x is deleted\n", i, ugd->gl_multi_connect_peers[i]);
        if (ugd->gl_multi_connect_peers[i].gl_item != NULL)
        {
            elm_object_item_del(ugd->gl_multi_connect_peers[i].gl_item);
            ugd->gl_multi_connect_peers[i].gl_item = NULL;
            DBG(LOG_VERBOSE, "Deleted item\n");
        }
    }
	ugd->gl_multi_connect_peer_cnt = 0;

    if(ugd->nodevice_title_item != NULL)
    {
        elm_object_item_del(ugd->nodevice_title_item);
        ugd->nodevice_title_item = NULL;
    }
    if(ugd->nodevice_item != NULL)
    {
        elm_object_item_del(ugd->nodevice_item);
        ugd->nodevice_item = NULL;
    }
    if(ugd->nodevice_sep_low_item != NULL)
    {
        elm_object_item_del(ugd->nodevice_sep_low_item);
        ugd->nodevice_sep_low_item = NULL;
    }
    if(ugd->about_wfd_item != NULL)
    {
        elm_object_item_del(ugd->about_wfd_item);
        ugd->about_wfd_item = NULL;
    }

    if(ugd->conn_wfd_item != NULL)
    {
        elm_object_item_del(ugd->conn_wfd_item);
        ugd->conn_wfd_item = NULL;
    }
    if(ugd->conn_failed_wfd_item != NULL)
    {
        elm_object_item_del(ugd->conn_failed_wfd_item);
        ugd->conn_failed_wfd_item = NULL;
    }
    if(ugd->conn_failed_wfd_sep_item != NULL)
    {
        elm_object_item_del(ugd->conn_failed_wfd_sep_item);
        ugd->conn_failed_wfd_sep_item = NULL;
    }
    if(ugd->display_timer != NULL)
    {
        elm_object_item_del(ugd->display_timer);
        ugd->display_timer = NULL;
    }
    if(ugd->multi_connect_wfd_item != NULL)
    {
        elm_object_item_del(ugd->multi_connect_wfd_item);
        ugd->multi_connect_wfd_item = NULL;
    }
    if(ugd->avlbl_wfd_item != NULL)
    {
        elm_object_item_del(ugd->avlbl_wfd_item);
        ugd->avlbl_wfd_item = NULL;
    }
    if(ugd->busy_wfd_item != NULL)
    {
        elm_object_item_del(ugd->busy_wfd_item);
        ugd->busy_wfd_item = NULL;
    }
    if(ugd->busy_wfd_sep_item != NULL)
    {
        elm_object_item_del(ugd->busy_wfd_sep_item);
        ugd->busy_wfd_sep_item = NULL;
    }

    if(ugd->multi_button_item != NULL)
    {
        elm_object_item_del(ugd->multi_button_item);
        ugd->multi_button_item = NULL;
    }
    if(ugd->multi_button_sep_high_item != NULL)
    {
        elm_object_item_del(ugd->multi_button_sep_high_item);
        ugd->multi_button_sep_high_item = NULL;
    }
    if(ugd->multi_button_sep_low_item != NULL)
    {
        elm_object_item_del(ugd->multi_button_sep_low_item);
        ugd->multi_button_sep_low_item = NULL;
    }
    if(ugd->about_wfdsp_sep_end_item != NULL)
    {
        elm_object_item_del(ugd->about_wfdsp_sep_end_item);
        ugd->about_wfdsp_sep_end_item = NULL;
    }

    __FUNC_EXIT__;
}


void wfd_ug_view_update_peers(void *data)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data*) data;
    int no_of_busy_dev = 0;
    int no_of_available_dev = 0;
    int no_of_conn_dev = 0;
    int no_of_conn_failed_dev = 0;
    int i = 0 ;
    int res = 0;
    bool is_group_owner = FALSE;
    int count = 0;

    wfd_ug_view_free_peers(ugd);

    if(ugd->wfd_status == WFD_LINK_STATUS_DEACTIVATED)
    {
        DBG(LOG_VERBOSE, "Device is deactivated, no need to update UI.");
        // Add seperator...
     	ugd->multi_button_sep_high_item = elm_genlist_item_append(ugd->genlist, &sep_itc, NULL, NULL,
    											ELM_GENLIST_ITEM_NONE, NULL, NULL);
        _create_about_genlist(ugd);
        return;
    }

    res = wifi_direct_is_group_owner(&is_group_owner);
    if (res != WIFI_DIRECT_ERROR_NONE)
    {
        DBG(LOG_VERBOSE, "Fail to get group_owner_state. ret=[%d]", res);
        ugd->I_am_group_owner = FALSE;
        // continue...
    }
    else
    {
    	ugd->I_am_group_owner = is_group_owner;
    }

    __wfd_is_any_device_busy(ugd, &no_of_busy_dev);
    __wfd_is_any_device_available(ugd, &no_of_available_dev);
    __wfd_is_any_device_connect_failed(ugd, &no_of_conn_failed_dev);
    no_of_conn_dev = ugd->raw_connected_peer_cnt;

    ugd->gl_available_peer_cnt = no_of_available_dev;
    ugd->gl_connected_peer_cnt = no_of_conn_dev;
    ugd->gl_connected_failed_peer_cnt = no_of_conn_failed_dev;
    ugd->gl_busy_peer_cnt = no_of_busy_dev;

    DBG(LOG_VERBOSE, "conn_dev=[%d], conn_failed_dev=[%d], avail_dev=[%d], busy_dev=[%d], GO=[%d]\n",
    			no_of_conn_dev, no_of_conn_failed_dev, no_of_available_dev, no_of_busy_dev, is_group_owner);

    if (no_of_conn_dev == 0 && no_of_conn_failed_dev == 0 &&
    	no_of_available_dev == 0 && no_of_busy_dev == 0)
    {
        DBG(LOG_ERROR, "There are No peers\n");
        _create_no_device_genlist(ugd);
        _create_about_genlist(ugd);
        return;
    }

    if (no_of_conn_dev > 0)
    {
        if (!ugd->conn_wfd_item)
        	_create_connected_dev_genlist(ugd);

        count = 0;
        for(i = 0;  i < ugd->raw_connected_peer_cnt; i++)
        {
        	if (ugd->gl_connected_peers[count].gl_item)
        		elm_object_item_del(ugd->gl_connected_peers[count].gl_item);

        	memcpy(&ugd->gl_connected_peers[count], &ugd->raw_connected_peers[i], sizeof(device_type_s));

            ugd->gl_connected_peers[count].gl_item =
            		elm_genlist_item_append(ugd->genlist, &peer_conn_itc, (void*) &(ugd->gl_connected_peers[i]), NULL,
								ELM_GENLIST_ITEM_NONE, NULL, NULL);
            elm_genlist_item_select_mode_set(ugd->gl_connected_peers[count].gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
            count++;
        }
    }

    if (ugd->multi_connect_mode != WFD_MULTI_CONNECT_MODE_NONE)
    {
    	if (ugd->raw_multi_selected_peer_cnt > 0)
    	{
			if (ugd->avlbl_wfd_item == NULL)
				_create_multi_connect_dev_genlist(ugd);

			count = 0;
			for (i = 0; i < ugd->raw_multi_selected_peer_cnt; i++)
			{
				if (ugd->raw_multi_selected_peers[i].conn_status != PEER_CONN_STATUS_CONNECTED)
				{
					if (ugd->gl_multi_connect_peers[count].gl_item)
						elm_object_item_del(ugd->gl_multi_connect_peers[count].gl_item);

					memcpy(&ugd->gl_multi_connect_peers[count], &ugd->raw_multi_selected_peers[i], sizeof(device_type_s));

					ugd->gl_multi_connect_peers[count].gl_item =
							elm_genlist_item_append(ugd->genlist, &peer_itc, (void*) &(ugd->gl_multi_connect_peers[count]), NULL,
										ELM_GENLIST_ITEM_NONE, NULL, NULL);
					count++;
				}
				else
				{
					// device is connected..
					// skip it...
				}
			}
			ugd->gl_multi_connect_peer_cnt = count;
    	}

	_create_multi_button_genlist(ugd);
    }
    else
    {
    	// Note that
    	// If GC, no display available peers
    	// Otherwise, display available peers
#if 0
		if (no_of_available_dev > 0 && (no_of_conn_dev == 0 || is_group_owner==TRUE))
#else
		// display available peers
		if (no_of_available_dev > 0)
#endif
		{
			if (ugd->avlbl_wfd_item == NULL)
				_create_available_dev_genlist(ugd);

			count = 0;
			for (i = 0; i < ugd->raw_discovered_peer_cnt; i++)
			{
				if (!__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]) &&
					ugd->raw_discovered_peers[i].conn_status == PEER_CONN_STATUS_DISCONNECTED)
				{
					if (ugd->gl_available_peers[count].gl_item)
						elm_object_item_del(ugd->gl_available_peers[count].gl_item);

					memcpy(&ugd->gl_available_peers[count], &ugd->raw_discovered_peers[i], sizeof(device_type_s));

					ugd->gl_available_peers[count].gl_item =
							elm_genlist_item_append(ugd->genlist, &peer_itc, (void*) &(ugd->gl_available_peers[count]), NULL,
										ELM_GENLIST_ITEM_NONE, _gl_peer_sel, (void*) &(ugd->gl_available_peers[count]));
					count++;
				}
				else
				{
					// device is busy or connected..
					// skip it...
				}
			}
		}

		_create_multi_button_genlist(ugd);

		// If connected, not display busy device...
		if (no_of_conn_dev == 0 && no_of_busy_dev > 0)
		{
			if (ugd->busy_wfd_item == NULL)
				_create_busy_dev_list(ugd);

			count = 0;
			for (i = 0; i < ugd->raw_discovered_peer_cnt; i++)
			{
				if (__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]) == TRUE)
				{
					if (ugd->gl_busy_peers[count].gl_item)
						elm_object_item_del(ugd->gl_busy_peers[count].gl_item);

					memcpy(&ugd->gl_busy_peers[count], &ugd->raw_discovered_peers[i], sizeof(device_type_s));

					ugd->gl_busy_peers[count].gl_item =
							elm_genlist_item_append(ugd->genlist, &peer_busy_itc, (void*) &(ugd->gl_busy_peers[count]), NULL,
										ELM_GENLIST_ITEM_NONE, _gl_busy_peer_sel, ugd);
					//elm_genlist_item_select_mode_set(ugd->gl_busy_peers[count].gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
					count++;
				}
				else
				{
					// device is available or connected..
					// skip it...
				}
			}
			ugd->busy_wfd_sep_item = elm_genlist_item_append(ugd->genlist, &sep_itc, NULL, NULL,
					ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}

		/* display connect failed peers */
		if (no_of_conn_failed_dev > 0)
		{
			if (!ugd->conn_failed_wfd_item)
				_create_connected_failed_dev_genlist(ugd);

			/* add timer for disappearing failed peers after N secs */
			if (NULL == ugd->display_timer) {
				ugd->last_display_time = time(NULL);
				ugd->display_timer = ecore_timer_add(5.0, (Ecore_Task_Cb)_connect_failed_peers_display_cb, ugd);
			}

			count = 0;
			for (i = 0; i < ugd->raw_discovered_peer_cnt; i++)
			{
				if (!__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]) &&
					ugd->raw_discovered_peers[i].conn_status == PEER_CONN_STATUS_FAILED_TO_CONNECT)
				{
					if (ugd->gl_connected_failed_peers[count].gl_item)
						elm_object_item_del(ugd->gl_connected_failed_peers[count].gl_item);

					memcpy(&ugd->gl_connected_failed_peers[count], &ugd->raw_discovered_peers[i], sizeof(device_type_s));

					ugd->gl_connected_failed_peers[count].gl_item =
							elm_genlist_item_append(ugd->genlist, &peer_conn_failed_itc, (void*) &(ugd->gl_connected_failed_peers[count]), NULL,
										ELM_GENLIST_ITEM_NONE, NULL, ugd);
					elm_genlist_item_select_mode_set(ugd->gl_connected_failed_peers[count].gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
					count++;
				}
			}

			ugd->conn_failed_wfd_sep_item = elm_genlist_item_append(ugd->genlist, &sep_itc, NULL, NULL,
					ELM_GENLIST_ITEM_NONE, NULL, NULL);

		}
    }
    _create_about_genlist(ugd);

    __FUNC_EXIT__;
}


void create_wfd_ug_view(void *data)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data*) data;
    Evas_Object *back_btn = NULL;
    Elm_Object_Item *navi_item = NULL;
    Evas_Object *control_bar = NULL;
    Elm_Object_Item *item = NULL;

    if(ugd == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)");
        return;
    }

    ugd->naviframe = elm_naviframe_add(ugd->base);
    elm_object_part_content_set(ugd->base, "elm.swallow.content", ugd->naviframe);
    evas_object_show(ugd->naviframe);

    back_btn = elm_button_add(ugd->naviframe);
    elm_object_style_set(back_btn, "naviframe/back_btn/default");
    evas_object_smart_callback_add(back_btn, "clicked", _back_btn_cb, (void*) ugd);
    elm_object_focus_allow_set(back_btn, EINA_FALSE);


    ugd->genlist = _create_basic_genlist(ugd);
    if(ugd->genlist == NULL)
    {
        DBG(LOG_ERROR, "Failed to create basic genlist");
        return;
    }
    elm_object_style_set (ugd->genlist, "dialogue");
    evas_object_show(ugd->genlist);
    wfd_refresh_wifi_direct_state(ugd);
    if (ugd->wfd_status > WIFI_DIRECT_STATE_ACTIVATING)
    	ugd->wfd_onoff = TRUE;

    navi_item = elm_naviframe_item_push(ugd->naviframe, _("IDS_WFD_HEADER_WIFI_DIRECT"), back_btn, NULL, ugd->genlist, NULL); // dgettext("sys_string", "IDS_COM_OPT1_WI_FI_DIRECT")

    control_bar = elm_toolbar_add(ugd->naviframe);
    elm_toolbar_shrink_mode_set(control_bar, ELM_TOOLBAR_SHRINK_EXPAND);
    evas_object_show(control_bar);

    ugd->scan_btn = elm_toolbar_item_append(control_bar, NULL, _("IDS_WFD_BUTTON_SCAN"), _scan_btn_cb, (void*) ugd);
    item = elm_toolbar_item_append(control_bar, NULL, NULL, NULL, NULL);
    elm_object_item_disabled_set(item, EINA_TRUE);

    elm_object_item_disabled_set(ugd->scan_btn, !ugd->wfd_onoff);

    elm_object_item_part_content_set(navi_item, "controlbar", control_bar);

    __FUNC_EXIT__;
}
