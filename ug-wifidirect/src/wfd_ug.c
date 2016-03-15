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


#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif


#include <sys/time.h>
#include <libintl.h>
#include <sys/utsname.h>

#include <vconf.h>
#include <Elementary.h>
#include <ui-gadget-module.h>
#include <app_control.h>

#include <setting-cfg.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"
#ifdef MOTION_CONTROL_ENABLE
#include "wfd_motion_control.h"
#endif

void initialize_gen_item_class();

struct ug_data *global_ugd = NULL;

struct ug_data *wfd_get_ug_data()
{
	return global_ugd;
}

static void __wfd_main_vconf_change_cb(keynode_t *key, void *data)
{
	__FUNC_ENTER__;
	WFD_RET_IF(NULL == key, "ERROR : key is NULL !!\n");
	WFD_RET_IF(NULL == data, "ERROR : data is NULL");
	struct ug_data *ugd = (struct ug_data *) data;

	char *vconf_name = vconf_keynode_get_name(key);

	if (!g_strcmp0(vconf_name, VCONFKEY_SETAPPL_DEVICE_NAME_STR)){
		char *name_value = NULL;
		name_value = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);
		WFD_RET_IF (!name_value, "Get string is failed");
		DBG(LOG_INFO,"name : %s", name_value);

		if (ugd->device_name_item && g_strcmp0(ugd->dev_name, name_value))
			wfd_ug_view_refresh_glitem(ugd->device_name_item);

		free(name_value);
	} else {
		DBG(LOG_ERROR, "vconf_name is error");
	}
	__FUNC_EXIT__;
}

#ifdef WIFI_STATE_CB
static void _wifi_on_state_cb(keynode_t *key, void *data)
{
	WFD_RET_IF(NULL == key, "ERROR : key is NULL !!\n");
	WFD_RET_IF(NULL == data, "ERROR : data is NULL");

	struct ug_data *ugd = (struct ug_data *)data;
	int wifi_state = 0;

	if (vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state) < 0) {
		DBG(LOG_ERROR, "Failed to get vconf VCONFKEY_WIFI_STATE\n");
		return;
	}

	DBG(LOG_INFO, "WiFi State [%d]\n", wifi_state);
	if (wifi_state > VCONFKEY_WIFI_OFF && wifi_state < VCONFKEY_WIFI_STATE_MAX) {
		if (WIFI_DIRECT_ERROR_NONE != wifi_direct_get_state(&ugd->wfd_status)) {
			DBG(LOG_ERROR, "Failed to Get WiFi Direct State");
			return;
		}
		if (ugd->wfd_status <= WIFI_DIRECT_STATE_DEACTIVATING) {
			DBG(LOG_INFO, "Activate WiFi Direct...");
			if (FALSE != wfd_client_switch_on(ugd)) {
				DBG(LOG_ERROR, "Failed to Activate WiFi Direct");
			}
		}
	}
}
#endif


static void __wfd_hotspot_mode_vconf_change_cb(keynode_t *key, void *data)
{
	DBG(LOG_INFO, "__wfd_hotspot_mode_vconf_change_cb");
	if (NULL == key || NULL == data) {
		DBG(LOG_INFO, "Invalid parameters \n");
		return;
	}
	struct ug_data *ugd = (struct ug_data *) data;
	int hotspot_mode = 0;
	int res = 0;

	res = vconf_get_int(VCONFKEY_MOBILE_HOTSPOT_MODE, &hotspot_mode);
	if (res) {
		DBG(LOG_INFO, "Failed to get vconf value for PLMN(%d)", res);
		return;
	}
	DBG(LOG_INFO, "__wfd_hotspot_mode_vconf_change_cb mode %d", hotspot_mode);

	if (VCONFKEY_MOBILE_HOTSPOT_MODE_NONE == hotspot_mode) {
		if (NULL != ugd->act_popup) {
			evas_object_del(ugd->act_popup);
		}
#ifdef WFD_ON_OFF_GENLIST
		wfd_ug_refresh_on_off_check(ugd);
#endif
	} else if (hotspot_mode == VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI ||
				hotspot_mode == VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI_AP) {
		if (NULL != ugd->warn_popup) {
			evas_object_del(ugd->warn_popup);
			ugd->warn_popup = NULL;
		}
	}

}

/**
 *	This function let the ug create backgroud
 *	@return  backgroud
 *	@param[in] ugd the pointer to the parent object
 *	@param[in] ugd the pointer to the main data structure
 */
