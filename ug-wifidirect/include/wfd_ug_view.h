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

#ifndef __WFD_UG_VIEW_H__
#define __WFD_UG_VIEW_H__

#include <glib.h>

#define EDJDIR "/usr/apps/setting-wifidirect-efl/res/edje/ug-setting-wifidirect-efl"
#define WFD_UG_EDJ_PATH  EDJDIR"/wfd_ug.edj"
#define WFD_IMG_DIR "/usr/apps/setting-wifidirect-efl/res/images/ug-setting-wifidirect-efl"

/* Genlist new style for Tizen 2.4 */
#define WFD_GENLIST_1LINE_TEXT_STYLE "type1"
#define WFD_GENLIST_1LINE_TEXT_ICON_STYLE "type1"
#define WFD_GENLIST_2LINE_TOP_TEXT_STYLE "type1"
#define WFD_GENLIST_2LINE_TOP_TEXT_ICON_STYLE "type1"
#define WFD_GENLIST_2LINE_BOTTOM_TEXT_STYLE "type2"
#define WFD_GENLIST_2LINE_BOTTOM_TEXT_ICON_STYLE "type2"
#define WFD_GENLIST_MULTILINE_TEXT_STYLE "multiline"
#define WFD_GENLIST_GROUP_INDEX_STYLE "group_index"

/* Define icons */
#define WFD_ICON_DEVICE_COMPUTER			"A09_device_computer.png"
#define WFD_ICON_DEVICE_INPUT_DEVICE		"A09_device_input_device.png"
#define WFD_ICON_DEVICE_PRINTER				"A09_device_printer.png"
#define WFD_ICON_DEVICE_CAMERA				"A09_device_camera.png"
#define WFD_ICON_DEVICE_STORAGE				"A09_device_storage.png"
#define WFD_ICON_DEVICE_NETWORK_INFRA		"A09_device_network_infrastructure.png"
#define WFD_ICON_DEVICE_DISPLAY				"A09_device_display.png"
#define WFD_ICON_DEVICE_MULTIMEDIA			"A09_device_multimedia.png"
#define WFD_ICON_DEVICE_GAMING				"A09_device_gaming.png"
#define WFD_ICON_DEVICE_TELEPHONE			"A09_device_telephone.png"
#define WFD_ICON_DEVICE_HEADSET				"A09_device_headset.png"
#define WFD_ICON_DEVICE_UNKNOWN				"A09_device_unknown.png"
#define WFD_ICON_DEVICE_BD					"U04_device_BD.png"
#define WFD_ICON_DEVICE_DONGLE				"U04_device_Dongle.png"
#define WFD_ICON_DEVICE_HOME_THEATER		"U04_device_Home_Theater.png"
#define WFD_ICON_DEVICE_STB					"U04_device_STB.png"

/* Not support */
#define WFD_ICON_DEVICE_HEADPHONE			"A09_device_headphone.png"
#define WFD_ICON_DEVICE_MEDICAL				"A09_device_Medical.png"
#define WFD_ICON_DEVICE_MOUSE				"A09_device_mouse.png"

#define WFD_ICON_DEVICE_MORE_HELP				"A09_popup_help.png"

enum {
	HEAD_TEXT_TYPE_DIRECT,
	HEAD_TEXT_TYPE_DEACTIVATING,
	HEAD_TEXT_TYPE_ACTIVATING,
	HEAD_TEXT_TYPE_ACTIVATED,
};

enum {
	TITLE_CONTENT_TYPE_NONE,
	TITLE_CONTENT_TYPE_SCANNING,
	TITLE_CONTENT_TYPE_FOUND,
};


enum {
	/* User confirm */
#ifndef MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE
	POPUP_TYPE_WIFI_OFF,
#endif

	POPUP_TYPE_HOTSPOT_OFF = 1,

	/* Activation */
	POPUP_TYPE_ACTIVATE_FAIL,
	POPUP_TYPE_ACTIVATE_FAIL_POLICY_RESTRICTS,
	POPUP_TYPE_DEACTIVATE_FAIL,

	/* Connection */
	POPUP_TYPE_LINK_TIMEOUT,
	POPUP_TYPE_AUTH_FAIL,
	POPUP_TYPE_LINK_FAIL,
	POPUP_TYPE_UNKNOWN_ERROR,

	POPUP_TYPE_INFO,
	POPUP_TYPE_TERMINATE,
	POPUP_TYPE_TERMINATE_DEACTIVATE_FAIL,
	POPUP_TYPE_TERMINATE_NOT_SUPPORT,

