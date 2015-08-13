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
#include <vconf.h>
#include <vconf-keys.h>

#include <tethering.h>

#include <network-cm-intf.h>
#include <network-wifi-intf.h>

#include <wifi-direct.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

bool _wfd_discoverd_peer_cb(wifi_direct_discovered_peer_info_s *peer, void *user_data);

#ifndef MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE
/**
 *	This function let the ug make a change callback for wifi state
 *	@return   void
 *	@param[in] key the pointer to the key
 *	@param[in] data the pointer to the main data structure
 */
static void _wifi_state_cb(keynode_t *key, void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;
	int wifi_state;

	res = vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);
	if (res != 0) {
		DBG(LOG_ERROR, "Failed to get wifi state from vconf. [%d]\n", res);
		return;
	}

	if (wifi_state == VCONFKEY_WIFI_OFF) {
		DBG(LOG_INFO, "WiFi is turned off\n");
		wfd_client_swtch_force(ugd, TRUE);
	} else {
		DBG(LOG_INFO, "WiFi is turned on\n");
	}

	res = net_deregister_client();
	if (res != NET_ERR_NONE) {
		DBG(LOG_ERROR, "Failed to deregister network client. [%d]\n", res);
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug make a event callback for network registering
 *	@return   void
 *	@param[in] event_info the pointer to the information of network event
 *	@param[in] user_data the pointer to the user data
 */
static void _network_event_cb(net_event_info_t *event_info, void *user_data)
{
	__FUNC_ENTER__;
	DBG(LOG_INFO, "Event from network. [%d]\n", event_info->Event);
	__FUNC_EXIT__;
}

/**
 *	This function let the ug turn wifi off
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_wifi_off(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;

	res = vconf_notify_key_changed(VCONFKEY_WIFI_STATE, _wifi_state_cb, ugd);
	if (res == -1) {
		DBG(LOG_ERROR, "Failed to register vconf callback\n");
		return -1;
	}

	DBG(LOG_INFO, "Vconf key callback is registered\n");

	res = net_register_client((net_event_cb_t) _network_event_cb, NULL);
	if (res != NET_ERR_NONE) {
		DBG(LOG_ERROR, "Failed to register network client. [%d]\n", res);
		return -1;
	}

	DBG(LOG_INFO, "Network client is registered\n");

	res = net_wifi_power_off();
	if (res != NET_ERR_NONE) {
		DBG(LOG_ERROR, "Failed to turn off wifi. [%d]\n", res);
		return -1;
	}

	DBG(LOG_INFO, "WiFi power off\n");

	__FUNC_EXIT__;
	return 0;
}
#endif /* MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE */


/**
 *	This function let the ug make a callback for setting tethering mode enabled
 *	@return   void
 *	@param[in] error the returned error code
 *	@param[in] type the type of tethering
 *	@param[in] is_requested whether tethering mode is enabled
 *	@param[in] data the pointer to the user data
 */
static void __enabled_cb(tethering_error_e error, tethering_type_e type, bool is_requested, void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h th = NULL;
	bool is_wifi_enabled = false;

	if (error != TETHERING_ERROR_NONE) {
		if (is_requested != TRUE) {
			return;
		}

		DBG(LOG_ERROR, "error !!! TETHERING is not enabled.\n");
		return;
	}

	th = ugd->hotspot_handle;
	if (th != NULL) {
		is_wifi_enabled = tethering_is_enabled(th, TETHERING_TYPE_WIFI);
		if (is_wifi_enabled) {
			DBG(LOG_INFO, "Mobile hotspot is activated\n");
		}

		/* Deregister cbs */
		ret = tethering_unset_enabled_cb(th, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_unset_enabled_cb is failed(%d)\n", ret);
		}

		/* Destroy tethering handle */
		ret = tethering_destroy(th);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_destroy is failed(%d)\n", ret);
		}

		ugd->hotspot_handle = NULL;
	}

	DBG(LOG_INFO, "TETHERING is enabled.\n");

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug make a callback for setting tethering mode disabled
 *	@return   void
 *	@param[in] error the returned error code
 *	@param[in] type the type of tethering
 *	@param[in] code whether tethering mode is enabled
 *	@param[in] data the pointer to the user data
 */
static void __disabled_cb(tethering_error_e error, tethering_type_e type, tethering_disabled_cause_e code, void *data)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *)data;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h th = NULL;
	bool is_wifi_enabled = false;
	bool is_wifi_ap_enabled = false;

	if (error != TETHERING_ERROR_NONE) {
		if (code != TETHERING_DISABLED_BY_REQUEST) {
			return;
		}

		DBG(LOG_ERROR, "error !!! TETHERING is not disabled.\n");
		return;
	}

	th = ugd->hotspot_handle;
	if (th != NULL) {
		is_wifi_enabled = tethering_is_enabled(th, TETHERING_TYPE_WIFI);
		is_wifi_ap_enabled = tethering_is_enabled(th, TETHERING_TYPE_RESERVED);
		if (is_wifi_enabled || is_wifi_ap_enabled) {
			DBG(LOG_ERROR, "error !!! TETHERING is not disabled.\n");
			DBG(LOG_ERROR, "is_wifi_enabled:%d is_wifi_ap_enabled:%d\n", is_wifi_enabled, is_wifi_ap_enabled);
			return;
		}

		DBG(LOG_INFO, "Mobile hotspot is deactivated\n");
		wfd_client_swtch_force(ugd, TRUE);

		ret = tethering_unset_disabled_cb(th, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_unset_disabled_cb is failed(%d)\n", ret);
		}

		ret = tethering_unset_disabled_cb(th, TETHERING_TYPE_RESERVED);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_unset_disabled_cb is failed(%d)\n", ret);
		}

		/* Destroy tethering handle */
		ret = tethering_destroy(th);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_destroy is failed(%d)\n", ret);
		}

		ugd->hotspot_handle = NULL;
	}

	DBG(LOG_INFO, "TETHERING is disabled.\n");

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug turn AP on
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_mobile_ap_on(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	WFD_RETV_IF(ugd == NULL, -1, "Incorrect parameter(NULL)\n");

	if (NULL == ugd->hotspot_handle) {
		ret = tethering_create(&(ugd->hotspot_handle));
		if (TETHERING_ERROR_NONE != ret) {
			DBG(LOG_ERROR, "Failed to tethering_create() [%d]\n", ret);
			ugd->hotspot_handle = NULL;
			return -1;
		}
		DBG(LOG_INFO, "Succeeded to tethering_create()\n");
	}
	/* Register cbs */
	ret = tethering_set_enabled_cb(ugd->hotspot_handle, TETHERING_TYPE_WIFI, __enabled_cb, ugd);
	if (ret != TETHERING_ERROR_NONE) {
		DBG(LOG_ERROR, "tethering_set_enabled_cb is failed\n", ret);
		return -1;
	}

	/* Enable tethering */
	ret = tethering_enable(ugd->hotspot_handle, TETHERING_TYPE_WIFI);
	if (ret != TETHERING_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to turn on mobile hotspot. [%d]\n", ret);
		return -1;
	} else {
		DBG(LOG_INFO, "Succeeded to turn on mobile hotspot\n");
	}

	ugd->is_hotspot_off = FALSE;

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug turn AP off
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_mobile_ap_off(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	WFD_RETV_IF(ugd == NULL || ugd->hotspot_handle == NULL, -1, "Incorrect parameter(NULL)\n");
	tethering_error_e ret = TETHERING_ERROR_NONE;
	bool is_wifi_enabled = false;
	bool is_wifi_ap_enabled = false;

	is_wifi_enabled = tethering_is_enabled(ugd->hotspot_handle, TETHERING_TYPE_WIFI);
	is_wifi_ap_enabled = tethering_is_enabled(ugd->hotspot_handle, TETHERING_TYPE_RESERVED);

	if (is_wifi_enabled) {
		/* Register cbs */
		ret = tethering_set_disabled_cb(ugd->hotspot_handle, TETHERING_TYPE_WIFI, __disabled_cb, ugd);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_set_disabled_cb is failed\n", ret);
			return -1;
		}
		/* Disable tethering */
		ret = tethering_disable(ugd->hotspot_handle, TETHERING_TYPE_WIFI);
	} else if (is_wifi_ap_enabled) {
		ret = tethering_set_disabled_cb(ugd->hotspot_handle, TETHERING_TYPE_RESERVED, __disabled_cb, ugd);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_set_disabled_cb is failed\n", ret);
			return -1;
		}
		ret = tethering_disable(ugd->hotspot_handle, TETHERING_TYPE_RESERVED);
	}

	if (ret != TETHERING_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to turn off mobile hotspot. [%d]\n", ret);
		return -1;
	} else {
		DBG(LOG_INFO, "Succeeded to turn off mobile hotspot\n");
	}

	ugd->is_hotspot_off = TRUE;

	__FUNC_EXIT__;
	return 0;
}

