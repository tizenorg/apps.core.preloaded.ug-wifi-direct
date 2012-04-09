/*
 * Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of ug-setting-wifidirect-efl
 * Written by Gibyoung Kim <lastkgb.kim@samsung.com>
 *            Dongwook Lee <dwmax.lee@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of
 * SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with SAMSUNG ELECTRONICS. 
 *
 * SAMSUNG make no representations or warranties about the suitability
 * of the software, either express or implied, including but not limited
 * to the implied warranties of merchantability, fitness for a particular
 * purpose, or non-infringement. SAMSUNG shall not be liable for any
 * damages suffered by licensee as a result of using, modifying or
 * distributing this software or its derivatives.
 *
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
