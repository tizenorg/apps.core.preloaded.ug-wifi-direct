/*
*  WiFi-Direct UG
*
* Copyright 2012 Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://floralicense.org/license

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

/**
 * This file implements wifi direct application client  functions.
 *
 * @file    wfd-app-client.c
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <notification.h>
#include <vconf.h>

#include "wfd-app.h"
#include "wfd-app-util.h"
#include "wfd-app-strings.h"

/**
 *	This function let the app make a callback for connected peer
 *	@return   TRUE
 *	@param[in] peer the pointer to the connected peer
 *	@param[in] user_data the pointer to the main data structure
 */
bool _wfd_connected_peer_cb(wifi_direct_connected_peer_info_s *peer, void *user_data)
{
	__WDPOP_LOG_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	if (NULL == ad || NULL == peer || NULL == peer->device_name || NULL == peer->mac_address) {
		WDPOP_LOGD( "NULL parameters.\n");
		return FALSE;
	}

	int peer_cnt = ad->raw_connected_peer_cnt;
	WDPOP_LOGD( "%dth connected peer. [%s]\n", peer_cnt, peer->device_name);

	strncpy(ad->raw_connected_peers[peer_cnt].ssid, peer->device_name, sizeof(ad->raw_connected_peers[peer_cnt].ssid));
	ad->raw_connected_peers[peer_cnt].ssid[31] = '\0';
	strncpy(ad->raw_connected_peers[peer_cnt].mac_address, peer->mac_address, WFD_MAC_ADDRESS_SIZE);
	ad->raw_connected_peers[peer_cnt].mac_address[17] = '\0';
	WDPOP_LOGD( "\tSSID: [%s]\n", ad->raw_connected_peers[peer_cnt].ssid);
	ad->raw_connected_peer_cnt++;

	free(peer->device_name);
	free(peer->mac_address);
	free(peer);

	__WDPOP_LOG_FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the app get the connected peers
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int _wfd_app_get_connected_peers(void *user_data)
{
	__WDPOP_LOG_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	if (NULL == ad) {
		WDPOP_LOGD( "NULL parameters.\n");
		return -1;
	}

	int res = 0;

	ad->raw_connected_peer_cnt = 0;
	res = wifi_direct_foreach_connected_peers(_wfd_connected_peer_cb, (void *)ad);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		ad->raw_connected_peer_cnt = 0;
		WDPOP_LOGD( "Get connected peer failed: %d\n", res);
	}

	__WDPOP_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the app delete the notification
 *	@return   void
 */
void _del_wfd_notification()
{
	__WDPOP_LOG_FUNC_ENTER__;

	/* delete the notification */
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	noti_err  = notification_delete_all_by_type(NULL, NOTIFICATION_TYPE_NOTI);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_delete_all_by_type.(%d)\n", noti_err);
		return;
	}

	__WDPOP_LOG_FUNC_EXIT__;
}

/**
 *	This function let the app add the notification when it is connected
 *	@return   void
 *	@param[in] user_data the pointer to the main data structure
 */