void wfd_client_free_raw_discovered_peers(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	WFD_RET_IF(ugd->raw_discovered_peer_list == NULL, "Incorrect parameter(NULL)\n");

	g_list_free(ugd->raw_discovered_peer_list);
	ugd->raw_discovered_peer_list = NULL;

	__FUNC_EXIT__;
}

/**
 *	This function let the ug find the peer by mac address
 *	@return   the found peer
 *	@param[in] data the pointer to the main data structure
 *	@param[in] mac_addr the pointer to mac address
 */
static device_type_s *wfd_client_find_peer_by_mac(void *data, const char *mac_addr)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	wifi_direct_discovered_peer_info_s *peer_info = NULL;
	GList *iterator = NULL;
	int i;
	WFD_RETV_IF(ugd == NULL, NULL, "Incorrect parameter(NULL)\n");

	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS) {
		for (i = 0; i < ugd->raw_multi_selected_peer_cnt; i++) {
			if (!strncmp(mac_addr, (const char *)ugd->raw_multi_selected_peers[i].mac_addr, MAC_LENGTH)) {
				return &ugd->raw_multi_selected_peers[i];
			}
		}
	} else {
		for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
			if (!strncmp(mac_addr, ((device_type_s *)iterator->data)->mac_addr, MAC_LENGTH)) {
				return (device_type_s *)iterator->data;
			}
		}
	}

	/*
	 * In case, device is not in raw discovered list, then get peer info.
	 * There could be situation in which device is not yet discovered and
	 * connected process started.
	 */
	if (WIFI_DIRECT_ERROR_NONE != wifi_direct_get_peer_info((char *)mac_addr, &peer_info) ||
		NULL == peer_info) {
		DBG(LOG_ERROR, "Peer Not Found !!!");
		return NULL;
	}

	/* Update peer list */
	DBG(LOG_INFO, "Update Peer info");
	_wfd_discoverd_peer_cb(peer_info, (void *)ugd);

	/* Get the device from peer list */
	for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
		if (!strncmp(mac_addr, ((device_type_s *)iterator->data)->mac_addr, MAC_LENGTH)) {
			return (device_type_s *)iterator->data;
		}
	}

	__FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the ug make a callback for registering activation event
 *	@return   void
 *	@param[in] error_code the returned error code
 *	@param[in] device_state the state of device
 *	@param[in] user_data the pointer to the main data structure
 */
