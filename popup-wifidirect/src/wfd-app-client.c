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
 * This file implements wifi direct application client  functions.
 *
 * @file    wfd-app-client.c
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wifi-direct.h"
#include "wfd-app.h"
#include "wfd-app-util.h"
#include "vconf.h"


bool _wfd_connected_peer_cb(wifi_direct_connected_peer_info_s *peer, void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	if (NULL == ad || NULL == peer || NULL == peer->ssid || NULL == peer->mac_address) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "NULL parameters.\n");
		return FALSE;
	}

	int peer_cnt = ad->raw_connected_peer_cnt;
	WFD_APP_LOG(WFD_APP_LOG_LOW, "%dth connected peer. [%s]\n", peer_cnt, peer->ssid);

	strncpy(ad->raw_connected_peers[peer_cnt].ssid, peer->ssid, sizeof(ad->raw_connected_peers[peer_cnt].ssid));
	strncpy(ad->raw_connected_peers[peer_cnt].mac_address, peer->mac_address, WFD_MAC_ADDRESS_SIZE);
	WFD_APP_LOG(WFD_APP_LOG_LOW, "\tSSID: [%s]\n", ad->raw_connected_peers[peer_cnt].ssid);
	ad->raw_connected_peer_cnt++;

	free(peer->ssid);
	free(peer->mac_address);
	free(peer);

	__WFD_APP_FUNC_EXIT__;
	return TRUE;
}

int _wfd_app_get_connected_peers(void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	if (NULL == ad) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "NULL parameters.\n");
		return -1;
	}

	int res = 0;

	ad->raw_connected_peer_cnt = 0;
	res = wifi_direct_foreach_connected_peers(_wfd_connected_peer_cb, (void *)ad);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		ad->raw_connected_peer_cnt = 0;
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Get connected peer failed: %d\n", res);
	}

	__WFD_APP_FUNC_EXIT__;
	return 0;
}

void _del_wfd_notification()
{
	__WFD_APP_FUNC_ENTER__;

	/* delete the notification */
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	noti_err  = notification_delete_all_by_type(NULL, NOTIFICATION_TYPE_NOTI);
	if(noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_delete_all_by_type.(%d)\n", noti_err);
		return;
	}

	__WFD_APP_FUNC_EXIT__;
}

#if 0
void _add_wfd_actived_notification(void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	if (NULL == ad || NULL == ad->noti) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "NULL parameters.\n");
		return;
	}

	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	/* set the icon */
	noti_err = notification_set_image(ad->noti, NOTIFICATION_IMAGE_TYPE_ICON,  RESDIR"/images/A09_notification_icon.png");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_image. (%d)\n", noti_err);
		return;
	}

	/* set the title and content */
	noti_err = notification_set_text(ad->noti, NOTIFICATION_TEXT_TYPE_TITLE,
		"Wi-Fi Direct activated", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_text. (%d)\n", noti_err);
		return;
	}

	noti_err = notification_set_text(ad->noti, NOTIFICATION_TEXT_TYPE_CONTENT,
		"Tap to change settings", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_text. (%d)\n", noti_err);
		return;
	}

	bundle *b = NULL;
	b = bundle_create();
	appsvc_set_pkgname(b, PACKAGE);
	appsvc_add_data(b, NOTIFICATION_BUNDLE_PARAM, NOTIFICATION_BUNDLE_VALUE);

	int res = NOTIFICATION_ERROR_NONE;
	res = notification_set_execute_option(ad->noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, /*Button Text*/NULL, NULL, b);
	if (res != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW,"Failed to notification_set_execute_option. [%d]", res);
		return;
	}

	bundle_free(b);

	/* set display application list */
	noti_err = notification_set_display_applist(ad->noti, NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
	if(noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_display_applist : %d\n", noti_err);
		return;
	}

	/* notify the quick panel */
	if (0 == ad->is_insert) {
		noti_err = notification_insert(ad->noti, NULL);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_insert.(%d)\n", noti_err);
			return;
		}

		ad->is_insert= 1;
	} else {
		noti_err  = notification_update(ad->noti);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_update. (%d)\n", noti_err);
			return;
		}
	}

	__WFD_APP_FUNC_EXIT__;
}
#endif

