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

#define WFD_ICON_DEVICE_COMPUTER 			WFD_IMG_DIR"/A09_device_computer.png"
#define WFD_ICON_DEVICE_INPUT_DEVICE			WFD_IMG_DIR"/A09_device_input_device.png"
#define WFD_ICON_DEVICE_PRINTER 			WFD_IMG_DIR"/A09_device_printer.png"
#define WFD_ICON_DEVICE_CAMERA 				WFD_IMG_DIR"/A09_device_camera.png"
#define WFD_ICON_DEVICE_STORAGE				WFD_IMG_DIR"/A09_device_storage.png"
#define WFD_ICON_DEVICE_NETWORK_INFRA			WFD_IMG_DIR"/A09_device_network_infrastructure.png"
#define WFD_ICON_DEVICE_DISPLAY				WFD_IMG_DIR"/A09_device_display.png"
#define WFD_ICON_DEVICE_MULTIMEDIA_DEVICE 		WFD_IMG_DIR"/A09_device_multimedia_devices.png"
#define WFD_ICON_DEVICE_GAMING_DEVICE			WFD_IMG_DIR"/A09_device_gaming_devices.png"
#define WFD_ICON_DEVICE_TELEPHONE 			WFD_IMG_DIR"/A09_device_telephone.png"
#define WFD_ICON_DEVICE_AUDIO_DEVICE			WFD_IMG_DIR"/A09_device_audio_devices.png"

#define WFD_ICON_DEVICE_COMPUTER_CONNECT 		WFD_IMG_DIR"/A09_device_computer_connect.png"
#define WFD_ICON_DEVICE_INPUT_DEVICE_CONNECT		WFD_IMG_DIR"/A09_device_input_device_connect.png"
#define WFD_ICON_DEVICE_PRINTER_CONNECT 		WFD_IMG_DIR"/A09_device_printer_connect.png"
#define WFD_ICON_DEVICE_CAMERA_CONNECT 			WFD_IMG_DIR"/A09_device_camera_connect.png"
#define WFD_ICON_DEVICE_STORAGE_CONNECT			WFD_IMG_DIR"/A09_device_storage_connect.png"
#define WFD_ICON_DEVICE_NETWORK_INFRA_CONNECT		WFD_IMG_DIR"/A09_device_network_infrastructure_connect.png"
#define WFD_ICON_DEVICE_DISPLAY_CONNECT			WFD_IMG_DIR"/A09_device_display_connect.png"
#define WFD_ICON_DEVICE_MULTIMEDIA_DEVICE_CONNECT 	WFD_IMG_DIR"/A09_device_multimedia_devices_connect.png"
#define WFD_ICON_DEVICE_GAMING_DEVICE_CONNECT		WFD_IMG_DIR"/A09_device_gaming_devices_connect.png"
#define WFD_ICON_DEVICE_TELEPHONE_CONNECT 		WFD_IMG_DIR"/A09_device_telephone_connect.png"
#define WFD_ICON_DEVICE_AUDIO_DEVICE_CONNECT		WFD_IMG_DIR"/A09_device_audio_devices_connect.png"

#define WFD_ICON_CONNECTED				WFD_IMG_DIR"/A09_Connect.png"


enum
{
    HEAD_TEXT_TYPE_DIRECT,
    HEAD_TEXT_TYPE_DEACTIVATING,
    HEAD_TEXT_TYPE_ACTIVATING,
    HEAD_TEXT_TYPE_ACTIVATED,
    HEAD_TEXT_TYPE_SCANING,
};

enum
{
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
};

struct ug_data *wfd_get_ug_data();
void create_wfd_ug_view(void *data);
void destroy_wfd_ug_view(void *data);
void wfd_ug_view_refresh_glitem(void *obj);
void wfd_ug_view_refresh_button(void *obj, const char *text, int enable);
void wfd_ug_view_update_peers(void *data);
void wfd_ug_view_free_peers(void *data);
void wfd_ug_act_popup(void *data, const char *message, int popup_type);
void wfg_ug_act_popup_remove(void *data);
void wfd_ug_warn_popup(void *data, const char *message, int popup_type);
void wfg_ug_warn_popup_remove(void *data);
int _create_connected_dev_list(void *data);
int _change_multi_button_title(void *data);

void _wifid_create_about_view(struct ug_data *ugd);
void _wifid_create_multiconnect_view(struct ug_data *ugd);

void _sub_view_back_btn_cb(void *data, Evas_Object * obj, void *event_info);
void _back_btn_cb(void *data, Evas_Object * obj, void *event_info);
void _scan_btn_cb(void *data, Evas_Object * obj, void *event_info);

void _wfd_onoff_btn_cb(void *data, Evas_Object *obj, void *event_info);
void _wifid_create_multibutton_cb(void *data, Evas_Object * obj, void *event_info);

int wfd_ug_get_discovered_peers(struct ug_data *ugd);
int wfd_ug_get_connected_peers(struct ug_data *ugd);
int wfd_refresh_wifi_direct_state(void* data);

int wfd_multi_connect_next(void* data);
int wfd_stop_multi_connect(void *data);

gboolean wfd_multi_connect_next_cb(void* data);

void wfd_ug_tickernoti_popup(char *msg);

#endif  /* __WFD_UG_VIEW_H__ */
