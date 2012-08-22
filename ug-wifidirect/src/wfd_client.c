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

#include <stdio.h>
#include <stdbool.h>
#include <libintl.h>

#include <Elementary.h>
#include <pmapi.h>
#include <vconf.h>
//#include <vconf-keys.h>
#include <tethering.h>
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
        // TODO: set genlist head item as "WiFi Direct"
        return;
    }

    if(wifi_state == VCONFKEY_WIFI_OFF)
    {
        DBG(LOG_VERBOSE, "WiFi is turned off\n");
        wfd_client_swtch_force(ugd, TRUE);
    }
    else
    {
        DBG(LOG_VERBOSE, "WiFi is turned on\n");
    }

    res = net_deregister_client();
    if(res != NET_ERR_NONE)
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
    if(res != NET_ERR_NONE)
    {
        DBG(LOG_ERROR, "Failed to register network client. [%d]\n", res);
        return -1;
    }
    DBG(LOG_VERBOSE, "Network client is registered\n");
    res = net_wifi_power_off();
    if(res != NET_ERR_NONE)
    {
        DBG(LOG_ERROR, "Failed to turn off wifi. [%d]\n", res);
        return -1;
    }
    DBG(LOG_VERBOSE, "WiFi power off\n");
    __FUNC_EXIT__;
    return 0;
}

static void _hotspot_state_cb(keynode_t *key, void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data*) data;
	int res;
	int hotspot_mode;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h th = NULL;

	res = vconf_get_int(VCONFKEY_MOBILE_HOTSPOT_MODE, &hotspot_mode);
	if (res != 0)
	{
		DBG(LOG_ERROR, "Failed to get mobile hotspot state from vconf. [%d]\n", res);
		// TODO: set genlist head item as "WiFi Direct"
		return;
	}

	if(hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI)
	{
		DBG(LOG_VERBOSE, " Mobile hotspot is activated\n");
	}
	else
	{
		DBG(LOG_VERBOSE, " Mobile hotspot is deactivated\n");
		wfd_client_swtch_force(ugd, TRUE);
	}

	th = ugd->hotspot_handle;

	if(th != NULL)
	{
		/* Deregister cbs */
		ret = tethering_unset_disabled_cb(th, TETHERING_TYPE_WIFI);
		if(ret != TETHERING_ERROR_NONE)
			DBG(LOG_ERROR, "tethering_unset_disabled_cb is failed(%d)\n", ret);

		/* Destroy tethering handle */
		ret = tethering_destroy(th);
		if(ret != TETHERING_ERROR_NONE)
			DBG(LOG_ERROR, "tethering_destroy is failed(%d)\n", ret);

		ugd->hotspot_handle = NULL;
	}

	__FUNC_EXIT__;
}

static void __disabled_cb(tethering_error_e error, tethering_type_e type, tethering_disabled_cause_e code, void *data)
{
	__FUNC_ENTER__;

	if (error != TETHERING_ERROR_NONE) {
	
		if (code != TETHERING_DISABLED_BY_REQUEST) {
			return;
		}
		DBG(LOG_ERROR, "error !!! TETHERING is not disabled.\n");
		return;
	}

	DBG(LOG_VERBOSE, "TETHERING is disabled.\n");

	__FUNC_EXIT__;

	return;
}


int wfd_mobile_ap_off(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data*) data;
	int res;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h th = NULL;

	res = vconf_notify_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE, _hotspot_state_cb, ugd);
	if (res == -1)
	{
		DBG(LOG_ERROR, "Failed to register vconf callback\n");
		return -1;
	}

	/* Create tethering handle */
	ret = tethering_create(&th);
	if(ret != TETHERING_ERROR_NONE)
	{
		DBG(LOG_ERROR, "Failed to tethering_create() [%d]\n", ret);
		return -1;
	}
	else
	{
		DBG(LOG_VERBOSE, "Succeeded to tethering_create()\n");
	}

	/* Register cbs */
	ret = tethering_set_disabled_cb(th, TETHERING_TYPE_WIFI,
			__disabled_cb, NULL);
	if(ret != TETHERING_ERROR_NONE)
	{
		DBG(LOG_ERROR, "tethering_set_disabled_cb is failed\n", ret);
		return -1;
	}

	/* Disable tethering */
	ret = tethering_disable(th, TETHERING_TYPE_WIFI);
	if(ret != TETHERING_ERROR_NONE)
	{
		DBG(LOG_ERROR, "Failed to turn off mobile hotspot. [%d]\n", ret);
		return -1;
	}
	else
	{
		DBG(LOG_VERBOSE, "Succeeded to turn off mobile hotspot\n");
	}

	ugd->hotspot_handle = th;


	__FUNC_EXIT__;
	return 0;
}

