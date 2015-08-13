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
 * This file implements wifi direct application utils functions.
 *
 * @file    wfd-app-util.c
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */

#include <linux/unistd.h>
#include <stdio.h>
#include <string.h>

#include <Elementary.h>
#if defined(X)
#include <utilX.h>
#endif
#include <vconf.h>
#include <app_control_internal.h>
#include <notification.h>
#include <notification_internal.h>
#include <notification_text_domain.h>
#include <bundle_internal.h>

#include <wifi-direct.h>

#include "wfd-app.h"
#include "wfd-app-util.h"

char *wfd_app_trim_path(const char *filewithpath)
{
#if 0
	char *filename = NULL;
	if ((filename = strrchr(filewithpath, '/')) == NULL)
	    return (char *) filewithpath;
	else
	    return (filename + 1);
#else
	static char *filename[100];
	char *strptr = NULL;
	int start = 0;
	const char *space = "                                        ";
	int len = strlen(filewithpath);

	if (len > 20) {
		strptr = (char *) filewithpath + (len - 20);
		start = 0;
	} else if (len < 20) {
		strptr = (char *) filewithpath;
		start = 20 - len;
	}

	strncpy((char *) filename, space, strlen(space));
	strncpy((char *) filename + start, strptr, 50);

	return (char *) filename;
#endif
}


int wfd_app_gettid()
{
#ifdef __NR_gettid
	return syscall(__NR_gettid);
#else
	fprintf(stderr, "__NR_gettid is not defined, please include linux/unistd.h ");
	return -1;
#endif
}

static void __launch_app_result_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data)
{
	__WFD_APP_FUNC_ENTER__;

	if(result == APP_CONTROL_RESULT_FAILED) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "User cancel to reconnect screen mirroring\n");
#ifdef WFD_SCREEN_MIRRORING_ENABLED
		ad->screen_mirroring_state = WFD_POP_SCREEN_MIRROR_NONE;
#endif
	} else {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "app_control launch result: [%d]\n", result);
	}

	__WFD_APP_FUNC_EXIT__;
}

static void _move_data_to_app_control(const char *key, const char *val, void *data)
{
	__WFD_APP_FUNC_ENTER__;

	WFD_RET_IF(data == NULL || key == NULL || val == NULL, , "Invialid parameter!");

	app_control_h control = data;
	app_control_add_extra_data(control, key, val);

	__WFD_APP_FUNC_EXIT__;
}

static void _launch_app(char *app_id, void *data)
{
	__WFD_APP_FUNC_ENTER__;
	WFD_RET_IF(app_id == NULL || data == NULL, "Invialid parameter!");

	int ret = APP_CONTROL_ERROR_NONE;
	app_control_h control = NULL;
	ret = app_control_create(&control);
	if (ret != APP_CONTROL_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "app_control_create() return error : %d", ret);
		return;
	}
	WFD_RET_IF(control == NULL, "Failed to create app_control handle!");

	app_control_set_operation(control, APP_CONTROL_OPERATION_DEFAULT);
	app_control_set_app_id(control, app_id);
	bundle_iterate((bundle *)data, _move_data_to_app_control, control);

	char *launch_type = (char*)bundle_get_val(data, "-t");
	if (!strcmp(launch_type, "reconnect_by_connecting_wifi_ap")) {
		ret = app_control_send_launch_request(control, __launch_app_result_cb, NULL);
	} else {
		ret = app_control_send_launch_request(control, NULL, NULL);
	}

	if (ret != APP_CONTROL_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "app_control_send_launch_request() is failed : %d", ret);
		app_control_destroy(control);
		return;
	}
	app_control_destroy(control);

	__WFD_APP_FUNC_EXIT__;
	return;
}

