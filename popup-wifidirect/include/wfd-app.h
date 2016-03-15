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

/**
 * This file declares wifi direct application functions.
 *
 * @file    wfd-app.h
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */


#ifndef __WFD_SYS_POPAPP_MAIN_H__
#define __WFD_SYS_POPAPP_MAIN_H__

#include <wifi-direct.h>
#include <wifi-direct-internal.h>

#define PACKAGE "org.tizen.wifi-direct-popup"
#define PACKAGE_ALLSHARE_CAST "org.tizen.allshare-cast-popup"
#define EDJ_NAME RESDIR"/edje/wifi-direct-popup.edj"
#define WFD_MAX_CONNECTED_PEER	7
#define WFD_POP_STR_MAX_LEN	256
#define NO_ACTION_TIME_OUT	300            /*5min*/

#define NOTIFICATION_BUNDLE_PARAM "NotiType"
#define NOTIFICATION_BUNDLE_VALUE "WiFi-Direct"

#define LOCALE_FILE_NAME "ug-setting-wifidirect-efl"
#define LOCALEDIR "/usr/ug/res/locale"
#define WFD_NOTI_ICON_PATH "/usr/apps/org.tizen.quickpanel/shared/res/noti_icons/Wi-Fi/noti_wifi_direct_auto_off.png"
#define WFD_INDICATOR_ICON_PATH "/usr/apps/org.tizen.wifi-direct-popup/res/images/B03_wi-fi_direct_on_connected.png"
#define WFD_ACTIVATED_NOTI_ICON_PATH "/usr/apps/org.tizen.wifi-direct-popup/res/images/B03_wi-fi_direct_on_not_connected.png"
#define WFD_EDJ_POPUP_PATH "/usr/apps/org.tizen.wifi-direct-popup/res/edje/wfd_popup.edj"
#define SCREEN_MIRRIONG_INDICATOR_ICON_PATH "/usr/apps/org.tizen.wifi-direct-popup/res/images/B03_event_screen_mirroring.png"
#define SCREEN_MIRRIONG_INDICATOR_PLAY_ICON_PATH "/usr/apps/org.tizen.wifi-direct-popup/res/images/B03_event_screen_mirroring_play.png"
#define SCREEN_MIRRIONG_NOTI_ICON_PATH "/usr/apps/org.tizen.wifi-direct-popup/res/images/ug-setting-allshare-cast-efl.png"

#define D_(s)        dgettext(LOCALE_FILE_NAME, s)
#define N_(s)      dgettext_noop(s)
#define S_(s)      dgettext("sys_string", s)

#define MAX_NO_ACTION_TIME_OUT	300            /*5min*/
#define MAX_POPUP_TEXT_SIZE 256
#define DEV_NAME_LENGTH 32
#define MACSTR_LENGTH 17
#define PIN_LENGTH 64

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

enum {
	WFD_POP_TIMER_3 = 3,
	WFD_POP_TIMER_10 = 10,
	WFD_POP_TIMER_30 = 30,
	WFD_POP_TIMER_120 = 120,
};

enum {
	WFD_POP_APRV_CONNECTION_WPS_PUSHBUTTON_REQ,
	WFD_POP_APRV_CONNECTION_WPS_DISPLAY_REQ,
	WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ,

	WFD_POP_PROG_CONNECT,
	WFD_POP_PROG_DISCONNECT,
	WFD_POP_PROG_CONNECT_CANCEL,
	WFD_POP_PROG_CONNECT_WITH_PIN,
	WFD_POP_PROG_CONNECT_WITH_KEYPAD,
	WFD_POP_NOTI_DISCONNECTED,

	WFD_POP_FAIL_INIT,
};

enum {
	WFD_POP_RESP_OK,
	WFD_POP_RESP_CANCEL,
	WFD_POP_RESP_APRV_CONNECT_PBC_YES = 1,
	WFD_POP_RESP_APRV_CONNECT_DISPLAY_OK,
	WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES,
	WFD_POP_RESP_APRV_CONNECT_NO,
	WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK,
	WFD_POP_RESP_APRV_ENTER_PIN_YES,
	WFD_POP_RESP_APRV_ENTER_PIN_NO,
};