#if 0
static device_type_s *wfd_client_find_peer_by_ssid(void *data, const char *ssid)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    int i;

    if(ugd == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    for(i=0; i<ugd->gl_available_peer_cnt; i++)
    {
        DBG(LOG_VERBOSE, "check %dth peer\n", i);
        if(!strcmp(ugd->gl_available_peers[i].ssid, ssid))
        {
            DBG(LOG_VERBOSE, "found peer. [%d]\n", i);
            __FUNC_EXIT__;
            return &ugd->gl_available_peers[i];
        }
    }
    __FUNC_EXIT__;

    return NULL;
}
#endif

static device_type_s *wfd_client_find_peer_by_mac(void *data, const char *mac_addr)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    int i;

    if(ugd == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (ugd->multi_connect_mode != WFD_MULTI_CONNECT_MODE_NONE)
    {
		for(i=0; i<ugd->raw_multi_selected_peer_cnt; i++)
		{
			DBG(LOG_VERBOSE, "[Multi Connect] check %dth peer\n", i);
			if(!strncmp(mac_addr, (const char*) ugd->raw_multi_selected_peers[i].mac_addr, MAC_LENGTH))
			{
				DBG(LOG_VERBOSE, "selected found peer. [%d]\n", i);
				__FUNC_EXIT__;
				return &ugd->raw_multi_selected_peers[i];
			}
		}
    }
    else
    {
		for(i=0; i<ugd->raw_discovered_peer_cnt; i++)
		{
			DBG(LOG_VERBOSE, "check %dth peer\n", i);
			if(!strncmp(mac_addr, (const char*) ugd->raw_discovered_peers[i].mac_addr, MAC_LENGTH))
			{
				DBG(LOG_VERBOSE, "found peer. [%d]\n", i);
				__FUNC_EXIT__;
				return &ugd->raw_discovered_peers[i];
			}
		}

    }
    __FUNC_EXIT__;

    return NULL;
}

int _wfd_ug_view_clean_on_off(struct ug_data *ugd)
{
	wfd_ug_view_update_peers(ugd);
	return 0;
}

void _activation_cb(int error_code, wifi_direct_device_state_e device_state, void *user_data)
{
    __FUNC_ENTER__;
    int res;
    struct ug_data *ugd = (struct ug_data*) user_data;

    wfd_refresh_wifi_direct_state(ugd);

    switch(device_state)
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

            wfg_ug_act_popup_remove(ugd);

            res = vconf_ignore_key_changed(VCONFKEY_WIFI_STATE, _wifi_state_cb);
            if(res == -1)
            {
                DBG(LOG_ERROR, "Failed to ignore vconf key callback for wifi state\n");
            }

            res = vconf_ignore_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE, _hotspot_state_cb);
            if(res == -1)
            {
                DBG(LOG_ERROR, "Failed to ignore vconf key callback for hotspot state\n");
            }

            res = wifi_direct_start_discovery(FALSE, 0);
            if(res != WIFI_DIRECT_ERROR_NONE)
            {
                DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", res);
            }
            DBG(LOG_VERBOSE, "Discovery is started\n");
            break;

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

            wfd_ug_view_update_peers(ugd);

            break;
        default:
            break;
    }

    wfd_ug_view_refresh_glitem(ugd->head);

    if (ugd->scan_btn) {
    	wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"), TRUE);
    }

    if (ugd->multi_connect_btn) {
    	wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"), TRUE);
    }

    __FUNC_EXIT__;
    return;
}