void _add_wfd_peers_connected_notification(void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	if (NULL == ad || NULL == ad->noti) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "NULL parameters.\n");
		return;
	}

	char msg[WFD_MAX_SIZE] = {0};
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	/* delete all notifications */
	_del_wfd_notification();

	/* set the icon */
	noti_err = notification_set_image(ad->noti, NOTIFICATION_IMAGE_TYPE_ICON,  RESDIR"/images/A09_notification_icon.png");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_image. (%d)\n", noti_err);
		return;
	}

	/* set the title and content */
	_wfd_app_get_connected_peers(ad);
	snprintf(msg, WFD_MAX_SIZE, "Connected with %d devices via Wi-Fi Direct", ad->raw_connected_peer_cnt);
	noti_err = notification_set_text(ad->noti, NOTIFICATION_TEXT_TYPE_TITLE, msg, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_text. (%d)\n", noti_err);
		return;
	}

	noti_err = notification_set_text(ad->noti, NOTIFICATION_TEXT_TYPE_CONTENT,
		"Tap to change settings", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_text. (%d)\n", noti_err);
		return;
	}

	bundle *b = NULL;
	b = bundle_create();
	appsvc_set_pkgname(b, PACKAGE);
	appsvc_add_data(b, NOTIFICATION_BUNDLE_PARAM, NOTIFICATION_BUNDLE_VALUE);

	int res = NOTIFICATION_ERROR_NONE;
	res = notification_set_execute_option(ad->noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, /*Button Text*/NULL, NULL, b);
	if (res != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW,"Failed to notification_set_execute_option. [%d]", res);
		return;
	}

	bundle_free(b);

	/* set display application list */
	noti_err = notification_set_display_applist(ad->noti, NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
	if(noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_display_applist : %d\n", noti_err);
		return;
	}

	/* notify the quick panel */
	noti_err = notification_insert(ad->noti, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_insert.(%d)\n", noti_err);
		return;
	}

	__WFD_APP_FUNC_EXIT__;
}

void _add_wfd_turn_off_notification(void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	if (NULL == ad || NULL == ad->noti) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "NULL parameters.\n");
		return;
	}

	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	/* delete all notifications */
	_del_wfd_notification();

	/* set the icon */
	noti_err = notification_set_image(ad->noti, NOTIFICATION_IMAGE_TYPE_ICON,  RESDIR"/images/A09_notification_icon.png");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_image. (%d)\n", noti_err);
		return;
	}

	/* set the title and content */
	noti_err = notification_set_text(ad->noti, NOTIFICATION_TEXT_TYPE_TITLE,
		"Turn off Wi-Fi direct after using", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_text. (%d)\n", noti_err);
		return;
	}

	noti_err = notification_set_text(ad->noti, NOTIFICATION_TEXT_TYPE_CONTENT,
		"To save battery turn off Wi-Fi direct after using", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_text. (%d)\n", noti_err);
		return;
	}

	bundle *b = NULL;
	b = bundle_create();
	appsvc_set_pkgname(b, PACKAGE);
	appsvc_add_data(b, NOTIFICATION_BUNDLE_PARAM, NOTIFICATION_BUNDLE_VALUE);

	int res = NOTIFICATION_ERROR_NONE;
	res = notification_set_execute_option(ad->noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, /*Button Text*/NULL, NULL, b);
	if (res != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW,"Failed to notification_set_execute_option. [%d]", res);
		return;
	}

	bundle_free(b);

	/* set display application list */
	noti_err = notification_set_display_applist(ad->noti, NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
	if(noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_set_display_applist : %d\n", noti_err);
		return;
	}

	/* notify the quick panel */
	noti_err = notification_insert(ad->noti, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_insert.(%d)\n", noti_err);
		return;
	}

	__WFD_APP_FUNC_EXIT__;
}

static Eina_Bool _wfd_automatic_deactivated_for_connection_cb(void *user_data)
{
	int interval = 0;
	unsigned int transmit_packet = 0;
	int wfd_transfer_state = 0;
	wfd_appdata_t *ad = (wfd_appdata_t*) user_data;

	if (NULL == ad) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "NULL parameters.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	/* check the timeout, if not timeout, keep the cb */
	interval = time(NULL) - ad->last_wfd_transmit_time;
	if (interval < NO_ACTION_TIME_OUT) {
		return ECORE_CALLBACK_RENEW;
	}

	/* get transfer state */
	if (vconf_get_int(VCONFKEY_WIFI_DIRECT_TRANSFER_STATE, &wfd_transfer_state) < 0) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Error reading vconf (%s)\n",
			VCONFKEY_WIFI_DIRECT_TRANSFER_STATE);
		return ECORE_CALLBACK_CANCEL;
	}

	/* show tickernoti*/
	if (wfd_transfer_state > VCONFKEY_WIFI_DIRECT_TRANSFER_START) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "No RX/TX packet, turn off WFD automatically.\n");
		_add_wfd_turn_off_notification(ad);
	} else {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Has RX/TX packet, restart.\n");
		ad->last_wfd_transmit_time = time(NULL);
		return ECORE_CALLBACK_RENEW;
	}

	return ECORE_CALLBACK_CANCEL;
}