void _activation_cb(int error_code, wifi_direct_device_state_e device_state, void *user_data)
{
	__FUNC_ENTER__;
	int res = -1;
	struct ug_data *ugd = (struct ug_data *)user_data;
	wfd_refresh_wifi_direct_state(ugd);

	switch (device_state) {
	case WIFI_DIRECT_DEVICE_STATE_ACTIVATED:
		DBG(LOG_INFO, "WIFI_DIRECT_DEVICE_STATE_ACTIVATED\n");
		if(ugd->scan_toolbar == NULL) {
			scan_button_create(ugd);
		}
		if (error_code != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Error in Activation/Deactivation [%d]\n", error_code);
			if (WIFI_DIRECT_ERROR_AUTH_FAILED == error_code) {
				wfd_ug_warn_popup(ugd, _("IDS_COM_POP_SECURITY_POLICY_RESTRICTS_USE_OF_WI_FI"), POPUP_TYPE_ACTIVATE_FAIL_POLICY_RESTRICTS);
			} else {
				wfd_ug_warn_popup(ugd, _("IDS_COM_POP_FAILED"), POPUP_TYPE_ACTIVATE_FAIL);
			}

#ifdef WFD_ON_OFF_GENLIST
			ugd->wfd_onoff = 0;
			wfd_ug_refresh_on_off_check(ugd);
#endif
			return;
		}

		ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_NONE;
#ifdef WFD_ON_OFF_GENLIST
		ugd->wfd_onoff = 1;
		wfd_ug_refresh_on_off_check(ugd);
#endif
		wfg_ug_act_popup_remove(ugd);

		if (ugd->wfd_discovery_status == WIFI_DIRECT_DISCOVERY_BACKGROUND) {
			DBG(LOG_INFO, "Background mode\n");
			return;
		}

#ifndef MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE
		res = vconf_ignore_key_changed(VCONFKEY_WIFI_STATE, _wifi_state_cb);
		if (res == -1) {
			DBG(LOG_ERROR, "Failed to ignore vconf key callback for wifi state\n");
		}
#endif /* MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE */

		ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
		res = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
		if (res != WIFI_DIRECT_ERROR_NONE) {
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
			DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", res);
			wifi_direct_cancel_discovery();
		}

		break;
	case WIFI_DIRECT_DEVICE_STATE_DEACTIVATED:
		DBG(LOG_INFO, "WIFI_DIRECT_DEVICE_STATE_DEACTIVATED\n");
		if (error_code != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Error in Activation/Deactivation [%d]\n", error_code);
			wfd_ug_warn_popup(ugd, _("IDS_WIFI_POP_DEACTIVATION_FAILED"), POPUP_TYPE_DEACTIVATE_FAIL);
#ifdef WFD_ON_OFF_GENLIST
			ugd->wfd_onoff = 1;
			wfd_ug_refresh_on_off_check(ugd);
#endif
			return;
		}

		WFD_IF_DEL_ITEM(ugd->multi_connect_toolbar_item);

		if (ugd->ctxpopup) {
			ctxpopup_dismissed_cb(ugd, NULL, NULL);
		}

		/*
		 * When multi-connect is on ongoing and deactivte happened destroy
		 * disconnect button.
		 */
		if (ugd->disconnect_btn) {
			Evas_Object *content;
			content = elm_object_part_content_unset(ugd->layout, "button.next");
			WFD_IF_DEL_OBJ(content);
			ugd->disconnect_btn = NULL;
			elm_object_part_content_set(ugd->layout, "button.big",
			ugd->scan_toolbar);
		}

		/* When connect is on ongoing and deactivte happened refresh scan */
		if (ugd->scan_toolbar) {
			wfd_ug_view_refresh_button(ugd->scan_toolbar,
				"IDS_WIFI_SK4_SCAN", FALSE);
		}
		/* Delete pop-up when deactivate happens */
		WFD_IF_DEL_OBJ(ugd->act_popup);
		/* Remove timeout handlers */
		if (ugd->timer_stop_progress_bar > 0)
			g_source_remove(ugd->timer_stop_progress_bar);

		if (ugd->timer_delete_not_alive_peer > 0)
			g_source_remove(ugd->timer_delete_not_alive_peer);

		if (ugd->g_source_multi_connect_next > 0)
			g_source_remove(ugd->g_source_multi_connect_next);

		if (ugd->timer_multi_reset > 0)
			g_source_remove(ugd->timer_multi_reset);

		/* Delete warn popups for Airplane mode */
		if (NULL != ugd->warn_popup) {
			evas_object_del( ugd->warn_popup);
			ugd->warn_popup = NULL;
		}

#ifdef WFD_ON_OFF_GENLIST
		ugd->wfd_onoff = 0;
		wfd_ug_refresh_on_off_check(ugd);
#endif
		/*
		* when deactivated, clear all the
		*  discovered peers and connected peers
		*/
		wfd_client_free_raw_discovered_peers(ugd);
		if (ugd->raw_connected_peer_cnt > 0) {
			memset(ugd->raw_connected_peers, 0x00, ugd->raw_connected_peer_cnt*sizeof(device_type_s));
		}

		ugd->raw_discovered_peer_cnt = 0;
		ugd->raw_connected_peer_cnt = 0;

		wfd_free_nodivice_item(ugd);
		wfd_ug_view_init_genlist(ugd, true);

		if (ugd->multi_navi_item != NULL) {
			elm_naviframe_item_pop(ugd->naviframe);
		}

		if (TRUE == ugd->is_hotspot_off && TRUE == ugd->is_hotspot_locally_disabled) {
			if (0 == wfd_mobile_ap_on(ugd)) {
				ugd->is_hotspot_locally_disabled = FALSE;
			}
		}

		if(ugd->scan_toolbar) {
			evas_object_del(ugd->scan_toolbar);
			ugd->scan_toolbar = NULL;
		}
		break;
	default:
		break;
	}

	/*if (ugd->scan_toolbar) {
		wfd_ug_view_refresh_button(ugd->scan_toolbar, _("IDS_WIFI_SK4_SCAN"), TRUE);
	}*/

	if (ugd->multiconn_scan_stop_btn) {
		wfd_ug_view_refresh_button(ugd->multiconn_scan_stop_btn, "IDS_WIFI_SK4_SCAN", TRUE);
	}

	if (ugd->back_btn) {
		elm_object_disabled_set(ugd->back_btn, FALSE);
	}

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug make a callback for discovering peer
 *	@return   TRUE
 *	@param[in] peer the pointer to the discovered peer
 *	@param[in] user_data the pointer to the main data structure
 */
bool _wfd_discoverd_peer_cb(wifi_direct_discovered_peer_info_s *peer, void *user_data)
{
	__FUNC_ENTER__;
	WFD_RETV_IF(NULL == peer || NULL == user_data, FALSE, "Incorrect parameter(NULL)\n");

	struct ug_data *ugd = (struct ug_data *)user_data;
	int peer_cnt = ugd->raw_discovered_peer_cnt;
	device_type_s *peer_tmp = g_new(device_type_s, 1);
	int i;

	DBG_SECURE(LOG_INFO, "%dth discovered peer. [%s] ["MACSECSTR"]\n", peer_cnt,
		peer->device_name, MAC2SECSTR(peer->mac_address));

	if (ugd->device_filter < 0 || peer->primary_device_type == ugd->device_filter) {
		strncpy(peer_tmp->ssid, peer->device_name, sizeof(peer_tmp->ssid) - 1);
		peer_tmp->ssid[SSID_LENGTH - 1] = '\0';
		peer_tmp->category = peer->primary_device_type;
		peer_tmp->sub_category = peer->secondary_device_type;
		strncpy(peer_tmp->mac_addr, peer->mac_address, MAC_LENGTH - 1);
		peer_tmp->mac_addr[MAC_LENGTH - 1] = '\0';
		strncpy(peer_tmp->if_addr, peer->interface_address, MAC_LENGTH - 1);
		peer_tmp->if_addr[MAC_LENGTH - 1] = '\0';
		peer_tmp->is_group_owner = peer->is_group_owner;
		peer_tmp->is_persistent_group_owner = peer->is_persistent_group_owner;
		peer_tmp->is_connected = peer->is_connected;
		peer_tmp->dev_sel_state = FALSE;

		if (TRUE == peer->is_connected) {
			peer_tmp->conn_status = PEER_CONN_STATUS_CONNECTED;
		} else {
			peer_tmp->conn_status = PEER_CONN_STATUS_DISCONNECTED;
		}

		ugd->raw_discovered_peer_list = g_list_append(ugd->raw_discovered_peer_list, peer_tmp);
		DBG(LOG_INFO, "\tSSID: [%s]\n", peer_tmp->ssid);
		DBG(LOG_INFO, "\tPeer category [%d] -> [%d]\n", peer->primary_device_type, peer_tmp->category);
		DBG(LOG_INFO, "\tStatus: [%d]\n", peer_tmp->conn_status);
		DBG(LOG_INFO, "\tservice_count: [%d]\n", peer->service_count);
		ugd->raw_discovered_peer_cnt++;
	} else {
		DBG(LOG_INFO, "Unavailable WiFi-Direct Device\n");
	}

	WFD_IF_FREE_MEM(peer->device_name);
	WFD_IF_FREE_MEM(peer->mac_address);
	WFD_IF_FREE_MEM(peer->interface_address);

	if (NULL != peer->service_list)
	{
		for (i=0; i<peer->service_count && peer->service_list[i] != NULL; i++) {
			free(peer->service_list[i]);
		}
		WFD_IF_FREE_MEM(peer->service_list);
	}

	WFD_IF_FREE_MEM(peer);

	__FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the ug make a callback for connected peer
 *	@return   TRUE
 *	@param[in] peer the pointer to the connected peer
 *	@param[in] user_data the pointer to the main data structure
 */
bool _wfd_connected_peer_cb(wifi_direct_connected_peer_info_s *peer, void *user_data)
{
	__FUNC_ENTER__;
	WFD_RETV_IF(NULL == peer || NULL == user_data, FALSE, "Incorrect parameter(NULL)\n");

	struct ug_data *ugd = (struct ug_data *)user_data;
	int peer_cnt = ugd->raw_connected_peer_cnt;
	int i;

	DBG_SECURE(LOG_INFO, "%dth connected peer. [%s] ["MACSECSTR"]\n", peer_cnt,
		peer->device_name, MAC2SECSTR(peer->mac_address));

	/*
	* check wether ug needs to exit
	* automatically after successed connection
	*/

	char services[256] = {0,};
	DBG(LOG_INFO, "\tservice_count: [%d]\n", peer->service_count);
	if (peer->service_count>0) {
		unsigned int len = 0;
		for (i=0; i<peer->service_count && peer->service_list != NULL; i++) {
			snprintf(services + len, 256-len, "%s ", peer->service_list[i]);
			len = len + strlen(peer->service_list[i]) + 1;
		}
		DBG(LOG_INFO, "\tServices: [%s]\n", services);
	}

	strncpy(ugd->raw_connected_peers[peer_cnt].ssid, peer->device_name, sizeof(ugd->raw_connected_peers[peer_cnt].ssid) - 1);
	ugd->raw_connected_peers[peer_cnt].category = peer->primary_device_type;
	ugd->raw_connected_peers[peer_cnt].sub_category = peer->secondary_device_type;
	strncpy(ugd->raw_connected_peers[peer_cnt].mac_addr, peer->mac_address, MAC_LENGTH - 1);
	strncpy(ugd->raw_connected_peers[peer_cnt].if_addr, peer->interface_address, MAC_LENGTH - 1);
	ugd->raw_connected_peers[peer_cnt].conn_status = PEER_CONN_STATUS_CONNECTED;

	DBG(LOG_INFO, "\tStatus: [%d]\n", ugd->raw_connected_peers[peer_cnt].conn_status);
	DBG(LOG_INFO, "\tCategory: [%d]\n", ugd->raw_connected_peers[peer_cnt].category);
	DBG(LOG_INFO, "\tSSID: [%s]\n", ugd->raw_connected_peers[peer_cnt].ssid);

	ugd->raw_connected_peer_cnt++;

	int error = -1;
	bool is_group_owner = FALSE;
	error = wifi_direct_is_group_owner(&is_group_owner);
	if (error != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Fail to get group_owner_state. ret=[%d]", error);
		return FALSE;
	}

	if (FALSE == is_group_owner) {
		/* to send ip_addr*/
		int ret = -1;
		app_control_h control = NULL;
		ret = app_control_create(&control);
		if (ret) {
			DBG(LOG_ERROR, "Failed to create control");
			return FALSE;
		}

		if(peer->ip_address != NULL && strlen(services) != 0 ) {
			app_control_add_extra_data(control, "ip_address", peer->ip_address);
			app_control_add_extra_data(control, "wfds", services);
			ug_send_result(ugd->ug, control);
		}
		app_control_destroy(control);
	}

	WFD_IF_FREE_MEM(peer->device_name);
	WFD_IF_FREE_MEM(peer->mac_address);
	WFD_IF_FREE_MEM(peer->interface_address);

	if (NULL != peer->service_list)
	{
		for (i=0; i<peer->service_count && peer->service_list[i] != NULL; i++) {
			free(peer->service_list[i]);
		}
		WFD_IF_FREE_MEM(peer->service_list);
	}

	WFD_IF_FREE_MEM(peer);

	__FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the ug get the found peers
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int wfd_ug_get_discovered_peers(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	int res = 0;
	WFD_RETV_IF(ugd == NULL, -1, "Incorrect parameter(NULL)\n");

	ugd->raw_discovered_peer_cnt = 0;
	wfd_client_free_raw_discovered_peers(ugd);
	res = wifi_direct_foreach_discovered_peers(_wfd_discoverd_peer_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		ugd->raw_discovered_peer_cnt = 0;
		DBG(LOG_ERROR, "Get discovery result failed: %d\n", res);
	}

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug get the connecting peer
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int wfd_ug_get_connecting_peer(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	int res = 0;
	WFD_RETV_IF(ugd == NULL, -1, "Incorrect parameter(NULL)\n");
	char *mac_addr = NULL;
	GList *iterator = NULL;

	ugd->mac_addr_connecting = NULL;
	res = wifi_direct_get_connecting_peer(&mac_addr);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Get connecting device mac failed: %d\n", res);
		return -1;
	}
	DBG_SECURE(LOG_INFO, "Mac Addr Connecting: ["MACSECSTR"]\n",
		MAC2SECSTR(mac_addr));
	ugd->mac_addr_connecting = mac_addr;

	for (iterator = ugd->raw_discovered_peer_list; iterator; iterator = iterator->next) {
		if (!strncmp(mac_addr, ((device_type_s *)iterator->data)->mac_addr, MAC_LENGTH)) {
			((device_type_s *)iterator->data)->conn_status = PEER_CONN_STATUS_CONNECTING;
		}
	}


	__FUNC_EXIT__;
	return 0;
}


/**
 *	This function let the ug get the connected peers
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int wfd_ug_get_connected_peers(struct ug_data *ugd)
{
	__FUNC_ENTER__;
	int res = 0;
	WFD_RETV_IF(ugd == NULL, -1, "Incorrect parameter(NULL)\n");

	ugd->raw_connected_peer_cnt = 0;
	res = wifi_direct_foreach_connected_peers(_wfd_connected_peer_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		ugd->raw_connected_peer_cnt = 0;
		DBG(LOG_ERROR, "Get connected peer failed: %d\n", res);
	}

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug exits automatically after successed connection
 *	@return   void
 *	@param[in] user_data the pointer to the main data structure
 */
void _wfd_ug_auto_exit(void *user_data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)user_data;
	WFD_RET_IF(ugd == NULL, "Incorrect parameter(NULL)\n");

	usleep(1000);
	deinit_wfd_client(ugd);
	wfd_destroy_ug(ugd);

	__FUNC_EXIT__;
}

gboolean wfd_delete_not_alive_peer_cb(void *user_data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)user_data;
	WFD_RETV_IF(ugd == NULL, FALSE, "Incorrect parameter(NULL)\n");

	delete_not_alive_peers(ugd, &ugd->gl_avlb_peers_start, &ugd->gl_available_peer_cnt);
	delete_not_alive_peers(ugd, &ugd->gl_busy_peers_start, &ugd->gl_busy_peer_cnt);
	delete_not_alive_peers(ugd, &ugd->multi_conn_dev_list_start, &ugd->gl_available_dev_cnt_at_multiconn_view);
	wfd_ug_view_init_genlist(ugd, false);
	wfd_update_multiconnect_device(ugd, false);
	__FUNC_EXIT__;
	return FALSE;
}


gboolean wfd_delete_progressbar_cb(void *user_data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)user_data;
	WFD_RETV_IF(ugd == NULL, FALSE, "Incorrect parameter(NULL)\n");

	ugd->title_content_mode = TITLE_CONTENT_TYPE_NONE;
	if (ugd->raw_discovered_peer_cnt == 0 &&
		ugd->nodevice_title_item == NULL &&
		ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_NONE &&
		ugd->gl_available_peer_cnt == 0) {
			_create_no_device_genlist(ugd);
	};

	wfd_ug_view_refresh_glitem(ugd->mcview_title_item);
	wfd_ug_view_refresh_glitem(ugd->avlbl_wfd_item);

	if (0 == ugd->gl_available_dev_cnt_at_multiconn_view) {
		_create_no_device_multiconnect_genlist(ugd);
	}

	wfd_refresh_wifi_direct_state(ugd);
	if (WIFI_DIRECT_STATE_CONNECTING != ugd->wfd_status &&
		WIFI_DIRECT_STATE_DISCONNECTING != ugd->wfd_status) {
		if (ugd->scan_toolbar) {
			wfd_ug_view_refresh_button(ugd->scan_toolbar, "IDS_WIFI_SK4_SCAN", TRUE);
			evas_object_data_set(ugd->toolbar, "scan", "scan");
		}

		if (ugd->multiconn_layout) {
			wfd_ug_view_refresh_button(ugd->multiconn_scan_stop_btn, "IDS_WIFI_SK4_SCAN", TRUE);
			DBG(LOG_INFO, "Multiconn button text IDS_WIFI_SK4_SCAN \n");

		}
	}

	__FUNC_EXIT__;
	return FALSE;
}