void _add_wfd_peers_connected_notification(void *user_data)
{
	__WDPOP_LOG_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	if (NULL == ad || NULL == ad->noti) {
		WDPOP_LOGD( "NULL parameters.\n");
		return;
	}

	char msg[WFD_MAX_SIZE] = {0};
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	/* delete all notifications */
	_del_wfd_notification();

	/* set the icon */
	noti_err = notification_set_image(ad->noti, NOTIFICATION_IMAGE_TYPE_ICON,  RESDIR"/images/A09_notification_icon.png");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_set_image. (%d)\n", noti_err);
		return;
	}

	/* set the title and content */
	_wfd_app_get_connected_peers(ad);
	snprintf(msg, WFD_MAX_SIZE, "Connected with %d devices via Wi-Fi Direct", ad->raw_connected_peer_cnt);
	noti_err = notification_set_text(ad->noti, NOTIFICATION_TEXT_TYPE_TITLE, msg, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_set_text. (%d)\n", noti_err);
		return;
	}

	noti_err = notification_set_text(ad->noti, NOTIFICATION_TEXT_TYPE_CONTENT,
		"Tap to change settings", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_set_text. (%d)\n", noti_err);
		return;
	}

	bundle *b = NULL;
	b = bundle_create();
	appsvc_set_pkgname(b, PACKAGE);
	appsvc_add_data(b, NOTIFICATION_BUNDLE_PARAM, NOTIFICATION_BUNDLE_VALUE);

	int res = NOTIFICATION_ERROR_NONE;
	res = notification_set_execute_option(ad->noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, /*Button Text*/NULL, NULL, b);
	if (res != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Failed to notification_set_execute_option. [%d]", res);
		bundle_free(b);
		return;
	}
	bundle_free(b);

	/* set display application list */
	noti_err = notification_set_display_applist(ad->noti, NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_set_display_applist : %d\n", noti_err);
		return;
	}

	/* notify the quick panel */
	noti_err = notification_insert(ad->noti, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_insert.(%d)\n", noti_err);
		return;
	}

	__WDPOP_LOG_FUNC_EXIT__;
}

/**
 *	This function let the app add the notification when it shoule be turned off
 *	@return   void
 *	@param[in] user_data the pointer to the main data structure
 */
void _add_wfd_turn_off_notification(void *user_data)
{
	__WDPOP_LOG_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	if (NULL == ad || NULL == ad->noti) {
		WDPOP_LOGD( "NULL parameters.\n");
		return;
	}

	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	/* delete all notifications */
	_del_wfd_notification();

	/* set the icon */
	noti_err = notification_set_image(ad->noti, NOTIFICATION_IMAGE_TYPE_ICON,  RESDIR"/images/A09_notification_icon.png");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_set_image. (%d)\n", noti_err);
		return;
	}

	/* set the title and content */
	noti_err = notification_set_text(ad->noti, NOTIFICATION_TEXT_TYPE_TITLE,
		"Disable Wi-Fi Direct after use", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_set_text. (%d)\n", noti_err);
		return;
	}

	noti_err = notification_set_text(ad->noti, NOTIFICATION_TEXT_TYPE_CONTENT,
		"Disable Wi-Fi Direct after use to save battery", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_set_text. (%d)\n", noti_err);
		return;
	}

	bundle *b = NULL;
	b = bundle_create();
	appsvc_set_pkgname(b, PACKAGE);
	appsvc_add_data(b, NOTIFICATION_BUNDLE_PARAM, NOTIFICATION_BUNDLE_VALUE);

	int res = NOTIFICATION_ERROR_NONE;
	res = notification_set_execute_option(ad->noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, /*Button Text*/NULL, NULL, b);
	if (res != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Failed to notification_set_execute_option. [%d]", res);
		bundle_free(b);
		return;
	}
	bundle_free(b);

	/* set display application list */
	noti_err = notification_set_display_applist(ad->noti, NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_set_display_applist : %d\n", noti_err);
		return;
	}

	/* notify the quick panel */
	noti_err = notification_insert(ad->noti, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_insert.(%d)\n", noti_err);
		return;
	}

	__WDPOP_LOG_FUNC_EXIT__;
}

/**
 *	This function let the app make a callback for deactivating wfd automatically when connected
 *	@return   if stop the timer, return ECORE_CALLBACK_CANCEL, else return ECORE_CALLBACK_RENEW
 *	@param[in] user_data the pointer to the main data structure
 */