#ifdef WFD_SCREEN_MIRRORING_ENABLED
void _add_screen_mirroring_activated_indicator(void *user_data)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	WFD_RET_IF(ad->noti_screen_mirroring_on, "Indicator noti_screen_mirroring_on already exists");

	if (ad->noti_screen_mirroring_play) {
		notification_delete(ad->noti_screen_mirroring_play);
		notification_free(ad->noti_screen_mirroring_play);
		ad->noti_screen_mirroring_play = NULL;
	}

	if(ad->noti_screen_mirroring_on != NULL) {
		noti_err = notification_free(ad->noti_screen_mirroring_on);
		WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_free. [%d]", noti_err);
	}

	ad->noti_screen_mirroring_on = notification_new(NOTIFICATION_TYPE_ONGOING, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
	WFD_RET_IF(NULL == ad->noti_screen_mirroring_on, "NULL parameters.\n");

	noti_err = notification_set_image(ad->noti_screen_mirroring_on, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, SCREEN_MIRRIONG_INDICATOR_ICON_PATH);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_image. [%d]", noti_err);

	noti_err = notification_set_property(ad->noti_screen_mirroring_on, NOTIFICATION_PROP_DISABLE_TICKERNOTI);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Unable to notification_set_property. [%d]", noti_err);

	noti_err = notification_set_display_applist(ad->noti_screen_mirroring_on, NOTIFICATION_DISPLAY_APP_INDICATOR);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Unable to notification_set_display_applist. [%d]", noti_err);

	/* notify the quick panel */
	noti_err = notification_insert(ad->noti_screen_mirroring_on, NULL);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_insert. [%d]", noti_err);

	__WFD_APP_FUNC_EXIT__;
	return;
}
#endif


#ifdef WFD_SCREEN_MIRRORING_ENABLED
/**
 *	This function let the app add the notification when it is connected
 *	@return   void
 *	@param[in] user_data the pointer to the main data structure
 */
void _add_wfd_peers_connected_notification(void *user_data, char* package_name)
{
	__WFD_APP_FUNC_ENTER__;

	int res = NOTIFICATION_ERROR_NONE;
	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	WFD_RET_IF(NULL == ad || NULL == package_name, "NULL parameters.\n");

	if (ad->noti_screen_mirroring_on) {
		notification_delete(ad->noti_screen_mirroring_on);
		notification_free(ad->noti_screen_mirroring_on);
		ad->noti_screen_mirroring_on = NULL;
	}

	if(ad->noti_screen_mirroring_play != NULL) {
		noti_err = notification_free(ad->noti_screen_mirroring_play);
		WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_free. [%d]", noti_err);
	}

	ad->noti_screen_mirroring_play = notification_new(NOTIFICATION_TYPE_ONGOING, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
	WFD_RET_IF(NULL == ad->noti_screen_mirroring_play, "NULL parameters.\n");

	char msg[WFD_MAX_SIZE] = {0};

	bundle *b = NULL;
	app_control_h control;
	res = app_control_create(&control);
	WFD_RET_IF(res != APP_CONTROL_ERROR_NONE, "app_control_create() return error : %d", res);

	app_control_set_package(control, package_name);
	app_control_add_extra_data(control, "-t", "notification");
	res = app_control_to_bundle(control, &b);
	if (res != APP_CONTROL_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "app_control_to_bundle() return error : %d", res);
		app_control_destroy(control);
		return;
	}

	res = notification_set_execute_option(ad->noti_screen_mirroring_play, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, /*Button Text*/NULL, NULL, b);
	if (res != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to notification_set_execute_option. [%d]", res);
		app_control_destroy(control);
		return;
	}

	app_control_destroy(control);

	noti_err = notification_set_layout(ad->noti_screen_mirroring_play, NOTIFICATION_LY_ONGOING_EVENT);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_layout. [%d]", noti_err);

	/* set the icon */
	noti_err = notification_set_image(ad->noti_screen_mirroring_play, NOTIFICATION_IMAGE_TYPE_ICON, SCREEN_MIRRIONG_NOTI_ICON_PATH);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_image. [%d]", noti_err);

	noti_err = notification_set_image(ad->noti_screen_mirroring_play, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, SCREEN_MIRRIONG_INDICATOR_PLAY_ICON_PATH);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_image. [%d]", noti_err);

	/* set the title and content */
	wfd_app_get_connected_peers(ad);
	noti_err = notification_set_text(ad->noti_screen_mirroring_play, NOTIFICATION_TEXT_TYPE_TITLE, _("IDS_SMR_BODY_SCREEN_MIRRORING_IS_ENABLED"),
			NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_text. [%d]", noti_err);

	snprintf(msg, WFD_MAX_SIZE, _("IDS_WIFI_BODY_CONNECTED_TO_PS"), ad->raw_connected_peers[0].ssid);
	noti_err = notification_set_text(ad->noti_screen_mirroring_play, NOTIFICATION_TEXT_TYPE_CONTENT,
			msg, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_text. [%d]", noti_err);


	notification_set_property(ad->noti_screen_mirroring_play, NOTIFICATION_PROP_DISABLE_TICKERNOTI);

	/* notify the quick panel */
	noti_err = notification_insert(ad->noti_screen_mirroring_play, NULL);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_insert. [%d]", noti_err);

	__WFD_APP_FUNC_EXIT__;
	return;
}
#endif

/**
 *	This function let the app make a change callback for flight mode
 *	@return   void
 *	@param[in] key the pointer to the key
 *	@param[in] user_data the pointer to the main data structure
 */
static void _wfd_flight_mode_changed(keynode_t *node, void *user_data)
{
	__WFD_APP_FUNC_ENTER__;
	int res = -1;
	int flight_mode = 0;
#ifdef WFD_SCREEN_MIRRORING_ENABLED
	int screen_mirroring_status = 0;
#endif
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;
	WFD_RET_IF(NULL == ad, "NULL parameters.\n");

	res = vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &flight_mode);
	if (res != 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get flight state from vconf. [%d]\n", res);
		return;
	}

	if (flight_mode == FALSE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Flight mode is off\n");
		return;
	}

	/* If flight mode is on, turn off WFD */
	wifi_direct_get_state(&ad->wfd_status);
	if (WIFI_DIRECT_STATE_DEACTIVATED == ad->wfd_status) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Wi-Fi Direct is deactivated.\n");
		return;
	}

	/* If connected, disconnect all devices*/
	if (WIFI_DIRECT_STATE_CONNECTED == ad->wfd_status) {
		res = wifi_direct_disconnect_all();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to send disconnection request to all. [%d]\n", res);
			return;
		}
	}

	WFD_APP_LOG(WFD_APP_LOG_HIGH, "Deactivating WiFi DIrect..."
		"Due to Flight Mode is Enabled\n");
	res = wifi_direct_deactivate();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
		return;
	}

