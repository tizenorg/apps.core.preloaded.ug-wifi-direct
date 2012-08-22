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

#include <appcore-efl.h>
#include <Ecore_X.h>
#include <Elementary.h>
#include <appsvc.h>
#include <aul.h>
#include <notification.h>
#include <syspopup_caller.h>

#define PACKAGE "org.tizen.wifi-direct-popup"
#define EDJ_NAME RESDIR"/edje/wifi-direct-popup.edj"
#define WFD_MAX_PEER_NUM	10
#define WFD_POP_STR_MAX_LEN	256
#define NO_ACTION_TIME_OUT	300            /*5min*/

#define NOTIFICATION_BUNDLE_PARAM "NotiType"
#define NOTIFICATION_BUNDLE_VALUE "WiFi-Direct"
#define TICKERNOTI_SYSPOPUP "tickernoti-syspopup"


#define LOCALE_FILE_NAME "wifi-direct-popup"
#define LOCALEDIR "/opt/apps/org.tizen.wifi-direct-popup/res/locale"

#define _(s)        dgettext(LOCALE_FILE_NAME, s)
#define N_(s)      dgettext_noop(s)
#define S_(s)      dgettext("sys_string", s)



enum
{
    WFD_POP_TIMER_3 = 3,
    WFD_POP_TIMER_10 = 10,
    WFD_POP_TIMER_30 = 30,
    WFD_POP_TIMER_120 = 120,
};

enum
{
    WFD_POP_APRV_CONNECTION_WPS_PUSHBUTTON_REQ,
    WFD_POP_APRV_CONNECTION_WPS_DISPLAY_REQ,
    WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ,

    WFD_POP_PROG_CONNECT,
    WFD_POP_PROG_DISCONNECT,
    WFD_POP_PROG_CONNECT_CANCEL,
    WFD_POP_PROG_CONNECT_WITH_PIN,
    WFD_POP_PROG_CONNECT_WITH_KEYPAD,
    WFD_POP_NOTI_CONNECTED,
    WFD_POP_NOTI_DISCONNECTED,

    WFD_POP_FAIL_INIT,
    WFD_POP_FAIL_CONNECT,
    WFD_POP_INCORRECT_PIN,

    //Automatic turn off wfd
    WFD_POP_AUTOMATIC_TURN_OFF,

};

enum
{
    WFD_POP_RESP_OK,
    WFD_POP_RESP_CANCEL,
    WFD_POP_RESP_APRV_CONNECT_PBC_YES = 1,
    WFD_POP_RESP_APRV_CONNECT_DISPLAY_YES,
    WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES,
    WFD_POP_RESP_APRV_CONNECT_NO,
    WFD_POP_RESP_PROG_CONNECT_CANCEL,
    WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK,
    WFD_POP_RESP_APRV_ENTER_PIN_YES,
    WFD_POP_RESP_APRV_ENTER_PIN_NO,
    WFD_POP_RESP_AUTOMATIC_TURNOFF_OK,
};

typedef struct
{
    int type;
    char text[WFD_POP_STR_MAX_LEN];
    char label1[WFD_POP_STR_MAX_LEN];
    char label2[WFD_POP_STR_MAX_LEN];
    int timeout;
    int resp_data1;
    int resp_data2;
    int data;
} wfd_popup_t;

typedef struct
{
    char ssid[32];
    char mac_address[18];
} wfd_device_info_t;

typedef struct
{
    Evas_Object *win;
    Evas_Object *popup;
    Evas_Object *pin_entry;
    wfd_popup_t *popup_data;
    uint popup_timeout_handle;
    char pin_number[32];
    char peer_mac[18];
    char peer_name[32];
    wfd_device_info_t *discovered_peers;
    int discovered_peer_count;

    /* notification */
    notification_h noti;
    wfd_device_info_t raw_connected_peers[WFD_MAX_PEER_NUM];
    int raw_connected_peer_cnt;

    /* used for automatic turn off */
    int wfd_status;
    int last_wfd_status;
    int last_wfd_time;
    int last_wfd_transmit_time;
    Ecore_Timer *monitor_timer;
    Ecore_Timer *transmit_timer;
} wfd_appdata_t;

typedef struct
{
    int step;
    Evas_Object *progressbar;
    Evas_Object *time;
} wfd_wps_display_popup_t;

extern wfd_appdata_t *wfd_get_appdata();
extern void wfd_destroy_popup();
extern void wfd_prepare_popup(int type, void *userdata);
extern void wfd_tickernoti_popup(char *msg);

#endif                          /* __WFD_SYS_POPAPP_MAIN_H__ */