/* automatic deactivated wfd callback*/
static Eina_Bool _wfd_automatic_deactivated_for_no_connection_cb(void *user_data)
{
	int res = -1;
	int interval = 0;
	wfd_appdata_t *ad = (wfd_appdata_t*) user_data;

	if (NULL == ad) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "NULL parameters.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	/* check the action, if action is exist, keep the cb */
	res = wifi_direct_get_state(&ad->wfd_status);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Failed to get link status. [%d]\n", res);
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->last_wfd_status != ad->wfd_status) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Action is exist, last status: %d\n",
			ad->last_wfd_status);
		ad->last_wfd_status = ad->wfd_status;
		ad->last_wfd_time = time(NULL);
		return ECORE_CALLBACK_RENEW;
	}

	/* check the timeout, if not timeout, keep the cb */
	interval = time(NULL) - ad->last_wfd_time;
	if (interval < NO_ACTION_TIME_OUT) {
		return ECORE_CALLBACK_RENEW;
	}

	/* turn off the Wi-Fi Direct */
	wifi_direct_get_state(&ad->wfd_status);
	if (ad->wfd_status < WIFI_DIRECT_STATE_ACTIVATING) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Wi-Fi Direct is already deactivated\n");
	} else {
		wfd_prepare_popup(WFD_POP_AUTOMATIC_TURN_OFF, NULL);
	}

	return ECORE_CALLBACK_CANCEL;
}

void _cb_activation(int error_code, wifi_direct_device_state_e device_state,
                    void *user_data)
{
    __WFD_APP_FUNC_ENTER__;

    wfd_appdata_t *ad = (wfd_appdata_t *) user_data;

    switch (device_state)
    {
    case WIFI_DIRECT_DEVICE_STATE_ACTIVATED:
        WFD_APP_LOG(WFD_APP_LOG_LOW,
                    "event ------------------ WIFI_DIRECT_DEVICE_STATE_ACTIVATED\n");
        break;

    case WIFI_DIRECT_DEVICE_STATE_DEACTIVATED:
        WFD_APP_LOG(WFD_APP_LOG_LOW,
                    "event ------------------ WIFI_DIRECT_DEVICE_STATE_DEACTIVATED\n");
        WFD_APP_LOG(WFD_APP_LOG_LOW,
                    "Termination process of wifi-direct popup begins...\n");

	/* when deactivated, stop the timer */
	if (ad->transmit_timer) {
	    ecore_timer_del(ad->transmit_timer);
	    ad->transmit_timer = NULL;
	}

	if (ad->monitor_timer) {
	    ecore_timer_del(ad->monitor_timer);
	    ad->monitor_timer = NULL;
	}

        elm_exit();
        break;

    default:
        break;
    }

    __WFD_APP_FUNC_EXIT__;

}


static wfd_device_info_t *_wfd_app_find_peer_by_mac_address(void *data,
	                                                  const char *mac_address)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) data;
	
	int i;

	if (ad == NULL)
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	WFD_APP_LOG(WFD_APP_LOG_LOW, "find peer by MAC [%s] \n", mac_address);

	for (i = 0; i < ad->discovered_peer_count; i++)
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "check %dth peer\n", i);
		
		if (!strncmp(mac_address, (const char *) ad->discovered_peers[i].mac_address, 18))
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "found peer. [%d]\n", i);
			__WFD_APP_FUNC_EXIT__;
			return &ad->discovered_peers[i];
		}
	}
    
	__WFD_APP_FUNC_EXIT__;

    return NULL;
}


