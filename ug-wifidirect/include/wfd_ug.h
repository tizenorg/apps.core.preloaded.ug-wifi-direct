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
#include <notification.h>
#include <ui-gadget-module.h>

#include <tethering.h>

#include <wifi-direct.h>
#include <efl-assist/efl_assist.h>

#include <glib-object.h>
#include <gio/gio.h>
#include <glib.h>
#include <assert.h>

#define PACKAGE "ug-setting-wifidirect-efl"
#define LOCALEDIR "/usr/share/locale"
#define COLOR_TABLE "/usr/apps/setting-wifidirect-efl/shared/res/tables/setting-wifidirect-efl_ChangeableColorTable.xml"
#define FONT_TABLE "/usr/apps/setting-wifidirect-efl/shared/res/tables/setting-wifidirect-efl_FontInfoTable.xml"

#define DIRECT_TAG  "wfd_ug"
/* TODO:: To change the log level as LOG_INFO */
#define DBG(log_level, format, args...) \
	LOG(LOG_ERROR, DIRECT_TAG, "[%s()][%d] " format, __FUNCTION__, __LINE__, ##args)
#define DBG_SECURE(log_level, format, args...) \
	SECURE_LOG(LOG_ERROR, DIRECT_TAG, "[%s()][%d] " format, __FUNCTION__, __LINE__, ##args)

#define MAC2SECSTR(a) (a)[0], (a)[4], (a)[5]
#define MACSECSTR "%02x:%02x:%02x"
#if 0
#define IP2SECSTR(a) (a)[0], (a)[3]
#define IPSECSTR "%d..%d"
#endif

#define __FUNC_ENTER__  DBG(LOG_INFO, "+\n")
#define __FUNC_EXIT__   DBG(LOG_INFO, "-\n")
#if 0
#define __FUNC_ENTER__
#define __FUNC_EXIT__
#endif

#define VCONF_WFD_APNAME			"db/setting/device_name"

#define assertm_if(expr, fmt, arg...) do { \
	if (expr) { \
	  DBG(LOG_VERBOSE, " ##(%s) -> %s() assert!!## "fmt, #expr, __FUNCTION__, ##arg); \
		 assert(1); \
	} \
} while (0)

#define WFD_IF_DEL_OBJ(obj) \
		do { \
			if(obj) { \
				evas_object_del(obj); \
				obj = NULL; \
			} \
		} while (0)

#define WFD_IF_DEL_ITEM(obj) \
		do { \
			if(obj) { \
				elm_object_item_del(obj); \
				obj = NULL; \
			} \
		} while (0)

#define WFD_RET_IF(expr, fmt, args...) \
		do { \
			if(expr) { \
				DBG(LOG_ERROR, "[%s] Return, message "fmt, #expr, ##args );\
				return; \
			} \
		} while (0)

#define WFD_RETV_IF(expr, val, fmt, args...) \
		do { \
			if(expr) { \
				DBG(LOG_ERROR,"[%s] Return value, message "fmt, #expr, ##args );\
				return (val); \
			} \
		} while (0)

#define WFD_IF_FREE_MEM(mem) \
		do { \
			if(mem) { \
				free(mem); \
				mem = NULL; \
			} \
		} while (0)

#define AP_NAME_LENGTH_MAX		32
#define AP_PASSWORD_LENGTH_MAX	64
#define AP_PASSWORD_LENGTH_MIN	8
#define AP_REJECT_CHAR_LIST		"=,"

#define DEFAULT_DEV_NAME        "ZEQ"
#define MAC_LENGTH  18
#define SSID_LENGTH 32
#define MAX_CONNECTED_PEER_NUM 7
#define MAX_PEER_NUM 10
#define MAX_POPUP_TEXT_SIZE 256
#define MAX_DISPLAY_TIME_OUT 3
#define MAX_NO_ACTION_TIME_OUT	300            /*5min*/
#define MAX_SCAN_TIME_OUT 0

#define GENLIST_HEADER_POS 1
#define SR_CHECKBOX_ON_MSG "on/off button on"
#define SR_CHECKBOX_OFF_MSG "on/off button off"
#define SR_BUTTON_MSG "multiple connect button"

#define _(s)        dgettext(PACKAGE, s)
#define N_(s)      dgettext_noop(s)
#define S_(s)      dgettext("sys_string", s)

#define WFD_GLOBALIZATION_STR_LENGTH 256

typedef enum {
	WFD_MULTI_CONNECT_MODE_NONE,
	WFD_MULTI_CONNECT_MODE_IN_PROGRESS,
	WFD_MULTI_CONNECT_MODE_COMPLETED,
} wfd_multi_connect_mode_e;

typedef enum {
	PEER_CONN_STATUS_DISCONNECTED,
	PEER_CONN_STATUS_DISCONNECTING,
	PEER_CONN_STATUS_CONNECTING = PEER_CONN_STATUS_DISCONNECTING,
	PEER_CONN_STATUS_CONNECTED,
	PEER_CONN_STATUS_FAILED_TO_CONNECT,
	PEER_CONN_STATUS_WAIT_FOR_CONNECT,
} conn_status_e;

typedef enum {
	WIFI_DIRECT_DISCOVERY_NONE,
	WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START,
	WIFI_DIRECT_DISCOVERY_FULL_SCAN_START,
	WIFI_DIRECT_DISCOVERY_STOPPED,
	WIFI_DIRECT_DISCOVERY_BACKGROUND,
} discovery_status_e;


typedef struct device_type_s_{
	char ssid[SSID_LENGTH];
	unsigned int category;
	unsigned int sub_category;
	char mac_addr[MAC_LENGTH];
	char if_addr[MAC_LENGTH];
	conn_status_e conn_status;
	bool is_group_owner;  /** Is an active P2P Group Owner */
	bool is_persistent_group_owner;  /** Is a stored Persistent GO */
	bool is_connected;  /** Is peer connected*/
	bool is_alive;
	bool dev_sel_state;
	Elm_Object_Item *gl_item;
	struct device_type_s_ *next;
} device_type_s;

struct ug_data {
	Evas_Object *base;
	ui_gadget_h ug;

	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *layout;
	Evas_Object *naviframe;
	Elm_Object_Item *navi_item;
	Elm_Object_Item *multi_navi_item;
	Elm_Object_Item *head;
	Evas_Object *genlist;
	Evas_Object *multiconn_view_genlist;
	Evas_Object *multiconn_layout;
	Evas_Object *popup;
	Evas_Object *act_popup;
	Evas_Object *warn_popup;
	Evas_Object *rename_popup;
	Evas_Object *ctxpopup;
	Evas_Object *scan_btn;
	Evas_Object *disconnect_btn;
	Evas_Object *toolbar;
	Evas_Object *rename_entry;
	Evas_Object *rename_button;

#ifdef WFD_ON_OFF_GENLIST
	Evas_Object *on_off_check;
#endif

	Evas_Object *scan_toolbar;
	Evas_Object *multiconn_scan_stop_btn;
	Evas_Object *multiconn_conn_btn;
	Evas_Object *select_all_icon;



	Elm_Object_Item *multi_connect_toolbar_item;
	Elm_Object_Item *multi_view_connect_toolbar_item;

	Elm_Object_Item *select_all_view_genlist;
#ifdef WFD_ON_OFF_GENLIST
	Elm_Object_Item *item_wifi_onoff;
#endif
	Elm_Object_Item *device_name_item;
	Elm_Genlist_Item_Class *rename_entry_itc;
	Elm_Genlist_Item_Class *rename_desc_itc;
	Elm_Object_Item *multi_connect_sep_item;

	Elm_Object_Item *nodevice_title_item;
	Elm_Object_Item *nodevice_item;

	Elm_Object_Item *conn_wfd_item;
	Elm_Object_Item *conn_failed_wfd_item;
	Elm_Object_Item *avlbl_wfd_item;
	Elm_Object_Item *busy_wfd_item;
	Elm_Object_Item *multi_connect_wfd_item;

	Elm_Object_Item *mcview_title_item;
	Elm_Object_Item *mcview_nodevice_item;
	Elm_Object_Item *more_btn_multiconnect_item;

	Evas_Object *back_btn;

	// Notify
	Evas_Object *notify;
	Evas_Object *notify_layout;

	int head_text_mode;

	char *mac_addr_connecting;
	char *mac_addr_req;

	//Connection is incoming or not
	bool is_conn_incoming;


	// title mode of device list
	int title_content_mode;

	// Raw peer data
	device_type_s raw_connected_peers[MAX_CONNECTED_PEER_NUM];
	int raw_connected_peer_cnt;
	GList *raw_discovered_peer_list;
	int raw_discovered_peer_cnt;

	// Peer data in the Genlist
	int gl_connected_peer_cnt;
	device_type_s *gl_conn_peers_start;

	int gl_connected_failed_peer_cnt;
	device_type_s *gl_failed_peers_start;

	int gl_available_peer_cnt;
	device_type_s *gl_avlb_peers_start;

	int gl_busy_peer_cnt;
	device_type_s *gl_busy_peers_start;

	device_type_s raw_multi_selected_peers[MAX_PEER_NUM];
	int raw_multi_selected_peer_cnt;

	int gl_multi_connect_peer_cnt;
	device_type_s *gl_mul_conn_peers_start;

	// My status
	bool I_am_group_owner;
	bool I_am_connected;

	// Following variables are used at the Multi connect view.
	wfd_multi_connect_mode_e multi_connect_mode;
	device_type_s *multi_conn_dev_list_start;
	int gl_available_dev_cnt_at_multiconn_view;
	int g_source_multi_connect_next;

#ifdef WFD_ON_OFF_GENLIST
	int wfd_onoff;
#endif
	wifi_direct_state_e wfd_status;
	char *dev_name;
	char *dev_pass;

	// For connect failed peers
	int last_display_time;
	Ecore_Timer *display_timer;

	// Tethering
	bool is_hotspot_off;
	bool is_hotspot_locally_disabled;
	tethering_h hotspot_handle;

	// wfds service
	char *wfds;

	// Device filter
	int device_filter;

	// Whether support auto exit after successed connection
	bool is_auto_exit;

	//Whether support multi connection
	bool is_multi_connect;
	bool is_select_all_checked;

	//view type for search
	char *view_type;

	//Title of UG
	char *title;

	// The ip address of connected peer
	char *peer_ip_address;

	// The service name that supported WFDSP
	char *service_name;

	// Whether initialize wfd-namager ok
	bool is_init_ok;

	//timer for deleting progress bar
	int timer_stop_progress_bar;

	//timer for multi connect reset
	int timer_multi_reset;

	//if all the items are selected or not
	bool is_multi_check_all_selected;

	//timer for remove not alive peer
	int timer_delete_not_alive_peer;

	//wifi direct discovery status
	int wfd_discovery_status;

	bool is_paused;

#ifdef WFD_DBUS_LAUNCH
	GCancellable *dbus_cancellable;
	GDBusConnection *conn;
#endif
#ifdef MOTION_CONTROL_ENABLE
	void *motion_handle
#endif
};

extern Elm_Gen_Item_Class device_name_title_itc;
#ifdef WFD_ON_OFF_GENLIST
extern Elm_Gen_Item_Class wfd_onoff_itc;
#endif
extern Elm_Gen_Item_Class device_name_itc;
extern Elm_Gen_Item_Class title_itc;
extern Elm_Gen_Item_Class multi_view_title_itc;
extern Elm_Gen_Item_Class peer_itc;
extern Elm_Gen_Item_Class title_no_device_itc;
extern Elm_Gen_Item_Class noitem_itc;
extern Elm_Gen_Item_Class title_available_itc;

extern Elm_Gen_Item_Class title_conn_itc;
extern Elm_Gen_Item_Class peer_conn_itc;

extern Elm_Gen_Item_Class title_busy_itc;
extern Elm_Gen_Item_Class peer_busy_itc;

extern Elm_Gen_Item_Class title_multi_connect_itc;
extern Elm_Gen_Item_Class peer_multi_connect_itc;
extern Elm_Gen_Item_Class select_all_multi_connect_itc;

extern Elm_Gen_Item_Class title_conn_failed_itc;
extern Elm_Gen_Item_Class peer_conn_failed_itc;

/**
 *	This function let the ug destroy the ug
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_destroy_ug(void *data);

/**
 *	This function is called when ON/OFF button is turned ON/OFF
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] object
 *	@param[in] event
 */
void _onoff_changed_cb(void *data, Evas_Object *obj, void *event_info);
void toolbar_language_changed(void *data, Evas_Object *obj, void *event_info);
void ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
void wfd_free_nodivice_item(struct ug_data *ugd);
void discover_cb(int error_code, wifi_direct_discovery_state_e discovery_state, void *user_data);


#endif  /* __WFD_UG_H__ */