/**
 *	This function let the ug make a callback for registering discover event
 *	@return   void
 *	@param[in] error_code the returned error code
 *	@param[in] discovery_state the state of discover
 *	@param[in] user_data the pointer to the main data structure
 */
void discover_cb(int error_code, wifi_direct_discovery_state_e discovery_state, void *user_data)
{
	__FUNC_ENTER__;
	int ret;
	struct ug_data *ugd = (struct ug_data *)user_data;

	if (ugd == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS) {
		return;
	}

	DBG(LOG_INFO, "Discovery event [%d], error_code [%d]\n", discovery_state, error_code);

	if (discovery_state == WIFI_DIRECT_DISCOVERY_STARTED) {
		if (ugd->wfd_discovery_status == WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START) {
			ugd->title_content_mode = TITLE_CONTENT_TYPE_SCANNING;
			wfd_cancel_progressbar_stop_timer(ugd);
			ugd->timer_stop_progress_bar = g_timeout_add(1000*30, wfd_delete_progressbar_cb, ugd);
			/* clear all the previous discovered peers */
			wfd_client_free_raw_discovered_peers(ugd);

			ugd->raw_discovered_peer_cnt = 0;
			wfd_ug_view_init_genlist(ugd, false);

			if (ugd->avlbl_wfd_item == NULL) {
				_create_available_dev_genlist(ugd);
			}

			wfd_ug_view_refresh_glitem(ugd->mcview_title_item);
			/* clear not alive peers after 5 secs */
			wfd_cancel_not_alive_delete_timer(ugd);
			ugd->timer_delete_not_alive_peer = g_timeout_add(1000*5, wfd_delete_not_alive_peer_cb, ugd);
			set_not_alive_peers(ugd->gl_avlb_peers_start);
			set_not_alive_peers(ugd->gl_busy_peers_start);
			set_not_alive_peers(ugd->multi_conn_dev_list_start);
		}
	} else if (discovery_state == WIFI_DIRECT_DISCOVERY_FOUND) {
		if (ugd->wfd_discovery_status != WIFI_DIRECT_DISCOVERY_NONE) {
			wfd_ug_get_discovered_peers(ugd);
			wfd_ug_update_available_peers(ugd);
			wfd_update_multiconnect_device(ugd, false);
		}
	} else if (discovery_state == WIFI_DIRECT_DISCOVERY_FINISHED) {
		if (ugd->wfd_discovery_status == WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START) {
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_FULL_SCAN_START;
			ret = wifi_direct_start_discovery_specific_channel(false, 0, WIFI_DIRECT_DISCOVERY_FULL_SCAN);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
				DBG(LOG_ERROR, "Failed to start discovery with full scan. [%d]\n", ret);
				wifi_direct_cancel_discovery();
			}
		}
	}

	if (WIFI_DIRECT_DISCOVERY_STARTED == discovery_state &&
		ugd->wfd_discovery_status == WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START) {
		WFD_IF_DEL_ITEM(ugd->multi_connect_toolbar_item);
		if (!ugd->conn_wfd_item) {
			elm_object_part_content_set(ugd->layout, "button.big", ugd->scan_toolbar);
		}
		wfd_ug_view_refresh_button(ugd->scan_toolbar, "IDS_WIFI_SK_STOP", TRUE);
		if (ugd->multiconn_scan_stop_btn) {
			wfd_ug_view_refresh_button(ugd->multiconn_scan_stop_btn, "IDS_WIFI_SK_STOP", TRUE);
		}
	}

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug make a callback for registering connection event
 *	@return   void
 *	@param[in] error_code the returned error code
 *	@param[in] connection_state the state of connection
 *	@param[in] mac_address the mac address of peer
 *	@param[in] user_data the pointer to the main data structure
 */