static Evas_Object *_create_bg(Evas_Object *parent, char *style)
{
	__FUNC_ENTER__;
	Evas_Object *bg;

	bg = elm_bg_add(parent);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_style_set(bg, style);
	evas_object_show(bg);

	__FUNC_EXIT__;
	return bg;
}

/**
 *	This function let the ug create full view
 *	@return  full view
 *	@param[in] ugd the pointer to the parent object
 *	@param[in] ugd the pointer to the main data structure
 */
static Evas_Object *_create_fullview(Evas_Object *parent, struct ug_data *ugd)
{
	__FUNC_ENTER__;
	Evas_Object *base;

	if (parent == NULL) {
		DBG(LOG_ERROR, "Incorrenct parameter");
		return NULL;
	}

	/* Create Full view */
	base = elm_layout_add(parent);
	if (!base) {
		DBG(LOG_ERROR, "Failed to add layout");
		return NULL;
	}

	elm_layout_theme_set(base, "layout", "application", "default");
	evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(base, EVAS_HINT_FILL, EVAS_HINT_FILL);

	__FUNC_EXIT__;
	return base;
}

/**
 *	This function let the ug create frame view
 *	@return  frame view
 *	@param[in] ugd the pointer to the parent object
 *	@param[in] ugd the pointer to the main data structure
 */
static Evas_Object *_create_frameview(Evas_Object *parent, struct ug_data *ugd)
{
	__FUNC_ENTER__;
	Evas_Object *base;

	if (parent == NULL) {
		DBG(LOG_ERROR, "Incorrenct parameter");
		return NULL;
	}

	/* Create Frame view */
	base = elm_layout_add(parent);
	if (!base) {
		DBG(LOG_ERROR, "Failed to add layout");
		return NULL;
	}

	elm_layout_theme_set(base, "layout", "application", "default");
	evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(base, EVAS_HINT_FILL, EVAS_HINT_FILL);

	__FUNC_EXIT__;
	return base;
}