#ifdef WFD_SCREEN_MIRRORING_ENABLED
	/* checking Screen Mirroring */
	if (vconf_get_int(VCONFKEY_SCREEN_MIRRORING_STATE, &screen_mirroring_status) < 0)
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SCREEN_MIRRORING_STATE\n");
	}
	WFD_APP_LOG(WFD_APP_LOG_LOW, "screen_mirroring_status: %d\n", screen_mirroring_status);

	if(screen_mirroring_status > VCONFKEY_SCREEN_MIRRORING_DEACTIVATED) {
		res = vconf_set_int(VCONFKEY_SCREEN_MIRRORING_STATE, VCONFKEY_SCREEN_MIRRORING_DEACTIVATED);
		if (res < 0) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to set vconf VCONFKEY_SCREEN_MIRRORING_STATE\n");
		}
	}
#endif

	__WFD_APP_FUNC_EXIT__;
}

static void _wfd_cpu_limit_mode_changed(keynode_t *node, void *user_data)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;
	WFD_RET_IF(NULL == ad, "NULL parameters.\n");
	int power_mode = 0;
	int screen_mirroring_status = 0;
	int cup_limit_mode = 0;

	if (vconf_get_int(VCONFKEY_SCREEN_MIRRORING_STATE, &screen_mirroring_status) < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SCREEN_MIRRORING_STATE\n");
		return;
	}

	if (screen_mirroring_status != VCONFKEY_SCREEN_MIRRORING_CONNECTED) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Allshare cast is not connected\n");
		return;
	}

	if(vconf_get_bool(VCONFKEY_SETAPPL_PWRSV_CUSTMODE_CPU, &cup_limit_mode) < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SETAPPL_PWRSV_CUSTMODE_CPU\n");
		return;
	}

	if (vconf_get_int(VCONFKEY_SETAPPL_PSMODE, &power_mode) < 0)
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SETAPPL_PSMODE\n");
		return;
	}

	if ((power_mode == SETTING_PSMODE_POWERFUL || power_mode == SETTING_PSMODE_ADVISOR) &&
				cup_limit_mode){
		bundle *b = NULL;
		b = bundle_create();
		bundle_add(b, "-t", "notification_power_saving_on");
		_launch_app(PACKAGE_ALLSHARE_CAST, b);
		bundle_free(b);
	}

	__WFD_APP_FUNC_EXIT__;
	return;
}

