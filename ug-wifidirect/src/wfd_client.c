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

#include <stdio.h>
#include <stdbool.h>
#include <libintl.h>

#include <Elementary.h>
#include <pmapi.h>
#include <vconf.h>
#include <network-cm-intf.h>
#include <network-wifi-intf.h>
#include <wifi-direct.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"




static void _wifi_state_cb(keynode_t *key, void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    int res;
    int wifi_state;

    res = vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);
    if (res != 0)
    {
        DBG(LOG_ERROR, "Failed to get wifi state from vconf. [%d]\n", res);
        return;
    }

    if (wifi_state == VCONFKEY_WIFI_OFF)
    {
        DBG(LOG_VERBOSE, "WiFi is turned off\n");
        wfd_client_swtch_force(ugd, TRUE);
    }
    else
    {
        DBG(LOG_VERBOSE, "WiFi is turned on\n");
    }

    res = net_deregister_client();
    if (res != NET_ERR_NONE)
    {
        DBG(LOG_ERROR, "Failed to deregister network client. [%d]\n", res);
    }

    __FUNC_EXIT__;
}

static void _network_event_cb(net_event_info_t *event_info, void *user_data)
{
    __FUNC_ENTER__;
    DBG(LOG_VERBOSE, "Event from network. [%d]\n", event_info->Event);
    __FUNC_EXIT__;
}

int wfd_wifi_off(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    int res;

    res = vconf_notify_key_changed(VCONFKEY_WIFI_STATE, _wifi_state_cb, ugd);
    if (res == -1)
    {
        DBG(LOG_ERROR, "Failed to register vconf callback\n");
        return -1;
    }
    DBG(LOG_VERBOSE, "Vconf key callback is registered\n");
    res = net_register_client((net_event_cb_t) _network_event_cb, NULL);
    if (res != NET_ERR_NONE)
    {
        DBG(LOG_ERROR, "Failed to register network client. [%d]\n", res);
        return -1;
    }
    DBG(LOG_VERBOSE, "Network client is registered\n");
    res = net_wifi_power_off();
    if (res != NET_ERR_NONE)
    {
        DBG(LOG_ERROR, "Failed to turn off wifi. [%d]\n", res);
        return -1;
    }
    DBG(LOG_VERBOSE, "WiFi power off\n");
    __FUNC_EXIT__;
    return 0;
}

static device_type_s *wfd_client_find_peer_by_ssid(void *data, const char *ssid)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    int i;

    if (ugd == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    for (i = 0; i < ugd->peer_cnt; i++)
    {
        DBG(LOG_VERBOSE, "check %dth peer\n", i);
        if (!strcmp(ugd->peers[i].ssid, ssid))
        {
            DBG(LOG_VERBOSE, "found peer. [%d]\n", i);
            __FUNC_EXIT__;
            return &ugd->peers[i];
        }
    }
    __FUNC_EXIT__;

    return NULL;
}

static device_type_s *wfd_client_find_peer_by_mac(void *data,
                                                  const char *mac_addr)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    int i;

    if (ugd == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    for (i = 0; i < ugd->peer_cnt; i++)
    {
        DBG(LOG_VERBOSE, "check %dth peer\n", i);
        if (!strncmp
            (mac_addr, (const char *) ugd->peers[i].mac_addr, MAC_LENGTH))
        {
            DBG(LOG_VERBOSE, "found peer. [%d]\n", i);
            __FUNC_EXIT__;
            return &ugd->peers[i];
        }
    }
    __FUNC_EXIT__;

    return NULL;
}