bool _wfd_discoverd_peer_cb(wifi_direct_discovered_peer_info_s* peer, void *user_data)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data*) user_data;
    int peer_cnt = ugd->raw_discovered_peer_cnt;

    DBG(LOG_VERBOSE, "%dth discovered peer. [%s] [%s]\n", peer_cnt, peer->ssid, peer->mac_address);

    strncpy(ugd->raw_discovered_peers[peer_cnt].ssid, peer->ssid, sizeof(ugd->raw_discovered_peers[peer_cnt].ssid));
    ugd->raw_discovered_peers[peer_cnt].category = peer->primary_device_type;
    strncpy(ugd->raw_discovered_peers[peer_cnt].mac_addr, peer->mac_address, MAC_LENGTH);
    strncpy(ugd->raw_discovered_peers[peer_cnt].if_addr, peer->interface_address, MAC_LENGTH);
    ugd->raw_discovered_peers[peer_cnt].is_group_owner = peer->is_group_owner;
    ugd->raw_discovered_peers[peer_cnt].is_persistent_group_owner = peer->is_persistent_group_owner;
    ugd->raw_discovered_peers[peer_cnt].is_connected = peer->is_connected;

    if (TRUE == peer->is_connected) {
	ugd->raw_discovered_peers[peer_cnt].conn_status = PEER_CONN_STATUS_CONNECTED;
    } else {
	ugd->raw_discovered_peers[peer_cnt].conn_status = PEER_CONN_STATUS_DISCONNECTED;
    }

    DBG(LOG_VERBOSE, "\tSSID: [%s]\n", ugd->raw_discovered_peers[peer_cnt].ssid);
    DBG(LOG_VERBOSE, "\tPeer category [%d] -> [%d]\n", peer->primary_device_type, ugd->raw_discovered_peers[peer_cnt].category);
    DBG(LOG_VERBOSE, "\tStatus: [%d]\n", ugd->raw_discovered_peers[peer_cnt].conn_status);

    ugd->raw_discovered_peer_cnt ++;

    free(peer->ssid);
    free(peer->mac_address);
    free(peer->interface_address);
    free(peer);

    __FUNC_EXIT__;
    return TRUE;
}

bool _wfd_connected_peer_cb(wifi_direct_connected_peer_info_s* peer, void *user_data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) user_data;
    int peer_cnt = ugd->raw_connected_peer_cnt;

    DBG(LOG_VERBOSE, "%dth connected peer. [%s] [%s]\n", peer_cnt, peer->ssid, peer->mac_address);

    strncpy(ugd->raw_connected_peers[peer_cnt].ssid, peer->ssid, sizeof(ugd->raw_connected_peers[peer_cnt].ssid));
    ugd->raw_connected_peers[peer_cnt].category = peer->primary_device_type;
    strncpy(ugd->raw_connected_peers[peer_cnt].mac_addr, peer->mac_address, MAC_LENGTH);
    strncpy(ugd->raw_connected_peers[peer_cnt].if_addr, peer->interface_address, MAC_LENGTH);
    ugd->raw_connected_peers[peer_cnt].conn_status = PEER_CONN_STATUS_CONNECTED;

    DBG(LOG_VERBOSE, "\tStatus: [%d]\n", ugd->raw_connected_peers[peer_cnt].conn_status);
    DBG(LOG_VERBOSE, "\tCategory: [%d]\n", ugd->raw_connected_peers[peer_cnt].category);
    DBG(LOG_VERBOSE, "\tSSID: [%s]\n", ugd->raw_connected_peers[peer_cnt].ssid);

    ugd->raw_connected_peer_cnt++;

    free(peer->ssid);
    free(peer->mac_address);
    free(peer->interface_address);
    free(peer);
    __FUNC_EXIT__;
    return TRUE;
}

int wfd_ug_get_discovered_peers(struct ug_data *ugd)
{
	int res = 0;

	if (ugd==NULL)
		return -1;

	ugd->raw_discovered_peer_cnt = 0;
    res = wifi_direct_foreach_discovered_peers(_wfd_discoverd_peer_cb, (void*) ugd);
    if (res != WIFI_DIRECT_ERROR_NONE)
    {
    	ugd->raw_discovered_peer_cnt = 0;
        DBG(LOG_ERROR, "Get discovery result failed: %d\n", res);
    }

	return 0;
}


