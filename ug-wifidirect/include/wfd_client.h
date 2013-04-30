/*
*  WiFi-Direct UG
*
* Copyright 2012-2013 Samsung Electronics Co., Ltd

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


#ifndef __WFD_CLIENT_H__
#define __WFD_CLIENT_H__


typedef enum {
	WFD_DEVICE_TYPE_COMPUTER = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_COMPUTER,
	WFD_DEVICE_TYPE_INPUT_DEVICE = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_INPUT_DEVICE,
	WFD_DEVICE_TYPE_PRINTER = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_PRINTER,
	WFD_DEVICE_TYPE_CAMERA = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_CAMERA,
	WFD_DEVICE_TYPE_STORAGE = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_STORAGE,
	WFD_DEVICE_TYPE_NW_INFRA = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_NETWORK_INFRA,
	WFD_DEVICE_TYPE_DISPLAYS = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_DISPLAY,
	WFD_DEVICE_TYPE_MM_DEVICES = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_MULTIMEDIA_DEVICE,
	WFD_DEVICE_TYPE_GAME_DEVICES = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_GAME_DEVICE,
	WFD_DEVICE_TYPE_TELEPHONE = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_TELEPHONE,
	WFD_DEVICE_TYPE_AUDIO = WIFI_DIRECT_PRIMARY_DEVICE_TYPE_AUDIO,
	WFD_DEVICE_TYPE_OTHER =  WIFI_DIRECT_PRIMARY_DEVICE_TYPE_OTHER,
} device_type_e;

typedef enum {
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

/**
 *	This function let the ug get wi-fi direct status from vconf
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_get_vconf_status(void *data);

/**
 *	This function let the ug turn wifi off
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_wifi_off(void *data);

/**
 *	This function let the ug turn AP on
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_mobile_ap_on(void *data);

/**
 *	This function let the ug turn AP off
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_mobile_ap_off(void *data);

/**
 *	This function let the ug do initialization
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int init_wfd_client(void *data);

/**
 *	This function let the ug do de-initialization
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int deinit_wfd_client(void *data);

/**
 *	This function let the ug turn wi-fi direct on
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_client_switch_on(void *data);

/**
 *	This function let the ug turn wi-fi direct off
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
 */
int wfd_client_switch_off(void *data);

/**
 *	This function let the ug turn wi-fi direct on/off forcely
 *	@return   If success, return 0, else return -1
 *	@param[in] data the pointer to the main data structure
  *	@param[in] onoff whether to turn on/off wi-fi direct
 */
int wfd_client_swtch_force(void *data, int onoff);

/**
 *	This function let the ug create a group
 *	@return   If success, return 0, else return -1
 */
int wfd_client_group_add();

/**
 *	This function let the ug connect to the device by mac address
 *	@return   If success, return 0, else return -1
 *	@param[in] mac_addr the pointer to the mac address of device
 */
int wfd_client_connect(const char *mac_addr);

/**
 *	This function let the ug disconnect to the device by mac address
 *	@return   If success, return 0, else return -1
 *	@param[in] mac_addr the pointer to the mac address of device
 */
int wfd_client_disconnect(const char *mac_addr);

/**
 *	This function let the ug set the intent of a group owner
 *	@return   If success, return 0, else return -1
 *	@param[in] go_intent the intent parameter
 */
int wfd_client_set_p2p_group_owner_intent(int go_intent);

#endif /* __WFD_CLIENT_H__ */
