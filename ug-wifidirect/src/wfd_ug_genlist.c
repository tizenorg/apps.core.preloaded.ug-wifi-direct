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

#include <libintl.h>

#include <assert.h>
#include <glib.h>

#include <Elementary.h>
#include <vconf.h>
#include <ui-gadget-module.h>
#include <wifi-direct.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

Elm_Gen_Item_Class head_itc;
Elm_Gen_Item_Class name_itc;
Elm_Gen_Item_Class title_itc;
Elm_Gen_Item_Class peer_itc;
Elm_Gen_Item_Class noitem_itc;
Elm_Gen_Item_Class button_itc;

Elm_Gen_Item_Class title_conn_itc;
Elm_Gen_Item_Class peer_conn_itc;

Elm_Gen_Item_Class title_busy_itc;
Elm_Gen_Item_Class peer_busy_itc;

Elm_Gen_Item_Class title_multi_connect_itc;
Elm_Gen_Item_Class peer_multi_connect_itc;

Elm_Gen_Item_Class title_conn_failed_itc;
Elm_Gen_Item_Class peer_conn_failed_itc;

/**
 *	This function let the ug get the label of header
 *	@return   the label of header
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_header_label_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	WDUG_LOGI("%s", part);

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text.1")) {
		WDUG_LOGI("Current text mode [%d]\n", ugd->head_text_mode);
		switch (ugd->head_text_mode) {
		case HEAD_TEXT_TYPE_DIRECT:
		case HEAD_TEXT_TYPE_ACTIVATED:
		case HEAD_TEXT_TYPE_SCANING:
			return strdup(dgettext("sys_string", "IDS_COM_OPT1_WI_FI_DIRECT"));
			break;
		case HEAD_TEXT_TYPE_DEACTIVATING:
			return strdup(_("IDS_WFD_BODY_DEACTIVATING"));
			break;
		case HEAD_TEXT_TYPE_ACTIVATING:
			return strdup(_("IDS_WFD_BODY_ACTIVATING"));
			break;
		default:
			break;
		}
	} else if (!strcmp(part, "elm.text.1")) {
		return strdup(dgettext("sys_string", "IDS_COM_OPT1_WI_FI_DIRECT"));
	} else if (!strcmp(part, "elm.text.2")) {
		return strdup(ugd->dev_name);
	}

	__WDUG_LOG_FUNC_EXIT__;
	return NULL;
}

/**
 *  This function let the ug call it when click header
 *  @return   void
 *  @param[in] data the pointer to the main data structure
 *  @param[in] obj the pointer to the evas object
 *  @param[in] event_info the pointer to the event information
 */
static void _gl_header_sel(void *data, Evas_Object *obj, void *event_info)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	if (item != NULL)
		elm_genlist_item_selected_set(item, EINA_FALSE);

	if (ugd == NULL)
		WDUG_LOGE("Incorrect parameter(NULL)\n");
	else {
		if(!ugd->wfd_onoff) {
			WDUG_LOGD("Wi-Fi direct switch on\n");
			wfd_client_switch_on(ugd);
		} else {
			WDUG_LOGD("Wi-Fi direct switch off\n");
			wfd_client_switch_off(ugd);
		}
	}
	__WDUG_LOG_FUNC_EXIT__;
}

