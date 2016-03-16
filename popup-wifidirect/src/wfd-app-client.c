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

#include <Elementary.h>
#include <app_control.h>
#include <vconf.h>
#include <notification.h>

#include <tethering.h>
#include <network-cm-intf.h>
#include <network-wifi-intf.h>

#include <dd-display.h>

#include <dbus/dbus.h>

#include "wfd-app.h"
#include "wfd-app-util.h"
#include "wfd-app-strings.h"
#include "wfd-app-popup-view.h"

/**
 *	This function let the app make a callback for connected peer
 *	@return   TRUE
 *	@param[in] peer the pointer to the connected peer
 *	@param[in] user_data the pointer to the main data structure
 */
bool _wfd_connected_peer_cb(wifi_direct_connected_peer_info_s *peer, void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;

	if (NULL == ad || NULL == peer || NULL == peer->device_name || NULL == peer->mac_address) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "NULL parameters.\n");
		return FALSE;
	}

	int peer_cnt = ad->raw_connected_peer_cnt;
	strncpy(ad->raw_connected_peers[peer_cnt].ssid, peer->device_name, sizeof(ad->raw_connected_peers[peer_cnt].ssid) - 1);
	strncpy(ad->raw_connected_peers[peer_cnt].mac_address, peer->mac_address, WFD_MAC_ADDRESS_SIZE - 1);

	ad->raw_connected_peer_cnt++;

	free(peer->device_name);
	free(peer->mac_address);
	free(peer);

	__WFD_APP_FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the app get the connected peers
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int wfd_app_get_connected_peers(void *user_data)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *) user_data;
	if (NULL == ad) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "NULL parameters.\n");
		return -1;
	}

	int res = 0;

	ad->raw_connected_peer_cnt = 0;
	res = wifi_direct_foreach_connected_peers(_wfd_connected_peer_cb, (void *)ad);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		ad->raw_connected_peer_cnt = 0;
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Get connected peer failed: %d\n", res);
	}

	__WFD_APP_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the app make a callback for deactivating wfd automatically when connected
 *	@return   if stop the timer, return ECORE_CALLBACK_CANCEL, else return ECORE_CALLBACK_RENEW
 *	@param[in] user_data the pointer to the main data structure
 */
Eina_Bool wfd_automatic_deactivated_for_connection_cb(void *user_data)
{
	__WFD_APP_FUNC_ENTER__;
	int interval = 0;
	int wfd_transfer_state = 0;
	int res = 0;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;

	if (NULL == ad) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "NULL parameters.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	/* check the timeout, if not timeout, keep the cb */
	interval = time(NULL) - ad->last_wfd_transmit_time;
	if (interval < NO_ACTION_TIME_OUT) {
		return ECORE_CALLBACK_RENEW;
	}

	/* get transfer state */
	if (vconf_get_int(VCONFKEY_WIFI_DIRECT_TRANSFER_STATE, &wfd_transfer_state) < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Error reading vconf (%s)\n",
			VCONFKEY_WIFI_DIRECT_TRANSFER_STATE);
		return ECORE_CALLBACK_CANCEL;
	}

	res = wifi_direct_get_state(&ad->wfd_status);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->wfd_status < WIFI_DIRECT_STATE_CONNECTED) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Wi-Fi Direct is unconnected!");
		return ECORE_CALLBACK_CANCEL;
	}

	/* show tickernoti*/
	if (wfd_transfer_state > VCONFKEY_WIFI_DIRECT_TRANSFER_START) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Display Toast popup.\n");
		notification_status_message_post(D_("IDS_WIFI_BODY_TO_SAVE_BATTERY_POWER_DISABLE_WI_FI_DIRECT_AFTER_USE"));
		WFD_APP_LOG(WFD_APP_LOG_LOW, "No RX/TX packet, turn off WFD automatically.\n");
		wfd_app_util_add_wfd_turn_off_notification(ad);
	} else {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Has RX/TX packet, restart.\n");
		ad->last_wfd_transmit_time = time(NULL);
		return ECORE_CALLBACK_RENEW;
	}

	ad->transmit_timer = NULL;
	__WFD_APP_FUNC_EXIT__;
	return ECORE_CALLBACK_CANCEL;
}

