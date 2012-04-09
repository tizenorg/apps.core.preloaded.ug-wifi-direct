/*
 * Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of ug-setting-wifidirect-efl
 * Written by Gibyoung Kim <lastkgb.kim@samsung.com>
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


#ifndef __WFD_UG_H__
#define __WFD_UG_H__

#include <dlog.h>

#define PACKAGE "ug-setting-wifidirect-efl"
#define LOCALEDIR "/opt/ug/res/locale"

#define DIRECT_TAG  "wfd_ug"
#define DBG(log_level, format, args...) \
       LOG(log_level, DIRECT_TAG, "[%s()][%d] " format, __FUNCTION__, __LINE__, ##args)

#define __FUNC_ENTER__  DBG(LOG_VERBOSE, "+\n")
#define __FUNC_EXIT__   DBG(LOG_VERBOSE, "-\n")

#define VCONF_WFD_ONOFF 			"db/wifi_direct/onoff"
#define VCONF_WFD_CONNECTION_STATUS "db/wifi_direct/connection_status"
#define VCONF_WFD_APNAME 			"db/setting/device_name"
#define VCONF_WFD_PASSWORD 			"db/mobile_hotspot/wifi_key"
#define VCONF_WFD_PREV_STATUS 		"db/wifi_direct/prev_status"

#define assertm_if(expr, fmt, arg...) do { \
   if(expr) { \
	  DBG(LOG_VERBOSE, " ##(%s) -> %s() assert!!## "fmt, #expr, __FUNCTION__, ##arg); \
		 assert(1); \
   } \
} while (0)

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

#define AP_NAME_LENGTH_MAX		32
#define AP_PASSWORD_LENGTH_MAX	64
#define AP_PASSWORD_LENGTH_MIN	8
#define AP_REJECT_CHAR_LIST		"=,"

#define DEFAULT_DEV_NAME        "Tizen"
#define MAC_LENGTH  18
#define SSID_LENGTH 32
#define MAX_PEER_NUM 10

#define _(s)        dgettext(PACKAGE, s)
#define N_(s)      dgettext_noop(s)
#define S_(s)      dgettext("sys_string", s)


typedef struct
{
    char ssid[SSID_LENGTH];
    unsigned int category;
    char mac_addr[MAC_LENGTH];
    char if_addr[MAC_LENGTH];
    int conn_status;
    Elm_Object_Item *gl_item;
} device_type_s;

struct ug_data
{
    Evas_Object *base;
    struct ui_gadget *ug;

    Evas_Object *win;
    Evas_Object *bg;
    Evas_Object *naviframe;
    Evas_Object *genlist;
    Elm_Object_Item *head;
    Elm_Object_Item *noitem;
    Elm_Object_Item *scan_btn;
    Evas_Object *popup;
    Evas_Object *act_popup;
    Evas_Object *warn_popup;

    int head_text_mode;

    device_type_s *peers;
    int peer_cnt;

    int wfd_onoff;
    int wfd_status;
    char *dev_name;
    char *dev_pass;
};


#endif                          /* __WFD_UG_H__ */