/**
 *	This function let the ug destroy the ug
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void wfd_destroy_ug(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

#ifdef WFD_DBUS_LAUNCH
	if (ugd->dbus_cancellable != NULL) {
		g_cancellable_cancel(ugd->dbus_cancellable);
		g_object_unref(ugd->dbus_cancellable);
		ugd->dbus_cancellable = NULL;
		if (ugd->conn) {
			g_object_unref(ugd->conn);
			ugd->conn = NULL;
		}
		DBG(LOG_INFO, "Cancelled dbus call");
		return;
	} else
#endif
	{
		DBG(LOG_INFO, "dbus_cancellable is NULL");
		ug_destroy_me(ugd->ug);
	}

	__FUNC_EXIT__;
	return;
}

static void wfd_ug_layout_del_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	if (ugd == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)");
		return;
	}

	wfd_client_free_raw_discovered_peers(ugd);
	wfd_ug_view_free_peers(ugd);
	destroy_wfd_ug_view(ugd);

	__FUNC_EXIT__;
}

#ifdef WFD_DBUS_LAUNCH
/**
 *	This function let the ug initialize wfd
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] evas the pointer to the evas canvas
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _wfd_init_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	int res = -1;
	struct ug_data *ugd = (struct ug_data *)data;
	WFD_RET_IF(ugd == NULL || ugd->base == NULL, "Incorrect parameter(NULL)\n");

	evas_object_event_callback_del(ugd->base, EVAS_CALLBACK_SHOW, _wfd_init_cb);

	res = launch_wifi_direct_manager(ugd);
	if (res != 0) {
		DBG(LOG_ERROR, "Failed to launch wifi direct manager\n");
	}

	__FUNC_EXIT__;
	return;
}
#endif

static void *on_create(ui_gadget_h ug, enum ug_mode mode, app_control_h control, void *priv)
{
	__FUNC_ENTER__;
	struct ug_data *ugd;
	int wfd_status = -1;
	int ret;

	if (!ug || !priv) {
		return NULL;
	}

	ugd = priv;
	ugd->ug = ug;

	bindtextdomain(PACKAGE, LOCALEDIR);

	ugd->win = ug_get_window();
	if (!ugd->win) {
		return NULL;
	}

	/* set rotation */
	if (elm_win_wm_rotation_supported_get(ugd->win)) {
		int rots[4] = {0, 90, 180, 270};
		elm_win_wm_rotation_available_rotations_set(ugd->win, (const int *)(&rots), 4);
	}

	/* check the input parameters from app at first */
	ugd->wfds = NULL;
	ugd->device_filter = -1; /* show all devices */
	ugd->is_auto_exit = false;
	ugd->is_multi_connect = true;
	ugd->is_init_ok = false;
	ugd->title = strdup(D_("IDS_WIFI_BODY_WI_FI_DIRECT_ABB"));

	if (control) {
		char *wfds = NULL;
		char *device_filter = NULL;
		char *auto_exit = NULL;
		char *multi_connect = NULL;
		char *title = NULL;
		char* viewtype = NULL;

		/*
		* get the control name
		* default value: Wi-Fi Direct
		*/
		ret = app_control_get_extra_data(control, "wfds", &wfds);
		if (ret == APP_CONTROL_ERROR_NONE && wfds) {
			DBG_SECURE(LOG_INFO, "Wfds name: %s", wfds);
			ugd->wfds = strdup(wfds);
			WFD_IF_FREE_MEM(wfds);
		}

		ret = app_control_get_extra_data(control, "viewtype", &viewtype);
		if(ret == APP_CONTROL_ERROR_NONE && viewtype) {
			DBG(LOG_INFO, "viewtype: %s\n", viewtype);
			ugd->view_type = strdup(viewtype);
			WFD_IF_FREE_MEM(viewtype);
		}

		/*
		* get the device filter
		* default value: NULL
		*/
		ret = app_control_get_extra_data(control, "device_filter", &device_filter);
		if (ret == APP_CONTROL_ERROR_NONE && device_filter) {
			DBG(LOG_INFO, "Device filter: %s", device_filter);
			if (0 == strncmp(device_filter, "computer", 8)) {
				ugd->device_filter = WFD_DEVICE_TYPE_COMPUTER;
			} else if (0 == strncmp(device_filter, "input_device", 12)) {
				ugd->device_filter = WFD_DEVICE_TYPE_INPUT_DEVICE;
			} else if (0 == strncmp(device_filter, "printer", 6)) {
				ugd->device_filter = WFD_DEVICE_TYPE_PRINTER;
			} else if (0 == strncmp(device_filter, "camera", 6)) {
				ugd->device_filter = WFD_DEVICE_TYPE_CAMERA;
			} else if (0 == strncmp(device_filter, "storage", 7)) {
				ugd->device_filter = WFD_DEVICE_TYPE_STORAGE;
			} else if (0 == strncmp(device_filter, "network_infra", 13)) {
				ugd->device_filter = WFD_DEVICE_TYPE_NW_INFRA;
			} else if (0 == strncmp(device_filter, "display", 7)) {
				ugd->device_filter = WFD_DEVICE_TYPE_DISPLAYS;
			} else if (0 == strncmp(device_filter, "multimedia_device", 17)) {
				ugd->device_filter = WFD_DEVICE_TYPE_MM_DEVICES;
			} else if (0 == strncmp(device_filter, "game_device", 11)) {
				ugd->device_filter = WFD_DEVICE_TYPE_GAME_DEVICES;
			} else if (0 == strncmp(device_filter, "telephone", 9)) {
				ugd->device_filter = WFD_DEVICE_TYPE_TELEPHONE;
			} else if (0 == strncmp(device_filter, "audio", 5)) {
				ugd->device_filter = WFD_DEVICE_TYPE_AUDIO;
			} else {
				ugd->device_filter = WFD_DEVICE_TYPE_OTHER;
			}
			WFD_IF_FREE_MEM(device_filter);
		}

		/*
		* get whether support auto exit after connection
		* default value: false
		*/
		ret = app_control_get_extra_data(control, "auto_exit", &auto_exit);
		if (ret == APP_CONTROL_ERROR_NONE && auto_exit) {
			DBG(LOG_INFO, "Auto exit: %s", auto_exit);
			if (0 == strncmp(auto_exit, "on", 2)) {
				ugd->is_auto_exit = true;
			} else {
				ugd->is_auto_exit = false;
			}
			WFD_IF_FREE_MEM(auto_exit);
		}

		/*
		* get whether support multi connection,
		* default value: true
		*/
		ret = app_control_get_extra_data(control, "multi_connect", &multi_connect);
		if (ret == APP_CONTROL_ERROR_NONE && multi_connect) {
			DBG(LOG_INFO, "Multi connection: %s", multi_connect);
			if (0 == strncmp(multi_connect, "off", 2)) {
				ugd->is_multi_connect = false;
			} else {
				ugd->is_multi_connect = true;
			}
			WFD_IF_FREE_MEM(multi_connect);
		}

		/*
		* get the title of UG
		* default value: Wi-Fi Direct
		*/
		ret = app_control_get_extra_data(control, "title_string", &title);
		if (ret == APP_CONTROL_ERROR_NONE && title) {
			DBG(LOG_INFO, "Title of UG: %s", title);
			WFD_IF_FREE_MEM(ugd->title);
			ugd->title = strdup(title);
			WFD_IF_FREE_MEM(title);
		}
	}

	if (mode == UG_MODE_FULLVIEW) {
		ugd->base = _create_fullview(ugd->win, ugd);
	} else {
		ugd->base = _create_frameview(ugd->win, ugd);
	}

	if (ugd->base) {
		evas_object_event_callback_add(ugd->base, EVAS_CALLBACK_DEL, wfd_ug_layout_del_cb, ugd);
		ugd->bg = _create_bg(ugd->win, "group_list");
		elm_object_part_content_set(ugd->base, "elm.swallow.bg", ugd->bg);
	} else {
		DBG(LOG_ERROR, "Failed to create base layout\n");
		return NULL;
	}

	/* check status of wifi-direct from vconf */
	wfd_status = wfd_get_vconf_status();
	if (wfd_status < 0) {
		return NULL;
	}

	/* draw UI */
	initialize_gen_item_class();
	create_wfd_ug_view(ugd);
	wfd_ug_view_init_genlist(ugd, true);
