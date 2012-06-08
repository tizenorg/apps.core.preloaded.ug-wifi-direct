/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.tizenopensource.org/license
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * This file implements wifi direct application client  functions.
 *
 * @file    wfd-app-client.c
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */

#include <stdio.h>
#include <string.h>
#include "wifi-direct.h"
#include "wfd-app.h"
#include "wfd-app-util.h"


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
    int result;

    switch (connection_state)
    {
    case WIFI_DIRECT_CONNECTION_RSP:
        {
            WFD_APP_LOG(WFD_APP_LOG_LOW,
                        "event ------------------ WIFI_DIRECT_CONNECTION_RSP\n");

            if (error_code == WIFI_DIRECT_ERROR_NONE)
            {
                WFD_APP_LOG(WFD_APP_LOG_LOW, "Link Complete!\n");
                wfd_prepare_popup(WFD_POP_NOTI_CONNECTED, NULL);
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

                wfd_prepare_popup(WFD_POP_FAIL_CONNECT, NULL);

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

            if (config->wps_config == WIFI_DIRECT_WPS_PUSHBUTTON)
            {
                WFD_APP_LOG(WFD_APP_LOG_LOW,
                            "wps_config is WFD_WPS_PUSHBUTTON. Ignore it..\n");
            }
            else if (config->wps_config == WIFI_DIRECT_WPS_KEYPAD)
            {
                WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is WFD_WPS_KEYPAD\n");

                result = wifi_direct_generate_wps_pin();
                WFD_APP_LOG(WFD_APP_LOG_LOW,
                            "wifi_direct_client_generate_wps_pin() result=[%d]\n",
                            result);

                char *pin_number = NULL;
                result = wifi_direct_get_wps_pin(&pin_number);
                WFD_APP_LOG(WFD_APP_LOG_LOW,
                            "wifi_direct_client_get_wps_pin() result=[%d]. pin=[%s]\n",
                            result, ad->pin_number);

                strncpy(ad->pin_number, pin_number, 32);

                result = wifi_direct_accept_connection(ad->peer_mac);
                WFD_APP_LOG(WFD_APP_LOG_LOW,
                            "wifi_direct_accept_connection[%s] result=[%d].\n",
                            ad->peer_mac, result);

                result = wifi_direct_activate_pushbutton();
                wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_PIN, NULL);

                if (pin_number != NULL)
                    free(pin_number);
            }
            else if (config->wps_config == WIFI_DIRECT_WPS_DISPLAY)
            {
                WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is WFD_WPS_DISPLAY\n");
                wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_KEYPAD,
                                  (void *) NULL);
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
		wfd_device_info_t *peer_info = NULL;

		if (NULL == mac_address)
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "ERROR : incomming_peer_mac is NULL !!\n");
			return;
		}

		WFD_APP_LOG(WFD_APP_LOG_LOW, "Connection Request from MAC[%s]\n", mac_address);
		strncpy(ad->peer_mac, mac_address, strlen(mac_address));

		peer_info = _wfd_app_find_peer_by_mac_address(ad, mac_address);

		if (NULL != peer_info->ssid)
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "Connection Request from SSID[%s]\n", peer_info->ssid);
			strncpy(ad->peer_name, peer_info->ssid, strlen(peer_info->ssid));
		}
		else
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "incomming_peer SSID is NULL !!\n");
		}

		if (ad->peer_name == NULL || strlen(ad->peer_name) == 0)
			strncpy(ad->peer_name, ad->peer_mac, strlen(ad->peer_mac));

		result = wifi_direct_get_config_data(&config);
		WFD_APP_LOG(WFD_APP_LOG_LOW, "wifi_direct_client_get_config_data() result=[%d]\n", result);

		if (config->wps_config == WIFI_DIRECT_WPS_PUSHBUTTON)
		{
			char pushbutton;
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is WFD_WPS_PUSHBUTTON\n");

			wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_PUSHBUTTON_REQ, NULL);
		}
		else if (config->wps_config == WIFI_DIRECT_WPS_DISPLAY)
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is WFD_WPS_DISPLAY\n");

			result = wifi_direct_generate_wps_pin();
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wifi_direct_client_generate_wps_pin() result=[%d]\n", result);

			char *pin_number = NULL;
			result = wifi_direct_get_wps_pin(&pin_number);
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wifi_direct_get_wps_pin() result=[%d]\n", result);

			strncpy(ad->pin_number, pin_number, 32);

			wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_DISPLAY_REQ, NULL);
			
			if (pin_number != NULL)
				free(pin_number);
		}
		else if (config->wps_config == WIFI_DIRECT_WPS_KEYPAD)
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is WFD_WPS_KEYPAD\n");
			wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ, (void *) NULL);
		}
		else
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wps_config is unkown!\n");
		}
		
		if (config != NULL)
			free(config);
        }
        break;

    case WIFI_DIRECT_DISCONNECTION_IND:
        {
            WFD_APP_LOG(WFD_APP_LOG_LOW,
                        "event ------------------ WIFI_DIRECT_DISCONNECTION_IND\n");
        }
        break;

    case WIFI_DIRECT_DISCONNECTION_RSP:
        {
            wfd_destroy_popup();

            result = wifi_direct_start_discovery(FALSE, 0);
            WFD_APP_LOG(WFD_APP_LOG_LOW,
                        "wifi_direct_start_discovery() result=[%d]\n", result);
        }
        break;

    default:
        break;

    }

    __WFD_APP_FUNC_EXIT__;
}


int init_wfd_popup_client(wfd_appdata_t * ad)
{
    __WFD_APP_FUNC_ENTER__;
    int ret;

    ret = wifi_direct_initialize();

    ret = wifi_direct_set_device_state_changed_cb(_cb_activation, (void *) ad);
    ret = wifi_direct_set_discovery_state_changed_cb(_cb_discover, (void *) ad);
    ret =
        wifi_direct_set_connection_state_changed_cb(_cb_connection,
                                                    (void *) ad);

    __WFD_APP_FUNC_EXIT__;

    if (ret)
        return TRUE;
    else
        return FALSE;
}

int deinit_wfd_popup_client(void)
{
    __WFD_APP_FUNC_ENTER__;

    int ret;

    ret = wifi_direct_deinitialize();

    __WFD_APP_FUNC_EXIT__;

    if (ret)
        return TRUE;
    else
        return FALSE;
}