int wfd_app_client_switch_off(void *data)
{
	wfd_appdata_t *ad = (wfd_appdata_t *)data;
	int res;
	if(NULL == ad) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "NULL == ad!\n");
		return -1;
	}
	res = wifi_direct_get_state(&ad->wfd_status);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->wfd_status >= WIFI_DIRECT_STATE_ACTIVATING) {
		/*if connected, disconnect all devices*/
		if (WIFI_DIRECT_STATE_CONNECTED == ad->wfd_status) {
			res = wifi_direct_disconnect_all();
			if (res != WIFI_DIRECT_ERROR_NONE) {
				return -1;
			}
		}
		res = wifi_direct_deactivate();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
			return -1;
		}
	}

	return 0;
}

#ifdef WFD_FIVE_MIN_IDLE_DEACTIVATION
/**
 *	This function let the ug make a callback for deactivating wfd automatically
 *	@return   if stop the timer, return ECORE_CALLBACK_CANCEL, else return ECORE_CALLBACK_RENEW
 *	@param[in] user_data the pointer to the main data structure
 */
static Eina_Bool _wfd_automatic_deactivated_for_no_connection_cb(void *user_data)
{
	int res = -1;
	int interval = 0;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;
#ifdef WFD_SCREEN_MIRRORING_ENABLED
	int screen_mirroring_status = 0;
#endif

	if (NULL == ad) {
		return ECORE_CALLBACK_CANCEL;
	}

	/* check the action, if action is exist, keep the cb */
	res = wifi_direct_get_state(&ad->wfd_status);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Get wifi-direct status WIFI_DIRECT_ERROR_NONE!\n");
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->last_wfd_status != ad->wfd_status) {
		ad->last_wfd_status = ad->wfd_status;
		ad->last_wfd_time = time(NULL);
		return ECORE_CALLBACK_RENEW;
	}

	/* check the timeout, if not timeout, keep the cb */
	interval = time(NULL) - ad->last_wfd_time;
	if (interval < MAX_NO_ACTION_TIME_OUT) {
		return ECORE_CALLBACK_RENEW;
	}

	/* turn off the Wi-Fi Direct */
	wifi_direct_get_state(&ad->wfd_status);
	if (ad->wfd_status < WIFI_DIRECT_STATE_ACTIVATING) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "wfd_status < WIFI_DIRECT_STATE_ACTIVATING!\n");
	} else {
#ifdef WFD_SCREEN_MIRRORING_ENABLED
		if (vconf_get_int(VCONFKEY_SCREEN_MIRRORING_STATE, &screen_mirroring_status) < 0)
		{
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get vconf VCONFKEY_SCREEN_MIRRORING_STATE\n");
		}

		if (screen_mirroring_status == VCONFKEY_SCREEN_MIRRORING_ACTIVATED) {
			ad->last_wfd_time = time(NULL);
			return ECORE_CALLBACK_RENEW;
		} else {
			/* turn off the Wi-Fi Direct */
			wfd_app_client_switch_off(ad);
			ad->popup = wfd_draw_pop_type_auto_deactivation(ad->win, user_data);
		}
#endif
	}

	/* reset monitor timer */
	if (ad->monitor_timer) {
		ad->monitor_timer = NULL;	//ECORE_CALLBACK_CANCEL will release timer.
	}

	return ECORE_CALLBACK_CANCEL;
}
#endif

/**
 *	This function let the app make a callback for registering activation event
 *	@return   void
 *	@param[in] error_code the returned error code
 *	@param[in] device_state the state of device
 *	@param[in] user_data the pointer to the main data structure
 */
void _cb_activation(int error_code, wifi_direct_device_state_e device_state, void *user_data)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;

	switch (device_state) {
	case WIFI_DIRECT_DEVICE_STATE_ACTIVATED:
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "---------------WIFI_DIRECT_DEVICE_STATE_ACTIVATED\n");
#ifdef NOT_CONNECTED_INDICATOR_ICON
		wfd_app_util_add_indicator_icon(ad);
#endif
#ifdef WFD_FIVE_MIN_IDLE_DEACTIVATION
		if (NULL == ad->monitor_timer) {
			ad->last_wfd_time = time(NULL);
			ad->monitor_timer = ecore_timer_add(5.0,
					(Ecore_Task_Cb)_wfd_automatic_deactivated_for_no_connection_cb, ad);
		}