static void _wfd_power_saving_mode_changed(keynode_t *node, void *user_data)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;
	WFD_RET_IF(NULL == ad, "NULL parameters.\n");
	int power_mode = 0;
	int screen_mirroring_status = 0;
	int cup_limit_mode = 0;

	if (vconf_get_int(VCONFKEY_SCREEN_MIRRORING_STATE, &screen_mirroring_status) < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SCREEN_MIRRORING_STATE\n");
		return;
	}

	if (screen_mirroring_status != VCONFKEY_SCREEN_MIRRORING_CONNECTED) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Allshare cast is not connected\n");
		return;
	}

	if (vconf_get_int(VCONFKEY_SETAPPL_PSMODE, &power_mode) < 0)
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SETAPPL_PSMODE\n");
		return;
	}

	if (power_mode == SETTING_PSMODE_SURVIVAL ||
			power_mode == SETTING_PSMODE_EMERGENCY) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Ultra power saving mode on\n");
		bundle *b = NULL;
		b = bundle_create();
		bundle_add(b, "-t", "quit_by_ultra_power_saving_on");

		_launch_app(PACKAGE_ALLSHARE_CAST, b);
		bundle_free(b);
	} else if (power_mode == SETTING_PSMODE_POWERFUL ||
			power_mode == SETTING_PSMODE_ADVISOR){
			if(vconf_get_bool(VCONFKEY_SETAPPL_PWRSV_CUSTMODE_CPU, &cup_limit_mode) < 0) {
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SETAPPL_PWRSV_CUSTMODE_CPU\n");
				return;
			}

			if (cup_limit_mode) {
				bundle *b = NULL;
				b = bundle_create();
				bundle_add(b, "-t", "notification_power_saving_on");
				_launch_app(PACKAGE_ALLSHARE_CAST, b);
				bundle_free(b);
			}
	}

	__WFD_APP_FUNC_EXIT__;
	return;
}

static void _wfd_wifi_status_changed(keynode_t *node, void *user_data)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;
	WFD_RET_IF(NULL == ad, "NULL parameters.\n");
	int wifi_status = 0;
#ifdef WFD_SCREEN_MIRRORING_ENABLED
	int screen_mirroring_status = 0;

	if (vconf_get_int(VCONFKEY_SCREEN_MIRRORING_STATE, &screen_mirroring_status) < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SCREEN_MIRRORING_STATE\n");
		return;
	}

	if (screen_mirroring_status != VCONFKEY_SCREEN_MIRRORING_CONNECTED) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Allshare cast is not connected\n");
		return;
	}
#endif

	if (vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_status) < 0)
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_WIFI_STATE\n");
		return;
	}

	WFD_APP_LOG(WFD_APP_LOG_LOW, "Wi-Fi state is [%d]\n", wifi_status);
	if (wifi_status == VCONFKEY_WIFI_CONNECTED) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Wi-Fi is connected\n");
#ifdef WFD_SCREEN_MIRRORING_ENABLED
		ad->screen_mirroring_state = WFD_POP_SCREEN_MIRROR_DISCONNECT_BY_RECONNECT_WIFI_AP;
		bundle *b = NULL;
		b = bundle_create();
		bundle_add(b, "-t", "reconnect_by_connecting_wifi_ap");

		_launch_app(PACKAGE_ALLSHARE_CAST, b);
		bundle_free(b);
#endif
	} else if (VCONFKEY_WIFI_OFF == wifi_status) {
		/* Deactivate WiFi Direct */
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Deactivate WiFi Direct...");
		wfd_destroy_popup();
		/*
		 * Currently, WiFi Direct OFF is handled at net-config.
		 * Also, this patch is added to support ON-DEMAND launch destroy popup.
		 * This patch will handle 5sec deadlock of popup destory from
		 * wfd-manager.
		 */
		/* wfd_app_client_switch_off(ad); */
	} else {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Wi-Fi state is [%d]\n", wifi_status);
	}

	__WFD_APP_FUNC_EXIT__;
	return;
}

#ifdef WFD_SCREEN_MIRRORING_ENABLED
/**
 *	This function let the app make a change callback for allshare cast
 *	@return   void
 *	@param[in] key the pointer to the key
 *	@param[in] user_data the pointer to the main data structure
 */