/**
 *	This function let the ug get the icon of header
 *	@return   the icon of header
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static Evas_Object *_gl_header_icon_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	Evas_Object *onoff = NULL;

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (ugd->head_text_mode == HEAD_TEXT_TYPE_ACTIVATING ||
		ugd->head_text_mode == HEAD_TEXT_TYPE_DEACTIVATING) {
		return NULL;
	}

	if (!strcmp(part, "elm.icon")) {
		onoff = elm_check_add(obj);
		elm_object_style_set(onoff, "on&off");
		elm_check_state_set(onoff, ugd->wfd_onoff);
		evas_object_smart_callback_add(onoff, "changed", _gl_header_sel , ugd);
		evas_object_show(onoff);
	}

	__WDUG_LOG_FUNC_EXIT__;
	return onoff;
}

/**
 *	This function let the ug get the label of about item
 *	@return   the label of about item
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_name_label_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	WDUG_LOGI("%s", part);

	if (!strcmp(part, "elm.text")) {
		return strdup(IDS_WFD_TITLE_ABOUT_WIFI_DIRECT);
	} else if (!strcmp(part, "elm.text.2")) {
		return strdup(ugd->dev_name);
	}

	__WDUG_LOG_FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the ug get the label of titile
 *	@return   the label of titile
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_title_label_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text")) {
		if (ugd->multiconn_view_genlist != NULL) {
			// It's called at Multi connect view...
			if (ugd->gl_available_dev_cnt_at_multiconn_view > 0) {
				return strdup(_("IDS_WFD_BODY_AVAILABLE_DEVICES"));
			} else {
				return strdup(_("IDS_WFD_BODY_WIFI_DIRECT_DEVICES"));
			}
		} else {
			// It's called at Main View
			if (ugd->gl_available_peer_cnt > 0) {
				return strdup(_("IDS_WFD_BODY_AVAILABLE_DEVICES"));
			} else {
				return strdup(_("IDS_WFD_BODY_WIFI_DIRECT_DEVICES"));
			}
		}
	}

	__WDUG_LOG_FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the ug get the content of titile
 *	@return   the content of titile
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static Evas_Object *_gl_title_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *progressbar = NULL;
	struct ug_data *ugd = (struct ug_data *) data;

	if (data == NULL) {
	    WDUG_LOGE("Incorrect parameter(NULL)\n");
	    return NULL;
	}

	if (!strcmp(part, "elm.icon")) {
		if (HEAD_TEXT_TYPE_SCANING == ugd->head_text_mode) {
			progressbar = elm_progressbar_add(obj);
			elm_object_style_set(progressbar, "list_process_small");
			elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
			elm_progressbar_pulse(progressbar, EINA_TRUE);
			evas_object_show(progressbar);
		} else {
			return NULL;
		}
	}

	return progressbar;
}

/**
 *	This function let the ug get the label of peer item
 *	@return   the label of peer item
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_peer_label_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
	WDUG_LOGI("%s", part);

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text.1")) {
		__WDUG_LOG_FUNC_EXIT__;
		return strdup(peer->ssid);
	} else if (!strcmp(part, "elm.text.2")) {
		switch (peer->conn_status) {
		case PEER_CONN_STATUS_DISCONNECTED:
			g_strlcpy(buf, _("IDS_WFD_TAP_TO_CONNECT"), WFD_GLOBALIZATION_STR_LENGTH);
			break;
		case PEER_CONN_STATUS_CONNECTING:
			g_strlcpy(buf, _("IDS_WFD_CONNECTING"), WFD_GLOBALIZATION_STR_LENGTH);
			break;
		case PEER_CONN_STATUS_CONNECTED:
			if (peer->is_group_owner == FALSE) {
				g_strlcpy(buf, _("IDS_WFD_CONNECTED"), WFD_GLOBALIZATION_STR_LENGTH);
			} else {
				g_strlcpy(buf, _("IDS_WFD_TAP_TO_CONNECT"), WFD_GLOBALIZATION_STR_LENGTH);
			}
			break;
		case PEER_CONN_STATUS_FAILED_TO_CONNECT:
			g_strlcpy(buf, _("IDS_WFD_FAILED_TO_CONNECT"), WFD_GLOBALIZATION_STR_LENGTH);
			break;
		case PEER_CONN_STATUS_WAIT_FOR_CONNECT:
			g_strlcpy(buf, _("IDS_WFD_WAITING_FOR_CONNECT"), WFD_GLOBALIZATION_STR_LENGTH);
			break;
		default:
			g_strlcpy(buf, _("IDS_WFD_TAP_TO_CONNECT"), WFD_GLOBALIZATION_STR_LENGTH);
			break;
		}
	} else {
		__WDUG_LOG_FUNC_EXIT__;
		return NULL;
	}

	__WDUG_LOG_FUNC_EXIT__;
	return strdup(buf);
}

/**
 *	This function let the ug get the icon of peer item
 *	@return   the icon of peer item
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static Evas_Object *_gl_peer_icon_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	Evas_Object *icon = NULL;
	struct ug_data *ugd = wfd_get_ug_data();

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.icon.2")) {
		WDUG_LOGI("elm.icon.2 - connection status [%d]\n", peer->conn_status);
		if (peer->conn_status == PEER_CONN_STATUS_CONNECTING) {
			icon = elm_progressbar_add(obj);
			elm_object_style_set(icon, "list_process");
			elm_progressbar_pulse(icon, EINA_TRUE);
		} else if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
			return NULL;
		}

		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_icon_resizable_set(icon, 1, 1);
		evas_object_show(icon);
	} else if (!strcmp(part, "elm.icon.1")) {
		WDUG_LOGI("elm.icon.1 - category [%d]\n", peer->category);
		char *img_path = NULL;
		int status = -1;

		status = wfd_get_device_status(ugd, peer);

		/*
		* the icon of connected device is
		* different from available and busy device
		*/
		switch (peer->category) {
		case WFD_DEVICE_TYPE_COMPUTER:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_COMPUTER_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_COMPUTER;
			}
			break;
		case WFD_DEVICE_TYPE_INPUT_DEVICE:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_INPUT_DEVICE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_INPUT_DEVICE;
			}
			break;
		case WFD_DEVICE_TYPE_PRINTER:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_PRINTER_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_PRINTER;
			}
			break;
		case WFD_DEVICE_TYPE_CAMERA:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_CAMERA_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_CAMERA;
			}
			break;
		case WFD_DEVICE_TYPE_STORAGE:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_STORAGE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_STORAGE;
			}
			break;
		case WFD_DEVICE_TYPE_NW_INFRA:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_NETWORK_INFRA_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_NETWORK_INFRA;
			}
			break;
		case WFD_DEVICE_TYPE_DISPLAYS:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_DISPLAY_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_DISPLAY;
			}
			break;
		case WFD_DEVICE_TYPE_MM_DEVICES:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_MULTIMEDIA_DEVICE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_MULTIMEDIA_DEVICE;
			}
			break;
		case WFD_DEVICE_TYPE_GAME_DEVICES:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_GAMING_DEVICE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_GAMING_DEVICE;
			}
			break;
		case WFD_DEVICE_TYPE_TELEPHONE:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_TELEPHONE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_TELEPHONE;
			}
			break;
		case WFD_DEVICE_TYPE_AUDIO:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_AUDIO_DEVICE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_AUDIO_DEVICE;
			}
			break;
		default:
			if (1 == status) {
				img_path = WFD_ICON_DEVICE_COMPUTER_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_COMPUTER;
			}
			break;
		}

		if (img_path != NULL) {
			icon = elm_icon_add(obj);
			elm_icon_file_set(icon, img_path, NULL);
			evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			elm_icon_resizable_set(icon, 1, 1);
			evas_object_show(icon);
		} else {
			return NULL;
		}
	}

	__WDUG_LOG_FUNC_EXIT__;
	return icon;
}