#endif
	}
	break;

	case WIFI_DIRECT_DEVICE_STATE_DEACTIVATED:
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "---------------WIFI_DIRECT_DEVICE_STATE_DEACTIVATED\n");
		/* when deactivated, stop the timer */
		if (ad->transmit_timer) {
			ecore_timer_del(ad->transmit_timer);
			ad->transmit_timer = NULL;
		}

#ifdef WFD_FIVE_MIN_IDLE_DEACTIVATION
		/* when deactivated, stop the timer */
		if (ad->monitor_timer) {
			ecore_timer_del(ad->monitor_timer);
			ad->monitor_timer = NULL;
		}
#endif
		wfd_app_util_del_notification(ad);
#ifdef WFD_SCREEN_MIRRORING_ENABLED
		wfd_app_util_set_screen_mirroring_deactivated(ad);
#endif
	}
	break;

	default:
		break;
	}

	__WFD_APP_FUNC_EXIT__;
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
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = (wfd_appdata_t *)user_data;
	int result = -1;
	int wfd_state = -1;
#ifdef WFD_SCREEN_MIRRORING_ENABLED
	int screen_mirroring_status = 0;
#endif
	wifi_direct_discovered_peer_info_s *peer_info = NULL;
	wfd_connection_info_s *connection = ad->connection;