static void _wfd_allshare_cast_status_changed(keynode_t *node, void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	int screen_mirroring_status = 0;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;
	WFD_RET_IF(NULL == ad, "NULL parameters.\n");

	if (vconf_get_int(VCONFKEY_SCREEN_MIRRORING_STATE, &screen_mirroring_status) < 0)
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SCREEN_MIRRORING_STATE\n");
	}

	WFD_APP_LOG(WFD_APP_LOG_ERROR, "VCONFKEY_SCREEN_MIRRORING_STATE:%d\n", screen_mirroring_status);

	if (screen_mirroring_status == VCONFKEY_SCREEN_MIRRORING_CONNECTED) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Allshare cast is connected\n");
		if (ad->transmit_timer) {
			ecore_timer_del(ad->transmit_timer);
			ad->transmit_timer = NULL;
		}
		/* add connected notification */
		_add_wfd_peers_connected_notification(ad, PACKAGE_ALLSHARE_CAST);
	} else if (screen_mirroring_status == VCONFKEY_SCREEN_MIRRORING_ACTIVATED) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Allshare cast is ACTIVATED\n");
		_add_screen_mirroring_activated_indicator(ad);
	} else {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Allshare cast is not connected\n");
		if (ad->noti_screen_mirroring_on) {
			notification_delete(ad->noti_screen_mirroring_on);
			notification_free(ad->noti_screen_mirroring_on);
			ad->noti_screen_mirroring_on = NULL;
		}

		if (ad->noti_screen_mirroring_play) {
			notification_delete(ad->noti_screen_mirroring_play);
			notification_free(ad->noti_screen_mirroring_play);
			ad->noti_screen_mirroring_play = NULL;
		}
	}

	return;

	__WFD_APP_FUNC_EXIT__;
}
#endif

static Eina_Bool _wfd_hard_key_down_cb(void *data, int type, void *event)
{
	wfd_appdata_t *ad = (wfd_appdata_t *)data;
	Ecore_Event_Key *ev = (Ecore_Event_Key *)event;
	int res = 0;

	WFD_APP_LOG(WFD_APP_LOG_HIGH, "Hard Key Pressed CB...");
	if (NULL == ad || NULL == ev) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Invalid AppData parameter");
		return EINA_FALSE;
	}
#if defined(KEY)
	if (!strcmp(ev->keyname, KEY_SELECT)) {
		WFD_APP_LOG(WFD_APP_LOG_HIGH, "[KEY]KEY_SELECT pressed");
		WFD_APP_LOG(WFD_APP_LOG_HIGH, "Mac : %s", ad->mac_addr_connecting);

		if (strnlen(ad->mac_addr_connecting, MACSTR_LENGTH) > 0) {
			res = wifi_direct_reject_connection(ad->mac_addr_connecting);
			if (res != WIFI_DIRECT_ERROR_NONE) {
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to reject connection(%d)", res);
			}
		}
		memset(ad->mac_addr_connecting, 0x00, MACSTR_LENGTH);
		wfd_destroy_popup();
	} else {
		WFD_APP_LOG(WFD_APP_LOG_HIGH, "[KEY][%s] pressed not Handled",
			ev->keyname);
	}
#endif
	return EINA_FALSE;
}

int wfd_app_util_register_hard_key_down_cb(void *data)
{
	wfd_appdata_t *ad = (wfd_appdata_t *)data;

	if (NULL == ad) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Invalid AppData parameter");
		return -1;
	}

	WFD_APP_LOG(WFD_APP_LOG_HIGH, "Register hard key down press CB !!!");
	if (NULL == ad->downkey_handler)
		ad->downkey_handler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
		_wfd_hard_key_down_cb, ad);

	return 0;
}

int wfd_app_util_deregister_hard_key_down_cb(void *data)
{
	wfd_appdata_t *ad = (wfd_appdata_t *)data;

	if (NULL == ad) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Invalid AppData parameter");
		return -1;
	}
	WFD_APP_LOG(WFD_APP_LOG_HIGH, "Deregister hard key down press CB !!!");
	if (NULL != ad->downkey_handler) {
		ecore_event_handler_del(ad->downkey_handler);
		ad->downkey_handler = NULL;
	}
	return 0;
}