static Eina_Bool _wfd_automatic_deactivated_for_connection_cb(void *user_data)
{
	int interval = 0;
	int wfd_transfer_state = 0;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;

	if (NULL == ad) {
		WDPOP_LOGD( "NULL parameters.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	/* check the timeout, if not timeout, keep the cb */
	interval = time(NULL) - ad->last_wfd_transmit_time;
	if (interval < NO_ACTION_TIME_OUT) {
		return ECORE_CALLBACK_RENEW;
	}

	/* get transfer state */
	if (vconf_get_int(VCONFKEY_WIFI_DIRECT_TRANSFER_STATE, &wfd_transfer_state) < 0) {
		WDPOP_LOGD( "Error reading vconf (%s)\n",
			VCONFKEY_WIFI_DIRECT_TRANSFER_STATE);
		return ECORE_CALLBACK_CANCEL;
	}

	/* show tickernoti*/
	if (wfd_transfer_state > VCONFKEY_WIFI_DIRECT_TRANSFER_START) {
		WDPOP_LOGD( "No RX/TX packet, turn off WFD automatically.\n");
		_add_wfd_turn_off_notification(ad);
	} else {
		WDPOP_LOGD( "Has RX/TX packet, restart.\n");
		ad->last_wfd_transmit_time = time(NULL);
		return ECORE_CALLBACK_RENEW;
	}

	return ECORE_CALLBACK_CANCEL;
}

/**
 *	This function let the app make a callback for registering activation event
 *	@return   void
 *	@param[in] error_code the returned error code
 *	@param[in] device_state the state of device
 *	@param[in] user_data the pointer to the main data structure
 */
void _cb_activation(int error_code, wifi_direct_device_state_e device_state, void *user_data)
{
	__WDPOP_LOG_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;

	switch (device_state) {
	case WIFI_DIRECT_DEVICE_STATE_ACTIVATED:
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_DEVICE_STATE_ACTIVATED\n");
		break;

	case WIFI_DIRECT_DEVICE_STATE_DEACTIVATED:
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_DEVICE_STATE_DEACTIVATED\n");
		WDPOP_LOGD( "Termination process of wifi-direct popup begins...\n");

		/* when deactivated, stop the timer */
		if (ad->transmit_timer) {
			ecore_timer_del(ad->transmit_timer);
			ad->transmit_timer = NULL;
		}

		elm_exit();
		break;

	default:
		break;
	}

	__WDPOP_LOG_FUNC_EXIT__;
}

/**
 *	This function let the app find the peer by mac address
 *	@return   the found peer
 *	@param[in] data the pointer to the main data structure
 *	@param[in] mac_address the pointer to mac address
 */
static wfd_device_info_t *_wfd_app_find_peer_by_mac_address(void *data, const char *mac_address)
{
	__WDPOP_LOG_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *) data;
	int i;

	if (ad == NULL) {
		WDPOP_LOGD( "Incorrect parameter(NULL)\n");
		return NULL;
	}

	WDPOP_LOGD( "find peer by MAC [%s] \n", mac_address);

	for (i = 0; i < ad->discovered_peer_count; i++) {
		WDPOP_LOGD( "check %dth peer\n", i);

		if (!strncmp(mac_address, (const char *) ad->discovered_peers[i].mac_address, 18)) {
			WDPOP_LOGD( "found peer. [%d]\n", i);
			__WDPOP_LOG_FUNC_EXIT__;
			return &ad->discovered_peers[i];
		}
	}

	__WDPOP_LOG_FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the app make a callback for discovering peer
 *	@return   TRUE
 *	@param[in] peer the pointer to the discovered peer
 *	@param[in] user_data the pointer to the main data structure
 */
bool _wfd_app_discoverd_peer_cb(wifi_direct_discovered_peer_info_s *peer, void *user_data)
{
	__WDPOP_LOG_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;

	if (NULL != peer->device_name) {
		WDPOP_LOGD( "discovered peer ssid[%s]\n", peer->device_name);
		strncpy(ad->discovered_peers[ad->discovered_peer_count].ssid, peer->device_name, 32);
		ad->discovered_peers[ad->discovered_peer_count].ssid[31] = '\0';
	} else {
		WDPOP_LOGD( "peer's device name is NULL\n");
	}

	if (NULL != peer->mac_address) {
		WDPOP_LOGD( "discovered peer mac[%s]\n", peer->mac_address);
		strncpy(ad->discovered_peers[ad->discovered_peer_count].mac_address, peer->mac_address, 18);
		ad->discovered_peers[ad->discovered_peer_count].mac_address[17] = '\0';
	} else {
		WDPOP_LOGD( "peer's mac is NULL\n");
	}

	ad->discovered_peer_count++;

	__WDPOP_LOG_FUNC_EXIT__;
	return TRUE;

}

/**
 *	This function let the app make a callback for registering discover event
 *	@return   void
 *	@param[in] error_code the returned error code
 *	@param[in] discovery_state the state of discover
 *	@param[in] user_data the pointer to the main data structure
 */
void _cb_discover(int error_code, wifi_direct_discovery_state_e discovery_state, void *user_data)
{
	__WDPOP_LOG_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;
	int ret;

	switch (discovery_state) {
	case WIFI_DIRECT_DISCOVERY_STARTED:
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_DISCOVERY_STARTED\n");
		break;

	case WIFI_DIRECT_ONLY_LISTEN_STARTED:
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_ONLY_LISTEN_STARTED\n");
		break;

	case WIFI_DIRECT_DISCOVERY_FINISHED:
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_DISCOVERY_FINISHED\n");
		break;

	case WIFI_DIRECT_DISCOVERY_FOUND:
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_DISCOVERY_FOUND\n");

		if (NULL != ad->discovered_peers) {
			free(ad->discovered_peers);
			ad->discovered_peers = NULL;
		}

		ad->discovered_peers = calloc(10, sizeof(wfd_device_info_t));
		ad->discovered_peer_count = 0;

		ret = wifi_direct_foreach_discovered_peers(_wfd_app_discoverd_peer_cb, (void *) ad);
		if (ret != WIFI_DIRECT_ERROR_NONE) {
			WDPOP_LOGD( "get discovery result failed: %d\n", ret);
		}
		break;

	default:
		break;
	}

	__WDPOP_LOG_FUNC_EXIT__;
}

/**
 *	This function let the app make a callback for registering connection event
 *	@return   void
 *	@param[in] error_code the returned error code
 *	@param[in] connection_state the state of connection
 *	@param[in] mac_address the mac address of peer
 *	@param[in] user_data the pointer to the main data structure
 */
void _cb_connection(int error_code, wifi_direct_connection_state_e connection_state, const char *mac_address, void *user_data)
{
	__WDPOP_LOG_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;
	int result = -1;
	char msg[WFD_POP_STR_MAX_LEN] = {0};
	wfd_device_info_t *peer_info = NULL;

	/* find the peer's name by the mac address */
	if (NULL == mac_address) {
		WDPOP_LOGE("ERROR : mac address is NULL !!\n");
		return;
	}

	/* when disconnection, mac_address is empty */
	if (connection_state <= WIFI_DIRECT_CONNECTION_RSP ||
		connection_state == WIFI_DIRECT_INVITATION_REQ) {
		memset(ad->peer_mac, 0, sizeof(ad->peer_mac));
		memset(ad->peer_name, 0, sizeof(ad->peer_name));
		strncpy(ad->peer_mac, mac_address, strlen(mac_address));
		ad->peer_mac[17] = '\0';
		peer_info = _wfd_app_find_peer_by_mac_address(ad, mac_address);

		if (NULL == peer_info) {
			WDPOP_LOGD( "peer_info is NULL !!\n");
		} else if (0 == strlen(peer_info->ssid)) {
			WDPOP_LOGD( "SSID from connection is invalid !!\n");
		} else {
			WDPOP_LOGD( "SSID from connection is %s.\n", peer_info->ssid);
			strncpy(ad->peer_name, peer_info->ssid, strlen(peer_info->ssid));
			ad->peer_name[31] = '\0';
		}

		if (0 == strlen(ad->peer_name)) {
			strncpy(ad->peer_name, ad->peer_mac, strlen(ad->peer_mac));
			ad->peer_name[31] = '\0';
		}
	}

	switch (connection_state) {
	case WIFI_DIRECT_CONNECTION_RSP:
	{
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_CONNECTION_RSP\n");
		wfd_destroy_popup();

		if (error_code == WIFI_DIRECT_ERROR_NONE) {
			WDPOP_LOGD( "Link Complete!\n");

			/* add connected notification */
			_add_wfd_peers_connected_notification(ad);

			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, IDS_WFD_POP_CONNECTED, ad->peer_name);
			notification_status_message_post(msg);
		} else if (error_code == WIFI_DIRECT_ERROR_AUTH_FAILED) {
			WDPOP_LOGD(
					"Error Code - WIFI_DIRECT_ERROR_AUTH_FAILED\n");
			notification_status_message_post(_("IDS_WFD_POP_PIN_INVALID"));
		} else {
			if (error_code == WIFI_DIRECT_ERROR_CONNECTION_TIME_OUT) {
				WDPOP_LOGD(
						"Error Code - WIFI_DIRECT_ERROR_CONNECTION_TIME_OUT\n");
			} else if (error_code == WIFI_DIRECT_ERROR_CONNECTION_FAILED) {
				WDPOP_LOGD(
						"Error Code - WIFI_DIRECT_ERROR_CONNECTION_FAILED\n");
			}

			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, IDS_WFD_POP_CONNECT_FAILED, ad->peer_name);
			notification_status_message_post(msg);
		}
	}
	break;

	case WIFI_DIRECT_CONNECTION_WPS_REQ:
	{
		wifi_direct_wps_type_e wps_mode;

		memcpy(ad->peer_mac, mac_address, sizeof(ad->peer_mac));

		WDPOP_LOGD(
				"event ------------------ WIFI_DIRECT_CONNECTION_WPS_REQ\n");
		result = wifi_direct_get_wps_type(&wps_mode);
		WDPOP_LOGD(
				"wifi_direct_get_wps_type() result=[%d]\n", result);

		if (wps_mode == WIFI_DIRECT_WPS_TYPE_PBC) {
			WDPOP_LOGD(
					"wps_config is WIFI_DIRECT_WPS_TYPE_PBC. Ignore it..\n");
		} else if (wps_mode == WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY) {
			char *pin;
			WDPOP_LOGD( "wps_config is WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY\n");

			if (wifi_direct_generate_wps_pin() != WIFI_DIRECT_ERROR_NONE) {
				WDPOP_LOGD( "wifi_direct_generate_wps_pin() is failed\n");
				return;
			}

			if (wifi_direct_get_wps_pin(&pin) != WIFI_DIRECT_ERROR_NONE) {
				WDPOP_LOGD( "wifi_direct_generate_wps_pin() is failed\n");
				return;
			}

			strncpy(ad->pin_number, pin, 64);
			ad->pin_number[63] = '\0';
			free(pin);
			pin = NULL;

			WDPOP_LOGD( "pin=[%s]\n", ad->pin_number);

			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_PIN, NULL);
		} else if (wps_mode == WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD) {
			WDPOP_LOGD( "wps_config is WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD\n");
			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_KEYPAD, (void *) NULL);
		} else {
			WDPOP_LOGD( "wps_config is unkown!\n");

		}
	}
	break;

	case WIFI_DIRECT_CONNECTION_REQ:
	{
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_CONNECTION_REQ\n");

		wifi_direct_wps_type_e wps_mode;
		bool auto_connection_mode;

		result = wifi_direct_get_wps_type(&wps_mode);
		WDPOP_LOGD( "wifi_direct_get_wps_type() result=[%d]\n", result);

		result = wifi_direct_is_autoconnection_mode(&auto_connection_mode);
		WDPOP_LOGD( "wifi_direct_is_autoconnection_mode() result=[%d]\n", result);

		if (auto_connection_mode == TRUE) {
			result = wifi_direct_accept_connection(ad->peer_mac);
			printf("wifi_direct_accept_connection() result=[%d]\n", result);
		} else {
			if (wps_mode == WIFI_DIRECT_WPS_TYPE_PBC) {
				WDPOP_LOGD( "wps_config is WIFI_DIRECT_WPS_TYPE_PBC\n");
				wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_PUSHBUTTON_REQ, NULL);
			} else if (wps_mode == WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY) {
				WDPOP_LOGD( "wps_config is WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY\n");
				wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_DISPLAY_REQ, NULL);
			} else if (wps_mode == WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD) {
				WDPOP_LOGD( "wps_config is WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD\n");
				wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ, (void *) NULL);
			} else {
				WDPOP_LOGD( "wps_config is unkown!\n");
			}
		}
	}
	break;

	case WIFI_DIRECT_DISCONNECTION_IND:
	{
		_del_wfd_notification();
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_DISCONNECTION_IND\n");

		result = wifi_direct_set_autoconnection_mode(false);
		WDPOP_LOGD( "wifi_direct_set_autoconnection_mode() result=[%d]\n", result);

		/* tickernoti popup */
		snprintf(msg, WFD_POP_STR_MAX_LEN, IDS_WFD_POP_DISCONNECTED, ad->peer_name);
		notification_status_message_post(msg);
	}
	break;

	case WIFI_DIRECT_DISCONNECTION_RSP:
	{
		_del_wfd_notification();
		wfd_destroy_popup();

		result = wifi_direct_set_autoconnection_mode(false);
		WDPOP_LOGD( "wifi_direct_set_autoconnection_mode() result=[%d]\n", result);

		/* tickernoti popup */
		snprintf(msg, WFD_POP_STR_MAX_LEN, IDS_WFD_POP_DISCONNECTED, ad->peer_name);
		notification_status_message_post(msg);
	}
	break;
	case WIFI_DIRECT_CONNECTION_IN_PROGRESS:
	{
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_CONNECTION_IN_PROGRESS\n");
		/* tickernoti popup */
		notification_status_message_post(_("IDS_WFD_POP_CONNECTING"));
	}
	break;
	case WIFI_DIRECT_INVITATION_REQ:
	{
		WDPOP_LOGD( "event ------------------ WIFI_DIRECT_INVITATION_REQ\n");
		bool auto_connection_mode = FALSE;

		wifi_direct_is_autoconnection_mode(&auto_connection_mode);
		if (auto_connection_mode == TRUE) {
			result = wifi_direct_connect(ad->peer_mac);
			printf("wifi_direct_accept_connection() result=[%d]\n", result);
		} else {
			wfd_prepare_popup(WFD_POP_APRV_CONNECTION_INVITATION_REQ, NULL);
		}
	}
	break;
	default:
		break;

	}

	/* if connected, start the transmit timer */
	wifi_direct_get_state(&ad->wfd_status);
	WDPOP_LOGD( "status: %d", ad->wfd_status);

	if (ad->wfd_status < WIFI_DIRECT_STATE_CONNECTED) {
	    if (ad->transmit_timer) {
		    ecore_timer_del(ad->transmit_timer);
		    ad->transmit_timer = NULL;
	    }
	} else {
		if (NULL == ad->transmit_timer) {
			WDPOP_LOGD( "start the transmit timer\n");
			ad->last_wfd_transmit_time = time(NULL);
			ad->transmit_timer = ecore_timer_add(5.0,
				(Ecore_Task_Cb)_wfd_automatic_deactivated_for_connection_cb, ad);
		}
	}

	__WDPOP_LOG_FUNC_EXIT__;
}

