/*
*  WiFi-Direct UG
*
* Copyright 2012  Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.0 (the "License");
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

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

/**
 *	This function let the ug make a change callback for wifi state
 *	@return   void
 *	@param[in] key the pointer to the key
 *	@param[in] data the pointer to the main data structure
 */
static void _wifi_state_cb(keynode_t *key, void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;
	int wifi_state;

	res = vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);
	if (res != 0) {
		WDUG_LOGE("Failed to get wifi state from vconf. [%d]\n", res);
		return;
	}

	if (wifi_state == VCONFKEY_WIFI_OFF) {
		WDUG_LOGI("WiFi is turned off\n");
		wfd_client_swtch_force(ugd, TRUE);
	} else {
		WDUG_LOGI("WiFi is turned on\n");
	}

	res = net_deregister_client();
	if (res != NET_ERR_NONE) {
		WDUG_LOGE("Failed to deregister network client. [%d]\n", res);
	}

	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug make a event callback for network registering
 *	@return   void
 *	@param[in] event_info the pointer to the information of network event
 *	@param[in] user_data the pointer to the user data
 */
static void _network_event_cb(net_event_info_t *event_info, void *user_data)
{
	__WDUG_LOG_FUNC_ENTER__;
	WDUG_LOGI("Event from network. [%d]\n", event_info->Event);
	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug turn wifi off
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_wifi_off(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;

	res = vconf_notify_key_changed(VCONFKEY_WIFI_STATE, _wifi_state_cb, ugd);
	if (res == -1) {
		WDUG_LOGE("Failed to register vconf callback\n");
		return -1;
	}

	WDUG_LOGI("Vconf key callback is registered\n");

	res = net_register_client((net_event_cb_t) _network_event_cb, NULL);
	if (res != NET_ERR_NONE) {
		WDUG_LOGE("Failed to register network client. [%d]\n", res);
		return -1;
	}

	WDUG_LOGI("Network client is registered\n");

	res = net_wifi_power_off();
	if (res != NET_ERR_NONE) {
		WDUG_LOGE("Failed to turn off wifi. [%d]\n", res);
		return -1;
	}

	WDUG_LOGI("WiFi power off\n");

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug make a change callback for enabling hotspot state
 *	@return   void
 *	@param[in] key the pointer to the key
 *	@param[in] data the pointer to the main data structure
 */
static void _enable_hotspot_state_cb(keynode_t *key, void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;
	int hotspot_mode;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h th = NULL;

	res = vconf_get_int(VCONFKEY_MOBILE_HOTSPOT_MODE, &hotspot_mode);
	if (res != 0) {
		WDUG_LOGE("Failed to get mobile hotspot state from vconf. [%d]\n", res);
		return;
	}

	if (hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI) {
		WDUG_LOGI(" Mobile hotspot is activated\n");
	}

	th = ugd->hotspot_handle;

	if (th != NULL) {
		/* Deregister cbs */
		ret = tethering_unset_enabled_cb(th, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			WDUG_LOGE("tethering_unset_enabled_cb is failed(%d)\n", ret);
		}

		/* Destroy tethering handle */
		ret = tethering_destroy(th);
		if (ret != TETHERING_ERROR_NONE) {
			WDUG_LOGE("tethering_destroy is failed(%d)\n", ret);
		}

		ugd->hotspot_handle = NULL;
	}

	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug make a change callback for disabling hotspot state
 *	@return   void
 *	@param[in] key the pointer to the key
 *	@param[in] data the pointer to the main data structure
 */
static void _disable_hotspot_state_cb(keynode_t *key, void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;
	int hotspot_mode;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h th = NULL;

	res = vconf_get_int(VCONFKEY_MOBILE_HOTSPOT_MODE, &hotspot_mode);
	if (res != 0) {
		WDUG_LOGE("Failed to get mobile hotspot state from vconf. [%d]\n", res);
		return;
	}

	if (!(hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI)) {
		WDUG_LOGI(" Mobile hotspot is deactivated\n");
		wfd_client_swtch_force(ugd, TRUE);
	}

	th = ugd->hotspot_handle;

	if (th != NULL) {
		/* Deregister cbs */
		ret = tethering_unset_disabled_cb(th, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			WDUG_LOGE("tethering_unset_disabled_cb is failed(%d)\n", ret);
		}

		/* Destroy tethering handle */
		ret = tethering_destroy(th);
		if (ret != TETHERING_ERROR_NONE) {
			WDUG_LOGE("tethering_destroy is failed(%d)\n", ret);
		}

		ugd->hotspot_handle = NULL;
	}

	__WDUG_LOG_FUNC_EXIT__;
}

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
	__WDUG_LOG_FUNC_ENTER__;

	if (error != TETHERING_ERROR_NONE) {
		if (is_requested != TRUE) {
			return;
		}

		WDUG_LOGE("error !!! TETHERING is not enabled.\n");
		return;
	}

	WDUG_LOGI("TETHERING is enabled.\n");

	__WDUG_LOG_FUNC_EXIT__;
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
	__WDUG_LOG_FUNC_ENTER__;

	if (error != TETHERING_ERROR_NONE) {
		if (code != TETHERING_DISABLED_BY_REQUEST) {
			return;
		}

		WDUG_LOGE("error !!! TETHERING is not disabled.\n");
		return;
	}

	WDUG_LOGI("TETHERING is disabled.\n");

	__WDUG_LOG_FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug turn AP on
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_mobile_ap_on(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h th = NULL;

	res = vconf_notify_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE, _enable_hotspot_state_cb, ugd);
	if (res == -1) {
		WDUG_LOGE("Failed to register vconf callback\n");
		return -1;
	}

	/* Create tethering handle */
	ret = tethering_create(&th);
	if (ret != TETHERING_ERROR_NONE) {
		WDUG_LOGE("Failed to tethering_create() [%d]\n", ret);
		return -1;
	} else {
		WDUG_LOGI("Succeeded to tethering_create()\n");
	}

	/* Register cbs */
	ret = tethering_set_enabled_cb(th, TETHERING_TYPE_WIFI, __enabled_cb, NULL);
	if (ret != TETHERING_ERROR_NONE) {
		WDUG_LOGE("tethering_set_enabled_cb is failed\n", ret);
		return -1;
	}

	/* Enable tethering */
	ret = tethering_enable(th, TETHERING_TYPE_WIFI);
	if (ret != TETHERING_ERROR_NONE) {
		WDUG_LOGE("Failed to turn on mobile hotspot. [%d]\n", ret);
		return -1;
	} else {
		WDUG_LOGI("Succeeded to turn on mobile hotspot\n");
	}

	ugd->hotspot_handle = th;
	ugd->is_hotspot_off = FALSE;

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug turn AP off
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_mobile_ap_off(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h th = NULL;

	res = vconf_notify_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE, _disable_hotspot_state_cb, ugd);
	if (res == -1) {
		WDUG_LOGE("Failed to register vconf callback\n");
		return -1;
	}

	/* Create tethering handle */
	ret = tethering_create(&th);
	if (ret != TETHERING_ERROR_NONE) {
		WDUG_LOGE("Failed to tethering_create() [%d]\n", ret);
		return -1;
	} else {
		WDUG_LOGI("Succeeded to tethering_create()\n");
	}

	/* Register cbs */
	ret = tethering_set_disabled_cb(th, TETHERING_TYPE_WIFI, __disabled_cb, NULL);
	if (ret != TETHERING_ERROR_NONE) {
		WDUG_LOGE("tethering_set_disabled_cb is failed\n", ret);
		return -1;
	}

	/* Disable tethering */
	ret = tethering_disable(th, TETHERING_TYPE_WIFI);
	if (ret != TETHERING_ERROR_NONE) {
		WDUG_LOGE("Failed to turn off mobile hotspot. [%d]\n", ret);
		return -1;
	} else {
		WDUG_LOGI("Succeeded to turn off mobile hotspot\n");
	}

	ugd->hotspot_handle = th;
	ugd->is_hotspot_off = TRUE;

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug find the peer by mac address
 *	@return   the found peer
 *	@param[in] data the pointer to the main data structure
 *	@param[in] mac_addr the pointer to mac address
 */
static device_type_s *wfd_client_find_peer_by_mac(void *data, const char *mac_addr)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int i;

	if (ugd == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (ugd->multi_connect_mode != WFD_MULTI_CONNECT_MODE_NONE) {
		for (i = 0; i < ugd->raw_multi_selected_peer_cnt; i++) {
			WDUG_LOGI("[Multi Connect] check %dth peer\n", i);
			if (!strncmp(mac_addr, (const char *)ugd->raw_multi_selected_peers[i].mac_addr, MAC_LENGTH)) {
				WDUG_LOGI("selected found peer. [%d]\n", i);
				__WDUG_LOG_FUNC_EXIT__;
				return &ugd->raw_multi_selected_peers[i];
			}
		}
	} else {
		for (i = 0; i < ugd->raw_discovered_peer_cnt; i++) {
			WDUG_LOGI("check %dth peer\n", i);
			if (!strncmp(mac_addr, (const char *)ugd->raw_discovered_peers[i].mac_addr, MAC_LENGTH)) {
				WDUG_LOGI("found peer. [%d]\n", i);
				__WDUG_LOG_FUNC_EXIT__;
				return &ugd->raw_discovered_peers[i];
			}
		}
	}

	__WDUG_LOG_FUNC_EXIT__;
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
	__WDUG_LOG_FUNC_ENTER__;
	int res = -1;
	struct ug_data *ugd = (struct ug_data *)user_data;

	wfd_refresh_wifi_direct_state(ugd);

	switch (device_state) {
	case WIFI_DIRECT_DEVICE_STATE_ACTIVATED:
		WDUG_LOGI("WIFI_DIRECT_DEVICE_STATE_ACTIVATED\n");
		if (error_code != WIFI_DIRECT_ERROR_NONE) {
			WDUG_LOGE("Error in Activation/Deactivation [%d]\n", error_code);
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
		if (res == -1) {
			WDUG_LOGE("Failed to ignore vconf key callback for wifi state\n");
		}

		res = vconf_ignore_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE, _disable_hotspot_state_cb);
		if (res == -1) {
			WDUG_LOGE("Failed to ignore vconf key callback for hotspot state\n");
		}

		res = wifi_direct_start_discovery(FALSE, MAX_SCAN_TIME_OUT);
		if (res != WIFI_DIRECT_ERROR_NONE) {
			WDUG_LOGE("Failed to start discovery. [%d]\n", res);
			ugd->is_re_discover = TRUE;
			wifi_direct_cancel_discovery();
		} else {
			WDUG_LOGI("Discovery is started\n");
			ugd->is_re_discover = FALSE;
		}

		break;
	case WIFI_DIRECT_DEVICE_STATE_DEACTIVATED:
		WDUG_LOGI("WIFI_DIRECT_DEVICE_STATE_DEACTIVATED\n");
		if (error_code != WIFI_DIRECT_ERROR_NONE) {
			WDUG_LOGE("Error in Activation/Deactivation [%d]\n", error_code);
			wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_DEACTIVATE_FAIL"), POPUP_TYPE_DEACTIVATE_FAIL);
			ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
			ugd->wfd_onoff = 1;
			wfd_ug_view_refresh_glitem(ugd->head);
			return;
		}

		ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
		ugd->wfd_onoff = 0;

		/*
		* when deactivated, clear all the
		*  discovered peers and connected peers
		*/
		if (ugd->raw_discovered_peer_cnt > 0) {
			memset(ugd->raw_discovered_peers, 0x00, ugd->raw_discovered_peer_cnt*sizeof(device_type_s));
		}

		if (ugd->raw_connected_peer_cnt > 0) {
			memset(ugd->raw_connected_peers, 0x00, ugd->raw_connected_peer_cnt*sizeof(device_type_s));
		}

		ugd->raw_discovered_peer_cnt = 0;
		ugd->raw_connected_peer_cnt = 0;

		wfd_ug_view_update_peers(ugd);

		/* remove the callback for hotspot */
		res = vconf_ignore_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE, _enable_hotspot_state_cb);
		if (res == -1) {
			WDUG_LOGE("Failed to ignore vconf key callback for hotspot state\n");
		}

		/* when deactivated, stop the timer */
		if (ugd->monitor_timer) {
			ecore_timer_del(ugd->monitor_timer);
			ugd->monitor_timer = NULL;
		}
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

	if (ugd->back_btn) {
		elm_object_disabled_set(ugd->back_btn, FALSE);
	}

	__WDUG_LOG_FUNC_EXIT__;
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
	__WDUG_LOG_FUNC_ENTER__;
	if (NULL == peer || NULL == user_data) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		__WDUG_LOG_FUNC_EXIT__;
		return FALSE;
	}

	struct ug_data *ugd = (struct ug_data *)user_data;
	int peer_cnt = ugd->raw_discovered_peer_cnt;

	WDUG_LOGI("%dth discovered peer. [%s] [%s]\n", peer_cnt, peer->device_name, peer->mac_address);

	strncpy(ugd->raw_discovered_peers[peer_cnt].ssid, peer->device_name, sizeof(ugd->raw_discovered_peers[peer_cnt].ssid));
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

	WDUG_LOGI("\tSSID: [%s]\n", ugd->raw_discovered_peers[peer_cnt].ssid);
	WDUG_LOGI("\tPeer category [%d] -> [%d]\n", peer->primary_device_type, ugd->raw_discovered_peers[peer_cnt].category);
	WDUG_LOGI("\tStatus: [%d]\n", ugd->raw_discovered_peers[peer_cnt].conn_status);

	ugd->raw_discovered_peer_cnt++;

	free(peer->device_name);
	free(peer->mac_address);
	free(peer->interface_address);
	free(peer);

	__WDUG_LOG_FUNC_EXIT__;
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
	__WDUG_LOG_FUNC_ENTER__;
	if (NULL == peer || NULL == user_data) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		__WDUG_LOG_FUNC_EXIT__;
		return FALSE;
	}

	struct ug_data *ugd = (struct ug_data *)user_data;
	int peer_cnt = ugd->raw_connected_peer_cnt;

	WDUG_LOGI("%dth connected peer. [%s] [%s]\n", peer_cnt, peer->device_name, peer->mac_address);

	strncpy(ugd->raw_connected_peers[peer_cnt].ssid, peer->device_name, SSID_LENGTH);
	ugd->raw_connected_peers[peer_cnt].ssid[SSID_LENGTH-1] = '\0';
	ugd->raw_connected_peers[peer_cnt].category = peer->primary_device_type;
	strncpy(ugd->raw_connected_peers[peer_cnt].mac_addr, peer->mac_address, MAC_LENGTH);
	ugd->raw_connected_peers[peer_cnt].mac_addr[MAC_LENGTH-1] = '\0';
	strncpy(ugd->raw_connected_peers[peer_cnt].if_addr, peer->interface_address, MAC_LENGTH);
	ugd->raw_connected_peers[peer_cnt].if_addr[MAC_LENGTH-1] = '\0';
	ugd->raw_connected_peers[peer_cnt].conn_status = PEER_CONN_STATUS_CONNECTED;

	WDUG_LOGI("\tStatus: [%d]\n", ugd->raw_connected_peers[peer_cnt].conn_status);
	WDUG_LOGI("\tCategory: [%d]\n", ugd->raw_connected_peers[peer_cnt].category);
	WDUG_LOGI("\tSSID: [%s]\n", ugd->raw_connected_peers[peer_cnt].ssid);
	WDUG_LOGI("\tMAC addr: [" MACSTR "]\n", ugd->raw_connected_peers[peer_cnt].mac_addr);
	WDUG_LOGI("\tIface addr: [" MACSTR "]\n", ugd->raw_connected_peers[peer_cnt].if_addr);

	ugd->raw_connected_peer_cnt++;

	free(peer->device_name);
	free(peer->mac_address);
	free(peer->interface_address);
	free(peer);

	__WDUG_LOG_FUNC_EXIT__;
	return TRUE;
}

/**
 *	This function let the ug get the found peers
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int wfd_ug_get_discovered_peers(struct ug_data *ugd)
{
	__WDUG_LOG_FUNC_ENTER__;
	int res = 0;

	if (ugd == NULL) {
		return -1;
	}

	ugd->raw_discovered_peer_cnt = 0;
	res = wifi_direct_foreach_discovered_peers(_wfd_discoverd_peer_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		ugd->raw_discovered_peer_cnt = 0;
		WDUG_LOGE("Get discovery result failed: %d\n", res);
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug get the connected peers
 *	@return   If success, return 0, else return -1
 *	@param[in] ugd the pointer to the main data structure
 */
int wfd_ug_get_connected_peers(struct ug_data *ugd)
{
	__WDUG_LOG_FUNC_ENTER__;
	int res = 0;

	if (ugd == NULL) {
		return -1;
	}

	ugd->raw_connected_peer_cnt = 0;
	res = wifi_direct_foreach_connected_peers(_wfd_connected_peer_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		ugd->raw_connected_peer_cnt = 0;
		WDUG_LOGE("Get connected peer failed: %d\n", res);
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug make a callback for deactivating wfd automatically
 *	@return   if stop the timer, return ECORE_CALLBACK_CANCEL, else return ECORE_CALLBACK_RENEW
 *	@param[in] user_data the pointer to the main data structure
 */
static Eina_Bool _wfd_automatic_deactivated_for_no_connection_cb(void *user_data)
{
	int res = -1;
	int interval = 0;
	struct ug_data *ugd = (struct ug_data *)user_data;

	if (NULL == ugd) {
		WDUG_LOGE("NULL parameters.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	/* check the action, if action is exist, keep the cb */
	res = wifi_direct_get_state(&ugd->wfd_status);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to get link status. [%d]\n", res);
		return ECORE_CALLBACK_CANCEL;
	}

	if (ugd->last_wfd_status != ugd->wfd_status) {
		WDUG_LOGE("Action is exist, last status: %d\n",
			ugd->last_wfd_status);
		ugd->last_wfd_status = ugd->wfd_status;
		ugd->last_wfd_time = time(NULL);
		return ECORE_CALLBACK_RENEW;
	}

	/* check the timeout, if not timeout, keep the cb */
	interval = time(NULL) - ugd->last_wfd_time;
	if (interval < MAX_NO_ACTION_TIME_OUT) {
		return ECORE_CALLBACK_RENEW;
	}

	/* turn off the Wi-Fi Direct */
	wifi_direct_get_state(&ugd->wfd_status);
	if (ugd->wfd_status < WIFI_DIRECT_STATE_ACTIVATING) {
		WDUG_LOGE("Wi-Fi Direct is already deactivated\n");
	} else {
		wfd_ug_warn_popup(ugd, IDS_WFD_POP_AUTOMATIC_TURN_OFF, POP_TYPE_AUTOMATIC_TURN_OFF);
	}

	return ECORE_CALLBACK_CANCEL;
}

/**
 *	This function let the ug make a callback for registering discover event
 *	@return   void
 *	@param[in] error_code the returned error code
 *	@param[in] discovery_state the state of discover
 *	@param[in] user_data the pointer to the main data structure
 */
void _discover_cb(int error_code, wifi_direct_discovery_state_e discovery_state, void *user_data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)user_data;

	if (ugd == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return;
	}

	WDUG_LOGI("Discovery event [%d], error_code [%d]\n", discovery_state, error_code);

	if (discovery_state == WIFI_DIRECT_ONLY_LISTEN_STARTED) {
		__WDUG_LOG_FUNC_EXIT__;
		return;
	} else if (discovery_state == WIFI_DIRECT_DISCOVERY_STARTED) {
		ugd->head_text_mode = HEAD_TEXT_TYPE_SCANING;

		/* clear all the previous discovered peers */
		if (ugd->raw_discovered_peer_cnt > 0) {
			memset(ugd->raw_discovered_peers, 0x00, ugd->raw_discovered_peer_cnt*sizeof(device_type_s));
		}

		ugd->raw_discovered_peer_cnt = 0;
	} else if (discovery_state == WIFI_DIRECT_DISCOVERY_FOUND) {
		ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
		wfd_ug_get_discovered_peers(ugd);
	} else {
		ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;

		if (TRUE == ugd->is_re_discover) {
			ugd->is_re_discover = FALSE;
			wifi_direct_start_discovery(FALSE, MAX_SCAN_TIME_OUT);
		} else {
			/* start LISTEN ONLY mode */
			wifi_direct_start_discovery(TRUE, 0);
		}
	}

	wfd_ug_view_refresh_glitem(ugd->head);
	wfd_ug_view_update_peers(ugd);
	wfd_update_multiconnect_device(ugd);

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

	__WDUG_LOG_FUNC_EXIT__;
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
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)user_data;
	device_type_s *peer = NULL;
	bool owner = FALSE;
	int res = 0;

	WDUG_LOGI("Connection event [%d], error_code [%d], multi_connect_mode [%d]\n",
		connection_state, error_code, ugd->multi_connect_mode);

	if (mac_address == NULL) {
		WDUG_LOGE("Incorrect parameter(peer mac is NULL)\n");
		return;
	}

	/* when not in connection, mac_address is empty */
	if (connection_state <= WIFI_DIRECT_DISASSOCIATION_IND) {
		peer = wfd_client_find_peer_by_mac(ugd, mac_address);

		if (NULL == peer || '\0' == peer->ssid[0]) {
			WDUG_LOGE("invalid peer from connection !!\n");
			goto refresh_button;
		}
	}

	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS) {
		switch (connection_state) {
		case WIFI_DIRECT_CONNECTION_RSP:
			WDUG_LOGI("MULTI: WIFI_DIRECT_CONNECTION_RSP\n");

			if (error_code == WIFI_DIRECT_ERROR_NONE) {
				peer->conn_status = PEER_CONN_STATUS_CONNECTED;
				wfd_ug_get_connected_peers(ugd);
			} else {
				peer->conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
			}

			wfd_ug_view_update_peers(ugd);

			/* connect the next peer */
			ugd->g_source_multi_connect_next = g_timeout_add(1000, wfd_multi_connect_next_cb, ugd);
			break;
		case WIFI_DIRECT_CONNECTION_IN_PROGRESS:
			WDUG_LOGI("MULTI: WIFI_DIRECT_CONNECTION_IN_PROGRESS\n");
			peer->conn_status = PEER_CONN_STATUS_CONNECTING;
			wfd_ug_view_update_peers(ugd);
			break;
		case WIFI_DIRECT_GROUP_CREATED:
			WDUG_LOGI("MULTI: WIFI_DIRECT_GROUP_CREATED\n");
			wfd_multi_connect_next_cb(ugd);
			break;
		default:
			break;
		}
	} else {
		switch (connection_state) {
		case WIFI_DIRECT_CONNECTION_RSP:
			WDUG_LOGI("WIFI_DIRECT_CONNECTION_RSP\n");

			if (error_code == WIFI_DIRECT_ERROR_NONE) {
				peer->conn_status = PEER_CONN_STATUS_CONNECTED;
				wfd_ug_get_connected_peers(ugd);
			} else {
				peer->conn_status = PEER_CONN_STATUS_FAILED_TO_CONNECT;
			}

			wfd_ug_view_update_peers(ugd);
			break;
		case WIFI_DIRECT_DISASSOCIATION_IND:
			WDUG_LOGI("WIFI_DIRECT_DISASSOCIATION_IND\n");
			/* change the multi connection mode, it can be connected now */
			if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_COMPLETED) {
				ugd->multi_connect_mode = WFD_MULTI_CONNECT_MODE_IN_PROGRESS;
			}

			/* if other peer disconnected, get connected peers and update */
			peer->conn_status = PEER_CONN_STATUS_WAIT_FOR_CONNECT;
			wfd_ug_get_connected_peers(ugd);
			wfd_ug_view_update_peers(ugd);
			break;
		case WIFI_DIRECT_DISCONNECTION_RSP:
		case WIFI_DIRECT_DISCONNECTION_IND:
			WDUG_LOGI("WIFI_DIRECT_DISCONNECTION_X\n");

			/* when disconnection, clear all the connected peers */
			if (ugd->raw_connected_peer_cnt > 0) {
				memset(ugd->raw_connected_peers, 0x00, ugd->raw_connected_peer_cnt*sizeof(device_type_s));
			}

			ugd->raw_connected_peer_cnt = 0;

			/* start discovery again */
			res = wifi_direct_start_discovery(FALSE, MAX_SCAN_TIME_OUT);
			if (res != WIFI_DIRECT_ERROR_NONE) {
				WDUG_LOGE("Failed to start discovery. [%d]\n", res);
				ugd->is_re_discover = TRUE;
				wifi_direct_cancel_discovery();
			} else {
				WDUG_LOGI("Discovery is started\n");
				ugd->is_re_discover = FALSE;
			}

			break;
		case WIFI_DIRECT_CONNECTION_IN_PROGRESS:
			WDUG_LOGI("WIFI_DIRECT_CONNECTION_IN_PROGRESS\n");
			peer->conn_status = PEER_CONN_STATUS_CONNECTING;
			wfd_ug_view_update_peers(ugd);
			break;
		case WIFI_DIRECT_CONNECTION_REQ:
		case WIFI_DIRECT_CONNECTION_WPS_REQ:
			WDUG_LOGI("WIFI_DIRECT_CLI_EVENT_CONNECTION_REQ\n");
			break;
		default:
			break;
		}
	}

	if (peer != NULL) {
		wfd_ug_view_refresh_glitem(peer->gl_item);
	}

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
		    WDUG_LOGE("Failed to get whether client is group owner. [%d]\n", res);
		}
	} else {
		if (ugd->scan_btn) {
			wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"), TRUE);
		}

		if (ugd->multi_connect_btn) {
			wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"), TRUE);
		}
	}

	/* if no connection, start the monitor timer */
	wifi_direct_get_state(&ugd->wfd_status);
	WDUG_LOGI("status: %d", ugd->wfd_status);

	if (ugd->wfd_status >= WIFI_DIRECT_STATE_CONNECTED) {
		if (ugd->monitor_timer) {
			ecore_timer_del(ugd->monitor_timer);
			ugd->monitor_timer = NULL;
		}
	} else {
		if (NULL == ugd->monitor_timer) {
			WDUG_LOGI("start the monitor timer\n");
			ugd->last_wfd_time = time(NULL);
			ugd->monitor_timer = ecore_timer_add(5.0,
				(Ecore_Task_Cb)_wfd_automatic_deactivated_for_no_connection_cb, ugd);
		}
	}

	__WDUG_LOG_FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug get wi-fi direct status from vconf
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_get_vconf_status(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	char *dev_name;
	int wifi_direct_state = 0;

	/* get wifi direct status from vconf */
	if (vconf_get_int(VCONFKEY_WIFI_DIRECT_STATE, &wifi_direct_state) < 0) {
		WDUG_LOGE("Error reading vconf (%s)\n", VCONFKEY_WIFI_DIRECT_STATE);
		return -1;
	}

	ugd->wfd_status = wifi_direct_state;

	/* get device name from vconf */
	dev_name = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);
	if (dev_name == NULL) {
		ugd->dev_name = strdup(DEFAULT_DEV_NAME);
		WDUG_LOGE("The AP name is NULL(setting default value)\n");
	} else {
		ugd->dev_name = strdup(dev_name);
		free(dev_name);
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug refresh current status of wi-fi direct
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_refresh_wifi_direct_state(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;
	wifi_direct_state_e wfd_status;

	res = wifi_direct_get_state(&wfd_status);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to get link status. [%d]\n", res);
		return -1;
	}

	WDUG_LOGI("WFD status [%d]", wfd_status);
	ugd->wfd_status = wfd_status;

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug do initialization
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int init_wfd_client(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res = 0;

	res = wifi_direct_initialize();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to initialize wifi direct. [%d]\n", res);
		return -1;
	}

	res = wifi_direct_initialize();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to initialize Wi-Fi Direct. error code = [%d]\n", res);
		return -1;
	}

	res = wifi_direct_set_device_state_changed_cb(_activation_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to register _cb_activation. error code = [%d]\n", res);
		return -1;
	}

	res = wifi_direct_set_discovery_state_changed_cb(_discover_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to register _cb_discover. error code = [%d]\n", res);
		return -1;
	}

	res = wifi_direct_set_connection_state_changed_cb(_connection_cb, (void *)ugd);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to register _cb_connection. error code = [%d]\n", res);
		return -1;
	}

	/* update WFD status */
	wfd_refresh_wifi_direct_state(ugd);
	if (ugd->wfd_status > WIFI_DIRECT_STATE_ACTIVATING) {
		ugd->wfd_onoff = 1;
	} else {
		ugd->wfd_onoff = 0;
	}

	WDUG_LOGI("WFD link status. [%d]\n", ugd->wfd_status);

	/* start the monitor timer */
	ugd->last_wfd_time = time(NULL);
	ugd->last_wfd_status = WIFI_DIRECT_STATE_DEACTIVATED;
	ugd->monitor_timer = ecore_timer_add(5.0, (Ecore_Task_Cb)_wfd_automatic_deactivated_for_no_connection_cb, ugd);

	ugd->is_re_discover = FALSE;

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug do de-initialization
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int deinit_wfd_client(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res = 0;
	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h th = NULL;

	wfd_refresh_wifi_direct_state(ugd);

	if (ugd->wfd_status == WIFI_DIRECT_STATE_DISCOVERING) {
		WDUG_LOGI("Stop discovery before deregister client\n");
		wifi_direct_cancel_discovery();
	}

	res = wifi_direct_deinitialize();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to deregister client. [%d]\n", res);
	}

	/* release monitor timer */
	if (ugd->monitor_timer) {
		ecore_timer_del(ugd->monitor_timer);
		ugd->monitor_timer = NULL;
	}

	/* release vconf, hotspot..  */
	res = vconf_ignore_key_changed(VCONFKEY_WIFI_STATE, _wifi_state_cb);
	if (res == -1) {
		WDUG_LOGE("Failed to ignore vconf key callback for wifi state\n");
	}

	res = net_deregister_client();
	if (res != NET_ERR_NONE) {
		WDUG_LOGE("Failed to deregister network client. [%d]\n", res);
	}

	res = vconf_ignore_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE, _enable_hotspot_state_cb);
	if (res == -1) {
		WDUG_LOGE("Failed to ignore vconf key callback for hotspot state\n");
	}

	res = vconf_ignore_key_changed(VCONFKEY_MOBILE_HOTSPOT_MODE, _disable_hotspot_state_cb);
	if (res == -1) {
		WDUG_LOGE("Failed to ignore vconf key callback for hotspot state\n");
	}

	th = ugd->hotspot_handle;

	if (th != NULL) {
		/* Deregister cbs */
		ret = tethering_unset_enabled_cb(th, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			WDUG_LOGE("tethering_unset_enabled_cb is failed(%d)\n", ret);
		}

		ret = tethering_unset_disabled_cb(th, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			WDUG_LOGE("tethering_unset_disabled_cb is failed(%d)\n", ret);
		}

		/* Destroy tethering handle */
		ret = tethering_destroy(th);
		if (ret != TETHERING_ERROR_NONE) {
			WDUG_LOGE("tethering_destroy is failed(%d)\n", ret);
		}

		ugd->hotspot_handle = NULL;
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug turn wi-fi direct on
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_client_switch_on(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;

	wfd_refresh_wifi_direct_state(ugd);
	WDUG_LOGI("WFD status [%d]\n", ugd->wfd_status);

	if (ugd->wfd_status < WIFI_DIRECT_STATE_ACTIVATING) {
		int wifi_state;
		res = vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);
		if (res != 0) {
			WDUG_LOGE("Failed to get wifi state from vconf. [%d]\n", res);
			return -1;
		}

		int hotspot_mode;
		res = vconf_get_int(VCONFKEY_MOBILE_HOTSPOT_MODE, &hotspot_mode);
		if (res != 0) {
			WDUG_LOGE("Failed to get mobile hotspot state from vconf. [%d]\n", res);
			return -1;
		}

		if (wifi_state > VCONFKEY_WIFI_OFF) {
			WDUG_LOGI("WiFi is connected, so have to turn off WiFi");
			wfd_ug_act_popup(ugd, _("IDS_WFD_POP_WIFI_OFF"), POPUP_TYPE_WIFI_OFF);
		} else if (hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI) {
			WDUG_LOGI("WiFi is connected, so have to turn off WiFi");
			wfd_ug_act_popup(ugd, _("IDS_WFD_POP_HOTSPOT_OFF"), POPUP_TYPE_HOTSPOT_OFF);
		} else {
			res = wifi_direct_activate();
			if (res != WIFI_DIRECT_ERROR_NONE) {
				WDUG_LOGE("Failed to activate Wi-Fi Direct. error code = [%d]\n", res);
				wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_ACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

				ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
				wfd_ug_view_refresh_glitem(ugd->head);
				return -1;
			}

			/* refresh the header */
			ugd->head_text_mode = HEAD_TEXT_TYPE_ACTIVATING;
			wfd_ug_view_refresh_glitem(ugd->head);

			/* while activating, disable the buttons */
			if (ugd->scan_btn) {
				wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
			}

			if (ugd->multi_scan_btn) {
				wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
			}

			if (ugd->back_btn) {
				elm_object_disabled_set(ugd->back_btn, TRUE);
			}
		}
	} else {
		WDUG_LOGI("Wi-Fi Direct is already activated\n");
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug turn wi-fi direct off
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_client_switch_off(void *data)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;

	wfd_refresh_wifi_direct_state(ugd);
	WDUG_LOGI("WFD status [%d]\n", ugd->wfd_status);

	if (ugd->wfd_status < WIFI_DIRECT_STATE_ACTIVATING) {
		WDUG_LOGI("Wi-Fi Direct is already deactivated\n");
	} else {
		/*if connected, disconnect all devices*/
		if (WIFI_DIRECT_STATE_CONNECTED == ugd->wfd_status) {
			res = wifi_direct_disconnect_all();
			if (res != WIFI_DIRECT_ERROR_NONE) {
				WDUG_LOGE("Failed to send disconnection request to all. [%d]\n", res);
				return -1;
			}
		}

		res = wifi_direct_deactivate();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			WDUG_LOGE("Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
			wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_DEACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

			ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
			wfd_ug_view_refresh_glitem(ugd->head);
			return -1;
		}

		/* refresh the header */
		ugd->head_text_mode = HEAD_TEXT_TYPE_DEACTIVATING;
		wfd_ug_view_refresh_glitem(ugd->head);

		/* while deactivating, disable the buttons */
		if (ugd->scan_btn) {
			wfd_ug_view_refresh_button(ugd->scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
		}

		if (ugd->multi_scan_btn) {
			wfd_ug_view_refresh_button(ugd->multi_scan_btn, _("IDS_WFD_BUTTON_SCAN"), FALSE);
		}

		if (ugd->back_btn) {
			elm_object_disabled_set(ugd->back_btn, TRUE);
		}
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug turn wi-fi direct on/off forcely
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
  *	@param[in] onoff whether to turn on/off wi-fi direct
 */
int wfd_client_swtch_force(void *data, int onoff)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	int res;

	if (onoff) {
		res = wifi_direct_activate();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			WDUG_LOGE("Failed to activate Wi-Fi Direct. error code = [%d]\n", res);
			wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_ACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

			ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
			wfd_ug_view_refresh_glitem(ugd->head);
			return -1;
		}
	} else {
		res = wifi_direct_deactivate();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			WDUG_LOGE("Failed to deactivate Wi-Fi Direct. error code = [%d]\n", res);
			wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_DEACTIVATE_FAIL"), POPUP_TYPE_TERMINATE);

			ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
			wfd_ug_view_refresh_glitem(ugd->head);
			return -1;
		}
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug create a group
 *	@return   If success, return 0, else return -1
 */
int wfd_client_group_add()
{
	__WDUG_LOG_FUNC_ENTER__;
	int res;

	res = wifi_direct_create_group();
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to add group");
		__WDUG_LOG_FUNC_EXIT__;
		return -1;
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug connect to the device by mac address
 *	@return   If success, return 0, else return -1
 *	@param[in] mac_addr the pointer to the mac address of device
 */
int wfd_client_connect(const char *mac_addr)
{
	__WDUG_LOG_FUNC_ENTER__;
	int res;

	WDUG_LOGE("connect to peer=[%s]\n", mac_addr);
	res = wifi_direct_connect(mac_addr);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to send connection request. [%d]\n", res);
		return -1;
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug disconnect to the device by mac address
 *	@return   If success, return 0, else return -1
 *	@param[in] mac_addr the pointer to the mac address of device
 */
int wfd_client_disconnect(const char *mac_addr)
{
	__WDUG_LOG_FUNC_ENTER__;
	int res;

	if (mac_addr == NULL) {
		res = wifi_direct_disconnect_all();
		if (res != WIFI_DIRECT_ERROR_NONE) {
			WDUG_LOGE("Failed to send disconnection request to all. [%d]\n", res);
			return -1;
		}
	} else {
		res = wifi_direct_disconnect(mac_addr);
		if (res != WIFI_DIRECT_ERROR_NONE) {
			WDUG_LOGE("Failed to send disconnection request. [%d]\n", res);
			return -1;
		}
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}

/**
 *	This function let the ug set the intent of a group owner
 *	@return   If success, return 0, else return -1
 *	@param[in] go_intent the intent parameter
 */
int wfd_client_set_p2p_group_owner_intent(int go_intent)
{
	__WDUG_LOG_FUNC_ENTER__;
	int res;

	res = wifi_direct_set_group_owner_intent(go_intent);
	if (res != WIFI_DIRECT_ERROR_NONE) {
		WDUG_LOGE("Failed to wifi_direct_set_go_intent(%d). [%d]\n", go_intent, res);
		return -1;
	}

	__WDUG_LOG_FUNC_EXIT__;
	return 0;
}