#ifdef NOT_CONNECTED_INDICATOR_ICON
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
#endif
	char popup_text[MAX_POPUP_TEXT_SIZE] = {0, };

	/* find the peer's name by the mac address */
	WFD_RET_IF(NULL == mac_address, "ERROR : mac address is NULL !!\n");

	WFD_APP_LOG(WFD_APP_LOG_ERROR, "-------------------error_code:%d connection_state:%d \n",
			error_code, connection_state);

	/* when disconnection, mac_address is empty */
	if (connection_state <= WIFI_DIRECT_DISASSOCIATION_IND) {
		if (connection) {
			result = strncmp(connection->peer_addr, mac_address, MACSTR_LENGTH);
			if (result) {
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Connection event from unknown peer");
				return;
			}
		} else {
			result = wifi_direct_get_peer_info((char *)mac_address, &peer_info);
			if (result < 0 || !peer_info) {
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Unknown peer");
				if (NULL != peer_info)
					free(peer_info);
				return;
			}

			connection = (wfd_connection_info_s*) calloc(1, sizeof(wfd_connection_info_s));
			if (!connection) {
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to allocate memory for peer");
				free(peer_info);
				return;
			}

			if (TRUE != dbus_validate_utf8(peer_info->device_name, NULL)) {
				memcpy(connection->peer_name, peer_info->device_name, DEV_NAME_LENGTH-2);
				connection->peer_name[DEV_NAME_LENGTH-2] = '\0';
			} else {
				memcpy(connection->peer_name, peer_info->device_name, DEV_NAME_LENGTH);
				connection->peer_name[DEV_NAME_LENGTH] = '\0';
			}

			strncpy(connection->peer_addr, mac_address, MACSTR_LENGTH);
			connection->peer_addr[MACSTR_LENGTH] = '\0';
			connection->device_type = peer_info->primary_device_type;
			connection->wifi_display = peer_info->is_miracast_device;
			wifi_direct_get_local_wps_type(&connection->wps_type);
			wifi_direct_is_autoconnection_mode(&connection->auto_conn);

			ad->connection = connection;
			free(peer_info);
		}
	}

	switch (connection_state) {
	case WIFI_DIRECT_CONNECTION_REQ:
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_CONNECTION_REQ\n");
		char *format_str = D_("IDS_ST_POP_YOU_CAN_CONNECT_UP_TO_PD_DEVICES_AT_THE_SAME_TIME");

		memcpy((char*)ad->mac_addr_connecting, connection->peer_addr, MACSTR_LENGTH);

		wfd_app_get_connected_peers(ad);
		WFD_APP_LOG(WFD_APP_LOG_LOW, "No of connected peers = %d", ad->raw_connected_peer_cnt);
		if (ad->raw_connected_peer_cnt >= WFD_MAX_CONNECTED_PEER) {
			result = wifi_direct_reject_connection(connection->peer_addr);
			if (result != WIFI_DIRECT_ERROR_NONE)
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to reject connection(%d)", result);
			g_snprintf(popup_text, MAX_POPUP_TEXT_SIZE,
					format_str, WFD_MAX_CONNECTED_PEER);
			notification_status_message_post(popup_text);
			break;
		}

		if (connection->auto_conn) {
			result = wifi_direct_accept_connection(connection->peer_addr);
			if (result < 0) {
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to accept connection");
				break;
			}
			WFD_APP_LOG(WFD_APP_LOG_HIGH, "Succeeded to accept connection");
		} else {
			if (connection->wps_type == WIFI_DIRECT_WPS_TYPE_PBC) {
				WFD_APP_LOG(WFD_APP_LOG_LOW, "WPS type: WIFI_DIRECT_WPS_TYPE_PBC\n");
				wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_PUSHBUTTON_REQ, NULL);
			} else if (connection->wps_type == WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY) {
				char *pin;
				WFD_APP_LOG(WFD_APP_LOG_LOW, "WPS type: WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY");
				if (wifi_direct_get_wps_pin(&pin) != WIFI_DIRECT_ERROR_NONE) {
					WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get WPS pin");
					return;
				}
				strncpy(connection->wps_pin, pin, PIN_LENGTH);
				WFD_IF_FREE_MEM(pin);
				wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_DISPLAY_REQ, NULL);
			} else if (connection->wps_type == WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD) {
				WFD_APP_LOG(WFD_APP_LOG_LOW, "WPS type: WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD");
				wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ, NULL);
			} else {
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "wps_config is unkown!\n");
				break;
			}
			int res = display_change_state(LCD_NORMAL);
			if(res < 0)
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to change PM state(%d)", res);
		}
	}
	break;

	case WIFI_DIRECT_CONNECTION_WPS_REQ:
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_CONNECTION_WPS_REQ\n");
		if (connection->wps_type == WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD) {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "WPS type: WIFI_DIRECT_WPS_TYPE_PIN_KEYPAD\n");
			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_KEYPAD, NULL);
		} else if (connection->wps_type == WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY) {
			char *pin;
			WFD_APP_LOG(WFD_APP_LOG_LOW, "WPS type: WIFI_DIRECT_WPS_TYPE_PIN_DISPLAY\n");

			if (wifi_direct_get_wps_pin(&pin) != WIFI_DIRECT_ERROR_NONE) {
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get WPS pin");
				return;
			}

			strncpy(connection->wps_pin, pin, PIN_LENGTH);
			WFD_IF_FREE_MEM(pin);

			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_PIN, NULL);
		} else {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "WPS type: %d", connection->wps_type);
		}
	}
	break;

	case WIFI_DIRECT_CONNECTION_IN_PROGRESS:
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_CONNECTION_IN_PROGRESS\n");
	}
	break;

	case WIFI_DIRECT_CONNECTION_RSP:
	{
		char *msg = NULL;
		char txt[WFD_POP_STR_MAX_LEN] = {0};
		char *format_str = NULL;
		wfd_destroy_popup();

		memset(ad->mac_addr_connecting, 0x00, MACSTR_LENGTH);

		if (error_code != WIFI_DIRECT_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to connect with peer[%s] -(%d)",
								connection->peer_name, error_code);
			format_str = D_("IDS_WIFI_POP_FAILED_TO_CONNECT_TO_PS");
			snprintf(txt, WFD_POP_STR_MAX_LEN, format_str, connection->peer_name);
		} else {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "Succeeded to connect with peer[%s] -(%d)",
								connection->peer_name, error_code);
			format_str = D_("IDS_WIFI_BODY_CONNECTED_TO_PS");
			snprintf(txt, WFD_POP_STR_MAX_LEN, format_str, connection->peer_name);
#ifdef WFD_SCREEN_MIRRORING_ENABLED
			result = vconf_get_int(VCONFKEY_SCREEN_MIRRORING_STATE, &screen_mirroring_status);
			if (result < 0)
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to get VCONFKEY_SCREEN_MIRRORING_STATE");

			if (screen_mirroring_status == VCONFKEY_SCREEN_MIRRORING_DEACTIVATED)
				snprintf(txt, WFD_POP_STR_MAX_LEN,  D_("IDS_WIFI_BODY_CONNECTED_TO_PS"),
								connection->peer_name);