/**
 *	This function let the app make a change callback for flight mode
 *	@return   void
 *	@param[in] key the pointer to the key
 *	@param[in] user_data the pointer to the main data structure
 */
static void _wfd_flight_mode_changed(keynode_t *node, void *user_data)
{
	__WDPOP_LOG_FUNC_ENTER__;
	int res = -1;
	int flight_mode = 0;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;

	if (NULL == ad) {
		WDPOP_LOGE("NULL parameters.\n");
		return;
	}

	res = vconf_get_bool(VCONFKEY_SETAPPL_FLIGHT_MODE_BOOL, &flight_mode);
	if (res != 0) {
		WDPOP_LOGE("Failed to get flight state from vconf. [%d]\n", res);
		return;
	}

	if (flight_mode == FALSE) {
		WDPOP_LOGD( "Flight mode is off\n");
		return;
	}

	/* If flight mode is on, turn off WFD */
	wifi_direct_get_state(&ad->wfd_status);
	if (WIFI_DIRECT_STATE_DEACTIVATED == ad->wfd_status) {
		WDPOP_LOGD( "Wi-Fi Direct is deactivated.\n");
		return;
	}

	/*if connected, disconnect all devices*/
	if (WIFI_DIRECT_STATE_CONNECTED == ad->wfd_status) {
		res = wifi_direct_disconnect_all();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			WDPOP_LOGE("Failed to send disconnection request to all. [%d]\n", res);
			return;
		}
	}

	res = wifi_direct_deactivate();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDPOP_LOGE("Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
		return;
	}

	__WDPOP_LOG_FUNC_EXIT__;
}