int wfd_app_util_register_vconf_callbacks(void *data)
{
	wfd_appdata_t *ad = NULL;
	int ret = 0;

	if (!data) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Invalid parameter");
		return -1;
	}

	ad = data;

	/* register flight mode */
	ret = vconf_notify_key_changed(VCONFKEY_TELEPHONY_FLIGHT_MODE,
							_wfd_flight_mode_changed, ad);
	if (ret < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to register vconf callback for flight mode\n");
		return -1;
	}

#ifdef WFD_SCREEN_MIRRORING_ENABLED
	/* allshare cast */
	/* TODO: Make proper changes for vconfkey */
	ret = vconf_notify_key_changed(VCONFKEY_SCREEN_MIRRORING_STATE,
							_wfd_allshare_cast_status_changed, ad);
	if (ret < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to register vconf callback for allshare cast\n");
		return -1;
	}
#endif

	/* wifi */
	ret = vconf_notify_key_changed(VCONFKEY_WIFI_STATE, _wfd_wifi_status_changed, ad);
	if (ret < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to register vconf callback for wifi\n");
		return -1;
	}

	/* power mode */
	ret = vconf_notify_key_changed(VCONFKEY_SETAPPL_PSMODE, _wfd_power_saving_mode_changed, ad);
	if (ret < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to register vconf callback for power mode\n");
		return -1;
	}

	/* cpu limit mode */
	ret = vconf_notify_key_changed(VCONFKEY_SETAPPL_PWRSV_CUSTMODE_CPU, _wfd_cpu_limit_mode_changed, ad);
	if (ret < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to register vconf callback for cpu limit mode\n");
		return -1;
	}

	return 0;
}

int wfd_app_util_deregister_vconf_callbacks(void *data)
{
	int ret = 0;

	/* remove callback for flight mode */
	ret = vconf_ignore_key_changed(VCONFKEY_TELEPHONY_FLIGHT_MODE, _wfd_flight_mode_changed);
	if (ret == -1) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to ignore vconf key callback for flight mode\n");
	}

#ifdef WFD_SCREEN_MIRRORING_ENABLED
	/* remove callback for allshare cast */
	ret = vconf_ignore_key_changed(VCONFKEY_SCREEN_MIRRORING_STATE, _wfd_allshare_cast_status_changed);
	if (ret == -1) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to ignore vconf key callback for allshare cast\n");
	}
#endif

	/* remove callback for wifi */
	ret = vconf_ignore_key_changed(VCONFKEY_WIFI_STATE, _wfd_wifi_status_changed);
	if (ret == -1) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to ignore vconf key callback for wifi\n");
	}

	/* remove callback for power mode */
	ret = vconf_ignore_key_changed(VCONFKEY_SETAPPL_PSMODE, _wfd_power_saving_mode_changed);
	if (ret == -1) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to ignore vconf key callback for power mode\n");
	}

	/* remove callback for cpu limit mode */
	ret = vconf_ignore_key_changed(VCONFKEY_SETAPPL_PWRSV_CUSTMODE_CPU, _wfd_cpu_limit_mode_changed);
	if (ret == -1) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to ignore vconf key callback for cpu limit mode\n");
	}

	return 0;
}

/**
 *	This function let the app delete the notification
 *	@return   void
 */
void wfd_app_util_del_notification(wfd_appdata_t *ad)
{
	__WFD_APP_FUNC_ENTER__;
	WFD_RET_IF(NULL == ad, "NULL parameters.\n");

	/* delete the notification */
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_delete_all_by_type(NULL, NOTIFICATION_TYPE_ONGOING);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Fail to notification_delete_all_by_type.(%d)\n", noti_err);
		return;
	}

#ifdef WFD_SCREEN_MIRRORING_ENABLED
	if (ad->noti_screen_mirroring_on) {
		noti_err = notification_free(ad->noti_screen_mirroring_on);
		ad->noti_screen_mirroring_on = NULL;
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Fail to notification_free.(%d)\n", noti_err);
		}
	}

	if (ad->noti_screen_mirroring_play) {
		noti_err = notification_free(ad->noti_screen_mirroring_play);
		ad->noti_screen_mirroring_play = NULL;
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Fail to notification_free.(%d)\n", noti_err);
		}
	}
#endif

#ifdef NOT_CONNECTED_INDICATOR_ICON
	if (ad->noti_wifi_direct_on) {
		noti_err = notification_free(ad->noti_wifi_direct_on);
		ad->noti_wifi_direct_on = NULL;
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Fail to notification_free.(%d)\n", noti_err);
		}
	}