#endif
		}
		if (connection)
			free(ad->connection);
		ad->connection = NULL;

		wfd_app_get_connected_peers(ad);
		WFD_APP_LOG(WFD_APP_LOG_LOW, "No of connected peers = %d", ad->raw_connected_peer_cnt);
		/* tickernoti popup */
		if (ad->raw_connected_peer_cnt < WFD_MAX_CONNECTED_PEER) {
			if (strlen(txt) > 0) {
				msg = elm_entry_utf8_to_markup(txt);
				WFD_RET_IF(!msg, "Failed to elm_entry_markup_to_utf8()!");
				notification_status_message_post(msg);
				WFD_IF_FREE_MEM(msg);
			}
		}
	}
	break;

	case WIFI_DIRECT_DISASSOCIATION_IND:
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_DISASSOCIATION_IND\n");
		wfd_app_util_del_wfd_connected_notification(ad);

#ifdef WFD_SCREEN_MIRRORING_ENABLED
		if (connection && connection->wifi_display)
			wfd_app_util_set_screen_mirroring_deactivated(ad);
#endif
		if (connection)
			free(ad->connection);
		ad->connection = NULL;

	}
	break;

	case WIFI_DIRECT_DISCONNECTION_IND:
	{
#if 0	// changed to show notification only when allshare cast device is connected.
		_del_wfd_notification();
#endif
	WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_DISCONNECTION_IND\n");
#ifdef WFD_SCREEN_MIRRORING_ENABLED
		wfd_app_util_set_screen_mirroring_deactivated(ad);
#endif
		notification_status_message_post(D_("IDS_WIFI_BODY_THE_WI_FI_DIRECT_CONNECTION_HAS_BEEN_LOST"));
		wfd_app_util_del_wfd_connected_notification(ad);
	}
	break;

	case WIFI_DIRECT_DISCONNECTION_RSP:
	{
#if 0	// changed to show notification only when allshare cast device is connected.
		_del_wfd_notification();
#endif
		WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_DISCONNECTION_RSP\n");

		wfd_app_util_del_wfd_connected_notification(ad);

#ifdef WFD_SCREEN_MIRRORING_ENABLED
		wfd_app_util_set_screen_mirroring_deactivated(ad);
#endif
		wfd_destroy_popup();

		result = wifi_direct_set_autoconnection_mode(false);
		WFD_APP_LOG(WFD_APP_LOG_LOW, "wifi_direct_set_autoconnection_mode() result=[%d]\n", result);
	}
	break;

	case WIFI_DIRECT_GROUP_DESTROYED:
	{
		WFD_APP_LOG(WFD_APP_LOG_LOW, "event ------------------ WIFI_DIRECT_GROUP_DESTROYED\n");
		notification_status_message_post(D_("IDS_WIFI_BODY_THE_WI_FI_DIRECT_CONNECTION_HAS_BEEN_LOST"));
	}
	break;

	default:
		break;

	}

	/*
	 * To support ON DEMAND popup destroy. API request blocks and popup destroy
	 * fails.
	 */
	/* wifi_direct_get_state(&ad->wfd_status); */

	if (vconf_get_int(VCONFKEY_WIFI_DIRECT_STATE, &wfd_state) < 0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Error reading vconf (%s)\n", VCONFKEY_WIFI_DIRECT_STATE);
	}
	WFD_APP_LOG(WFD_APP_LOG_LOW, "wfd state: %d", wfd_state);

	if (wfd_state == VCONFKEY_WIFI_DIRECT_DEACTIVATED) {
		wfd_app_util_del_notification(ad);
	}

#ifdef NOT_CONNECTED_INDICATOR_ICON
	if (wfd_state >= VCONFKEY_WIFI_DIRECT_CONNECTED) {
		noti_err  = notification_delete(ad->noti_wifi_direct_on);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Fail to notification_delete.(%d)\n", noti_err);
		} else {
			noti_err = notification_free(ad->noti_wifi_direct_on);
			ad->noti_wifi_direct_on = NULL;
			if (noti_err != NOTIFICATION_ERROR_NONE) {
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Fail to notification_free.(%d)\n", noti_err);
			}
		}
	}