/**
 *	This function let the app do initialization
 *	@return   If success, return TRUE, else return FALSE
 *	@param[in] ad the pointer to the main data structure
 */
int init_wfd_popup_client(wfd_appdata_t *ad)
{
	__WDPOP_LOG_FUNC_ENTER__;

	if (NULL == ad) {
		WDPOP_LOGD( "NULL parameters.\n");
		return FALSE;
	}

	int ret = -1;

	ret = wifi_direct_initialize();
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		WDPOP_LOGE("Failed to initialize Wi-Fi Direct. error code = [%d]\n", ret);
		return FALSE;
	}

	ret = wifi_direct_set_device_state_changed_cb(_cb_activation, (void *)ad);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		WDPOP_LOGE("Failed to register _cb_activation. error code = [%d]\n", ret);
		return FALSE;
	}

	ret = wifi_direct_set_discovery_state_changed_cb(_cb_discover, (void *)ad);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		WDPOP_LOGE("Failed to register _cb_discover. error code = [%d]\n", ret);
		return FALSE;
	}

	ret = wifi_direct_set_connection_state_changed_cb(_cb_connection, (void *)ad);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		WDPOP_LOGE("Failed to register _cb_connection. error code = [%d]\n", ret);
		return FALSE;
	}

	/* initialize notification */
	ad->noti = notification_new(NOTIFICATION_TYPE_NOTI, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
	if (NULL == ad->noti) {
		WDPOP_LOGD( "notification_new failed.\n");
		return FALSE;
	}

	/* register flight mode */
	int result = -1;
	result = vconf_notify_key_changed(VCONFKEY_SETAPPL_FLIGHT_MODE_BOOL, _wfd_flight_mode_changed, ad);
	if (result == -1) {
		WDPOP_LOGE("Failed to register vconf callback for flight mode\n");
		return FALSE;
	}

	__WDPOP_LOG_FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the app do de-initialization
 *	@return   If success, return TRUE, else return FALSE
 *	@param[in] ad the pointer to the main data structure
 */
int deinit_wfd_popup_client(wfd_appdata_t *ad)
{
	__WDPOP_LOG_FUNC_ENTER__;

	if (NULL == ad || NULL == ad->noti) {
		WDPOP_LOGD( "NULL parameters.\n");
		return FALSE;
	}

	int ret = -1;

	ret = wifi_direct_deinitialize();

	_del_wfd_notification(ad);
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	noti_err = notification_free(ad->noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WDPOP_LOGD( "Fail to notification_free.(%d)\n", noti_err);
		ret = WIFI_DIRECT_ERROR_RESOURCE_BUSY;
	}

	/* remove callback for flight mode */
	int result = -1;
	result = vconf_ignore_key_changed(VCONFKEY_WIFI_STATE, _wfd_flight_mode_changed);
	if (result == -1) {
		WDPOP_LOGE("Failed to ignore vconf key callback for flight mode\n");
	}

	if (ad->transmit_timer) {
		ecore_timer_del(ad->transmit_timer);
		ad->transmit_timer = NULL;
	}

	__WDPOP_LOG_FUNC_EXIT__;
	if (ret == WIFI_DIRECT_ERROR_NONE) {
		return TRUE;
	} else {
		return FALSE;
	}
}