	/* Disconnect */
	POP_TYPE_DISCONNECT,

	POP_TYPE_CANCEL_CONNECT,

	POP_TYPE_ACCEPT_CONNECTION,

	/* Scan again */
	POP_TYPE_SCAN_AGAIN,

	/* multi connect */
	POP_TYPE_MULTI_CONNECT_POPUP,

	/* Busy device */
	POP_TYPE_BUSY_DEVICE_POPUP,

	/* Automaticlly turn off */
	POP_TYPE_AUTOMATIC_TURN_OFF,
};

struct ug_data *wfd_get_ug_data();

/**
 *	This function let the ug create the main view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void create_wfd_ug_view(void *data);

/**
 *	This function let the ug destroy the main view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void destroy_wfd_ug_view(void *data);

/**
 *	This function let the ug update the genlist item
 *	@return   void
 *	@param[in] gl_item the pointer to genlist item
 */
void wfd_ug_view_refresh_glitem(Elm_Object_Item *gl_item);

/**
 *	This function let the ug refresh the attributes of button
 *	@return   void
 *	@param[in] tb_item the pointer to the toolbar button
 *	@param[in] text the pointer to the text of button
 *	@param[in] enable whether the button is disabled
 */
void wfd_ug_view_refresh_button(Evas_Object *tb_item, const char *text,
		int enable);

/**
 *	This function let the ug update the peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_view_update_peers(void *data);

/**
 *	This function let the ug free the peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_view_free_peers(void *data);

/**
 *	This function let the ug create a action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] message the pointer to the text of popup
 *	@param[in] popup_type the message type
 */
void wfd_ug_act_popup(void *data, const char *message, int popup_type);

/**
 *	This function let the ug remove the action popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfg_ug_act_popup_remove(void *data);

/**
 *	This function let the ug create a warning popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] message the pointer to the text of popup
 *	@param[in] popup_type the message type
 */
void wfd_ug_warn_popup(void *data, const char *message, int popup_type);

/**
 *	This function let the ug create the view for multi connection
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 */
void _wifid_create_multiconnect_view(struct ug_data *ugd);

/**
 *	This function let the ug call it when click 'back' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
Eina_Bool _back_btn_cb(void *data, Elm_Object_Item *it);

/**
 *	This function let the ug call it when click 'scan' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _scan_btn_cb(void *data, Evas_Object * obj, void *event_info);

/**
 *	This function let the ug call it when click 'disconnect' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _wfd_ug_disconnect_button_cb(void *data, Evas_Object *obj, void *event_info);

/**
 *	This function let the ug call it when click "Cancel Connection" button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _wfd_ug_cancel_connection_button_cb(void *data, Evas_Object *obj, void *event_info);

/**
 *	This function let the ug get the found peers
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int wfd_ug_get_discovered_peers(struct ug_data *ugd);

/**
 *	This function let the ug get the connecting peer mac
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int wfd_ug_get_connecting_peer(struct ug_data *ugd);

/**
 *	This function let the ug get the connected peers
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int wfd_ug_get_connected_peers(struct ug_data *ugd);

/**
 *	This function let the ug get the device status
 *	@return  If success, return 0-3(available: 0, connected: 1, busy: 2, connected failed: 3), else return -1
 *	@param[in] ugd the pointer to the main data structure
 *	@param[in] device the pointer to the number of connected failed devices
 */
//int wfd_get_device_status(void *data, device_type_s *device);

/**
 *	This function let the ug refresh current status of wi-fi direct
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_refresh_wifi_direct_state(void *data);

/**
 *	This function let the ug free the selected peers in multi connect view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_free_multi_selected_peers(void *data);

/**
 *	This function let the ug stop to connect to selected peer
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_stop_multi_connect(void *data);

/**
 *	This function let the ug connect to the next selected peer automatically
 *	@return   If stop the timer, return false, else return true
 *	@param[in] data the pointer to the main data structure
 */
gboolean wfd_multi_connect_next_cb(void *data);

/**
 *	This function let the ug add a dialogue separator
 *	@return   the separator item
 *	@param[in] genlist the pointer to the genlist
 *	@param[in] separator_style the style of separator
 */
Elm_Object_Item *wfd_add_dialogue_separator(Evas_Object *genlist,
		const char *separator_style);

/**
 *	This function let the ug free the multi connect devices
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int wfd_free_multiconnect_device(struct ug_data *ugd);

/**
 *	This function let the ug update the multi connect devices
 *	@return   0
 *	@param[in] ugd the pointer to the main data structure
 *	@param[in] is_free_all_peers true to free all peers
 */