bool _wfd_app_discoverd_peer_cb(wifi_direct_discovered_peer_info_s * peer,
                            void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;

	if (NULL != peer->ssid)
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "discovered peer ssid[%s]\n", peer->ssid);
		strncpy(ad->discovered_peers[ad->discovered_peer_count].ssid, peer->ssid, 32);
	}
	else
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "peer's ssid is NULL\n");
	}

	if (NULL != peer->mac_address)
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "discovered peer mac[%s]\n", peer->mac_address);
		strncpy(ad->discovered_peers[ad->discovered_peer_count].mac_address, peer->mac_address, 18);
	}
	else
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "peer's mac is NULL\n");
	}
	
	ad->discovered_peer_count++;

	__WFD_APP_FUNC_EXIT__;
	
	return TRUE;

}


void _cb_discover(int error_code, wifi_direct_discovery_state_e discovery_state,
                  void *user_data)
{
    __WFD_APP_FUNC_ENTER__;

    wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
    int ret;

    switch (discovery_state)
    {
	case WIFI_DIRECT_DISCOVERY_STARTED:
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_DISCOVERY_STARTED\n");
		}
		break;

	case WIFI_DIRECT_ONLY_LISTEN_STARTED:
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_ONLY_LISTEN_STARTED\n");
		}
		break;

	case WIFI_DIRECT_DISCOVERY_FINISHED:
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_DISCOVERY_FINISHED\n");
		}
		break;

	case WIFI_DIRECT_DISCOVERY_FOUND:
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_DISCOVERY_FOUND\n");

			if (NULL != ad->discovered_peers)
				free(ad->discovered_peers);

			ad->discovered_peers = calloc(10, sizeof(wfd_device_info_t));
			ad->discovered_peer_count = 0;

			ret = wifi_direct_foreach_discovered_peers(_wfd_app_discoverd_peer_cb, (void *) ad);
			if (ret != WIFI_DIRECT_ERROR_NONE)
 				WFD_APP_LOG(WFD_APP_LOG_LOW, "get discovery result failed: %d\n", ret);
 		}
		break;

	default:
		break;
    }

    __WFD_APP_FUNC_EXIT__;

}