void _connection_cb(int error_code, wifi_direct_connection_state_e connection_state, const char *mac_address, void *user_data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)user_data;
	device_type_s *peer = NULL;
	bool owner = FALSE;
	int res = 0;

	if (mac_address == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(peer mac is NULL)\n");
		return;
	}

	DBG_SECURE(LOG_INFO, "Connection event [%d], error_code [%d], multi_connect_mode [%d] mac ["MACSECSTR"]\n",
		connection_state, error_code, ugd->multi_connect_mode, MAC2SECSTR(mac_address));

	/* when not in connection, mac_address is empty */
	if (connection_state <= WIFI_DIRECT_DISASSOCIATION_IND) {
		peer = wfd_client_find_peer_by_mac(ugd, mac_address);

		if (NULL == peer || '\0' == peer->ssid[0]) {
			DBG(LOG_ERROR, "invalid peer from connection !!\n");
			ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_NONE;
			goto refresh_button;
		}
	}

	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS) {
		switch (connection_state) {
		case WIFI_DIRECT_CONNECTION_RSP:
			DBG(LOG_INFO, "MULTI: WIFI_DIRECT_CONNECTION_RSP\n");
			ugd->mac_addr_connecting = NULL;
			if (error_code == WIFI_DIRECT_ERROR_NONE) {
				peer->conn_status = PEER_CONN_STATUS_CONNECTED;
				wfd_ug_get_connected_peers(ugd);
				wfd_ug_update_connected_peers(ugd);
			} else {
				peer->conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
				peer = find_peer_in_glist(ugd->gl_mul_conn_peers_start, peer->mac_addr);
				if ( peer != NULL) {
					peer->conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
					wfd_ug_view_refresh_glitem(peer->gl_item);
				}
			}
			/* connect the next peer */
			ugd->g_source_multi_connect_next = g_timeout_add(500, wfd_multi_connect_next_cb, ugd);
			break;
		case WIFI_DIRECT_CONNECTION_IN_PROGRESS:
			DBG(LOG_INFO, "MULTI: WIFI_DIRECT_CONNECTION_IN_PROGRESS\n");
			peer->conn_status = PEER_CONN_STATUS_CONNECTING;
			peer = find_peer_in_glist(ugd->gl_mul_conn_peers_start, peer->mac_addr);

			if ( peer != NULL) {
				peer->conn_status = PEER_CONN_STATUS_CONNECTING;
				wfd_ug_view_refresh_glitem(peer->gl_item);
			}

			wfd_ug_update_toolbar(ugd);
			break;
		case WIFI_DIRECT_GROUP_CREATED:
			DBG(LOG_INFO, "MULTI: WIFI_DIRECT_GROUP_CREATED\n");
			wfd_cancel_progressbar_stop_timer(ugd);
			wfd_delete_progressbar_cb(ugd);

			wfd_ug_view_init_genlist(ugd, true);
			wfd_ug_view_update_multiconn_peers(ugd);
			wfd_multi_connect_next_cb(ugd);
			break;
		default:
			break;
		}
	} else {
		switch (connection_state) {
		case WIFI_DIRECT_CONNECTION_RSP:
			DBG(LOG_INFO, "WIFI_DIRECT_CONNECTION_RSP\n");
			wfd_delete_progressbar_cb(ugd);

			if (ugd->act_popup) {
				evas_object_del(ugd->act_popup);
				ugd->act_popup = NULL;
			}
			ugd->mac_addr_connecting = NULL;

			if (error_code == WIFI_DIRECT_ERROR_NONE) {
				peer->conn_status = PEER_CONN_STATUS_CONNECTED;
				wfd_ug_get_connected_peers(ugd);

				/* when auto_exit and not multi-connect*/
				if ((ugd->is_auto_exit)&&(ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_NONE)) {
					_wfd_ug_auto_exit(ugd);
				}

				wfd_ug_update_connected_peers(ugd);
			} else {
				peer->conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
				wfd_ug_update_failed_peers(ugd);
			}

			wfd_ug_update_toolbar(ugd);
			break;
		case WIFI_DIRECT_DISASSOCIATION_IND:
			DBG(LOG_INFO, "WIFI_DIRECT_DISASSOCIATION_IND\n");
			/* remove any possible popup */
			WFD_IF_DEL_OBJ(ugd->act_popup);
			wfd_ug_view_refresh_button(ugd->scan_toolbar, "IDS_WIFI_SK4_SCAN", TRUE);

			/* change the multi connection mode, it can be connected now */
			if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_COMPLETED) {
				ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_NONE;
			}

			/* if other peer disconnected, get connected peers and update */
			peer->conn_status = PEER_CONN_STATUS_DISCONNECTED;
			wfd_ug_get_connected_peers(ugd);
			wfd_ug_update_available_peers(ugd);
			break;
		case WIFI_DIRECT_DISCONNECTION_RSP:
		case WIFI_DIRECT_DISCONNECTION_IND:
			DBG(LOG_INFO, "WIFI_DIRECT_DISCONNECTION_X\n");
			WFD_IF_DEL_OBJ(ugd->act_popup);

			Evas_Object *content;
			content = elm_object_part_content_unset(ugd->layout, "button.next");
			WFD_IF_DEL_OBJ(content);
			/* when disconnection, clear all the connected peers */
			if (ugd->raw_connected_peer_cnt > 0) {
				memset(ugd->raw_connected_peers, 0x00, ugd->raw_connected_peer_cnt*sizeof(device_type_s));
			}

			ugd->raw_connected_peer_cnt = 0;
			wfd_ug_view_init_genlist(ugd, true);
			if (ugd->wfd_discovery_status == WIFI_DIRECT_DISCOVERY_BACKGROUND) {
				DBG(LOG_INFO, "Background mode\n");
				break;
			}

			if (ugd->is_paused == false) {
				/* start discovery again */
				ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
				res = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
				if (res != WIFI_DIRECT_ERROR_NONE) {
					ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
					DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", res);
					wifi_direct_cancel_discovery();
				}
			}

			break;
		case WIFI_DIRECT_CONNECTION_IN_PROGRESS:
			DBG(LOG_INFO, "WIFI_DIRECT_CONNECTION_IN_PROGRESS\n");
			wfd_ug_update_toolbar(ugd);
			wfd_cancel_progressbar_stop_timer(ugd);
			wfd_delete_progressbar_cb(ugd);

			if (ugd->multi_navi_item) {
				elm_naviframe_item_pop(ugd->naviframe);
			}

			ugd->mac_addr_connecting = peer->mac_addr;
			ugd->is_conn_incoming = FALSE;
			peer->conn_status = PEER_CONN_STATUS_CONNECTING;
			peer = find_peer_in_glist(ugd->gl_avlb_peers_start, peer->mac_addr);
			if (peer != NULL) {
				peer->conn_status = PEER_CONN_STATUS_CONNECTING;
				wfd_ug_view_refresh_glitem(peer->gl_item);
			} else {
				wfd_ug_get_discovered_peers(ugd);
				wfd_ug_update_available_peers(ugd);
			}

			break;
		case WIFI_DIRECT_CONNECTION_REQ:
		case WIFI_DIRECT_CONNECTION_WPS_REQ:
			ugd->mac_addr_connecting = peer->mac_addr;
			ugd->is_conn_incoming = TRUE;
			DBG(LOG_INFO, "WIFI_DIRECT_CLI_EVENT_CONNECTION_REQ\n");
			break;
		case WIFI_DIRECT_GROUP_DESTROYED:
			wfd_ug_update_toolbar(ugd);
			if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_COMPLETED) {
				ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_NONE;
			} else {
				ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
				res = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
				if (res != WIFI_DIRECT_ERROR_NONE) {
					ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
					DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", res);
					wifi_direct_cancel_discovery();
				}
			}

			break;
		default:
			break;
		}
	}