int wfd_ug_get_connected_peers(struct ug_data *ugd)
{
	int res = 0;

	if (ugd==NULL)
		return -1;

	ugd->raw_connected_peer_cnt = 0;
	res = wifi_direct_foreach_connected_peers(_wfd_connected_peer_cb, (void*) ugd);
	if(res != WIFI_DIRECT_ERROR_NONE)
	{
		ugd->raw_connected_peer_cnt = 0;
		DBG(LOG_ERROR, "Get connected peer failed: %d\n", res);
	}
	return 0;
}

void _discover_cb(int error_code, wifi_direct_discovery_state_e discovery_state, void *user_data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) user_data;

    if(ugd == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return;
    }

    if(discovery_state == WIFI_DIRECT_ONLY_LISTEN_STARTED)
    {
        ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
    }
    else if(discovery_state == WIFI_DIRECT_DISCOVERY_STARTED)
    {
        ugd->head_text_mode = HEAD_TEXT_TYPE_SCANING;
    }
    else if(discovery_state == WIFI_DIRECT_DISCOVERY_FOUND)
    {
        ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
    }

    wfd_ug_view_refresh_glitem(ugd->head);

    if (WIFI_DIRECT_DISCOVERY_STARTED == discovery_state) {
	if (ugd->scan_btn) {
		wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_STOPSCAN"), TRUE);
	}

	if (ugd->multi_connect_btn) {
		wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_STOPSCAN"), TRUE);
	}
    } else {
	if (ugd->scan_btn) {
		wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"), TRUE);
	}

	if (ugd->multi_connect_btn) {
		wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"), TRUE);
	}
    }

    wfd_ug_get_discovered_peers(ugd);
    wfd_ug_get_connected_peers(ugd);

    wfd_ug_view_update_peers(ugd);

    _wfd_free_multiconnect_device(ugd);
    _wfd_update_multiconnect_device(ugd);
    //_change_multi_button_title(ugd);

    __FUNC_EXIT__;
    return;
}