void _activation_cb(int error_code, wifi_direct_device_state_e device_state,
                    void *user_data)
{
    __FUNC_ENTER__;
    int res;
    wifi_direct_state_e wfd_status;
    struct ug_data *ugd = (struct ug_data *) user_data;

    wifi_direct_get_state(&wfd_status);
    DBG(LOG_VERBOSE, "WFD status [%d]", wfd_status);
    ugd->wfd_status = wfd_status;

    switch (device_state)
    {
    case WIFI_DIRECT_DEVICE_STATE_ACTIVATED:
        DBG(LOG_VERBOSE, "WIFI_DIRECT_DEVICE_STATE_ACTIVATED\n");
        if(error_code != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Error in Activation/Deactivation [%d]\n", error_code);
            wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_ACTIVATE_FAIL"), POPUP_TYPE_ACTIVATE_FAIL);

            ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
            ugd->wfd_onoff = 0;
            wfd_ug_view_refresh_glitem(ugd->head);
            return;
        }

        ugd->head_text_mode = HEAD_TEXT_TYPE_ACTIVATED;
        ugd->wfd_onoff = 1;
        wfd_ug_view_refresh_glitem(ugd->head);
        res = vconf_set_int("db/wifi_direct/onoff", ugd->wfd_onoff);
        if (res != 0)
        {
            DBG(LOG_ERROR, "Failed to set vconf value for WFD onoff status\n");
        }
        wfg_ug_act_popup_remove(ugd);

        res = vconf_ignore_key_changed(VCONFKEY_WIFI_STATE, _wifi_state_cb);
        if (res == -1)
        {
            DBG(LOG_ERROR,
                "Failed to ignore vconf key callback for wifi state\n");
        }

        res = wifi_direct_start_discovery(FALSE, 0);
        if (res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", res);
        }
        break;
        DBG(LOG_VERBOSE, "Discovery is started\n");
    case WIFI_DIRECT_DEVICE_STATE_DEACTIVATED:
        DBG(LOG_VERBOSE, "WIFI_DIRECT_DEVICE_STATE_DEACTIVATED\n");
        if(error_code != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Error in Activation/Deactivation [%d]\n", error_code);
            wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_DEACTIVATE_FAIL"), POPUP_TYPE_DEACTIVATE_FAIL);
            ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
            ugd->wfd_onoff = 1;
            wfd_ug_view_refresh_glitem(ugd->head);
            return;
        }

        ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
        ugd->wfd_onoff = 0;
        wfd_ug_view_refresh_glitem(ugd->head);
        res = vconf_set_int("db/wifi_direct/onoff", ugd->wfd_onoff);
        if (res != 0)
        {
            DBG(LOG_ERROR, "Failed to set vconf value for WFD onoff status\n");
        }
        wfd_ug_view_free_peers(ugd);
        wfd_ug_view_update_peers(ugd);
        break;
    default:
        break;
    }
    wfd_ug_view_refresh_button(ugd->scan_btn, ugd->wfd_onoff);

    __FUNC_EXIT__;
    return;
}

static int peer_cnt;
static int connected_cnt;
static int discovered_cnt;

bool _wfd_discoverd_peer_cb(wifi_direct_discovered_peer_info_s * peer,
                            void *user_data)
{
    __FUNC_ENTER__;
    device_type_s *peers = (device_type_s *) user_data;

    DBG(LOG_VERBOSE, "%dth discovered peer. [%s]\n", peer_cnt, peer->ssid);
    if (peer->is_connected == TRUE)
        return FALSE;
    memcpy(peers[peer_cnt].ssid, peer->ssid, SSID_LENGTH);
    peers[peer_cnt].ssid[31] = '\0';
    DBG(LOG_VERBOSE, "\tSSID: [%s]\n", peers[peer_cnt].ssid);
    peers[peer_cnt].category = peer->primary_device_type;
    DBG(LOG_VERBOSE, "\tPeer category [%d] -> [%d]\n", peer->primary_device_type, peers[peer_cnt].category);
    strncpy(peers[peer_cnt].mac_addr, peer->mac_address, MAC_LENGTH);
    strncpy(peers[peer_cnt].if_addr, peer->interface_address, MAC_LENGTH);
    peers[peer_cnt].conn_status = PEER_CONN_STATUS_DISCONNECTED;
    DBG(LOG_VERBOSE, "\tStatus: [%d]\n", peers[peer_cnt].conn_status);
    peer_cnt++;

    free(peer->ssid);
    free(peer->mac_address);
    free(peer->interface_address);
    free(peer);
    __FUNC_EXIT__;
    return TRUE;
}