refresh_button:
	/* refresh the scan button */
	wfd_refresh_wifi_direct_state(ugd);
	if (WIFI_DIRECT_STATE_CONNECTING == ugd->wfd_status ||
		WIFI_DIRECT_STATE_DISCONNECTING == ugd->wfd_status) {
		res = wifi_direct_is_group_owner(&owner);
		if (res == WIFI_DIRECT_ERROR_NONE) {
			if (!owner) {
				if (ugd->scan_toolbar) {
					evas_object_data_set(ugd->toolbar, "scan", "scan");
				}

				if (ugd->multiconn_scan_stop_btn) {
					wfd_ug_view_refresh_button(ugd->multiconn_scan_stop_btn, "IDS_WIFI_SK4_SCAN", FALSE);
				}
			}
		} else {
		    DBG(LOG_ERROR, "Failed to get whether client is group owner. [%d]\n", res);
		}
	} else {
		if (ugd->scan_toolbar) {
			evas_object_data_set(ugd->toolbar, "scan", "scan");
		}

		if (ugd->multiconn_scan_stop_btn) {
			wfd_ug_view_refresh_button(ugd->multiconn_scan_stop_btn, "IDS_WIFI_SK4_SCAN", TRUE);
		}
	}

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug make a callback for registering ip assigned event
 *	@return   void
 *	@param[in] mac_address the mac address of peer
 *	@param[in] ip_address the ip address of peer
 *	@param[in] interface_address the interface address
 *	@param[in] user_data the pointer to the main data structure
 */
void _ip_assigned_cb(const char *mac_address, const char *ip_address, const char *interface_address, void *user_data)
{
	__FUNC_ENTER__;

	if (!user_data) {
		DBG(LOG_ERROR, "The user_data is NULL\n");
		return;
	}

	struct ug_data *ugd = (struct ug_data *)user_data;

	if (!ip_address || 0 == strncmp(ip_address, "0.0.0.0", 7)) {
		DBG(LOG_ERROR,"ip address is invalid.\n");
		return;
	}

	ugd->peer_ip_address = strdup(ip_address);


	/* to send ip_addr*/
	int ret = -1;
	app_control_h control = NULL;
	ret = app_control_create(&control);
	if (ret) {
		DBG(LOG_ERROR, "Failed to create control");
		return;
	}
	app_control_add_extra_data(control, "ip_address", ugd->peer_ip_address);
	app_control_add_extra_data(control, "wfds", ugd->service_name);
	ug_send_result(ugd->ug, control);
	app_control_destroy(control);

	/* when auto_exit and not multi-connect*/
	if ((ugd->is_auto_exit)&&(ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_NONE)) {
		_wfd_ug_auto_exit(ugd);
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug get wi-fi direct status from vconf
 *	@return   If success, return the wfd status, else return -1
 *	@param[in] void
 */
int wfd_get_vconf_status()
{
	__FUNC_ENTER__;
	int wifi_direct_state = 0;

	/* get wifi direct status from vconf */
	if (vconf_get_int(VCONFKEY_WIFI_DIRECT_STATE, &wifi_direct_state) < 0) {
		DBG(LOG_ERROR, "Error reading vconf (%s)\n", VCONFKEY_WIFI_DIRECT_STATE);
		return -1;
	}
	DBG(LOG_INFO, "WiFi Direct State [%d]", wifi_direct_state);

	__FUNC_EXIT__;
	return wifi_direct_state;
}

/**
 *	This function let the ug get device name from vconf
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_get_vconf_device_name(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	char *dev_name = NULL;

	/* get device name from vconf */
	dev_name = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);
	if (dev_name == NULL) {
		ugd->dev_name = strdup(DEFAULT_DEV_NAME);
		DBG(LOG_ERROR, "The AP name is NULL(setting default value)\n");
		return -1;
	}

	ugd->dev_name = strdup(dev_name);
	WFD_IF_FREE_MEM(dev_name);

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug refresh current status of wi-fi direct
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_refresh_wifi_direct_state(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;
	wifi_direct_state_e wfd_status;

	res = wifi_direct_get_state(&wfd_status);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to get link status. [%d]\n", res);
		return -1;
	}

	DBG(LOG_INFO, "WFD status [%d]", wfd_status);
	ugd->wfd_status = wfd_status;

	__FUNC_EXIT__;
	return 0;
}

void wfd_init_ug_by_status(void *user_data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)user_data;
	int res = 0;

	if(ugd == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return;
	}

	if (ugd->wfd_status >= WIFI_DIRECT_STATE_ACTIVATED) {
		//wfd_ug_get_discovered_peers(ugd);
		ugd->title_content_mode = TITLE_CONTENT_TYPE_NONE;
	}

	if (ugd->wfd_status >= WIFI_DIRECT_STATE_CONNECTED) {
		wfd_ug_get_connected_peers(ugd);
		wfd_ug_update_connected_peers(ugd);
		ugd->title_content_mode = TITLE_CONTENT_TYPE_NONE;
		wfd_ug_get_discovered_peers(ugd);
		wfd_ug_update_available_peers(ugd);
		wfd_ug_update_toolbar(ugd);
	}

	if (ugd->wfd_status == WIFI_DIRECT_STATE_CONNECTING) {
		ugd->title_content_mode = TITLE_CONTENT_TYPE_NONE;
		wfd_ug_get_discovered_peers(ugd);
		wfd_ug_get_connecting_peer(ugd);
		wfd_ug_update_available_peers(ugd);
		wfd_ug_update_toolbar(ugd);
	}

	if (ugd->wfd_status == WIFI_DIRECT_STATE_ACTIVATED ||
		ugd->wfd_status == WIFI_DIRECT_STATE_DISCOVERING) {
		/* start discovery */
		ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL_START;
		res = wifi_direct_start_discovery_specific_channel(false, 1, WIFI_DIRECT_DISCOVERY_SOCIAL_CHANNEL);
		if (res != WIFI_DIRECT_ERROR_NONE) {
			ugd->wfd_discovery_status = WIFI_DIRECT_DISCOVERY_NONE;
			DBG(LOG_ERROR, "Failed to start discovery. [%d]\n", res);
			wifi_direct_cancel_discovery();
		}
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug do initialization
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int init_wfd_client(void* data)
{
	__FUNC_ENTER__;
	WFD_RETV_IF(data == NULL, -1, "Incorrect parameter(NULL)\n");
	struct ug_data *ugd = (struct ug_data *)data;
	int res = 0;

	res = wifi_direct_initialize();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		if (res != WIFI_DIRECT_ERROR_ALREADY_INITIALIZED) {
			DBG(LOG_ERROR, "Failed to initialize wifi direct. [%d]\n", res);
			return -1;
		} else {
			DBG(LOG_ERROR, "Already registered\n");
		}
	}

	res = wifi_direct_set_device_state_changed_cb(_activation_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to register _cb_activation. error code = [%d]\n", res);
		return -1;
	}

	res = wifi_direct_set_discovery_state_changed_cb(discover_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to register _cb_discover. error code = [%d]\n", res);
		return -1;
	}

	res = wifi_direct_set_connection_state_changed_cb(_connection_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to register _cb_connection. error code = [%d]\n", res);
		return -1;
	}

	res = wifi_direct_set_client_ip_address_assigned_cb(_ip_assigned_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to register _ip_assigned_cb. error code = [%d]\n", res);
		return -1;
	}

	/* update WFD status */
	wfd_refresh_wifi_direct_state(ugd);
#ifdef WFD_ON_OFF_GENLIST
	if (ugd->wfd_status > WIFI_DIRECT_STATE_ACTIVATING) {
		ugd->wfd_onoff = 1;
		wfd_ug_refresh_on_off_check(ugd);
	} else {
		ugd->wfd_onoff = 0;
	}
#endif

	DBG(LOG_INFO, "WFD link status. [%d]\n", ugd->wfd_status);
	ugd->is_init_ok = TRUE;
	wfd_init_ug_by_status(ugd);

	__FUNC_EXIT__;
	return 0;
}