void _connection_cb(int error_code, wifi_direct_connection_state_e connection_state, const char *mac_address, void *user_data)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data*) user_data;
    device_type_s *peer = NULL;
    bool owner = FALSE;
    int res = 0;

    DBG(LOG_VERBOSE, "Connection event [%d], error_code [%d]\n", connection_state, error_code);

    if(mac_address == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(peer mac is NULL)\n");
        return;
    }
    DBG(LOG_VERBOSE, "Connection event from %s", mac_address);

    if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS)
    {
    	peer = wfd_client_find_peer_by_mac(ugd, mac_address);
    	if (peer != NULL)
    	{
			switch(connection_state)
			{
			case WIFI_DIRECT_CONNECTION_RSP:
				if(error_code == WIFI_DIRECT_ERROR_NONE)
				{
					ugd->wfd_status = WFD_LINK_STATUS_CONNECTED;
					peer->conn_status = PEER_CONN_STATUS_CONNECTED;
				}
				else
				{
					peer->conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
				}
				ugd->g_source_multi_connect_next = g_timeout_add(1000, wfd_multi_connect_next_cb, ugd);
				break;
			default:
				break;
			}
			wfd_ug_get_connected_peers(ugd);
			wfd_ug_view_update_peers(ugd);
    	}
    	else
    	{
    	    DBG(LOG_VERBOSE, "peer is not found [%s]", mac_address);
    	}
    	goto refresh_button;
    }


    peer = wfd_client_find_peer_by_mac(ugd, mac_address);

    if (NULL == peer || NULL == peer->ssid) {
	    DBG(LOG_ERROR, "SSID from connection is NULL !!\n");
	    goto refresh_button;
    }

    switch(connection_state)
    {
        case WIFI_DIRECT_CONNECTION_RSP:
            DBG(LOG_VERBOSE, "WIFI_DIRECT_CONNECTION_RSP\n");

            if(error_code == WIFI_DIRECT_ERROR_NONE)
            {
                ugd->wfd_status = WFD_LINK_STATUS_CONNECTED;
                peer->conn_status = PEER_CONN_STATUS_CONNECTED;
            }
            else
            {
		peer->conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
            }

            wfd_ug_get_connected_peers(ugd);
            wfd_ug_view_update_peers(ugd);
            break;
        case WIFI_DIRECT_DISCONNECTION_RSP:
        case WIFI_DIRECT_DISCONNECTION_IND:
        case WIFI_DIRECT_DISASSOCIATION_IND:
            DBG(LOG_VERBOSE, "WIFI_DIRECT_DISCONNECTION_X\n");
            if(error_code != WIFI_DIRECT_ERROR_NONE)
            {
                // TODO: show disconnection error popup
                return;
            }

            if (peer!=NULL)
            	peer->conn_status = PEER_CONN_STATUS_DISCONNECTED;
            else
            {
            	// In case of disconnect_all(), no specific peer is found.
            }

            wifi_direct_start_discovery(FALSE, 0);
            ugd->wfd_status = WFD_LINK_STATUS_DISCOVERING;
            ugd->head_text_mode = HEAD_TEXT_TYPE_SCANING;

            wfd_ug_view_refresh_glitem(ugd->head);

            wfd_ug_get_discovered_peers(ugd);
            wfd_ug_get_connected_peers(ugd);
            wfd_ug_view_update_peers(ugd);
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

    if(peer != NULL)
        wfd_ug_view_refresh_glitem(peer->gl_item);

    _change_multi_button_title(ugd);

refresh_button:
    /* refresh the scan button */
    wfd_refresh_wifi_direct_state(ugd);
    if (WIFI_DIRECT_STATE_CONNECTING == ugd->wfd_status ||
	    WIFI_DIRECT_STATE_DISCONNECTING == ugd->wfd_status) {

	res = wifi_direct_is_group_owner(&owner);
	if (res == WIFI_DIRECT_ERROR_NONE) {
	    if (!owner) {
		if (ugd->scan_btn) {
			wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
		}

		if (ugd->multi_connect_btn) {
			wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
		}
	    }
	} else {
	    DBG(LOG_ERROR, "Failed to get whether client is group owner. [%d]\n", res);
	}
    } else {
	if (ugd->scan_btn) {
		wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"), TRUE);
	}

	if (ugd->multi_connect_btn) {
		wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"), TRUE);
	}
    }

    __FUNC_EXIT__;
    return;
}

int wfd_get_vconf_status(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    char *dev_name;

    // TODO: get wifi direct status from vconf
    // db/mobile_hotspot/wifi_key (string)

    dev_name = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);  // "db/setting/device_name" (VCONF_WFD_APNAME)
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

int wfd_refresh_wifi_direct_state(void *data)
{
    struct ug_data *ugd = (struct ug_data*) data;
	int res;
	wifi_direct_state_e wfd_status;
    res = wifi_direct_get_state(&wfd_status);
    if(res != WIFI_DIRECT_ERROR_NONE)
    {
        DBG(LOG_ERROR, "Failed to get link status. [%d]\n", res);
        return -1;
    }
    DBG(LOG_VERBOSE, "WFD status [%d]", wfd_status);
    ugd->wfd_status = wfd_status;
    return 0;
}

int init_wfd_client(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    int res = 0;

    res = wifi_direct_initialize();
    if(res != WIFI_DIRECT_ERROR_NONE)
    {
        DBG(LOG_ERROR, "Failed to initialize wifi direct. [%d]\n", res);
        return -1;
    }

    res = wifi_direct_set_device_state_changed_cb(_activation_cb, (void*) ugd);
    res = wifi_direct_set_discovery_state_changed_cb(_discover_cb, (void*) ugd);
    res = wifi_direct_set_connection_state_changed_cb(_connection_cb, (void*) ugd);

    /* update WFD status */
    wfd_refresh_wifi_direct_state(ugd);
    if (ugd->wfd_status > WIFI_DIRECT_STATE_ACTIVATING)
    	ugd->wfd_onoff = 1;
    else
    	ugd->wfd_onoff = 0;

    DBG(LOG_VERBOSE, "WFD link status. [%d]\n", ugd->wfd_status);

    __FUNC_EXIT__;

    return 0;
}