bool _wfd_connected_peer_cb(wifi_direct_connected_peer_info_s * peer,
                            void *user_data)
{
    __FUNC_ENTER__;
    device_type_s *peers = (device_type_s *) user_data;

    DBG(LOG_VERBOSE, "%dth connected peer. [%s]\n", peer_cnt, peer->ssid);
    memcpy(peers[peer_cnt].ssid, peer->ssid, SSID_LENGTH);
    peers[peer_cnt].ssid[31] = '\0';
    DBG(LOG_VERBOSE, "\tSSID: [%s]\n", peers[peer_cnt].ssid);
    peers[peer_cnt].category = peer->primary_device_type;
    DBG(LOG_VERBOSE, "\tCategory: [%d]\n", peers[peer_cnt].category);
    strncpy(peers[peer_cnt].mac_addr, peer->mac_address, MAC_LENGTH);
    strncpy(peers[peer_cnt].if_addr, peer->interface_address, MAC_LENGTH);
    peers[peer_cnt].conn_status = PEER_CONN_STATUS_CONNECTED;
    DBG(LOG_VERBOSE, "\tStatus: [%d]\n", peers[peer_cnt].conn_status);
    peer_cnt++;

    free(peer->ssid);
    free(peer->mac_address);
    free(peer->interface_address);
    free(peer);
    __FUNC_EXIT__;
    return TRUE;
}

void _discover_cb(int error_code, wifi_direct_discovery_state_e discovery_state,
                  void *user_data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) user_data;
    int res;
    device_type_s *peers = NULL;

    if (ugd == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return;
    }

    peers = calloc(MAX_PEER_NUM, sizeof(device_type_s));

    if (discovery_state == WIFI_DIRECT_ONLY_LISTEN_STARTED)
    {
        ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
    }
    else if (discovery_state == WIFI_DIRECT_DISCOVERY_STARTED)
    {
        ugd->head_text_mode = HEAD_TEXT_TYPE_SCANING;
    }
    wfd_ug_view_refresh_glitem(ugd->head);

    if (ugd->wfd_status < WFD_LINK_STATUS_ACTIVATED
        || ugd->wfd_status > WFD_LINK_STATUS_GROUP_OWNER)
    {
        return;
    }
    else
    {
        peer_cnt = 0;
    }

    if (ugd->wfd_status >= WFD_LINK_STATUS_CONNECTED)
    {
        DBG(LOG_VERBOSE, "getting connected peer..\n");
        res =
            wifi_direct_foreach_connected_peers(_wfd_connected_peer_cb,
                                                (void *) peers);
        if (res != WIFI_DIRECT_ERROR_NONE)
        {
            connected_cnt = 0;
            DBG(LOG_ERROR, "get discovery result failed: %d\n", res);
        }
    }

    if (discovery_state == WIFI_DIRECT_DISCOVERY_FOUND)
    {
        DBG(LOG_VERBOSE, "Peer is found\n");
        ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
        wfd_ug_view_refresh_glitem(ugd->head);

        if (ugd->wfd_status >= WFD_LINK_STATUS_ACTIVATED)
        {
            DBG(LOG_VERBOSE, "getting discovered peer..\n");
            res =
                wifi_direct_foreach_discovered_peers(_wfd_discoverd_peer_cb,
                                                     (void *) peers);
            if (res != WIFI_DIRECT_ERROR_NONE)
            {
                discovered_cnt = 0;
                DBG(LOG_ERROR, "get discovery result failed: %d\n", res);
            }
        }
    }

    wfd_ug_view_free_peers(ugd);

    ugd->peers = peers;
    ugd->peer_cnt = peer_cnt;

    wfd_ug_view_update_peers(ugd);
    DBG(LOG_VERBOSE, "%d peers are updated\n", peer_cnt);

    __FUNC_EXIT__;
    return;
}