#endif

	if (wfd_state < VCONFKEY_WIFI_DIRECT_CONNECTED) {
	    if (ad->transmit_timer) {
		    ecore_timer_del(ad->transmit_timer);
		    ad->transmit_timer = NULL;
	    }
	} else {
		if (NULL == ad->transmit_timer) {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "start the transmit timer\n");
			ad->last_wfd_transmit_time = time(NULL);
			ad->transmit_timer = ecore_timer_add(5.0,
				(Ecore_Task_Cb)wfd_automatic_deactivated_for_connection_cb, ad);
		}
	}

	if (wfd_state >= VCONFKEY_WIFI_DIRECT_CONNECTED) {
#ifdef WFD_FIVE_MIN_IDLE_DEACTIVATION
		if (ad->monitor_timer) {
			ecore_timer_del(ad->monitor_timer);
			ad->monitor_timer = NULL;
		}
#endif
	} else {
#ifdef NOT_CONNECTED_INDICATOR_ICON
		wfd_app_util_add_indicator_icon(ad);
#endif
#ifdef WFD_FIVE_MIN_IDLE_DEACTIVATION
		if (NULL == ad->monitor_timer) {
			ad->last_wfd_time = time(NULL);
			ad->monitor_timer = ecore_timer_add(5.0,
				(Ecore_Task_Cb)_wfd_automatic_deactivated_for_no_connection_cb, ad);
		}
#endif
	}

	__WFD_APP_FUNC_EXIT__;
}

/**
 *	This function let the app do initialization
 *	@return   If success, return TRUE, else return FALSE
 *	@param[in] ad the pointer to the main data structure
 */
bool init_wfd_client(wfd_appdata_t *ad)
{
	__WFD_APP_FUNC_ENTER__;
	WFD_RETV_IF(NULL == ad, FALSE, "NULL parameters.\n");
	int ret = -1;
	int retrys = 3;

	ad->last_wfd_status = WIFI_DIRECT_STATE_DEACTIVATED;

	while (retrys > 0) {
		ret = wifi_direct_initialize();
		if (ret == WIFI_DIRECT_ERROR_NONE ||
			ret == WIFI_DIRECT_ERROR_ALREADY_INITIALIZED)
			break;
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to initialize Wi-Fi Direct(%d)\n", ret);

		retrys--;
		if (retrys == 0)
			return FALSE;
		usleep(100*1000);
	}

	ret = wifi_direct_set_device_state_changed_cb(_cb_activation, (void*) ad);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to register _cb_activation(%d)\n", ret);
		return FALSE;
	}

	ret = wifi_direct_set_connection_state_changed_cb(_cb_connection, (void*) ad);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to register _cb_connection(%d)\n", ret);
		return FALSE;
	}

#ifdef WFD_FIVE_MIN_IDLE_DEACTIVATION
	if (NULL == ad->monitor_timer) {
		ad->last_wfd_time = time(NULL);
		ad->monitor_timer = ecore_timer_add(5.0,
				(Ecore_Task_Cb)_wfd_automatic_deactivated_for_no_connection_cb, ad);
	}
#endif
	__WFD_APP_FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the app do de-initialization
 *	@return   If success, return TRUE, else return FALSE
 *	@param[in] ad the pointer to the main data structure
 */
int deinit_wfd_client(wfd_appdata_t *ad)
{
	__WFD_APP_FUNC_ENTER__;
	int ret = -1;

	if (NULL == ad) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "NULL parameter\n");
		return -1;
	}

#ifdef WFD_FIVE_MIN_IDLE_DEACTIVATION
	if (ad->monitor_timer) {
		ecore_timer_del(ad->monitor_timer);
		ad->monitor_timer = NULL;
	}
#endif

	ret = wifi_direct_unset_device_state_changed_cb();
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to deregister _cb_activation(%d)\n", ret);
	}

	ret = wifi_direct_unset_connection_state_changed_cb();
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to deregister _cb_connection(%d)\n", ret);
	}

	ret = wifi_direct_deinitialize();
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to deinitialize Wi-Fi Direct. error code = [%d]\n", ret);
	}

	__WFD_APP_FUNC_EXIT__;
	return 0;
}
