/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This file declares Wi-Fi direct client functions.
 *
 * @file    wfd_client.h
 * @author  Gibyoung Kim (lastkgb.kim@samsung.com)
 * @version 0.1
 */


#ifndef __WFD_CLIENT_H__
#define __WFD_CLIENT_H__


typedef enum
{
    WFD_DEVICE_TYPE_COMPUTER = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_COMPUTER,
    WFD_DEVICE_TYPE_INPUT_DEVICE = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_INPUT_DEVICE,
    WFD_DEVICE_TYPE_PRINTER = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_PRINTER,
    WFD_DEVICE_TYPE_CAMERA = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_CAMERA,
    WFD_DEVICE_TYPE_STORAGE = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_STORAGE,
    WFD_DEVICE_TYPE_NW_INFRA = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_NETWORK_INFRA,
    WFD_DEVICE_TYPE_DISPLAYS = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_DISPLAY,
    WFD_DEVICE_TYPE_MM_DEVICES =
        WIFI_DIRECT_PRIMARY_DEVICE_TYPE_MULTIMEDIA_DEVICE,
    WFD_DEVICE_TYPE_GAME_DEVICES = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_GAME_DEVICE,
    WFD_DEVICE_TYPE_TELEPHONE = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_TELEPHONE,
    WFD_DEVICE_TYPE_AUDIO = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_AUDIO,
    WFD_DEVICE_TYPE_OTHER = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_OTHER,
} device_type_e;

typedef enum
{
    WFD_LINK_STATUS_DEACTIVATED = WIFI_DIRECT_STATE_DEACTIVATED,
    WFD_LINK_STATUS_DEACTIVATING = WIFI_DIRECT_STATE_DEACTIVATING,
    WFD_LINK_STATUS_ACTIVATING = WIFI_DIRECT_STATE_ACTIVATING,
    WFD_LINK_STATUS_ACTIVATED = WIFI_DIRECT_STATE_ACTIVATED,
    WFD_LINK_STATUS_DISCOVERING = WIFI_DIRECT_STATE_DISCOVERING,
    WFD_LINK_STATUS_CONNECTING = WIFI_DIRECT_STATE_CONNECTING,
    WFD_LINK_STATUS_DISCONNECTING = WIFI_DIRECT_STATE_DISCONNECTING,
    WFD_LINK_STATUS_CONNECTED = WIFI_DIRECT_STATE_CONNECTED,
    WFD_LINK_STATUS_GROUP_OWNER = WIFI_DIRECT_STATE_GROUP_OWNER,
} link_status_e;

typedef enum
{
    PEER_CONN_STATUS_DISCONNECTED,
    PEER_CONN_STATUS_DISCONNECTING,
    PEER_CONN_STATUS_CONNECTING = PEER_CONN_STATUS_DISCONNECTING,
    PEER_CONN_STATUS_CONNECTED,
} conn_status_e;

int wfd_get_vconf_status(void *data);
int wfd_wifi_off();
int init_wfd_client(void *data);
int deinit_wfd_client(void *data);
int wfd_client_get_link_status(void);
int wfd_client_start_discovery(void *data);
int wfd_client_switch_on(void *data);
int wfd_client_switch_off(void *data);
int wfd_client_swtch_force(void *data, int onoff);
int wfd_client_connect(const char *mac_addr);
int wfd_client_disconnect(const char *mac_addr);

#endif                          /* __WFD_CLIENT_H__ */
