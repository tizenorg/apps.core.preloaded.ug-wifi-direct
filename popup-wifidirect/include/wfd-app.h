/*
 * Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of org.tizen.wifi-direct-popup
 * Written by Sungsik Jang <sungsik.jang@samsung.com>
 *            Dongwook Lee <dwmax.lee@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of
 * SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with SAMSUNG ELECTRONICS. 
 *
 * SAMSUNG make no representations or warranties about the suitability
 * of the software, either express or implied, including but not limited
 * to the implied warranties of merchantability, fitness for a particular
 * purpose, or non-infringement. SAMSUNG shall not be liable for any
 * damages suffered by licensee as a result of using, modifying or
 * distributing this software or its derivatives.
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

#define PACKAGE "wifi-direct-popup"
#define EDJ_NAME RESDIR"/edje/wifi-direct-popup.edj"
#define LOCALEDIR "/opt/apps/org.tizen.wifi-direct-popup/res/locale"
#define WFD_POP_STR_MAX_LEN		64

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
} wfd_appdata_t;


extern wfd_appdata_t *wfd_get_appdata();
extern void wfd_destroy_popup();
extern void wfd_prepare_popup(int type, void *userdata);

#endif                          /* __WFD_SYS_POPAPP_MAIN_H__ */