#ifdef WFD_DBUS_LAUNCH
void wfd_gdbus_callback(GObject *source_object, GAsyncResult *result, gpointer user_data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)user_data;
	WFD_RET_IF(ugd == NULL, "Incorrect parameter(NULL)\n");
	int res = -1;

	GError *error = NULL;
	GVariant *return_data;

	g_object_unref(ugd->dbus_cancellable);
	ugd->dbus_cancellable = NULL;
	ugd->conn = G_DBUS_CONNECTION (source_object);
	return_data = g_dbus_connection_call_finish(ugd->conn, result, &error);

	if (error != NULL) {
		DBG(LOG_ERROR,"DBus action failed. Error Msg [%s]\n", error->message);
		g_clear_error(&error);
	} else {
		DBG(LOG_INFO, "error msg is NULL\n");
	}

	if (return_data)
		g_variant_unref(return_data);

	if (ugd->conn) {
		g_object_unref(ugd->conn);
		ugd->conn = NULL;
	}

	res = init_wfd_client(ugd);
	WFD_RET_IF(res != 0, "Failed to initialize WFD client library\n");

	/* Activate WiFi Direct */
	DBG(LOG_INFO, "Activating WiFi Direct...");
	if (ugd->wfd_status <= WIFI_DIRECT_STATE_DEACTIVATING) {
		res = wfd_client_switch_on(ugd);
		WFD_RET_IF(res != 0, "Failed to activate WFD\n");
	}

	__FUNC_EXIT__;
}

int launch_wifi_direct_manager(void *data)
{
	__FUNC_ENTER__;

	gchar *addr = NULL;
	GError *error = NULL;

	struct ug_data *ugd = (struct ug_data *)data;
	WFD_RETV_IF(ugd == NULL, -1, "Incorrect parameter(NULL)\n");

	ugd->dbus_cancellable = g_cancellable_new();

	addr  = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	WFD_RETV_IF(addr == NULL, -1, "Fail to get dbus addr.\n");

	ugd->conn = g_dbus_connection_new_for_address_sync(addr,G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
			G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION, NULL, NULL, NULL);

	if(ugd->conn == NULL) {
		DBG(LOG_ERROR,"g_dbus_conn is NULL\n");
		return -1;
	} else {
		g_dbus_connection_call(ugd->conn, "net.netconfig", "/net/netconfig/wifi","net.netconfig.wifi",
		"LaunchDirect", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, ugd->dbus_cancellable, wfd_gdbus_callback, data);
	}

	__FUNC_EXIT__;
	return 0;
}
#endif

void wfd_client_destroy_tethering(struct ug_data *ugd)
{
	__FUNC_ENTER__;

	tethering_error_e ret = TETHERING_ERROR_NONE;

	if (ugd->hotspot_handle != NULL) {
		/* Deregister cbs */
		ret = tethering_unset_enabled_cb(ugd->hotspot_handle, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_unset_enabled_cb is failed(%d)\n", ret);
		}

		ret = tethering_unset_disabled_cb(ugd->hotspot_handle, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_unset_disabled_cb is failed(%d)\n", ret);
		}

		ret = tethering_unset_disabled_cb(ugd->hotspot_handle, TETHERING_TYPE_RESERVED);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_unset_disabled_cb is failed(%d)\n", ret);
		}

		/* Destroy tethering handle */
		ret = tethering_destroy(ugd->hotspot_handle);
		if (ret != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "tethering_destroy is failed(%d)\n", ret);
		}

		ugd->hotspot_handle = NULL;
	}

	__FUNC_EXIT__;
}

/**
 *	This function let the ug do de-initialization
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int deinit_wfd_client(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res = 0;

	wfd_refresh_wifi_direct_state(ugd);

	if ((WIFI_DIRECT_STATE_DISCOVERING == ugd->wfd_status) &&
		(WIFI_DIRECT_ERROR_NONE != wifi_direct_cancel_discovery())) {
		DBG(LOG_ERROR, "Failed to send cancel discovery state [%d]\n", ugd->wfd_status);
	}

	wfd_cancel_progressbar_stop_timer(ugd);
	wfd_cancel_not_alive_delete_timer(ugd);

	if(ugd->timer_multi_reset > 0) {
		g_source_remove(ugd->timer_multi_reset);
	}
	ugd->timer_multi_reset = 0;

	if (ugd->g_source_multi_connect_next > 0) {
		g_source_remove(ugd->g_source_multi_connect_next);
	}
	ugd->g_source_multi_connect_next = 0;

	res = wifi_direct_unset_discovery_state_changed_cb();
	if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to unset discovery state changed cb. [%d]\n", res);
	}

	wifi_direct_unset_device_state_changed_cb();
	if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to unset device state changed cb. [%d]\n", res);
	}

	wifi_direct_unset_connection_state_changed_cb();
	if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to unset connection state changed cb. [%d]\n", res);
	}

	wifi_direct_unset_client_ip_address_assigned_cb();
	if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to unset client ip address assigned cb. [%d]\n", res);
	}

	if (ugd->wfd_status == WIFI_DIRECT_STATE_CONNECTING &&
		NULL != ugd->mac_addr_connecting) {
		if (ugd->is_conn_incoming) {
			DBG(LOG_INFO, "Reject the incoming connection before client deregister \n");
			res = wifi_direct_reject_connection(ugd->mac_addr_connecting);
			if (res != WIFI_DIRECT_ERROR_NONE) {
				DBG(LOG_ERROR, "Failed to send reject request [%d]\n", res);
			}
		} else {
			DBG(LOG_INFO, "Cancel the outgoing connection before client deregister \n");
			res = wifi_direct_cancel_connection(ugd->mac_addr_connecting);
			if (res != WIFI_DIRECT_ERROR_NONE) {
				DBG(LOG_ERROR, "Failed to send cancel request [%d]\n", res);
			}
		}
		ugd->mac_addr_connecting = NULL;
	}


	res = wifi_direct_deinitialize();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to deregister client. [%d]\n", res);
	}

	/* release vconf, hotspot..  */
#ifndef MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE
	res = vconf_ignore_key_changed(VCONFKEY_WIFI_STATE, _wifi_state_cb);
	if (res == -1) {
		DBG(LOG_ERROR, "Failed to ignore vconf key callback for wifi state\n");
	}

	res = net_deregister_client();
	if (res != NET_ERR_NONE) {
		DBG(LOG_ERROR, "Failed to deregister network client. [%d]\n", res);
	}
