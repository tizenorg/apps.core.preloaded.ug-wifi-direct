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
#include <syspopup_caller.h>

#define EDJDIR "/usr/ug/res/edje/ug-setting-wifidirect-efl"
#define WFD_UG_EDJ_PATH  EDJDIR"/wfd_ug.edj"
#define WFD_IMG_DIR "/usr/ug/res/images/ug-setting-wifidirect-efl"
#define TICKERNOTI_SYSPOPUP "tickernoti-syspopup"


/* Define icons */

#define WFD_ICON_DEVICE_COMPUTER			WFD_IMG_DIR"/A09_device_computer.png"
#define WFD_ICON_DEVICE_INPUT_DEVICE			WFD_IMG_DIR"/A09_device_input_device.png"
#define WFD_ICON_DEVICE_PRINTER				WFD_IMG_DIR"/A09_device_printer.png"
#define WFD_ICON_DEVICE_CAMERA				WFD_IMG_DIR"/A09_device_camera.png"
#define WFD_ICON_DEVICE_STORAGE				WFD_IMG_DIR"/A09_device_storage.png"
#define WFD_ICON_DEVICE_NETWORK_INFRA			WFD_IMG_DIR"/A09_device_network_infrastructure.png"
#define WFD_ICON_DEVICE_DISPLAY				WFD_IMG_DIR"/A09_device_display.png"
#define WFD_ICON_DEVICE_MULTIMEDIA_DEVICE		WFD_IMG_DIR"/A09_device_multimedia_devices.png"
#define WFD_ICON_DEVICE_GAMING_DEVICE			WFD_IMG_DIR"/A09_device_gaming_devices.png"
#define WFD_ICON_DEVICE_TELEPHONE			WFD_IMG_DIR"/A09_device_telephone.png"
#define WFD_ICON_DEVICE_AUDIO_DEVICE			WFD_IMG_DIR"/A09_device_audio_devices.png"

#define WFD_ICON_DEVICE_COMPUTER_CONNECT		WFD_IMG_DIR"/A09_device_computer_connect.png"
#define WFD_ICON_DEVICE_INPUT_DEVICE_CONNECT		WFD_IMG_DIR"/A09_device_input_device_connect.png"
#define WFD_ICON_DEVICE_PRINTER_CONNECT			WFD_IMG_DIR"/A09_device_printer_connect.png"
#define WFD_ICON_DEVICE_CAMERA_CONNECT			WFD_IMG_DIR"/A09_device_camera_connect.png"
#define WFD_ICON_DEVICE_STORAGE_CONNECT			WFD_IMG_DIR"/A09_device_storage_connect.png"
#define WFD_ICON_DEVICE_NETWORK_INFRA_CONNECT		WFD_IMG_DIR"/A09_device_network_infrastructure_connect.png"
#define WFD_ICON_DEVICE_DISPLAY_CONNECT			WFD_IMG_DIR"/A09_device_display_connect.png"
#define WFD_ICON_DEVICE_MULTIMEDIA_DEVICE_CONNECT	WFD_IMG_DIR"/A09_device_multimedia_devices_connect.png"
#define WFD_ICON_DEVICE_GAMING_DEVICE_CONNECT		WFD_IMG_DIR"/A09_device_gaming_devices_connect.png"
#define WFD_ICON_DEVICE_TELEPHONE_CONNECT		WFD_IMG_DIR"/A09_device_telephone_connect.png"
#define WFD_ICON_DEVICE_AUDIO_DEVICE_CONNECT		WFD_IMG_DIR"/A09_device_audio_devices_connect.png"

#define WFD_ICON_CONNECTED				WFD_IMG_DIR"/A09_Connect.png"


enum {
	HEAD_TEXT_TYPE_DIRECT,
	HEAD_TEXT_TYPE_DEACTIVATING,
	HEAD_TEXT_TYPE_ACTIVATING,
	HEAD_TEXT_TYPE_ACTIVATED,
	HEAD_TEXT_TYPE_SCANING,
};

enum {
	/* User confirm */
	POPUP_TYPE_WIFI_OFF,
	POPUP_TYPE_HOTSPOT_OFF,

	/* Activation */
	POPUP_TYPE_ACTIVATE_FAIL,
	POPUP_TYPE_DEACTIVATE_FAIL,

	/* Connection */
	POPUP_TYPE_LINK_TIMEOUT,
	POPUP_TYPE_AUTH_FAIL,
	POPUP_TYPE_LINK_FAIL,
	POPUP_TYPE_UNKNOWN_ERROR,

	POPUP_TYPE_TERMINATE,

	/* Disconnect */
	POP_TYPE_DISCONNECT,

	/* Disconnect All*/
	POP_TYPE_DISCONNECT_ALL,

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
 *	@param[in] obj the pointer to genlist item
 */
void wfd_ug_view_refresh_glitem(void *obj);

/**
 *	This function let the ug refresh the attributes of button
 *	@return   void
 *	@param[in] obj the pointer to the button
 *	@param[in] text the pointer to the text of button
 *	@param[in] enable whether the button is disabled
 */
void wfd_ug_view_refresh_button(void *obj, const char *text, int enable);

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
 *	This function let the ug change the text of multi button
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int _change_multi_button_title(void *data);

/**
 *	This function let the ug create about view
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 */
void _wifid_create_about_view(struct ug_data *ugd);

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
void _back_btn_cb(void *data, Evas_Object * obj, void *event_info);

/**
 *	This function let the ug call it when click 'scan' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _scan_btn_cb(void *data, Evas_Object * obj, void *event_info);

/**
 *	This function let the ug call it when click 'multi connect' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _wifid_create_multibutton_cb(void *data, Evas_Object *obj, void *event_info);

/**
 *	This function let the ug get the found peers
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int wfd_ug_get_discovered_peers(struct ug_data *ugd);

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
int wfd_get_device_status(void *data, device_type_s *device);

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
Elm_Object_Item *wfd_add_dialogue_separator(Evas_Object *genlist, const char *separator_style);

/**
 *	This function let the ug fee the multi connect devices
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int wfd_free_multiconnect_device(struct ug_data *ugd);

/**
 *	This function let the ug update the multi connect devices
 *	@return   0
 *	@param[in] data the pointer to the main data structure
 */
int wfd_update_multiconnect_device(struct ug_data *ugd);

/**
 *	This function let the ug create the view for multi connection
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 */
void wfd_create_multiconnect_view(struct ug_data *ugd);


#endif  /* __WFD_UG_VIEW_H__ */