#ifdef MOTION_CONTROL_ENABLE
	motion_create(ugd);
#endif

	/*
	* if not deactivated, do initialization at once;
	* otherwise, do initialization later
	*/
	if (wfd_status != VCONFKEY_WIFI_DIRECT_DEACTIVATED) {
		init_wfd_client(ugd);
	} else {
		ugd->wfd_status = WIFI_DIRECT_STATE_DEACTIVATED;

#ifdef WFD_DBUS_LAUNCH
		evas_object_event_callback_add(ugd->base, EVAS_CALLBACK_SHOW, _wfd_init_cb, ugd);
#else
		ret = init_wfd_client(ugd);
		WFD_RETV_IF(ret != 0, NULL,  "Failed to initialize WFD client library\n");

		/* Activate WiFi Direct */
		DBG(LOG_INFO, "Activating WiFi Direct...");
		if (ugd->wfd_status <= WIFI_DIRECT_STATE_DEACTIVATING) {
			ret = wfd_client_switch_on(ugd);
			WFD_RETV_IF(ret != 0, NULL, "Failed to activate WFD\n");
		}
#endif
	}
	ret = vconf_notify_key_changed(VCONFKEY_SETAPPL_DEVICE_NAME_STR,
			__wfd_main_vconf_change_cb, ugd);
	if (ret) {
		DBG(LOG_ERROR, "Failed to set vconf notification callback(VCONFKEY_SETAPPL_DEVICE_NAME_STR)");
	}

#ifdef WIFI_STATE_CB
	ret = vconf_notify_key_changed(VCONFKEY_WIFI_STATE, _wifi_on_state_cb, ugd);
	if (ret) {
		DBG(LOG_ERROR, "Failed to set vconf notification callback(VCONFKEY_WIFI_STATE)");

	}
#endif

	ret = vconf_notify_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE,
		__wfd_hotspot_mode_vconf_change_cb, ugd);
	if (ret) {
		DBG(LOG_ERROR, "Failed to set vconf notification callback(MOBILE_HOTSPOT_MODE)");
	}

	__FUNC_EXIT__;
	return ugd->base;
}

static void on_start(ui_gadget_h ug, app_control_h control, void *priv)
{
	__FUNC_ENTER__;
	struct ug_data *ugd;
	int res;

	if (!ug || !priv) {
		return;
	}

	ugd = priv;

	struct utsname kernel_info;
	res = uname(&kernel_info);
	if (res != 0) {
		DBG(LOG_ERROR, "Failed to detect target type\n");
	} else {
		DBG_SECURE(LOG_INFO, "HW ID of this device [%s]\n", kernel_info.machine);
		if (strncmp(kernel_info.machine, "arm", 3) != 0) {
			wfd_ug_warn_popup(ugd, D_("IDS_ST_POP_NOT_SUPPORTED"), POPUP_TYPE_TERMINATE_NOT_SUPPORT);
			return;
		}
	}

	__FUNC_EXIT__;
}