#endif /* MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE */


	wfd_client_destroy_tethering(ugd);

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug turn wi-fi direct on
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_client_switch_on(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;

	bool is_wifi_enabled = false;
	bool is_wifi_ap_enabled = false;


	if(!ugd->is_init_ok) {
		DBG(LOG_ERROR, "device is initializing, please wait\n");
		return -1;
	}

	wfd_refresh_wifi_direct_state(ugd);
	DBG(LOG_INFO, "WFD status [%d]\n", ugd->wfd_status);

	if (ugd->wfd_status < WIFI_DIRECT_STATE_ACTIVATING) {

#ifndef MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE
		int wifi_state;
		res = vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);
		if (res != 0) {
			DBG(LOG_ERROR, "Failed to get wifi state from vconf. [%d]\n", res);
			return -1;
		}
#endif /* MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE */

		ugd->hotspot_handle = NULL;
		res = tethering_create(&(ugd->hotspot_handle));
		if (res != TETHERING_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to tethering_create() [%d]\n", res);
			return -1;
		} else {
			DBG(LOG_INFO, "Succeeded to tethering_create()\n");
		}

		is_wifi_enabled = tethering_is_enabled(ugd->hotspot_handle, TETHERING_TYPE_WIFI);
		is_wifi_ap_enabled = tethering_is_enabled(ugd->hotspot_handle, TETHERING_TYPE_RESERVED);

#ifndef MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE
		if (wifi_state > VCONFKEY_WIFI_OFF) {
			DBG(LOG_INFO, "WiFi is connected, so have to turn off WiFi");
			wfd_ug_act_popup(ugd, _("IDS_WIFI_BODY_USING_WI_FI_DIRECT_WILL_DISCONNECT_CURRENT_WI_FI_CONNECTION_CONTINUE_Q"), POPUP_TYPE_WIFI_OFF);
		} else
#endif /* MODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE */

		if (is_wifi_enabled || is_wifi_ap_enabled) {
			DBG(LOG_INFO, "WiFi is connected, so have to turn off WiFi");
			wfd_ug_act_popup(ugd, _("IDS_WIFI_BODY_USING_WI_FI_DIRECT_WILL_DISCONNECT_CURRENT_WI_FI_TETHERING_CONTINUE_Q"), POPUP_TYPE_HOTSPOT_OFF);
		} else

		{
			res = wifi_direct_activate();
			if (res != WIFI_DIRECT_ERROR_NONE) {
				DBG(LOG_ERROR, "Failed to activate Wi-Fi Direct. error code = [%d]\n", res);
				wfd_ug_warn_popup(ugd, _("IDS_COM_POP_FAILED"), POPUP_TYPE_TERMINATE);
#ifdef WFD_ON_OFF_GENLIST
				wfd_ug_refresh_on_off_check(ugd);
#endif
				return -1;
			}

#ifdef WFD_ON_OFF_GENLIST
			if (ugd->on_off_check) {
				elm_check_state_set(ugd->on_off_check, TRUE);
				elm_object_disabled_set(ugd->on_off_check, TRUE);
			}
#endif
			/* while activating, disable the buttons */
			if (ugd->scan_toolbar == NULL) {
				scan_button_create(ugd);
			}

			if (ugd->scan_toolbar) {
				wfd_ug_view_refresh_button(ugd->scan_toolbar, "IDS_WIFI_SK4_SCAN", FALSE);
			}

			if (ugd->multiconn_scan_stop_btn) {
				wfd_ug_view_refresh_button(ugd->multiconn_scan_stop_btn, "IDS_WIFI_SK4_SCAN", FALSE);
			}

			if (ugd->back_btn) {
				elm_object_disabled_set(ugd->back_btn, TRUE);
			}
		}
	} else {
		DBG(LOG_INFO, "Wi-Fi Direct is already activated\n");
	}

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug turn wi-fi direct off
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_client_switch_off(void *data)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;

	wfd_ug_view_free_peers(ugd);
	wfd_free_nodivice_item(ugd);

	wfd_refresh_wifi_direct_state(ugd);
	DBG(LOG_INFO, "WFD status [%d]\n", ugd->wfd_status);

	if (ugd->wfd_status < WIFI_DIRECT_STATE_ACTIVATING) {
		DBG(LOG_INFO, "Wi-Fi Direct is already deactivated\n");
	} else {

		wfd_client_destroy_tethering(ugd);

		wfd_cancel_progressbar_stop_timer(ugd);
		wfd_cancel_not_alive_delete_timer(ugd);

		if(ugd->timer_multi_reset > 0) {
			g_source_remove(ugd->timer_multi_reset);
		}
		ugd->timer_multi_reset = 0;

		if (ugd->g_source_multi_connect_next > 0) {
			g_source_remove(ugd->g_source_multi_connect_next);
		}
		ugd->g_source_multi_connect_next = 0;

		/*if connected, disconnect all devices*/
		if (WIFI_DIRECT_STATE_CONNECTED == ugd->wfd_status) {
			res = wifi_direct_disconnect_all();
			if (res != WIFI_DIRECT_ERROR_NONE) {
				DBG(LOG_ERROR, "Failed to send disconnection request to all. [%d]\n", res);
				return -1;
			}
		}

		res = wifi_direct_deactivate();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
			wfd_ug_warn_popup(ugd, _("IDS_WIFI_POP_DEACTIVATION_FAILED"), POPUP_TYPE_TERMINATE_DEACTIVATE_FAIL);
#ifdef WFD_ON_OFF_GENLIST
			wfd_ug_refresh_on_off_check(ugd);
#endif
			return -1;
		}

		/* while deactivating, disable the buttons */
		if (ugd->scan_toolbar) {
			wfd_ug_view_refresh_button(ugd->scan_toolbar, "IDS_WIFI_SK4_SCAN", FALSE);
			evas_object_del(ugd->scan_toolbar);
			ugd->scan_toolbar = NULL;
		}

		if (ugd->multiconn_scan_stop_btn) {
			wfd_ug_view_refresh_button(ugd->multiconn_scan_stop_btn, "IDS_WIFI_SK4_SCAN", FALSE);
		}

		if (ugd->multi_connect_toolbar_item) {
			elm_object_item_disabled_set(ugd->multi_connect_toolbar_item, TRUE);
		}

		if (ugd->back_btn) {
			elm_object_disabled_set(ugd->back_btn, TRUE);
		}
	}

	__FUNC_EXIT__;
	return 0;
}

#ifdef WFD_ON_OFF_GENLIST
/**
 *	This function let the ug turn wi-fi direct on/off forcely
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
  *	@param[in] onoff whether to turn on/off wi-fi direct
 */
int wfd_client_swtch_force(void *data, int onoff)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;

	if (onoff) {
		res = wifi_direct_activate();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to activate Wi-Fi Direct. error code = [%d]\n", res);
			wfd_ug_warn_popup(ugd, _("IDS_COM_POP_FAILED"), POPUP_TYPE_TERMINATE);
			wfd_ug_refresh_on_off_check(ugd);
			return -1;
		}
	} else {
		res = wifi_direct_deactivate();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
			wfd_ug_warn_popup(ugd, _("IDS_WIFI_POP_DEACTIVATION_FAILED"), POPUP_TYPE_TERMINATE);
			wfd_ug_refresh_on_off_check(ugd);
			return -1;
		}
	}

	__FUNC_EXIT__;
	return 0;
}
#endif

/**
 *	This function let the ug create a group
 *	@return   If success, return 0, else return -1
 */
int wfd_client_group_add()
{
	__FUNC_ENTER__;
	int res;

	res = wifi_direct_create_group();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to add group");
		__FUNC_EXIT__;
		return -1;
	}

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug connect to the device by mac address
 *	@return   If success, return 0, else return -1
 *	@param[in] mac_addr the pointer to the mac address of device
 */
int wfd_client_connect(const char *mac_addr)
{
	__FUNC_ENTER__;
	int res;

	DBG_SECURE(LOG_INFO, "connect to peer=["MACSECSTR"]\n", MAC2SECSTR(mac_addr));
	res = wifi_direct_connect((char *)mac_addr);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to send connection request. [%d]\n", res);
		return -1;
	}

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug disconnect to the device by mac address
 *	@return   If success, return 0, else return -1
 *	@param[in] mac_addr the pointer to the mac address of device
 */
int wfd_client_disconnect(const char *mac_addr)
{
	__FUNC_ENTER__;
	int res;

	wifi_direct_cancel_discovery();
	/*
	 * No need to handle return in cancel discovery as there maybe case
	 * when framework can return failure.
	 */

	if (mac_addr == NULL) {
		res = wifi_direct_disconnect_all();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to send disconnection request to all. [%d]\n", res);
			return -1;
		}
	} else {
		res = wifi_direct_disconnect((char *)mac_addr);
		if (res != WIFI_DIRECT_ERROR_NONE) {
			DBG(LOG_ERROR, "Failed to send disconnection request. [%d]\n", res);
			return -1;
		}
	}

	__FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug set the intent of a group owner
 *	@return   If success, return 0, else return -1
 *	@param[in] go_intent the intent parameter
 */
int wfd_client_set_p2p_group_owner_intent(int go_intent)
{
	__FUNC_ENTER__;
	int res;

	res = wifi_direct_set_group_owner_intent(go_intent);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		DBG(LOG_ERROR, "Failed to wifi_direct_set_go_intent(%d). [%d]\n", go_intent, res);
		return -1;
	}

	__FUNC_EXIT__;
	return 0;
}