void _connection_cb(int error_code,
                    wifi_direct_connection_state_e connection_state,
                    const char *mac_address, void *user_data)
{
    __FUNC_ENTER__;
    DBG(LOG_VERBOSE, "Connection event [%d], error_code [%d]\n",
        connection_state, error_code);
    struct ug_data *ugd = (struct ug_data *) user_data;
    device_type_s *peer = NULL;
    bool owner = FALSE;
    int res = 0;

    if (mac_address == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(peer mac is NULL)\n");
        return;
    }
    DBG(LOG_VERBOSE, "Connection event from %s", mac_address);

    peer = wfd_client_find_peer_by_mac(ugd, mac_address);
    if (peer == NULL)
    {
        DBG(LOG_ERROR, "Failed to find peer [mac: %s]\n", mac_address);
        return;
    }

    switch (connection_state)
    {
    case WIFI_DIRECT_CONNECTION_RSP:
        DBG(LOG_VERBOSE, "WIFI_DIRECT_CONNECTION_RSP\n");

        if (error_code == WIFI_DIRECT_ERROR_NONE)
        {
            ugd->wfd_status = WFD_LINK_STATUS_CONNECTED;
            peer->conn_status = PEER_CONN_STATUS_CONNECTED;
            res = wifi_direct_is_group_owner(&owner);
            if (res == WIFI_DIRECT_ERROR_NONE)
            {
                if (!owner)
                    wfd_ug_view_refresh_button(ugd->scan_btn, FALSE);
            }
            else
            {
                DBG(LOG_ERROR,
                    "Failed to get whether client is group owner. [%d]\n", res);
            }
        }
        else
        {
            peer->conn_status = PEER_CONN_STATUS_DISCONNECTED;
            wifi_direct_start_discovery(FALSE, 0);
        }
        break;
    case WIFI_DIRECT_DISCONNECTION_RSP:
    case WIFI_DIRECT_DISCONNECTION_IND:
    case WIFI_DIRECT_DISASSOCIATION_IND:
        DBG(LOG_VERBOSE, "WIFI_DIRECT_DISCONNECTION_X\n");
        if (error_code != WIFI_DIRECT_ERROR_NONE)
        {
            return;
        }

        peer->conn_status = PEER_CONN_STATUS_DISCONNECTED;

        ugd->wfd_status = WFD_LINK_STATUS_ACTIVATED;
        ugd->head_text_mode = HEAD_TEXT_TYPE_ACTIVATED;

        wfd_ug_view_refresh_button(ugd->scan_btn, TRUE);
        wifi_direct_start_discovery(FALSE, 0);
        ugd->wfd_status = WFD_LINK_STATUS_DISCOVERING;
        ugd->head_text_mode = HEAD_TEXT_TYPE_SCANING;
        wfd_ug_view_refresh_glitem(ugd->head);
        break;
    case WIFI_DIRECT_CONNECTION_IN_PROGRESS:
        DBG(LOG_VERBOSE, "WIFI_DIRECT_CONNECTION_IN_PROGRESS\n");
        peer->conn_status = PEER_CONN_STATUS_CONNECTING;
        break;
    case WIFI_DIRECT_CONNECTION_REQ:
    case WIFI_DIRECT_CONNECTION_WPS_REQ:
        DBG(LOG_VERBOSE, "WIFI_DIRECT_CLI_EVENT_CONNECTION_REQ\n");
        break;
    default:
        break;
    }

    if (peer != NULL)
        wfd_ug_view_refresh_glitem(peer->gl_item);

    __FUNC_EXIT__;
    return;
}

int wfd_get_vconf_status(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    int res;
    char *dev_name;

    res = vconf_get_int("db/wifi_direct/onoff", &ugd->wfd_onoff);
    if (res != 0)
    {
        DBG(LOG_ERROR, "vconf_get_int is failed\n");
    }
    DBG(LOG_VERBOSE, "VCONF_WFD_ONOFF : %d\n", ugd->wfd_onoff);

    dev_name = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);
    if (dev_name == NULL)
    {
        ugd->dev_name = strdup(DEFAULT_DEV_NAME);
        DBG(LOG_ERROR, "The AP name is NULL(setting default value)\n");
    }
    else
    {
        ugd->dev_name = strdup(dev_name);
        free(dev_name);
    }

    __FUNC_EXIT__;

    return 0;
}