#ifdef WFD_SCREEN_MIRRORING_ENABLED
enum {
	WFD_POP_SCREEN_MIRROR_NONE,
	WFD_POP_SCREEN_MIRROR_DISCONNECT_BY_RECONNECT_WIFI_AP,
};
#endif

typedef struct {
	int type;
	char text[WFD_POP_STR_MAX_LEN];
	char label1[WFD_POP_STR_MAX_LEN];
	char label2[WFD_POP_STR_MAX_LEN];
	int timeout;
	int resp_data1;
	int resp_data2;
	int data;
} wfd_popup_t;

typedef struct {
	char ssid[32];
	char mac_address[18];
	unsigned int category;
	bool is_miracast_device;
} wfd_device_info_t;

typedef struct {
	int status;
	char peer_name[DEV_NAME_LENGTH+1];
	char peer_addr[MACSTR_LENGTH+1];
	unsigned int device_type;
	int wifi_display;
	wifi_direct_wps_type_e wps_type;
	char wps_pin[PIN_LENGTH+1];
	bool auto_conn;
} wfd_connection_info_s;

typedef struct {
	Evas_Object *win;
	Evas_Object *popup;
	Evas_Object *pin_entry;
	Evas_Object *conformant;
	Evas_Object *layout;
	Evas_Object *back_grnd;
	wfd_popup_t *popup_data;
	uint popup_timeout_handle;

	wfd_connection_info_s *connection;

	/* notification */
#ifdef NOT_CONNECTED_INDICATOR_ICON
	notification_h noti_wifi_direct_on;
#endif
	notification_h noti_wifi_direct_connected;
	notification_h noti_screen_mirroring_on;
	notification_h noti_screen_mirroring_play;
	wfd_device_info_t raw_connected_peers[WFD_MAX_CONNECTED_PEER];
	int raw_connected_peer_cnt;

	/* Transmit timer */
	wifi_direct_state_e wfd_status;
	int last_wfd_transmit_time;
	Ecore_Timer *transmit_timer;

	/* auto deactivation after 5 mins if not connected*/
#ifdef WFD_FIVE_MIN_IDLE_DEACTIVATION
	Ecore_Timer *monitor_timer;
#endif
	wifi_direct_state_e last_wfd_status;
	int last_wfd_time;

#ifdef WFD_SCREEN_MIRRORING_ENABLED
	int screen_mirroring_state;
#endif
	int timeout;

	Elm_Genlist_Item_Class *pin_entry_itc;
	Elm_Genlist_Item_Class *pin_desc_itc;
	Elm_Genlist_Item_Class *paswd_itc;

	/*Mac address for connecting device*/
	char mac_addr_connecting[MACSTR_LENGTH];

	/* Down Key Press Handler */
	Ecore_Event_Handler *downkey_handler;
} wfd_appdata_t;

typedef struct {
	int step;
	Evas_Object *progressbar;
	Evas_Object *time;
} wfd_wps_display_popup_t;

extern wfd_appdata_t *wfd_get_appdata();

/**
 *	This function let the app do initialization
 *	@return   If success, return TRUE, else return FALSE
 *	@param[in] ad the pointer to the main data structure
 */
bool init_wfd_client(wfd_appdata_t *ad);

/**
 *	This function let the app do de-initialization
 *	@return   If success, return TRUE, else return FALSE
 *	@param[in] ad the pointer to the main data structure
 */
int deinit_wfd_client(wfd_appdata_t *ad);

void wfd_app_util_del_notification(wfd_appdata_t *ad);
Eina_Bool wfd_automatic_deactivated_for_connection_cb(void *user_data);
int wfd_app_util_deregister_hard_key_down_cb(void *data);
int wfd_app_get_connected_peers(void *user_data);
int wfd_app_client_switch_off(void *data);

/**
 *	This function let the app destroy the popup
 *	@return   void
 *	@param[in] null
 */
extern void wfd_destroy_popup();

/**
 *	This function let the app create a popup
 *	@return   void
 *	@param[in] type the type of popup
 *	@param[in] userdata the pointer to the data which will be used
 */
extern void wfd_prepare_popup(int type, void *userdata);

#endif                          /* __WFD_SYS_POPAPP_MAIN_H__ */