static void on_pause(ui_gadget_h ug, app_control_h control, void *priv)
{
	__FUNC_ENTER__;

	WFD_RET_IF(ug == NULL || priv == NULL, "The param is NULL\n");
	struct ug_data *ugd = priv;
	ugd->is_paused = true;

	wfd_refresh_wifi_direct_state(ugd);
	DBG(LOG_INFO, "on pause, wfd status: %d\n", ugd->wfd_status);

 	if ((WIFI_DIRECT_STATE_DISCOVERING == ugd->wfd_status) &&
		(WIFI_DIRECT_ERROR_NONE != wifi_direct_cancel_discovery())) {
		DBG(LOG_ERROR, "Failed to send cancel discovery state [%d]\n", ugd->wfd_status);
		__FUNC_EXIT__;
		return;
	}

	ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_STOPPED;

	__FUNC_EXIT__;
}

static void on_resume(ui_gadget_h ug, app_control_h control, void *priv)
{
	__FUNC_ENTER__;

	WFD_RET_IF(ug == NULL || priv == NULL, "The param is NULL\n");
	struct ug_data *ugd = priv;
	ugd->is_paused = false;
	int ret;
	wfd_refresh_wifi_direct_state(ugd);
	DBG(LOG_INFO, "on resume, status: %d\n", ugd->wfd_status);
	ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_STOPPED;

	elm_genlist_realized_items_update(ugd->genlist);

	if (ugd->wfd_status > WIFI_DIRECT_STATE_DEACTIVATED && ugd->wfd_status < WIFI_DIRECT_STATE_CONNECTED) {
		DBG(LOG_INFO, "Start discovery again\n");
		ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
		ret = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
		if (ret != WIFI_DIRECT_ERROR_NONE) {
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
			DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", ret);
			wifi_direct_cancel_discovery();
		}
	}

	__FUNC_EXIT__;
}

static void on_destroy(ui_gadget_h ug, app_control_h control, void *priv)
{
	__FUNC_ENTER__;

	WFD_RET_IF(ug == NULL || priv == NULL, "The param is NULL\n");

	struct ug_data *ugd = priv;
	int ret;
	WFD_RET_IF(ugd->base == NULL, "The param is NULL\n");

#ifdef MOTION_CONTROL_ENABLE
	motion_destroy();
#endif

	/* DeInit WiFi Direct */
	ret = deinit_wfd_client(ugd);
	if (ret) {
		DBG(LOG_ERROR,"Failed to DeInit WiFi Direct");
	}

	if (ugd->scan_toolbar) {
		wfd_ug_view_refresh_button(ugd->scan_toolbar,
			D_("IDS_WIFI_SK4_SCAN"), FALSE);
	}

	ret = vconf_ignore_key_changed(VCONFKEY_SETAPPL_DEVICE_NAME_STR,
			__wfd_main_vconf_change_cb);
	if (ret == -1) {
		DBG(LOG_ERROR,"Failed to ignore vconf key callback\n");
	}

#ifdef WIFI_STATE_CB
	ret = vconf_ignore_key_changed(VCONFKEY_WIFI_STATE, _wifi_on_state_cb);
	if (ret == -1) {
		DBG(LOG_ERROR,"Failed to ignore vconf key callback\n");
	}
#endif

	ret = vconf_ignore_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE,
			__wfd_hotspot_mode_vconf_change_cb);
	if (ret == -1) {
		DBG(LOG_ERROR,"Failed to ignore vconf key callback MOBILE_HOTSPOT_MODE\n");
	}

	wfd_ug_view_free_peers(ugd);
	WFD_IF_FREE_MEM(ugd->title);
	WFD_IF_FREE_MEM(ugd->wfds);
	WFD_IF_FREE_MEM(ugd->view_type);

	WFD_IF_DEL_OBJ(ugd->bg);

#ifdef WFD_DBUS_LAUNCH
	if (ugd->base)
		evas_object_event_callback_del(ugd->base, EVAS_CALLBACK_SHOW, _wfd_init_cb);
#endif
	WFD_IF_DEL_OBJ(ugd->base);
	DBG(LOG_INFO, "WFD client deregistered");

	__FUNC_EXIT__;
	return;
}

static void on_message(ui_gadget_h ug, app_control_h msg, app_control_h control, void *priv)
{
	__FUNC_ENTER__;
	char* app_msg = NULL;
	struct ug_data *ugd = priv;

	if (!ug || !priv) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	if (msg) {
		app_control_get_extra_data(msg, "msg", &app_msg);
		DBG(LOG_DEBUG, "Msg from app: %s", app_msg);

		if (!strcmp(app_msg, "destroy")) {
			if(!ugd->rename_popup) {
				DBG(LOG_INFO, "Destroying UG.");
				wfd_ug_view_free_peers(ugd);
				wfd_destroy_ug(ugd);
			}
		}
		WFD_IF_FREE_MEM(app_msg);
	}
	__FUNC_EXIT__;
}

