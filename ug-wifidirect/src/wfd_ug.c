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
#include <wifi-direct.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

void initialize_gen_item_class();

struct ug_data *global_ugd = NULL;

struct ug_data *wfd_get_ug_data()
{
	return global_ugd;
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
	elm_win_resize_object_add(parent, bg);
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
 *	This function let the ug destroy the main view
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
void destroy_wfd_ug_view(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	if (ugd->genlist) {
		evas_object_del(ugd->genlist);
		ugd->genlist = NULL;
	}

	if (ugd->naviframe) {
		evas_object_del(ugd->naviframe);
		ugd->naviframe = NULL;
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug initialize wfd when timeout
 *	@return   if stop the timer, return false, else return true
 *	@param[in] data the pointer to the main data structure
 */
gboolean _wfd_init_cb(void *data)
{
	__FUNC_ENTER__;
	int res = -1;
	struct ug_data *ugd = (struct ug_data *) data;

	if (ugd->wfd_status == WIFI_DIRECT_STATE_DEACTIVATED) {
		res = init_wfd_client(ugd);
		if (res != 0) {
			DBG(LOG_ERROR, "Failed to initialize WFD client library\n");
			return true;
		}
	}

	__FUNC_EXIT__;
	return false;
}

static void *on_create(ui_gadget_h ug, enum ug_mode mode, service_h service, void *priv)
{
	__FUNC_ENTER__;
	struct ug_data *ugd;
	int res = 0;

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

	if (mode == UG_MODE_FULLVIEW) {
		ugd->base = _create_fullview(ugd->win, ugd);
	} else {
		ugd->base = _create_frameview(ugd->win, ugd);
	}

	if (ugd->base) {
		ugd->bg = _create_bg(ugd->win, "group_list");
		elm_object_part_content_set(ugd->base, "elm.swallow.bg", ugd->bg);
	} else {
		DBG(LOG_ERROR, "Failed to create base layout\n");
		return NULL;
	}

	/* check status of wifi-direct from vconf */
	wfd_get_vconf_status(ugd);

	/*
	* if not deactivated, do initialization at once;
	* otherwise, do initialization later
	*/
	if (ugd->wfd_status > WIFI_DIRECT_STATE_DEACTIVATED || service) {
		res = init_wfd_client(ugd);
		if (res != 0) {
			DBG(LOG_ERROR, "Failed to initialize WFD client library\n");
		}

		if (service) {
			int status = 0;
			char *data = NULL;

			/* get the status from appsvc */
			service_get_extra_data(service, "status", &data);
			if (data) {
				status = atoi(data);
			}

			/*
			* status -
			* 0 : No operation,
			* 1 : Activate ,
			* 2 : Deactivate
			*/
			if (status == 0x01) {
				wfd_client_switch_on(ugd);
			} else if (status == 0x02) {
				wfd_client_switch_off(ugd);
			}
		}
	} else {
		g_timeout_add(100, _wfd_init_cb, ugd);
	}

	if (ugd->wfd_status >= WIFI_DIRECT_STATE_ACTIVATED) {
		wfd_ug_get_discovered_peers(ugd);
	}

	if (ugd->wfd_status >= WIFI_DIRECT_STATE_CONNECTED) {
		wfd_ug_get_connected_peers(ugd);
	}

	if (ugd->wfd_status == WIFI_DIRECT_STATE_ACTIVATED) {
		/* start discovery */
		res = wifi_direct_start_discovery(FALSE, MAX_SCAN_TIME_OUT);
		if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", res);
			ugd->is_re_discover = TRUE;
			wifi_direct_cancel_discovery();
		} else {
			DBG(LOG_VERBOSE, "Discovery is started\n");
			ugd->is_re_discover = FALSE;
		}
	}

	/* draw UI */
	initialize_gen_item_class();
	create_wfd_ug_view(ugd);
	wfd_ug_view_update_peers(ugd);

	evas_object_show(ugd->base);

	__FUNC_EXIT__;
	return ugd->base;
}

static void on_start(ui_gadget_h ug, service_h service, void *priv)
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
		DBG(LOG_VERBOSE, "HW ID of this device [%s]\n", kernel_info.machine);
		if (strncmp(kernel_info.machine, "arm", 3) != 0) {
			wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_NOT_SUPPORTED_DEVICE"), POPUP_TYPE_TERMINATE);
			return;
		}
	}

	__FUNC_EXIT__;
}

static void on_pause(ui_gadget_h ug, service_h service, void *priv)
{
	__FUNC_ENTER__;
	__FUNC_EXIT__;
}

static void on_resume(ui_gadget_h ug, service_h service, void *priv)
{
	__FUNC_ENTER__;
	__FUNC_EXIT__;
}

static void on_destroy(ui_gadget_h ug, service_h service, void *priv)
{
	__FUNC_ENTER__;

	if (!ug || !priv) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	struct ug_data *ugd = priv;
	if (ugd == NULL || ugd->base == NULL) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	deinit_wfd_client(ugd);
	DBG(LOG_VERBOSE, "WFD client deregistered");

	destroy_wfd_ug_view(ugd);
	DBG(LOG_VERBOSE, "Destroying About item");

	wfd_ug_view_free_peers(ugd);

	DBG(LOG_VERBOSE, "WFD client deregistered");
	if (ugd->bg) {
		evas_object_del(ugd->bg);
		ugd->bg = NULL;
	}
	DBG(LOG_VERBOSE, "WFD client deregistered");

	if (ugd->base) {
		evas_object_del(ugd->base);
		ugd->base = NULL;
	}

	__FUNC_EXIT__;
	return;
}

static void on_message(ui_gadget_h ug, service_h msg, service_h service, void *priv)
{
	__FUNC_ENTER__;
	__FUNC_EXIT__;
}

static void on_event(ui_gadget_h ug, enum ug_event event, service_h service, void *priv)
{
	__FUNC_ENTER__;

	if (!ug || !priv) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		DBG(LOG_VERBOSE, "UG_EVENT_LOW_MEMORY\n");
		break;
	case UG_EVENT_LOW_BATTERY:
		DBG(LOG_VERBOSE, "UG_EVENT_LOW_BATTERY\n");
		break;
	case UG_EVENT_LANG_CHANGE:
		DBG(LOG_VERBOSE, "UG_EVENT_LANG_CHANGE\n");
		break;
	case UG_EVENT_ROTATE_PORTRAIT:
		DBG(LOG_VERBOSE, "UG_EVENT_ROTATE_PORTRAIT\n");
		break;
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		DBG(LOG_VERBOSE, "UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN\n");
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
		DBG(LOG_VERBOSE, "UG_EVENT_ROTATE_LANDSCAPE\n");
		break;
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		DBG(LOG_VERBOSE, "UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN\n");
		break;
	default:
		DBG(LOG_VERBOSE, "default\n");
		break;
	}

	__FUNC_EXIT__;
}

static void on_key_event(ui_gadget_h ug, enum ug_key_event event, service_h service, void *priv)
{
	__FUNC_ENTER__;

	if (!ug || !priv) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	switch (event) {
	case UG_KEY_EVENT_END:
		DBG(LOG_VERBOSE, "UG_KEY_EVENT_END\n");
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

	if (ugd) {
		free(ugd);
	}

	__FUNC_EXIT__;
}

UG_MODULE_API int setting_plugin_reset(service_h service, void *priv)
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
		DBG(LOG_VERBOSE, "No need to reset Wi-Fi Direct.\n");
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

