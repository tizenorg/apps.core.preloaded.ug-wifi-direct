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

/**
 *	This function let the ug call it when click 'back' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _back_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	if (!ugd) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	wfd_ug_view_free_peers(ugd);

	int ret = -1;
	service_h service = NULL;
	ret = service_create(&service);
	if (ret) {
		DBG(LOG_ERROR, "Failed to create service");
		return;
	}

	wfd_refresh_wifi_direct_state(ugd);
	if (ugd->wfd_status > WIFI_DIRECT_STATE_CONNECTING) {
		service_add_extra_data(service, "Connection", "TRUE");
	} else {
		service_add_extra_data(service, "Connection", "FALSE");
	}

	ug_send_result(ugd->ug, service);
	service_destroy(service);
	ug_destroy_me(ugd->ug);

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
	int ret = -1;
	char *btn_text = NULL;

	if (NULL == ugd) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	btn_text = (char *)elm_object_text_get(obj);
	if (NULL == btn_text) {
		DBG(LOG_ERROR, "Incorrect button text(NULL)\n");
		return;
	}

	if (0 == strcmp(btn_text, _("IDS_WFD_BUTTON_SCAN"))) {
		wfd_refresh_wifi_direct_state(ugd);
		DBG(LOG_VERBOSE, "Start discovery again, status: %d\n", ugd->wfd_status);

		/* if connected, show the popup*/
		if (ugd->wfd_status >= WIFI_DIRECT_STATE_CONNECTED) {
			wfd_ug_act_popup(ugd, IDS_WFD_POP_SCAN_AGAIN, POP_TYPE_SCAN_AGAIN);
		} else if (WIFI_DIRECT_STATE_DEACTIVATED == ugd->wfd_status) {
			wfd_client_switch_on(ugd);
			__FUNC_EXIT__;
			return;
		} else {
			ret = wifi_direct_start_discovery(FALSE, MAX_SCAN_TIME_OUT);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", ret);
				ugd->is_re_discover = TRUE;
				wifi_direct_cancel_discovery();
			} else {
				DBG(LOG_VERBOSE, "Discovery is started\n");
				ugd->is_re_discover = FALSE;
			}
		}
	} else if (0 == strcmp(btn_text, _("IDS_WFD_BUTTON_STOPSCAN"))) {
		DBG(LOG_VERBOSE, "Stop discoverying.\n");
		ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
		wfd_ug_view_refresh_glitem(ugd->head);

		/* stop scaning */
		ugd->is_re_discover = FALSE;
		wifi_direct_cancel_discovery();
	}

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug call it when click header
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _gl_header_sel(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	if (NULL == data) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		__FUNC_EXIT__;
		return;
	}

	struct ug_data *ugd = (struct ug_data *) data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (item != NULL) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	/* turn on/off wfd */
	if (!ugd->wfd_onoff) {
		DBG(LOG_VERBOSE, "wifi-direct switch on\n");
		wfd_client_switch_on(ugd);
	} else {
		DBG(LOG_VERBOSE, "wifi-direct switch off\n");
		wfd_client_switch_off(ugd);
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
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == data, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	int res;

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	if (item != NULL) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	if (peer->conn_status == PEER_CONN_STATUS_DISCONNECTED || peer->is_group_owner == TRUE) {
		DBG(LOG_VERBOSE, "Connect with peer [%s]\n", peer->mac_addr);
		res = wfd_client_connect((const char *) peer->mac_addr);
		if (res != 0) {
			DBG(LOG_ERROR, "Failed to send connection request. [%d]\n", res);
			return;
		}
	} else {
		res = wfd_client_disconnect((const char *)peer->mac_addr);
		if (res != 0) {
			DBG(LOG_ERROR, "Failed to send disconnection request. [%d]\n", res);
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
	struct ug_data *ugd = (struct ug_data *) data;

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	DBG(LOG_VERBOSE, "Busy device is clicked");
	wfd_ug_warn_popup(ugd, IDS_WFD_POP_WARN_BUSY_DEVICE, POP_TYPE_BUSY_DEVICE_POPUP);

	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click about item
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _gl_about_wifi_sel(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	DBG(LOG_VERBOSE, "About wifi clicked");
	_wifid_create_about_view(ugd);
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	__FUNC_EXIT__;
}

/**
 *	This function let the ug call it when click 'multi connect' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _wifid_create_multibutton_cb(void *data, Evas_Object * obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	const char *text_lbl = NULL;

	text_lbl = elm_object_text_get(ugd->multi_btn);
	DBG(LOG_VERBOSE, "text_lbl = %s", text_lbl);

	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS) {
		ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_NONE;
		if (0 == strcmp(_("IDS_WFD_BUTTON_CANCEL"), text_lbl)) {
			wfd_ug_act_popup(ugd, _("IDS_WFD_POP_CANCEL_CONNECT"), POP_TYPE_DISCONNECT_ALL);
		} else {
			DBG(LOG_VERBOSE, "Invalid Case\n");
		}
	} else {
		if (0 == strcmp(_("IDS_WFD_BUTTON_MULTI"), text_lbl)) {
			wfd_create_multiconnect_view(ugd);
		} else if (0 == strcmp(_("IDS_WFD_BUTTON_CANCEL"), text_lbl)) {
			wfd_ug_act_popup(ugd, _("IDS_WFD_POP_CANCEL_CONNECT"), POP_TYPE_DISCONNECT_ALL);
		} else if (0 == strcmp(_("IDS_WFD_BUTTON_DISCONNECT_ALL"), text_lbl)) {
			wfd_ug_act_popup(ugd, _("IDS_WFD_POP_DISCONNECT"), POP_TYPE_DISCONNECT_ALL);
		} else if (0 == strcmp(_("IDS_WFD_BUTTON_DISCONNECT"), text_lbl)) {
			wfd_ug_act_popup(ugd, _("IDS_WFD_POP_DISCONNECT"), POP_TYPE_DISCONNECT);
		} else {
			DBG(LOG_VERBOSE, "Invalid Case\n");
		}
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug change the text of multi button
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int _change_multi_button_title(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	if (ugd->multi_button_item == NULL) {
		return -1;
	}

	wfd_refresh_wifi_direct_state(ugd);
	if (ugd->wfd_status == WIFI_DIRECT_STATE_CONNECTING) {
		elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_CANCEL"));
	} else if (ugd->wfd_status > WIFI_DIRECT_STATE_CONNECTING) {
		if (ugd->gl_connected_peer_cnt > 1) {
			elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_DISCONNECT_ALL"));
		} else {
			elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_DISCONNECT"));
		}
	} else {
		elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_MULTI"));
	}

	evas_object_show(ugd->multi_btn);
	__FUNC_EXIT__;

	return 0;
}

/**
 *	This function let the ug update the genlist item
 *	@return   void
 *	@param[in] obj the pointer to genlist item
 */
void wfd_ug_view_refresh_glitem(void *obj)
{
	__FUNC_ENTER__;
	elm_genlist_item_update(obj);
	__FUNC_EXIT__;
}

/**
 *	This function let the ug refresh the attributes of button
 *	@return   void
 *	@param[in] obj the pointer to the button
 *	@param[in] text the pointer to the text of button
 *	@param[in] enable whether the button is disabled
 */
void wfd_ug_view_refresh_button(void *obj, const char *text, int enable)
{
	__FUNC_ENTER__;

	if (NULL == obj || NULL == text) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	DBG(LOG_VERBOSE, "Set the attributes of button: text[%s], enabled[%d]\n", text, enable);
	elm_object_text_set(obj, text);
	elm_object_disabled_set(obj, !enable);

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
	int i = 0;

	for (i = 0; i < ugd->raw_discovered_peer_cnt; i++) {
		/* Not include the device which is connected with me */
		if (__wfd_is_device_connected_with_me(ugd, &ugd->raw_discovered_peers[i])) {
			continue;
		}
		if (!__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]) &&
			ugd->raw_discovered_peers[i].conn_status != PEER_CONN_STATUS_FAILED_TO_CONNECT) {
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
	int i = 0;

	for (i = 0; i < ugd->raw_discovered_peer_cnt; i++) {
		/* Not include the device which is connected with me */
		if (__wfd_is_device_connected_with_me(ugd, &ugd->raw_discovered_peers[i])) {
			continue;
		}
		if (__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i])) {
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
	int i = 0;

	for (i = 0; i < ugd->raw_discovered_peer_cnt; i++) {
		/* Not include the device which is connected with me */
		if (__wfd_is_device_connected_with_me(ugd, &ugd->raw_discovered_peers[i])) {
			continue;
		}
		if (!__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]) &&
			ugd->raw_discovered_peers[i].conn_status == PEER_CONN_STATUS_FAILED_TO_CONNECT) {
			(*no_of_connect_failed_dev)++;
		}

	}

	__FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the ug get the device status
 *	@return  If success, return 0-3(available: 0, connected: 1, busy: 2, connected failed: 3), else return -1
 *	@param[in] ugd the pointer to the main data structure
 *	@param[in] device the pointer to the number of connected failed devices
 */
int wfd_get_device_status(void *data, device_type_s *device)
{
	__FUNC_ENTER__;
	int ret = -1;
	int status = -1;
	struct ug_data *ugd = (struct ug_data *) data;

	if (ugd == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)");
		return -1;
	}

	/* check whether it is connected device  */
	ret = __wfd_is_device_connected_with_me(ugd, device);
	if (ret) {
		DBG(LOG_VERBOSE, "This is connected device");
		status = 1;
		goto err_exit;
	}

	/* check whether it is busy device  */
	ret = __wfd_is_device_busy(ugd, device);
	if (ret) {
		DBG(LOG_VERBOSE, "This is busy device");
		status = 2;
		goto err_exit;
	}

	/* check whether it is available device  */
	if (device->conn_status != PEER_CONN_STATUS_FAILED_TO_CONNECT) {
		DBG(LOG_VERBOSE, "This is available device");
		status = 0;
	} else {
		DBG(LOG_VERBOSE, "This is connected failed device");
		status = 3;
	}

err_exit:
	__FUNC_EXIT__;
	return status;
}

/**
 *	This function let the ug delete the separator
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 */
static void __wfd_separator_del(void *data, Evas_Object *obj)
{
	__FUNC_ENTER__;
	elm_genlist_item_class_free(data);
	return;
}

/**
 *	This function let the ug add a dialogue separator
 *	@return   the separator item
 *	@param[in] genlist the pointer to the genlist
 *	@param[in] separator_style the style of separator
 */
Elm_Object_Item *wfd_add_dialogue_separator(Evas_Object *genlist, const char *separator_style)
{
	__FUNC_ENTER__;
	assertm_if(NULL == genlist, "NULL!!");

	static Elm_Genlist_Item_Class *separator_itc;
	separator_itc = elm_genlist_item_class_new();
	separator_itc->item_style = separator_style;
	separator_itc->func.text_get = NULL;
	separator_itc->func.content_get = NULL;
	separator_itc->func.state_get = NULL;
	separator_itc->func.del = __wfd_separator_del;

	Elm_Object_Item *sep = elm_genlist_item_append(
					genlist,
					separator_itc,
					separator_itc,
					NULL,
					ELM_GENLIST_ITEM_GROUP,
					NULL,
					NULL);

	assertm_if(NULL == sep, "NULL!!");

	elm_genlist_item_select_mode_set(sep, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	__FUNC_EXIT__;
	return sep;
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

	genlist = elm_genlist_add(ugd->naviframe);
	wfd_add_dialogue_separator(genlist, "dialogue/separator");
	ugd->head = elm_genlist_item_append(genlist, &head_itc, ugd, NULL,
		ELM_GENLIST_ITEM_NONE, _gl_header_sel, (void *)ugd);

	__FUNC_EXIT__;
	return genlist;
}

/**
 *	This function let the ug create the about item to append the genlist
 *	@return   the main item
 *	@param[in] data the pointer to the main data structure
 */
static Evas_Object *_create_about_genlist(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	ugd->about_wfd_sep_high_item = wfd_add_dialogue_separator(ugd->genlist, "dialogue/separator");
	ugd->about_wfd_item = elm_genlist_item_append(ugd->genlist, &name_itc, ugd, NULL,
		ELM_GENLIST_ITEM_NONE, _gl_about_wifi_sel, (void *)ugd);
	ugd->about_wfd_sep_low_item = wfd_add_dialogue_separator(ugd->genlist, "dialogue/separator/end");

	__FUNC_EXIT__;
	return ugd->genlist;
}

/**
 *	This function let the ug create no device item to append the genlist
 *	@return   the main item
 *	@param[in] data the pointer to the main data structure
 */
static Evas_Object *_create_no_device_genlist(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	ugd->nodevice_title_item = elm_genlist_item_append(ugd->genlist, &title_itc, (void *)ugd, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
	ugd->nodevice_item = elm_genlist_item_append(ugd->genlist, &noitem_itc, (void *)ugd, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(ugd->nodevice_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	__FUNC_EXIT__;
	return ugd->genlist;
}

/**
 *	This function let the ug create multi connect button
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int _create_multi_button_genlist(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	wfd_refresh_wifi_direct_state(ugd);

	/* show the button */
	if (ugd->multi_connect_mode != WFD_MULTI_CONNECT_MODE_NONE) {
		if (ugd->raw_multi_selected_peer_cnt > 1 ||
			ugd->gl_connected_peer_cnt > 0 ||
			ugd->wfd_status == WIFI_DIRECT_STATE_CONNECTING) {
			ugd->multi_button_sep_item = wfd_add_dialogue_separator(ugd->genlist, "dialogue/separator");
			ugd->multi_button_item = elm_genlist_item_append(ugd->genlist, &button_itc, ugd, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	} else {
		if (ugd->gl_available_peer_cnt > 1 ||
			ugd->gl_connected_peer_cnt > 0 ||
			ugd->wfd_status == WIFI_DIRECT_STATE_CONNECTING) {
			ugd->multi_button_sep_item = wfd_add_dialogue_separator(ugd->genlist, "dialogue/separator");
			ugd->multi_button_item = elm_genlist_item_append(ugd->genlist, &button_itc, ugd, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	}

	__FUNC_EXIT__;
	return 0;
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

	ugd->busy_wfd_item = elm_genlist_item_append(ugd->genlist, &title_busy_itc, (void *)ugd, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(ugd->busy_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug create avaliable device list
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
static int _create_available_dev_genlist(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	ugd->avlbl_wfd_item = elm_genlist_item_append(ugd->genlist, &title_itc, (void *)ugd, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);

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

	ugd->multi_connect_wfd_item = elm_genlist_item_append(ugd->genlist, &title_multi_connect_itc, (void *)ugd, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);

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

	ugd->conn_wfd_item = elm_genlist_item_append(ugd->genlist, &title_conn_itc, (void *)ugd, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(ugd->conn_wfd_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

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

	ugd->conn_failed_wfd_item = elm_genlist_item_append(ugd->genlist, &title_conn_failed_itc, (void *)ugd, NULL,
		ELM_GENLIST_ITEM_NONE, NULL, NULL);
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
	int interval = 0;
	int res = -1;
	struct ug_data *ugd = (struct ug_data *) user_data;

	if (NULL == ugd) {
		DBG(LOG_ERROR, "NULL parameters.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	/* check the timeout, if not timeout, keep the cb */
	interval = time(NULL) - ugd->last_display_time;
	if (interval < MAX_DISPLAY_TIME_OUT) {
		return ECORE_CALLBACK_RENEW;
	}

	/* start discovery again */
	res = wifi_direct_start_discovery(FALSE, MAX_SCAN_TIME_OUT);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", res);
		ugd->is_re_discover = TRUE;
		wifi_direct_cancel_discovery();
	} else {
		DBG(LOG_VERBOSE, "Discovery is started\n");
		ugd->is_re_discover = FALSE;
	}

	return ECORE_CALLBACK_CANCEL;
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
	int i = 0;

	for (i = 0; i < ugd->gl_connected_peer_cnt; i++) {
		DBG(LOG_VERBOSE, "%dth connected peer = %x is deleted\n", i, ugd->gl_connected_peers[i]);
		if (ugd->gl_connected_peers[i].gl_item != NULL) {
			elm_object_item_del(ugd->gl_connected_peers[i].gl_item);
			ugd->gl_connected_peers[i].gl_item = NULL;
			DBG(LOG_VERBOSE, "Deleted item\n");
		}
	}

	ugd->gl_connected_peer_cnt = 0;

	for (i = 0; i < ugd->gl_connected_failed_peer_cnt; i++) {
		DBG(LOG_VERBOSE, "%dth connected failed peer = %x is deleted\n", i, ugd->gl_connected_failed_peers[i]);
		if (ugd->gl_connected_failed_peers[i].gl_item != NULL) {
		    elm_object_item_del(ugd->gl_connected_failed_peers[i].gl_item);
		    ugd->gl_connected_failed_peers[i].gl_item = NULL;
		    DBG(LOG_VERBOSE, "Deleted item\n");
		}
	}

	ugd->gl_connected_failed_peer_cnt = 0;

	for (i = 0; i < ugd->gl_available_peer_cnt; i++) {
		DBG(LOG_VERBOSE, "%dth discovered peer = %x is deleted\n", i, ugd->gl_available_peers[i]);
		if (ugd->gl_available_peers[i].gl_item != NULL) {
			elm_object_item_del(ugd->gl_available_peers[i].gl_item);
			ugd->gl_available_peers[i].gl_item = NULL;
			DBG(LOG_VERBOSE, "Deleted item\n");
		}
	}

	ugd->gl_available_peer_cnt = 0;

	for (i = 0; i < ugd->gl_busy_peer_cnt; i++) {
		DBG(LOG_VERBOSE, "%dth busy peer = %x is deleted\n", i, ugd->gl_busy_peers[i]);
		if (ugd->gl_busy_peers[i].gl_item != NULL) {
			elm_object_item_del(ugd->gl_busy_peers[i].gl_item);
			ugd->gl_busy_peers[i].gl_item = NULL;
			DBG(LOG_VERBOSE, "Deleted item\n");
		}
	}

	ugd->gl_busy_peer_cnt = 0;

	for (i = 0; i < ugd->gl_multi_connect_peer_cnt; i++) {
		DBG(LOG_VERBOSE, "%dth multi connect peer = %x is deleted\n", i, ugd->gl_multi_connect_peers[i]);
		if (ugd->gl_multi_connect_peers[i].gl_item != NULL) {
			elm_object_item_del(ugd->gl_multi_connect_peers[i].gl_item);
			ugd->gl_multi_connect_peers[i].gl_item = NULL;
			DBG(LOG_VERBOSE, "Deleted item\n");
		}
	}

	ugd->gl_multi_connect_peer_cnt = 0;

	if (ugd->nodevice_title_item != NULL) {
		elm_object_item_del(ugd->nodevice_title_item);
		ugd->nodevice_title_item = NULL;
	}

	if (ugd->nodevice_item != NULL) {
		elm_object_item_del(ugd->nodevice_item);
		ugd->nodevice_item = NULL;
	}

	if (ugd->about_wfd_item != NULL) {
		elm_object_item_del(ugd->about_wfd_item);
		ugd->about_wfd_item = NULL;
	}

	if (ugd->about_wfd_sep_high_item != NULL) {
		elm_object_item_del(ugd->about_wfd_sep_high_item);
		ugd->about_wfd_sep_high_item = NULL;
	}

	if (ugd->about_wfd_sep_low_item != NULL) {
		elm_object_item_del(ugd->about_wfd_sep_low_item);
		ugd->about_wfd_sep_low_item = NULL;
	}

	if (ugd->conn_wfd_item != NULL) {
		elm_object_item_del(ugd->conn_wfd_item);
		ugd->conn_wfd_item = NULL;
	}

	if (ugd->conn_failed_wfd_item != NULL) {
		elm_object_item_del(ugd->conn_failed_wfd_item);
		ugd->conn_failed_wfd_item = NULL;
	}

	if (ugd->display_timer != NULL) {
		ecore_timer_del(ugd->display_timer);
		ugd->display_timer = NULL;
	}

	if (ugd->multi_connect_wfd_item != NULL) {
		elm_object_item_del(ugd->multi_connect_wfd_item);
		ugd->multi_connect_wfd_item = NULL;
	}

	if (ugd->avlbl_wfd_item != NULL) {
		elm_object_item_del(ugd->avlbl_wfd_item);
		ugd->avlbl_wfd_item = NULL;
	}

	if (ugd->busy_wfd_item != NULL) {
		elm_object_item_del(ugd->busy_wfd_item);
		ugd->busy_wfd_item = NULL;
	}

	if (ugd->multi_button_item != NULL) {
		elm_object_item_del(ugd->multi_button_item);
		ugd->multi_button_item = NULL;
	}

	if (ugd->multi_button_sep_item != NULL) {
		elm_object_item_del(ugd->multi_button_sep_item);
		ugd->multi_button_sep_item = NULL;
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug update the peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_view_update_peers(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	int no_of_busy_dev = 0;
	int no_of_available_dev = 0;
	int no_of_conn_dev = 0;
	int no_of_conn_failed_dev = 0;
	int i = 0 ;
	int res = 0;
	bool is_group_owner = FALSE;
	int count = 0;

	wfd_ug_view_free_peers(ugd);

	if (ugd->wfd_status == WIFI_DIRECT_STATE_DEACTIVATED) {
		DBG(LOG_VERBOSE, "Device is deactivated, no need to update UI.");
		_create_about_genlist(ugd);
		return;
	}

	res = wifi_direct_is_group_owner(&is_group_owner);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_VERBOSE, "Fail to get group_owner_state. ret=[%d]", res);
		ugd->I_am_group_owner = FALSE;
	} else {
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
		no_of_available_dev == 0 && no_of_busy_dev == 0) {
		DBG(LOG_ERROR, "There are No peers\n");
		_create_no_device_genlist(ugd);
		_create_about_genlist(ugd);
		return;
	}

	/* display connect peers */
	if (no_of_conn_dev > 0) {
		if (!ugd->conn_wfd_item) {
			_create_connected_dev_genlist(ugd);
		}

		count = 0;
		for (i = 0; i < ugd->raw_connected_peer_cnt; i++) {
			if (ugd->gl_connected_peers[count].gl_item) {
				elm_object_item_del(ugd->gl_connected_peers[count].gl_item);
			}

			memcpy(&ugd->gl_connected_peers[count], &ugd->raw_connected_peers[i], sizeof(device_type_s));
			ugd->gl_connected_peers[count].gl_item = elm_genlist_item_append(ugd->genlist, &peer_conn_itc,
				(void *)&(ugd->gl_connected_peers[i]), NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(ugd->gl_connected_peers[count].gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			count++;
		}
	}

	if (ugd->multi_connect_mode != WFD_MULTI_CONNECT_MODE_NONE) {
		if (ugd->raw_multi_selected_peer_cnt > 0) {
			if (ugd->raw_connected_peer_cnt < ugd->raw_multi_selected_peer_cnt &&
				ugd->multi_connect_wfd_item == NULL) {
				_create_multi_connect_dev_genlist(ugd);
			}

			count = 0;
			for (i = 0; i < ugd->raw_multi_selected_peer_cnt; i++) {
				if (ugd->raw_multi_selected_peers[i].conn_status != PEER_CONN_STATUS_CONNECTED) {
					if (ugd->gl_multi_connect_peers[count].gl_item) {
						elm_object_item_del(ugd->gl_multi_connect_peers[count].gl_item);
					}

					memcpy(&ugd->gl_multi_connect_peers[count], &ugd->raw_multi_selected_peers[i], sizeof(device_type_s));
					ugd->gl_multi_connect_peers[count].gl_item = elm_genlist_item_append(ugd->genlist, &peer_itc,
						(void *) &(ugd->gl_multi_connect_peers[count]), NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
					count++;
				}
			}

			ugd->gl_multi_connect_peer_cnt = count;
		}

		_create_multi_button_genlist(ugd);
	} else {
		/*
		* Note that
		* If GC, no display available peers
		* Otherwise, display available peers
		*/
		if (no_of_available_dev > 0 && (no_of_conn_dev == 0 || is_group_owner == TRUE)) {
			if (ugd->avlbl_wfd_item == NULL) {
				_create_available_dev_genlist(ugd);
			}

			count = 0;
			for (i = 0; i < ugd->raw_discovered_peer_cnt; i++) {
				/* Not include the device which is connected with me */
				if (__wfd_is_device_connected_with_me(ugd, &ugd->raw_discovered_peers[i])) {
					continue;
				}
				if (!__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]) &&
					ugd->raw_discovered_peers[i].conn_status != PEER_CONN_STATUS_FAILED_TO_CONNECT) {
					if (ugd->gl_available_peers[count].gl_item) {
						elm_object_item_del(ugd->gl_available_peers[count].gl_item);
					}

					memcpy(&ugd->gl_available_peers[count], &ugd->raw_discovered_peers[i], sizeof(device_type_s));
					ugd->gl_available_peers[count].gl_item = elm_genlist_item_append(ugd->genlist, &peer_itc,
						(void *)&(ugd->gl_available_peers[count]), NULL, ELM_GENLIST_ITEM_NONE, _gl_peer_sel,
						(void *)&(ugd->gl_available_peers[count]));
					count++;
				}
			}
		}

		/* display connect failed peers */
		if (no_of_conn_failed_dev > 0) {
			if (!ugd->conn_failed_wfd_item) {
				_create_connected_failed_dev_genlist(ugd);
			}

			/* add timer for disappearing failed peers after N secs */
			if (NULL == ugd->display_timer) {
				ugd->last_display_time = time(NULL);
				ugd->display_timer = ecore_timer_add(5.0, (Ecore_Task_Cb)_connect_failed_peers_display_cb, ugd);
			}

			count = 0;
			for (i = 0; i < ugd->raw_discovered_peer_cnt; i++) {
				/* Not include the device which is connected with me */
				if (__wfd_is_device_connected_with_me(ugd, &ugd->raw_discovered_peers[i])) {
					continue;
				}
				if (!__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]) &&
					ugd->raw_discovered_peers[i].conn_status == PEER_CONN_STATUS_FAILED_TO_CONNECT) {
					if (ugd->gl_connected_failed_peers[count].gl_item) {
						elm_object_item_del(ugd->gl_connected_failed_peers[count].gl_item);
					}

					memcpy(&ugd->gl_connected_failed_peers[count], &ugd->raw_discovered_peers[i], sizeof(device_type_s));
					ugd->gl_connected_failed_peers[count].gl_item = elm_genlist_item_append(ugd->genlist, &peer_conn_failed_itc,
						(void *)&(ugd->gl_connected_failed_peers[count]), NULL, ELM_GENLIST_ITEM_NONE, NULL, ugd);
					elm_genlist_item_select_mode_set(ugd->gl_connected_failed_peers[count].gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
					count++;
				}
			}
		}

		_create_multi_button_genlist(ugd);

		/* If connected, not display busy device */
		if (no_of_conn_dev == 0 && no_of_busy_dev > 0) {
			if (ugd->busy_wfd_item == NULL) {
				_create_busy_dev_list(ugd);
			}

			count = 0;
			for (i = 0; i < ugd->raw_discovered_peer_cnt; i++) {
				/* Not include the device which is connected with me */
				if (__wfd_is_device_connected_with_me(ugd, &ugd->raw_discovered_peers[i])) {
					continue;
				}
				if (__wfd_is_device_busy(ugd, &ugd->raw_discovered_peers[i]) == TRUE) {
					if (ugd->gl_busy_peers[count].gl_item) {
						elm_object_item_del(ugd->gl_busy_peers[count].gl_item);
					}

					memcpy(&ugd->gl_busy_peers[count], &ugd->raw_discovered_peers[i], sizeof(device_type_s));
					ugd->gl_busy_peers[count].gl_item = elm_genlist_item_append(ugd->genlist, &peer_busy_itc,
						(void *)&(ugd->gl_busy_peers[count]), NULL, ELM_GENLIST_ITEM_NONE, _gl_busy_peer_sel, ugd);
					count++;
				}
			}
		}
	}
	_create_about_genlist(ugd);

	__FUNC_EXIT__;
}

/**
 *	This function let the ug create the main view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void create_wfd_ug_view(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	Elm_Object_Item *navi_item = NULL;

	if (ugd == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)");
		return;
	}

	ugd->naviframe = elm_naviframe_add(ugd->base);
	elm_object_part_content_set(ugd->base, "elm.swallow.content", ugd->naviframe);
	evas_object_show(ugd->naviframe);

	ugd->back_btn = elm_button_add(ugd->naviframe);
	elm_object_style_set(ugd->back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(ugd->back_btn, "clicked", _back_btn_cb, (void *)ugd);
	elm_object_focus_allow_set(ugd->back_btn, EINA_FALSE);

	ugd->genlist = _create_basic_genlist(ugd);
	if (ugd->genlist == NULL) {
		DBG(LOG_ERROR, "Failed to create basic genlist");
		return;
	}

	evas_object_show(ugd->genlist);
	wfd_refresh_wifi_direct_state(ugd);
	if (ugd->wfd_status > WIFI_DIRECT_STATE_ACTIVATING) {
		ugd->wfd_onoff = TRUE;
	}

	navi_item = elm_naviframe_item_push(ugd->naviframe, _("IDS_WFD_HEADER_WIFI_DIRECT"), ugd->back_btn, NULL, ugd->genlist, NULL);
	/* create scan button */
	ugd->scan_btn = elm_button_add(ugd->naviframe);
	elm_object_style_set(ugd->scan_btn, "naviframe/toolbar/default");
	elm_object_text_set(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"));
	evas_object_smart_callback_add(ugd->scan_btn, "clicked", _scan_btn_cb, (void *)ugd);
	elm_object_item_part_content_set(navi_item, "toolbar_button1", ugd->scan_btn);

	__FUNC_EXIT__;
}