static void on_event(ui_gadget_h ug, enum ug_event event, app_control_h control, void *priv)
{
	__FUNC_ENTER__;

	if (!ug || !priv) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		DBG(LOG_INFO, "UG_EVENT_LOW_MEMORY\n");
		break;
	case UG_EVENT_LOW_BATTERY:
		DBG(LOG_INFO, "UG_EVENT_LOW_BATTERY\n");
		break;
	case UG_EVENT_LANG_CHANGE:
		DBG(LOG_INFO, "UG_EVENT_LANG_CHANGE\n");
		break;
	case UG_EVENT_ROTATE_PORTRAIT:
		_ctxpopup_move();
		DBG(LOG_INFO, "UG_EVENT_ROTATE_PORTRAIT\n");
		break;
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		DBG(LOG_INFO, "UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN\n");
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
		_ctxpopup_move();
		DBG(LOG_INFO, "UG_EVENT_ROTATE_LANDSCAPE\n");
		break;
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		_ctxpopup_move();
		DBG(LOG_INFO, "UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN\n");
		break;
	default:
		DBG(LOG_INFO, "default\n");
		break;
	}

	__FUNC_EXIT__;
}

static void on_key_event(ui_gadget_h ug, enum ug_key_event event, app_control_h control, void *priv)
{
	__FUNC_ENTER__;

	if (!ug || !priv) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	switch (event) {
	case UG_KEY_EVENT_END:
		DBG(LOG_INFO, "UG_KEY_EVENT_END\n");
		break;
	default:
		break;
	}

	__FUNC_EXIT__;
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	__FUNC_ENTER__;
	struct ug_data *ugd;

	if (!ops) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return -1;
	}

	ugd = calloc(1, sizeof(struct ug_data));
	if (ugd == NULL) {
		DBG(LOG_ERROR, "Failed to allocate memory for UG data\n");
		return -1;
	}

	global_ugd = ugd;

	ops->create = on_create;
	ops->start = on_start;
	ops->pause = on_pause;
	ops->resume = on_resume;
	ops->destroy = on_destroy;
	ops->message = on_message;
	ops->event = on_event;
	ops->key_event = on_key_event;
	ops->priv = ugd;
	ops->opt = UG_OPT_INDICATOR_ENABLE;

	__FUNC_EXIT__;
	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	__FUNC_ENTER__;

	struct ug_data *ugd;

	if (!ops) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	ugd = ops->priv;

	WFD_IF_FREE_MEM(ugd);

	__FUNC_EXIT__;
}

UG_MODULE_API int setting_plugin_reset(app_control_h control, void *priv)
{
	__FUNC_ENTER__;
	int res = -1;
	wifi_direct_state_e state;

	res = wifi_direct_initialize();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to initialize wifi direct. [%d]\n", res);
		return -1;
	}

	res = wifi_direct_get_state(&state);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to get link status. [%d]\n", res);
		return -1;
	}

	if (state < WIFI_DIRECT_STATE_ACTIVATING) {
		DBG(LOG_INFO, "No need to reset Wi-Fi Direct.\n");
	} else {
		/*if connected, disconnect all devices*/
		if (WIFI_DIRECT_STATE_CONNECTED == state) {
			res = wifi_direct_disconnect_all();
			if (res != WIFI_DIRECT_ERROR_NONE) {
				DBG(LOG_ERROR, "Failed to send disconnection request to all. [%d]\n", res);
				return -1;
			}
		}

		res = wifi_direct_deactivate();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to reset Wi-Fi Direct. [%d]\n", res);
			return -1;
		}
	}

	res = wifi_direct_deinitialize();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to deinitialize wifi direct. [%d]\n", res);
		return -1;
	}

	__FUNC_EXIT__;
	return 0;
}

UG_MODULE_API int setting_plugin_search_init(app_control_h control, void *priv, char** applocale)
{
	__FUNC_ENTER__;

	*applocale = strdup(PACKAGE);
	void *node = NULL;

	Eina_List **pplist = (Eina_List**)priv;

	node = setting_plugin_search_item_add("IDS_WIFI_BUTTON_MULTI_CONNECT", "viewtype:IDS_WIFI_BUTTON_MULTI_CONNECT", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);

	 __FUNC_EXIT__;
	return 0;
}