int wfd_update_multiconnect_device(struct ug_data *ugd, bool is_free_all_peers);

/**
 *	This function let the ug create the view for multi connection
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 */
void wfd_create_multiconnect_view(struct ug_data *ugd);

/**
 *	This function let the ug delete search progressbar
 *	@return   void
 *	@param[in] user_data the pointer to the main data structure
 */
int wfd_delete_progressbar_cb(void *user_data);

/**
 *	This function let the ug insert peer item to genlist
 *	@return   int
 *	@param[in] genlist the pointer to genlist
 *	@param[in] item the pointer to item to insert after
 *      @param[in] itc Elm_Gen_Item_Class
 *	@param[in] start_pos the pointer to the start position of peer array
 *	@param[in] peer_for_insert the pointer to the peer to insert
 *	@param[in] callback callback for peer select
 */
int insert_gl_item(Evas_Object *genlist, Elm_Object_Item *item,
		Elm_Gen_Item_Class *itc, device_type_s **start_pos,
		device_type_s *peer_for_insert, void *callback);

/**
 *	This function let the ug get the insert position for next item
 *	@return  item the position to insert after
 *	@param[in] peer the pointer to peer to search
 *	@param[in] pre_item the title item of peer list
 *	@param[in] peer_cnt the count of gl peers
 */
Elm_Object_Item *get_insert_postion(device_type_s *peer,
		Elm_Object_Item *pre_item, int peer_cnt);

/**
 *	This function let the ug find a peer in genlist
 *	@return  peer the pointer of peer that found
 *	@param[in] start_pos the start position of peers list
 *	@param[in] mac_addr the mac_addr of peer for search
 */
device_type_s *find_peer_in_glist(device_type_s *start_pos, const char *mac_addr);

/**
 *	This function let the ug free gl peers
 *	@return   void
 *	@param[in] gl_peers_start the start pointer of peer list that for free
 */
void wfd_ug_view_free_peer(device_type_s *gl_peers_start);

/**
 *	This function let the ug exits automatically after successed connection
 *	@return   void
 *	@param[in] user_data the pointer to the main data structure
 */
void _wfd_ug_auto_exit(void *user_data);

/**
 *	This function let the ug update the buttons of toolbar
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 */
void wfd_ug_update_toolbar(struct ug_data *ugd);

/**
 *	This function let the ug update the available and busy peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_update_available_peers(void *data);

/**
 *	This function let the ug init the genlist
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] is_free_all_peers true to free all peer items
 */
void wfd_ug_view_init_genlist(void *data, bool is_free_all_peers);

/**
 *	This function let the ug delete dead peers
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 *	@param[in] start_pos the pointer to start of peer list
 *	@param[in] cnt the pointer to the number of peets in list
 */
void delete_not_alive_peers(struct ug_data *ugd, device_type_s **start_pos,
		int *cnt);

/**
 *	This function let the ug mark up the dead peers
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 *	@param[in] start_pos the pointer to start of peer list
 */
void set_not_alive_peers(device_type_s *start_pos);

/**
 *	This function let the ug update connected peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_update_connected_peers(void *data);

/**
 *	This function let the ug update the multi-connect peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_view_update_multiconn_peers(void *data);

/**
 *	This function let the ug update the failed peers
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_ug_update_failed_peers(void *data);

/**
 *	This function let the ug cancel progressbar stop timer
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 */
void wfd_cancel_progressbar_stop_timer(struct ug_data *ugd);

/**
 *	This function let the ug cancel not-alive devices delete timer
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 */
void wfd_cancel_not_alive_delete_timer(struct ug_data *ugd);

/**
 *	This function let the ug move ctxpopup to specified location
 *	@return   void
 */
void _ctxpopup_move();

int _create_available_dev_genlist(void *data);

Evas_Object *_create_no_device_genlist(void *data);

Evas_Object *_create_no_device_multiconnect_genlist(struct ug_data *ugd);

/**
 *	This function let the ug create rename popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */

void _gl_rename_device_sel(void *data, Evas_Object *obj, void *event_info);

#ifdef WFD_ON_OFF_GENLIST
/**
 *	This function let the ug create on off check
 *	@return   void
 *	@param[in] parent the pointer to the naviframe
 */
void wfd_ug_create_on_off_check(struct ug_data *ugd);
#endif
/**
 *	This function is called after select all button is checked to select all available devices
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_genlist_select_all_check_changed_cb(void *data, Evas_Object *obj,
		void *event_info);

#endif  /* __WFD_UG_VIEW_H__ */