void _cb_connection(int error_code,
                    wifi_direct_connection_state_e connection_state,
                    const char *mac_address, void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	int result = -1;
	char msg[WFD_POP_STR_MAX_LEN] = {0};
	wfd_device_info_t *peer_info = NULL;

	/* find the peer's name by the mac address */
	if (NULL == mac_address) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "ERROR : mac address is NULL !!\n");
		return;
	}

	strncpy(ad->peer_mac, mac_address, strlen(mac_address));

	peer_info = _wfd_app_find_peer_by_mac_address(ad, mac_address);

	if (NULL == peer_info)
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "peer_info is NULL !!\n");
	}
	else if (NULL == peer_info->ssid)
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "SSID from connection is NULL !!\n");
	}
	else
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "SSID from connection is %s.\n", peer_info->ssid);
		strncpy(ad->peer_name, peer_info->ssid, strlen(peer_info->ssid));
	}

	if (ad->peer_name == NULL || strlen(ad->peer_name) == 0) {
		strncpy(ad->peer_name, ad->peer_mac, strlen(ad->peer_mac));
	}

	switch (connection_state)
	{
	case WIFI_DIRECT_CONNECTION_RSP:
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW,
				"event ------------------ WIFI_DIRECT_CONNECTION_RSP\n");
		wfd_destroy_popup();

		if (error_code == WIFI_DIRECT_ERROR_NONE)
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "Link Complete!\n");

			/* add connected notification */
			_add_wfd_peers_connected_notification(ad);

			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WFD_POP_CONNECTED"), ad->peer_name);
			wfd_tickernoti_popup(msg);
		}
		else
		{
			if (error_code == WIFI_DIRECT_ERROR_CONNECTION_TIME_OUT)
				WFD_APP_LOG(WFD_APP_LOG_LOW,
						"Error Code - WIFI_DIRECT_ERROR_CONNECTION_TIME_OUT\n");
			else if (error_code == WIFI_DIRECT_ERROR_AUTH_FAILED)
				WFD_APP_LOG(WFD_APP_LOG_LOW,
						"Error Code - WIFI_DIRECT_ERROR_AUTH_FAILED\n");
			else if (error_code == WIFI_DIRECT_ERROR_CONNECTION_FAILED)
				WFD_APP_LOG(WFD_APP_LOG_LOW,
						"Error Code - WIFI_DIRECT_ERROR_CONNECTION_FAILED\n");

			result = wifi_direct_start_discovery(FALSE, 0);
			WFD_APP_LOG(WFD_APP_LOG_LOW,
					"wifi_direct_start_discovery() result=[%d]\n",
					result);
		}
	}
	break;

	case WIFI_DIRECT_CONNECTION_WPS_REQ:
	{
		wifi_direct_config_data_s *config = NULL;

		memcpy(ad->peer_mac, mac_address, sizeof(ad->peer_mac));

		WFD_APP_LOG(WFD_APP_LOG_LOW,
				"event ------------------ WIFI_DIRECT_CONNECTION_WPS_REQ\n");
		result = wifi_direct_get_config_data(&config);
		WFD_APP_LOG(WFD_APP_LOG_LOW,
				"wifi_direct_client_get_config_data() result=[%d]\n",
				result);

		if (config->wps_config == WIFI_DIRECT_WPS_TYPE_PBC)
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW,
					"wps_config is WIFI_DIRECT_WPS_TYPE_PBC. Ignore it..\n");
		}
		else if (config->wps_config == WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD)
		{
			char *pin;
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD\n");

			if (wifi_direct_generate_wps_pin() != WIFI_DIRECT_ERROR_NONE)
			{
				WFD_APP_LOG(WFD_APP_LOG_LOW, "wifi_direct_generate_wps_pin() is failed\n");
				return;
			}

			if (wifi_direct_get_wps_pin(&pin) != WIFI_DIRECT_ERROR_NONE)
			{
				WFD_APP_LOG(WFD_APP_LOG_LOW, "wifi_direct_generate_wps_pin() is failed\n");
				return;
			}
			strncpy(ad->pin_number, pin, 32);
			free(pin);
			pin=NULL;

			WFD_APP_LOG(WFD_APP_LOG_LOW, "pin=[%s]\n", ad->pin_number);

			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_PIN, NULL);
		}
		else if (config->wps_config == WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY)
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY\n");
			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_KEYPAD, (void *) NULL);
		}
		else
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is unkown!\n");

		}
		if (config != NULL)
			free(config);
	}
	break;

	case WIFI_DIRECT_CONNECTION_REQ:
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_CONNECTION_REQ\n");

		wifi_direct_config_data_s *config = NULL;

		result = wifi_direct_get_config_data(&config);
		WFD_APP_LOG(WFD_APP_LOG_LOW, "wifi_direct_client_get_config_data() result=[%d]\n", result);

		if(config->auto_connection == TRUE)
		{
			result = wifi_direct_accept_connection(ad->peer_mac);
			printf("wifi_direct_accept_connection() result=[%d]\n", result);
		}
		else
		{

			if (config->wps_config == WIFI_DIRECT_WPS_TYPE_PBC)
			{
				char pushbutton;
				WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is WIFI_DIRECT_WPS_TYPE_PBC\n");

				wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_PUSHBUTTON_REQ, NULL);
			}
			else if (config->wps_config == WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY)
			{
				WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY\n");

				wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_DISPLAY_REQ, NULL);
			}
			else if (config->wps_config == WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD)
			{
				WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD\n");
				wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ, (void *) NULL);
			}
			else
			{
				WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is unkown!\n");
			}

			if (config != NULL)
				free(config);

		}
	}
	break;

	case WIFI_DIRECT_DISCONNECTION_IND:
	{
		_del_wfd_notification();
		WFD_APP_LOG(WFD_APP_LOG_LOW,
				"event ------------------ WIFI_DIRECT_DISCONNECTION_IND\n");

		result = wifi_direct_set_autoconnection_mode(false);
		WFD_APP_LOG(WFD_APP_LOG_LOW,"wifi_direct_set_autoconnection_mode() result=[%d]\n", result);

		/* tickernoti popup */
		snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WFD_POP_DISCONNECTED"), ad->peer_name);
		wfd_tickernoti_popup(msg);
	}
	break;

	case WIFI_DIRECT_DISCONNECTION_RSP:
	{
		_del_wfd_notification();
		wfd_destroy_popup();

		result = wifi_direct_set_autoconnection_mode(false);
		WFD_APP_LOG(WFD_APP_LOG_LOW,"wifi_direct_set_autoconnection_mode() result=[%d]\n", result);

		result = wifi_direct_start_discovery(FALSE, 0);
		WFD_APP_LOG(WFD_APP_LOG_LOW,
				"wifi_direct_start_discovery() result=[%d]\n", result);

		/* tickernoti popup */
		snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WFD_POP_DISCONNECTED"), ad->peer_name);
		wfd_tickernoti_popup(msg);
	}
	break;
	case WIFI_DIRECT_CONNECTION_IN_PROGRESS:
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW,
				"event ------------------ WIFI_DIRECT_CONNECTION_IN_PROGRESS\n");
		/* tickernoti popup */
		wfd_tickernoti_popup(_("IDS_WFD_POP_CONNECTING"));
	}
	default:
		break;

	}

	/* if connected, switch to the transmit timer; Otherwise, switch to monitor timer */
	wifi_direct_get_state(&ad->wfd_status);
	WFD_APP_LOG(WFD_APP_LOG_LOW,"status: %d", ad->wfd_status);

	if (ad->wfd_status > WIFI_DIRECT_STATE_CONNECTING) {
		if (ad->monitor_timer) {
			ecore_timer_del(ad->monitor_timer);
			ad->monitor_timer = NULL;
		}

		if (NULL == ad->transmit_timer) {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "switch to the transmit timer\n");
			ad->last_wfd_transmit_time = time(NULL);
			ad->transmit_timer = ecore_timer_add(5.0,
					(Ecore_Task_Cb)_wfd_automatic_deactivated_for_connection_cb, ad);
		}
	} else {
		if (ad->transmit_timer) {
			ecore_timer_del(ad->transmit_timer);
			ad->transmit_timer = NULL;
		}

		if (NULL == ad->monitor_timer) {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "switch to the monitor timer\n");
			ad->last_wfd_time = time(NULL);
			ad->monitor_timer = ecore_timer_add(5.0,
					(Ecore_Task_Cb)_wfd_automatic_deactivated_for_no_connection_cb, ad);
		}
	}

	__WFD_APP_FUNC_EXIT__;
}