int init_wfd_client(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    int res = 0;
    wifi_direct_state_e wfd_status;

    res = wifi_direct_initialize();
    if (res != WIFI_DIRECT_ERROR_NONE)
    {
        DBG(LOG_ERROR, "Failed to initialize wifi direct. [%d]\n", res);
        return -1;
    }

    res = wifi_direct_set_device_state_changed_cb(_activation_cb, (void *) ugd);
    res =
        wifi_direct_set_discovery_state_changed_cb(_discover_cb, (void *) ugd);
    res =
        wifi_direct_set_connection_state_changed_cb(_connection_cb,
                                                    (void *) ugd);

    res = wifi_direct_get_state(&wfd_status);
    if (res != WIFI_DIRECT_ERROR_NONE)
    {
        DBG(LOG_ERROR, "Failed to get link status. [%d]\n", res);
        return -1;
    }
    ugd->wfd_status = wfd_status;
    DBG(LOG_VERBOSE, "WFD link status. [%d]\n", wfd_status);

    if (wfd_status > WIFI_DIRECT_STATE_ACTIVATING)
    {
        vconf_set_int("db/wifi_direct/onoff", 1);
        ugd->wfd_onoff = 1;
    }
    else
    {
        vconf_set_int("db/wifi_direct/onoff", 0);
        ugd->wfd_onoff = 0;
    }
    wfd_ug_view_refresh_glitem(ugd->head);
    wfd_ug_view_refresh_button(ugd->scan_btn, ugd->wfd_onoff);

    if(wfd_status >= WIFI_DIRECT_STATE_CONNECTED)
    {
        device_type_s *peers = NULL;

        peers = calloc(MAX_PEER_NUM, sizeof(device_type_s));
        res = wifi_direct_foreach_connected_peers(_wfd_connected_peer_cb, (void*) peers);
        if(res != WIFI_DIRECT_ERROR_NONE)
        {
            connected_cnt = 0;
            DBG(LOG_ERROR, "get discovery result failed: %d\n", res);
        }
        wfd_ug_view_free_peers(ugd);

        ugd->peers = peers;
        ugd->peer_cnt = peer_cnt;

        wfd_ug_view_update_peers(ugd);
        DBG(LOG_VERBOSE, "%d peers are updated\n", peer_cnt);
    }

    if (wfd_status > WIFI_DIRECT_STATE_ACTIVATING)
    {
        int wifi_state;
        vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);

        if (wifi_state < VCONFKEY_WIFI_CONNECTED)
        {
            res = wifi_direct_start_discovery(FALSE, 0);
            if (res != WIFI_DIRECT_ERROR_NONE)
            {
                DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", res);
            }
            DBG(LOG_VERBOSE, "Discovery is started\n");
        }
        else
        {
            wfd_ug_act_popup(ugd, _("IDS_WFD_POP_WIFI_OFF"),
                             POPUP_TYPE_WIFI_OFF);
        }
    }

    __FUNC_EXIT__;

    return 0;
}

int deinit_wfd_client(void *data)
{
    __FUNC_ENTER__;
    int res = 0;
    wifi_direct_state_e status = 0;

    wifi_direct_get_state(&status);

    if (status == WIFI_DIRECT_STATE_DISCOVERING)
    {
        DBG(LOG_VERBOSE, "Stop discovery before deregister client\n");
        wifi_direct_cancel_discovery();
    }

    res = wifi_direct_deinitialize();
    if (res != WIFI_DIRECT_ERROR_NONE)
    {
        DBG(LOG_ERROR, "Failed to deregister client. [%d]\n", res);
    }

    res = vconf_ignore_key_changed(VCONFKEY_WIFI_STATE, _wifi_state_cb);
    if (res == -1)
    {
        DBG(LOG_ERROR, "Failed to ignore vconf key callback for wifi state\n");
    }

    res = net_deregister_client();
    if (res != NET_ERR_NONE)
    {
        DBG(LOG_ERROR, "Failed to deregister network client. [%d]\n", res);
    }


    __FUNC_EXIT__;

    return 0;
}

int wfd_client_get_link_status()
{
    __FUNC_ENTER__;
    wifi_direct_state_e status;
    int res;

    res = wifi_direct_get_state(&status);
    if (res != WIFI_DIRECT_ERROR_NONE)
    {
        DBG(LOG_ERROR, "Failed to get link status from wfd-server. [%d]", res);
        return -1;
    }

    __FUNC_EXIT__;
    return status;
}