/**
 *	This function let the ug get the text of no device item
 *	@return   the text of no device item
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_noitem_text_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	__WDUG_LOG_FUNC_EXIT__;
	return strdup(_("IDS_WFD_NOCONTENT"));
}

/**
 *	This function let the ug get the multi connect button
 *	@return   the multi connect button
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static Evas_Object *_gl_button_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	WDUG_LOGI("%s", part);
	ugd->multi_btn = elm_button_add(obj);
	wfd_refresh_wifi_direct_state(ugd);

	if (ugd->wfd_status == WIFI_DIRECT_STATE_CONNECTING) {
		elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_CANCEL"));
		WDUG_LOGI("button: Cancel connect\n");
	} else {
		if (ugd->gl_connected_peer_cnt > 1) {
			elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_DISCONNECT_ALL"));
			WDUG_LOGI("button: Disconnect All\n");
		} else if (ugd->gl_connected_peer_cnt == 1) {
			elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_DISCONNECT"));
			WDUG_LOGI("button: Disconnect\n");
		} else {
			elm_object_text_set(ugd->multi_btn, _("IDS_WFD_BUTTON_MULTI"));
			WDUG_LOGI("button: Multi connect\n");
		}
	}

	evas_object_smart_callback_add(ugd->multi_btn, "clicked", _wifid_create_multibutton_cb, ugd);
	evas_object_show(ugd->multi_btn);

	__WDUG_LOG_FUNC_EXIT__;
	return ugd->multi_btn;
}

/**
 *	This function let the ug get the title label of connected device list
 *	@return   the title label of connected device list
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_conn_dev_title_label_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text")) {
		return strdup(_("IDS_WFD_BODY_CONNECTED_DEVICES"));
	}

	__WDUG_LOG_FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the ug get the label of connected device
 *	@return   the label of connected device
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_peer_conn_dev_label_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
	WDUG_LOGI("%s", part);

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text.1")) {
		return strdup(peer->ssid);
	} else {
		g_strlcpy(buf, _("IDS_WFD_CONNECTED"), WFD_GLOBALIZATION_STR_LENGTH);
		__WDUG_LOG_FUNC_EXIT__;
		return strdup(buf);
	}
}

/**
 *	This function let the ug get the title label of connected failed device list
 *	@return   the label of connected device
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_conn_failed_dev_title_label_get(void *data, Evas_Object *obj,
		const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text")) {
		return strdup(IDS_WFD_BODY_FAILED_DEVICES);
	}

	__WDUG_LOG_FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the ug get the label of connected failed device
 *	@return   the label of connected device
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_peer_conn_failed_dev_label_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
	WDUG_LOGI("%s", part);

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text.1")) {
		return strdup(peer->ssid);
	} else {
		g_strlcpy(buf, _("IDS_WFD_FAILED_TO_CONNECT"), WFD_GLOBALIZATION_STR_LENGTH);
		__WDUG_LOG_FUNC_EXIT__;
		return strdup(buf);
	}
}

/**
 *	This function let the ug get the title label of multi connect list
 *	@return   the label of connected device
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_multi_connect_dev_title_label_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	struct ug_data *ugd = wfd_get_ug_data();

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text")) {
		if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS) {
			return strdup(_("IDS_WFD_BODY_AVAILABLE_DEVICES"));
		} else if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_COMPLETED) {
			return strdup(IDS_WFD_BODY_FAILED_DEVICES);
		}
	}

	__WDUG_LOG_FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the ug get the title label of busy device list
 *	@return   the label of connected device
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_busy_dev_title_label_get(void *data, Evas_Object *obj,
		const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text")) {
		return strdup(_("IDS_WFD_BODY_BUSY_DEVICES"));
	}

	__WDUG_LOG_FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the ug get the label of busy device
 *	@return   the label of connected device
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_peer_busy_dev_label_get(void *data, Evas_Object *obj, const char *part)
{
	__WDUG_LOG_FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
	WDUG_LOGI("%s", part);

	if (data == NULL) {
		WDUG_LOGE("Incorrect parameter(NULL)\n");
		return NULL;
	}

	WDUG_LOGI("peer->ssid = %s", peer->ssid);

	if (!strcmp(part, "elm.text.1")) {
		return strdup(peer->ssid);
	} else {
		g_strlcpy(buf, _("IDS_WFD_CONNECTED_WITH_OTHER_DEVICE"), WFD_GLOBALIZATION_STR_LENGTH);
		__WDUG_LOG_FUNC_EXIT__;
		return strdup(buf);
	}
}

/**
 *	This function let the ug delete the peer item
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 */
static void _gl_peer_del(void *data, Evas_Object *obj)
{
	__WDUG_LOG_FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == data, "NULL!!");

	__WDUG_LOG_FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug initialize the items of genlist
 *	@return   void
 */