int init_wfd_popup_client(wfd_appdata_t * ad)
{
    __WFD_APP_FUNC_ENTER__;

    if (NULL == ad) {
	WFD_APP_LOG(WFD_APP_LOG_LOW, "NULL parameters.\n");
	return FALSE;
    }

    int ret = -1;

    ret = wifi_direct_initialize();

    ret = wifi_direct_set_device_state_changed_cb(_cb_activation, (void *) ad);
    ret = wifi_direct_set_discovery_state_changed_cb(_cb_discover, (void *) ad);
    ret =
        wifi_direct_set_connection_state_changed_cb(_cb_connection,
                                                    (void *) ad);

    /* initialize notification */
    ad->noti = NULL;
    ad->raw_connected_peer_cnt = 0;

    ad->noti = notification_new(NOTIFICATION_TYPE_NOTI, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
    if (NULL == ad->noti) {
	WFD_APP_LOG(WFD_APP_LOG_LOW, "notification_new failed.\n");
	return FALSE;
    }

    /* start the monitor timer */
    ad->last_wfd_time = time(NULL);
    ad->last_wfd_status = WIFI_DIRECT_STATE_DEACTIVATED;
    ad->monitor_timer = ecore_timer_add(5.0, (Ecore_Task_Cb)_wfd_automatic_deactivated_for_no_connection_cb, ad);

    __WFD_APP_FUNC_EXIT__;

    if (ret == WIFI_DIRECT_ERROR_NONE)
        return TRUE;
    else
        return FALSE;
}

int deinit_wfd_popup_client(wfd_appdata_t * ad)
{
    __WFD_APP_FUNC_ENTER__;

    if (NULL == ad || NULL == ad->noti) {
	WFD_APP_LOG(WFD_APP_LOG_LOW, "NULL parameters.\n");
	return FALSE;
    }

    int ret = -1;

    ret = wifi_direct_deinitialize();

    _del_wfd_notification(ad);

    notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
    noti_err = notification_free(ad->noti);
    if (noti_err != NOTIFICATION_ERROR_NONE) {
	WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to notification_free.(%d)\n", noti_err);
	ret = WIFI_DIRECT_ERROR_RESOURCE_BUSY;
    }

    if (ad->transmit_timer) {
	ecore_timer_del(ad->transmit_timer);
	ad->transmit_timer = NULL;
    }

    if (ad->monitor_timer) {
	ecore_timer_del(ad->monitor_timer);
	ad->monitor_timer = NULL;
    }

    __WFD_APP_FUNC_EXIT__;

    if (ret == WIFI_DIRECT_ERROR_NONE)
        return TRUE;
    else
        return FALSE;
}