#endif

	if (ad->noti_wifi_direct_connected) {
		noti_err = notification_free(ad->noti_wifi_direct_connected);
		ad->noti_wifi_direct_connected = NULL;
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Fail to notification_free.(%d)\n", noti_err);
		}
	}

	__WFD_APP_FUNC_EXIT__;
	return;
}

#ifdef NOT_CONNECTED_INDICATOR_ICON
/**
 *	This function let the app add the indicator icon when wfd is activated
 *	@return   void
 *	@param[in] user_data the pointer to the main data structure
 */
void wfd_app_util_add_indicator_icon(void *user_data)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	WFD_RET_IF(ad->noti_wifi_direct_on, "Indicator already exists");

	if(ad->noti_wifi_direct_on != NULL) {
		noti_err = notification_free(ad->noti_wifi_direct_on);
		WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_free. [%d]", noti_err);
	}

	ad->noti_wifi_direct_on = notification_new(NOTIFICATION_TYPE_ONGOING, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
	WFD_RET_IF(NULL == ad->noti_wifi_direct_on, "NULL parameters.\n");

	noti_err = notification_set_image(ad->noti_wifi_direct_on, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, WFD_ACTIVATED_NOTI_ICON_PATH);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_image. [%d]", noti_err);

	noti_err = notification_set_property(ad->noti_wifi_direct_on, NOTIFICATION_PROP_DISABLE_TICKERNOTI);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Unable to notification_set_property. [%d]", noti_err);

	noti_err = notification_set_display_applist(ad->noti_wifi_direct_on, NOTIFICATION_DISPLAY_APP_INDICATOR);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Unable to notification_set_display_applist. [%d]", noti_err);

	/* notify the quick panel */
	noti_err = notification_insert(ad->noti_wifi_direct_on, NULL);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_insert. [%d]", noti_err);

	__WFD_APP_FUNC_EXIT__;
	return;
}
#endif

#ifdef WFD_SCREEN_MIRRORING_ENABLED
/**
 *	This function let the app set VCONFKEY_SCREEN_MIRRORING_STATE to be DEACTIVATED
*/
void wfd_app_util_set_screen_mirroring_deactivated(wfd_appdata_t *ad)
{
	__WFD_APP_FUNC_ENTER__;
	WFD_RET_IF(NULL == ad, "NULL == ad!\n");
	int screen_mirroring_status = -1;
	int result = -1;

	/* Reconnect by ap connected, no need to set vconf to DEACTIVATED, allshare cast itself will set ACTIVATED*/
	if (ad->screen_mirroring_state == WFD_POP_SCREEN_MIRROR_DISCONNECT_BY_RECONNECT_WIFI_AP) {
		ad->screen_mirroring_state = WFD_POP_SCREEN_MIRROR_NONE;
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Reconnect screen mirroring by app connected.\n");
		return;
	}

	if (vconf_get_int(VCONFKEY_SCREEN_MIRRORING_STATE, &screen_mirroring_status) < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SCREEN_MIRRORING_STATE\n");
	}

	WFD_APP_LOG(WFD_APP_LOG_LOW, "screen_mirroring_status: %d\n", screen_mirroring_status);
	/* Set the vconf value to DEACTIVATED only when the previous vconf value is CONNECTED.
	If the previous vconf value is ACTIVATED, it means that the Screen Mirroring UG changed that key already. So no need to change it. */
	if(screen_mirroring_status == VCONFKEY_SCREEN_MIRRORING_CONNECTED) {
		result = vconf_set_int(VCONFKEY_SCREEN_MIRRORING_STATE, VCONFKEY_SCREEN_MIRRORING_DEACTIVATED);
		if (result < 0) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to set vconf VCONFKEY_SCREEN_MIRRORING_STATE\n");
		}
		notification_status_message_post(_("IDS_SMR_POP_SCREEN_MIRRORING_HAS_BEEN_DISABLED"));
	}

	__WFD_APP_FUNC_EXIT__;
}
#endif
/**
 *	This function let the app add the notification when it shoule be turned off
 *	@return   void
 *	@param[in] user_data the pointer to the main data structure
 */