int deinit_wfd_client(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data*) data;
	int res = 0;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h th = NULL;

	wfd_refresh_wifi_direct_state(ugd);

	if(ugd->wfd_status == WIFI_DIRECT_STATE_DISCOVERING)
	{
		DBG(LOG_VERBOSE, "Stop discovery before deregister client\n");
		wifi_direct_cancel_discovery();
	}

	res = wifi_direct_deinitialize();
	if(res != WIFI_DIRECT_ERROR_NONE)
	{
		DBG(LOG_ERROR, "Failed to deregister client. [%d]\n", res);
	}

	res = vconf_ignore_key_changed(VCONFKEY_WIFI_STATE, _wifi_state_cb);
	if(res == -1)
	{
		DBG(LOG_ERROR, "Failed to ignore vconf key callback for wifi state\n");
	}

	res = net_deregister_client();
	if(res != NET_ERR_NONE)
	{
		DBG(LOG_ERROR, "Failed to deregister network client. [%d]\n", res);
	}

	res = vconf_ignore_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE, _hotspot_state_cb);
	if(res == -1)
	{
		DBG(LOG_ERROR, "Failed to ignore vconf key callback for hotspot state\n");
	}

	th = ugd->hotspot_handle;
	
	if(th != NULL)
	{
		/* Deregister cbs */
		ret = tethering_unset_disabled_cb(th, TETHERING_TYPE_WIFI);
		if(ret != TETHERING_ERROR_NONE)
			DBG(LOG_ERROR, "tethering_unset_disabled_cb is failed(%d)\n", ret);

		/* Destroy tethering handle */
		ret = tethering_destroy(th);
		if(ret != TETHERING_ERROR_NONE)
			DBG(LOG_ERROR, "tethering_destroy is failed(%d)\n", ret);

		ugd->hotspot_handle = NULL;
			
	}

	__FUNC_EXIT__;

	return 0;
}