int wfd_client_switch_on(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    int res;

    DBG(LOG_VERBOSE, "WFD status [%d]\n", ugd->wfd_status);

    if (ugd->wfd_status < WFD_LINK_STATUS_ACTIVATING)
    {
        ugd->wfd_status = WFD_LINK_STATUS_ACTIVATING;

        int wifi_state;
        res = vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);
        if (res != 0)
        {
            DBG(LOG_ERROR, "Failed to get wifi state from vconf. [%d]\n", res);
            return -1;
        }

        if (wifi_state > VCONFKEY_WIFI_OFF)
        {
            DBG(LOG_VERBOSE, "WiFi is connected, so have to turn off WiFi");
            wfd_ug_act_popup(ugd, _("IDS_WFD_POP_WIFI_OFF"),
                             POPUP_TYPE_WIFI_OFF);
        }
        else
        {
            res = wifi_direct_activate();
            if (res != WIFI_DIRECT_ERROR_NONE)
            {
                DBG(LOG_ERROR,
                    "Failed to activate Wi-Fi Direct. error code = [%d]\n",
                    res);
                wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_ACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

                ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
                wfd_ug_view_refresh_glitem(ugd->head);
                return -1;
            }
        }
    }
    else
    {
        DBG(LOG_VERBOSE, "Wi-Fi Direct is already activated\n");
    }

    __FUNC_EXIT__;
    return 0;
}

int wfd_client_switch_off(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    int res;

    DBG(LOG_VERBOSE, "WFD status [%d]\n", ugd->wfd_status);

    if (ugd->wfd_status < WFD_LINK_STATUS_ACTIVATING)
    {
        DBG(LOG_VERBOSE, "Wi-Fi Direct is already deactivated\n");
    }
    else
    {
        ugd->wfd_status = WFD_LINK_STATUS_DEACTIVATING;

        res = wifi_direct_deactivate();
        if (res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR,
                "Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
            wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_DEACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

            ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
            wfd_ug_view_refresh_glitem(ugd->head);
            return -1;
        }
    }

    __FUNC_EXIT__;
    return 0;
}

int wfd_client_swtch_force(void *data, int onoff)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    int res;

    if (onoff)
    {
        res = wifi_direct_activate();
        if (res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR,
                "Failed to activate Wi-Fi Direct. error code = [%d]\n", res);
            wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_ACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

            ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
            wfd_ug_view_refresh_glitem(ugd->head);
            return -1;
        }
    }
    else
    {
        res = wifi_direct_deactivate();
        if (res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR,
                "Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
            wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_DEACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

            ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
            wfd_ug_view_refresh_glitem(ugd->head);
            return -1;
        }
    }
    __FUNC_EXIT__;
    return 0;
}

int wfd_client_start_discovery(void *data)
{
    __FUNC_ENTER__;
    int res;
    wifi_direct_state_e status;

    wifi_direct_get_state(&status);
    if (status >= WIFI_DIRECT_STATE_ACTIVATED)
    {
        res = wifi_direct_start_discovery(FALSE, 0);
        if (res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Failed to start wfd discovery. [%d]", res);
        }
    }
    __FUNC_EXIT__;

    return 0;
}

int wfd_client_connect(const char *mac_addr)
{
    __FUNC_ENTER__;
    int res;

    res = wifi_direct_connect(mac_addr);
    if (res != WIFI_DIRECT_ERROR_NONE)
    {
        DBG(LOG_ERROR, "Failed to send connection request. [%d]\n", res);
        return -1;
    }
    __FUNC_EXIT__;
    return 0;
}

int wfd_client_disconnect(const char *mac_addr)
{
    __FUNC_ENTER__;
    int res;

    if (mac_addr == NULL)
    {
        res = wifi_direct_disconnect_all();
        if (res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR,
                "Failed to send disconnection request to all. [%d]\n", res);
            return -1;
        }
    }
    else
    {
        res = wifi_direct_disconnect(mac_addr);
        if (res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Failed to send disconnection request. [%d]\n", res);
            return -1;
        }
    }
    __FUNC_EXIT__;
    return 0;
}