void initialize_gen_item_class()
{
	__WDUG_LOG_FUNC_ENTER__;
	head_itc.item_style = "dialogue/2text.1icon.10";
	head_itc.func.text_get = _gl_header_label_get;
	head_itc.func.content_get = _gl_header_icon_get;
	head_itc.func.state_get = NULL;

	name_itc.item_style = "dialogue/1text";
	name_itc.func.text_get = _gl_name_label_get;
	name_itc.func.content_get = NULL;
	name_itc.func.state_get = NULL;
	name_itc.func.del = NULL;

	title_itc.item_style = "dialogue/title";
	title_itc.func.text_get = _gl_title_label_get;
	title_itc.func.content_get = _gl_title_content_get;
	title_itc.func.state_get = NULL;
	title_itc.func.del = NULL;

	peer_itc.item_style = "dialogue/2text.2icon.3";
	peer_itc.func.text_get = _gl_peer_label_get;
	peer_itc.func.content_get = _gl_peer_icon_get;
	peer_itc.func.state_get = NULL;
	peer_itc.func.del = _gl_peer_del;

	noitem_itc.item_style = "dialogue/1text";
	noitem_itc.func.text_get = _gl_noitem_text_get;
	noitem_itc.func.content_get = NULL;
	noitem_itc.func.state_get = NULL;
	noitem_itc.func.del = NULL;

	button_itc.item_style = "1icon";
	button_itc.func.text_get = NULL;
	button_itc.func.content_get = _gl_button_get;
	button_itc.func.state_get = NULL;
	button_itc.func.del = NULL;

	title_conn_itc.item_style = "dialogue/title";
	title_conn_itc.func.text_get = _gl_conn_dev_title_label_get;
	title_conn_itc.func.content_get = NULL;
	title_conn_itc.func.state_get = NULL;
	title_conn_itc.func.del = NULL;

	peer_conn_itc.item_style = "dialogue/2text.2icon.3";
	peer_conn_itc.func.text_get = _gl_peer_conn_dev_label_get;
	peer_conn_itc.func.content_get = _gl_peer_icon_get;
	peer_conn_itc.func.state_get = NULL;
	peer_conn_itc.func.del = _gl_peer_del;

	title_conn_failed_itc.item_style = "dialogue/title";
	title_conn_failed_itc.func.text_get = _gl_conn_failed_dev_title_label_get;
	title_conn_failed_itc.func.content_get = NULL;
	title_conn_failed_itc.func.state_get = NULL;
	title_conn_failed_itc.func.del = NULL;

	peer_conn_failed_itc.item_style = "dialogue/2text.2icon.3";
	peer_conn_failed_itc.func.text_get = _gl_peer_conn_failed_dev_label_get;
	peer_conn_failed_itc.func.content_get = _gl_peer_icon_get;
	peer_conn_failed_itc.func.state_get = NULL;
	peer_conn_failed_itc.func.del = _gl_peer_del;

	title_busy_itc.item_style = "dialogue/title";
	title_busy_itc.func.text_get = _gl_busy_dev_title_label_get;
	title_busy_itc.func.content_get = NULL;
	title_busy_itc.func.state_get = NULL;
	title_busy_itc.func.del = NULL;

	peer_busy_itc.item_style = "dialogue/2text.2icon.3";
	peer_busy_itc.func.text_get = _gl_peer_busy_dev_label_get;
	peer_busy_itc.func.content_get = _gl_peer_icon_get;
	peer_busy_itc.func.state_get = NULL;
	peer_busy_itc.func.del = _gl_peer_del;

	title_multi_connect_itc.item_style = "dialogue/title";
	title_multi_connect_itc.func.text_get = _gl_multi_connect_dev_title_label_get;
	title_multi_connect_itc.func.content_get = NULL;
	title_multi_connect_itc.func.state_get = NULL;
	title_multi_connect_itc.func.del = NULL;

	__WDUG_LOG_FUNC_EXIT__;

}