void wfd_app_util_add_wfd_turn_off_notification(void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	int res = NOTIFICATION_ERROR_NONE;
	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	WFD_RET_IF(NULL == ad, "NULL parameters.\n");

	/* delete all notifications */
	wfd_app_util_del_notification(ad);

	if(ad->noti_wifi_direct_connected!= NULL) {
		noti_err = notification_free(ad->noti_wifi_direct_connected);
		WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_free. [%d]", noti_err);
	}

	ad->noti_wifi_direct_connected = (notification_h) notification_new(NOTIFICATION_TYPE_ONGOING, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
	WFD_RET_IF(NULL == ad->noti_wifi_direct_connected, "NULL parameters.\n");

	bundle *b = NULL;
	app_control_h control;
	res = app_control_create(&control);
	WFD_RET_IF(res != APP_CONTROL_ERROR_NONE, "app_control_create() return error : %d", res);

	app_control_set_package(control, PACKAGE);
	app_control_add_extra_data(control, NOTIFICATION_BUNDLE_PARAM, NOTIFICATION_BUNDLE_VALUE);
	res = app_control_to_bundle(control, &b);
	if (res != APP_CONTROL_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "app_control_to_bundle() return error : %d", res);
		app_control_destroy(control);
		return;
	}

	noti_err = notification_set_execute_option(ad->noti_wifi_direct_connected, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, /*Button Text*/NULL, NULL, b);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to notification_set_execute_option. [%d]", noti_err);
		app_control_destroy(control);
		return;
	}

	app_control_destroy(control);

	noti_err = notification_set_layout(ad->noti_wifi_direct_connected, NOTIFICATION_LY_ONGOING_EVENT);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_layout. [%d]", noti_err);

	/* set the icon */
	WFD_APP_LOG(WFD_APP_LOG_ERROR, "Icon Path: %s\n", WFD_NOTI_ICON_PATH);
	noti_err = notification_set_image(ad->noti_wifi_direct_connected, NOTIFICATION_IMAGE_TYPE_ICON, WFD_NOTI_ICON_PATH);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_image. [%d]", noti_err);

	noti_err = notification_set_display_applist(ad->noti_wifi_direct_connected, NOTIFICATION_DISPLAY_APP_TICKER | NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Unable to notification_set_display_applist. [%d]", noti_err);

	noti_err = notification_set_text_domain(ad->noti_wifi_direct_connected, LOCALE_FILE_NAME, LOCALEDIR);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_text_domain. [%d]", noti_err);

	/* set the title and content */
	noti_err = notification_set_text(ad->noti_wifi_direct_connected, NOTIFICATION_TEXT_TYPE_TITLE,
		_("IDS_WIFI_BODY_WI_FI_DIRECT_ABB"), "IDS_WIFI_BODY_WI_FI_DIRECT_ABB", NOTIFICATION_VARIABLE_TYPE_NONE);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_text. [%d]", noti_err);

	noti_err = notification_set_text(ad->noti_wifi_direct_connected, NOTIFICATION_TEXT_TYPE_CONTENT,
		_("IDS_WIFI_BODY_DISABLE_WI_FI_DIRECT_AFTER_USE_ABB"),
		"IDS_WIFI_BODY_DISABLE_WI_FI_DIRECT_AFTER_USE_ABB", NOTIFICATION_VARIABLE_TYPE_NONE);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_set_text. [%d]", noti_err);

	/* notify the quick panel */
	noti_err = notification_insert(ad->noti_wifi_direct_connected, NULL);
	WFD_RET_IF(noti_err != NOTIFICATION_ERROR_NONE, "Failed to notification_insert. [%d]", noti_err);

	__WFD_APP_FUNC_EXIT__;
	return;

}

void wfd_app_util_del_wfd_connected_notification(wfd_appdata_t *ad)
{
	__WFD_APP_FUNC_ENTER__;
	WFD_RET_IF(NULL == ad, "NULL parameters.\n");

	/* delete the notification */
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_delete(ad->noti_wifi_direct_connected);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Fail to notification_delete.(%d)\n", noti_err);
		return;
	}

	if (ad->noti_wifi_direct_connected) {
		noti_err = notification_free(ad->noti_wifi_direct_connected);
		ad->noti_wifi_direct_connected = NULL;
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Fail to notification_free.(%d)\n", noti_err);
		}
	}

	__WFD_APP_FUNC_EXIT__;
	return;
}