int wfd_client_switch_on(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    int res;

    DBG(LOG_VERBOSE, "WFD status [%d]\n", ugd->wfd_status);

    if(ugd->wfd_status < WFD_LINK_STATUS_ACTIVATING)
    {
        ugd->wfd_status = WFD_LINK_STATUS_ACTIVATING;

        int wifi_state;
        res = vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);
        if (res != 0)
        {
            DBG(LOG_ERROR, "Failed to get wifi state from vconf. [%d]\n", res);
            // TODO: set genlist head item as "WiFi Direct"
            return -1;
        }

        int hotspot_mode;
        res = vconf_get_int(VCONFKEY_MOBILE_HOTSPOT_MODE, &hotspot_mode);
        if (res != 0)
        {
            DBG(LOG_ERROR, "Failed to get mobile hotspot state from vconf. [%d]\n", res);
            // TODO: set genlist head item as "WiFi Direct"
            return -1;
        }

        if(wifi_state > VCONFKEY_WIFI_OFF)
        {
            DBG(LOG_VERBOSE, "WiFi is connected, so have to turn off WiFi");
            wfd_ug_act_popup(ugd, _("IDS_WFD_POP_WIFI_OFF"), POPUP_TYPE_WIFI_OFF); // "This will turn off Wi-Fi client operation.<br>Continue?"
        }
        else if(hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI)
        {
            DBG(LOG_VERBOSE, "WiFi is connected, so have to turn off WiFi");
            wfd_ug_act_popup(ugd, _("IDS_WFD_POP_HOTSPOT_OFF"), POPUP_TYPE_HOTSPOT_OFF); // "This will turn off Portable Wi-Fi hotspots operation.<br>Continue?"
        }
        else    // (wifi_state < VCONFKEY_WIFI_CONNECTED && !(hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI))
        {
            res = wifi_direct_activate();
            if(res != WIFI_DIRECT_ERROR_NONE)
            {
                DBG(LOG_ERROR, "Failed to activate Wi-Fi Direct. error code = [%d]\n", res);
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
    struct ug_data *ugd = (struct ug_data*) data;
    int res;

    DBG(LOG_VERBOSE, "WFD status [%d]\n", ugd->wfd_status);

    if(ugd->wfd_status < WFD_LINK_STATUS_ACTIVATING)
    {
        DBG(LOG_VERBOSE, "Wi-Fi Direct is already deactivated\n");
    }
    else
    {
        ugd->wfd_status = WFD_LINK_STATUS_DEACTIVATING;

        res = wifi_direct_deactivate();
        if(res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
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

    if(onoff)
    {
        res = wifi_direct_activate();
        if(res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Failed to activate Wi-Fi Direct. error code = [%d]\n", res);
            wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_ACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

            ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
            wfd_ug_view_refresh_glitem(ugd->head);
            return -1;
        }
    }
    else
    {
        res = wifi_direct_deactivate();
        if(res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
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
    struct ug_data *ugd = (struct ug_data*) data;
    int res = -1;

#if 0
    ret = wifi_direct_cancel_discovery();
//    ret = wifi_direct_cancel_discovery();
    if (ret != WIFI_DIRECT_ERROR_NONE) {
    	DBG(LOG_ERROR, "Failed wfd discover() : %d", ret);
    	return FALSE;
    }


    if(ugd->wfd_status >= WIFI_DIRECT_STATE_ACTIVATED)
    {
        res = wifi_direct_start_discovery(FALSE, 30);
        if (res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Failed to start wfd discovery. [%d]", res);
            return -1;
        }
    }
#endif

    ugd->wfd_status = WIFI_DIRECT_DISCOVERY_STARTED;
    wfd_refresh_wifi_direct_state(ugd);
    ugd->head_text_mode = HEAD_TEXT_TYPE_SCANING;
    wfd_ug_view_refresh_glitem(ugd->head);
    wifi_direct_cancel_discovery();

    res = wifi_direct_start_discovery(FALSE, 0);
    if (res != WIFI_DIRECT_ERROR_NONE) {
    	DBG(LOG_ERROR, "Fail to restart scanning. %d\n", res);
	return -1;
    }

    ugd->wfd_status = WIFI_DIRECT_DISCOVERY_FOUND;
    wfd_refresh_wifi_direct_state(ugd);

    __FUNC_EXIT__;
    return 0;
}

int wfd_client_connect(const char *mac_addr)
{
    __FUNC_ENTER__;
    int res;

    DBG(LOG_ERROR, "connect to peer=[%s]\n", mac_addr);
    res = wifi_direct_connect(mac_addr);
    if(res != WIFI_DIRECT_ERROR_NONE)
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

    if(mac_addr == NULL)
    {
        res = wifi_direct_disconnect_all();
        if(res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Failed to send disconnection request to all. [%d]\n", res);
            return -1;
        }
    }
    else
    {
        res = wifi_direct_disconnect(mac_addr);
        if(res != WIFI_DIRECT_ERROR_NONE)
        {
            DBG(LOG_ERROR, "Failed to send disconnection request. [%d]\n", res);
            return -1;
        }
    }
    __FUNC_EXIT__;
    return 0;
}

int wfd_client_set_p2p_group_owner_intent(int go_intent)
{
    __FUNC_ENTER__;
    int res;

	res = wifi_direct_set_group_owner_intent(go_intent);
	if(res != WIFI_DIRECT_ERROR_NONE)
	{
		DBG(LOG_ERROR, "Failed to wifi_direct_set_go_intent(%d). [%d]\n", go_intent, res);
		return -1;
	}
    __FUNC_EXIT__;
    return 0;
}

int wfd_client_get_peers(struct ug_data *ugd)
{

	if(ugd->wfd_status < WFD_LINK_STATUS_ACTIVATED)
	{
		ugd->raw_discovered_peer_cnt = 0;
		ugd->raw_connected_peer_cnt = 0;
		return 0;
	}

    if(ugd->wfd_status > WIFI_DIRECT_STATE_ACTIVATING)
    {
        wfd_ug_get_discovered_peers(ugd);
    }

    if(ugd->wfd_status >= WIFI_DIRECT_STATE_CONNECTED)
    {
        wfd_ug_get_connected_peers(ugd);
    }

    wfd_ug_view_update_peers(ugd);
    _change_multi_button_title(ugd);
    return 0;
}
