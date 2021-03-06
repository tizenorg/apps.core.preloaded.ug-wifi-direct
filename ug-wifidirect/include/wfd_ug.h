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


#ifndef __WFD_UG_H__
#define __WFD_UG_H__

#include <dlog.h>
#include <ui-gadget-module.h>
#include <tethering.h>

#define PACKAGE "ug-setting-wifidirect-efl"
#define LOCALEDIR "/opt/ug/res/locale"

#define DIRECT_TAG  "wfd_ug"
#define DBG(log_level, format, args...) \
       LOG(log_level, DIRECT_TAG, "[%s()][%d] " format, __FUNCTION__, __LINE__, ##args)

#define __FUNC_ENTER__  DBG(LOG_VERBOSE, "+\n")
#define __FUNC_EXIT__   DBG(LOG_VERBOSE, "-\n")

#define VCONF_WFD_APNAME 			"db/setting/device_name"

#define assertm_if(expr, fmt, arg...) do { \
   if(expr) { \
	  DBG(LOG_VERBOSE, " ##(%s) -> %s() assert!!## "fmt, #expr, __FUNCTION__, ##arg); \
		 assert(1); \
   } \
} while (0)			// retvm if

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

#define AP_NAME_LENGTH_MAX		32	//allowed lenght is 31 + '\0'
#define AP_PASSWORD_LENGTH_MAX	64	//allowed lenght is 63 + '\0'
#define AP_PASSWORD_LENGTH_MIN	8	//min limit
#define AP_REJECT_CHAR_LIST		"=,"

#define DEFAULT_DEV_NAME        "GT_I9500"
#define MAC_LENGTH  18
#define SSID_LENGTH 32
#define MAX_PEER_NUM 10
#define MAX_POPUP_PEER_NUM 7
#define MAX_POPUP_TEXT_SIZE 256
#define MAX_DISPLAY_TIME_OUT	30

#define _(s)        dgettext(PACKAGE, s)
#define N_(s)      dgettext_noop(s)
#define S_(s)      dgettext("sys_string", s)


#if 1	//To-Do : Text should be translated.

#define IDS_WFD_POP_SCAN_AGAIN "To start new scanning,<br>current connection will be<br>ended.Continue?"
#define IDS_WFD_POP_WARN_BUSY_DEVICE "Unavailable device. Device is connected to another device."
#define IDS_WFD_BODY_FAILED_DEVICES "Failed Devices"
 
#endif


#define WFD_GLOBALIZATION_STR_LENGTH 256

typedef enum {
	WFD_MULTI_CONNECT_MODE_NONE,
	WFD_MULTI_CONNECT_MODE_IN_PROGRESS,
	WFD_MULTI_CONNECT_MODE_COMPLETED,
} wfd_multi_connect_mode_e;

typedef enum
{
    PEER_CONN_STATUS_DISCONNECTED,
    PEER_CONN_STATUS_DISCONNECTING,
    PEER_CONN_STATUS_CONNECTING = PEER_CONN_STATUS_DISCONNECTING,
    PEER_CONN_STATUS_CONNECTED,
    PEER_CONN_STATUS_FAILED_TO_CONNECT,
    PEER_CONN_STATUS_WAIT_FOR_CONNECT,
} conn_status_e;


typedef struct
{
    char ssid[SSID_LENGTH];    // 31 + 1('\0')
    unsigned int category;
    char mac_addr[MAC_LENGTH];
    char if_addr[MAC_LENGTH];
    conn_status_e conn_status;
    bool is_group_owner;  /** Is an active P2P Group Owner */
    bool is_persistent_group_owner;  /** Is a stored Persistent GO */
    bool is_connected;  /** Is peer connected*/
    Elm_Object_Item *gl_item;
} device_type_s;

typedef struct
{
    bool dev_sel_state;
    device_type_s peer;
} wfd_multi_sel_data_s;

struct ug_data
{
    Evas_Object *base;
    ui_gadget_h ug;

    Evas_Object *win;
    Evas_Object *bg;
    Evas_Object *naviframe;
    Evas_Object *genlist;
    Evas_Object *multiconn_view_genlist;
    Evas_Object *popup;
    Evas_Object *act_popup;
    Evas_Object *warn_popup;

    Elm_Object_Item *head;
    Elm_Object_Item *scan_btn;
    Elm_Object_Item *multi_scan_btn;
    Elm_Object_Item *multi_connect_btn;

    Elm_Object_Item *nodevice_title_item;
    Elm_Object_Item *nodevice_item;
    Elm_Object_Item *nodevice_sep_low_item;

    Elm_Object_Item *about_wfd_item;
    Elm_Object_Item *about_wfdsp_sep_end_item;

    Elm_Object_Item *conn_wfd_item;

    Elm_Object_Item *conn_failed_wfd_item;
    Elm_Object_Item *conn_failed_wfd_sep_item;

    Elm_Object_Item *avlbl_wfd_item;

    Elm_Object_Item *busy_wfd_item;
    Elm_Object_Item *busy_wfd_sep_item;

    Elm_Object_Item *multi_connect_wfd_item;

    Elm_Object_Item *multi_button_item;
    Elm_Object_Item *multi_button_sep_high_item;
    Elm_Object_Item *multi_button_sep_low_item;


    Elm_Object_Item *mcview_select_all_item;
    Elm_Object_Item *mcview_title_item;
    Elm_Object_Item *mcview_nodevice_item;


    Evas_Object *multi_btn;

    int head_text_mode;

    // Raw peer data
    device_type_s raw_connected_peers[MAX_PEER_NUM];
    int raw_connected_peer_cnt;
    device_type_s raw_discovered_peers[MAX_PEER_NUM];
    int raw_discovered_peer_cnt;

    // Peer data in the Genlist
    device_type_s gl_connected_peers[MAX_PEER_NUM];
    int gl_connected_peer_cnt;

    device_type_s gl_connected_failed_peers[MAX_PEER_NUM];
    int gl_connected_failed_peer_cnt;

    device_type_s gl_available_peers[MAX_PEER_NUM];
    int gl_available_peer_cnt;

    device_type_s gl_busy_peers[MAX_PEER_NUM];
    int gl_busy_peer_cnt;

    device_type_s raw_multi_selected_peers[MAX_PEER_NUM];
    int raw_multi_selected_peer_cnt;

    device_type_s gl_multi_connect_peers[MAX_PEER_NUM];
    int gl_multi_connect_peer_cnt;

    // My status
    bool I_am_group_owner;
    bool I_am_connected;

    // Following variables are used at the Multi connect view.
    wfd_multi_connect_mode_e multi_connect_mode;
    wfd_multi_sel_data_s multi_conn_dev_list[MAX_PEER_NUM];
    int gl_available_dev_cnt_at_multiconn_view;
    int g_source_multi_connect_next;

    int wfd_onoff;
    int wfd_status;
    char *dev_name;
    char *dev_pass;

    // For connect failed peers
    int last_display_time;
    Ecore_Timer *display_timer;

    //tethering
    tethering_h hotspot_handle;
    
};

extern Elm_Gen_Item_Class sep_itc;
extern Elm_Gen_Item_Class sep_itc_end;
extern Elm_Gen_Item_Class head_itc;
extern Elm_Gen_Item_Class name_itc;
extern Elm_Gen_Item_Class title_itc;
extern Elm_Gen_Item_Class peer_itc;
extern Elm_Gen_Item_Class noitem_itc;
extern Elm_Gen_Item_Class help_itc;
extern Elm_Gen_Item_Class button_itc;

extern Elm_Gen_Item_Class title_conn_itc;
extern Elm_Gen_Item_Class peer_conn_itc;

extern Elm_Gen_Item_Class title_busy_itc;
extern Elm_Gen_Item_Class peer_busy_itc;

extern Elm_Gen_Item_Class title_multi_connect_itc;
extern Elm_Gen_Item_Class peer_multi_connect_itc;

extern Elm_Gen_Item_Class title_conn_failed_itc;
extern Elm_Gen_Item_Class peer_conn_failed_itc;


#endif  /* __WFD_UG_H__ */
