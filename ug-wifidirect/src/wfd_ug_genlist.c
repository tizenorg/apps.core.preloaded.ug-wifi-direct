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

#include <libintl.h>
#include <glib.h>

#include <Elementary.h>
#include <vconf.h>
#include <ui-gadget-module.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

Elm_Gen_Item_Class device_name_title_itc;
#ifdef WFD_ON_OFF_GENLIST
Elm_Gen_Item_Class wfd_onoff_itc;
#endif
Elm_Gen_Item_Class device_name_itc;
Elm_Gen_Item_Class title_itc;
Elm_Gen_Item_Class multi_view_title_itc;
Elm_Gen_Item_Class peer_itc;
Elm_Gen_Item_Class title_no_device_itc;
Elm_Gen_Item_Class noitem_itc;
Elm_Gen_Item_Class title_available_itc;

Elm_Gen_Item_Class title_conn_itc;
Elm_Gen_Item_Class peer_conn_itc;

Elm_Gen_Item_Class title_busy_itc;
Elm_Gen_Item_Class peer_busy_itc;

Elm_Gen_Item_Class title_multi_connect_itc;
Elm_Gen_Item_Class peer_multi_connect_itc;
Elm_Gen_Item_Class select_all_multi_connect_itc;

Elm_Gen_Item_Class title_conn_failed_itc;
Elm_Gen_Item_Class peer_conn_failed_itc;

#ifdef WFD_ON_OFF_GENLIST
static char *_gl_wfd_onoff_text_get(void *data, Evas_Object *obj,
		const char *part)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	char *dev_name = NULL;
	DBG(LOG_INFO, "%s", part);
	WFD_RETV_IF(ugd == NULL, NULL, "Incorrect parameter(NULL)\n");
	wfd_get_vconf_device_name(ugd);

	if (!strcmp("elm.text.sub", part)) {
		__FUNC_EXIT__;
		return g_strdup(_("IDS_ST_HEADER_MY_DEVICE_NAME"));
	} else if (!strcmp("elm.text", part)) {
		DBG(LOG_INFO, "%s", ugd->dev_name);
		if (ugd->dev_name) {
			dev_name = elm_entry_utf8_to_markup(ugd->dev_name);
			if (NULL == dev_name) {
				DBG(LOG_ERROR, "elm_entry_utf8_to_markup failed.\n");
			}
		}
	}

	__FUNC_EXIT__;
	return dev_name;
}

static Evas_Object *_gl_wfd_onoff_content_get(void *data,Evas_Object *obj,
		const char *part)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	WFD_RETV_IF(ugd == NULL, NULL, "Incorrect parameter(NULL)\n");
	Evas_Object *btn = NULL;
	Evas_Object *icon = NULL;

	if (!strcmp("elm.swallow.end", part)) {
		icon = elm_layout_add(obj);
		elm_layout_theme_set(icon, "layout", "list/C/type.3", "default");

		/* Wi-Fi on indication button */
		btn= elm_check_add(icon);
		elm_object_style_set(btn, "on&off");
		elm_check_state_set(btn, ugd->wfd_onoff);
		evas_object_propagate_events_set(btn, EINA_FALSE);
		evas_object_smart_callback_add(btn, "changed",_onoff_changed_cb, ugd);
		ugd->on_off_check = btn;
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);
		elm_layout_content_set(icon, "elm.swallow.content", btn);
	}

	__FUNC_EXIT__;
	return icon;
}
#endif

/**
 *	This function let the ug get the label of header
 *	@return   the label of header
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_device_name_label_get(void *data, Evas_Object *obj,
		const char *part)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	DBG(LOG_INFO, "%s", part);
	WFD_RETV_IF(ugd == NULL, NULL, "Incorrect parameter(NULL)\n");
	wfd_get_vconf_device_name(ugd);
	char *dev_name = NULL;
	char str[WFD_GLOBALIZATION_STR_LENGTH] = {0, };
	char buf[WFD_GLOBALIZATION_STR_LENGTH] = {0, };

	if (!strcmp("elm.text.multiline", part)) {
		DBG(LOG_INFO, "%s", ugd->dev_name);
		if (ugd->dev_name) {
			dev_name = elm_entry_utf8_to_markup(ugd->dev_name);
			if (NULL == dev_name) {
				DBG(LOG_ERROR, "elm_entry_utf8_to_markup failed.\n");
				__FUNC_EXIT__;
				return NULL;
			}

			snprintf(str, WFD_GLOBALIZATION_STR_LENGTH,
				_("IDS_WIFI_BODY_YOUR_DEVICE_HPS_IS_CURRENTLY_VISIBLE_TO_NEARBY_DEVICES"),
				dev_name);

			snprintf(buf, WFD_GLOBALIZATION_STR_LENGTH,
				"<font_size=30>%s</font_size>", str);

			WFD_IF_FREE_MEM(dev_name);
			__FUNC_EXIT__;
			return g_strdup(buf);
		}
	}
	__FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the ug get the label of title
 *	@return   the label of titile
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_title_label_get(void *data, Evas_Object *obj, const char *part)
{
	__FUNC_ENTER__;

	WFD_RETV_IF(data == NULL, NULL, "Incorrect parameter(NULL)\n");
	DBG(LOG_INFO, "%s", part);
	if (!strcmp("elm.text", part)) {
		return g_strdup(_("IDS_WIFI_HEADER_AVAILABLE_DEVICES_ABB"));
	}
	__FUNC_EXIT__;
	return NULL;
}


static char *_gl_title_no_device_label_get(void *data, Evas_Object *obj,
		const char *part)
{
	__FUNC_ENTER__;

	WFD_RETV_IF(data == NULL, NULL, "Incorrect parameter(NULL)\n");
	DBG(LOG_INFO, "%s", part);
	if (!strcmp("elm.text", part)) {
		return g_strdup(_("IDS_WIFI_HEADER_AVAILABLE_DEVICES_ABB"));
	}

	__FUNC_EXIT__;
	return NULL;
}

static char *_gl_multi_title_label_get(void *data, Evas_Object *obj, const char *part)
{
	__FUNC_ENTER__;

	WFD_RETV_IF(data == NULL, NULL, "Incorrect parameter(NULL)\n");
	if (!strcmp("elm.text", part)) {
		return g_strdup(_("IDS_WIFI_HEADER_AVAILABLE_DEVICES_ABB"));
	}

	__FUNC_EXIT__;
	return NULL;
}

static Evas_Object *_gl_multi_title_content_get(void *data, Evas_Object *obj, const char *part)
{
	__FUNC_ENTER__;

	Evas_Object *progressbar = NULL;
	struct ug_data *ugd = (struct ug_data *) data;
	WFD_RETV_IF(ugd == NULL, NULL, "Incorrect parameter(NULL)\n");

	if (TITLE_CONTENT_TYPE_SCANNING == ugd->title_content_mode) {
		progressbar = elm_progressbar_add(obj);
		elm_object_style_set(progressbar, "process_small");
		elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
		elm_progressbar_pulse(progressbar, EINA_TRUE);
		evas_object_show(progressbar);
	}

	__FUNC_EXIT__;
	return progressbar;
}

static char *_gl_available_title_label_get(void *data, Evas_Object *obj, const char *part)
{
	__FUNC_ENTER__;

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}
	DBG(LOG_INFO, "available- %s", part);
	if (!strcmp("elm.text", part)) {
		return g_strdup(_("IDS_WIFI_HEADER_AVAILABLE_DEVICES_ABB"));
	}

	__FUNC_EXIT__;
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
	__FUNC_ENTER__;

	Evas_Object *progressbar = NULL;
	struct ug_data *ugd = (struct ug_data *) data;

	if (data == NULL) {
	    DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
	    return NULL;
	}

	DBG(LOG_INFO, "Title content- %s", part);

	if (TITLE_CONTENT_TYPE_SCANNING == ugd->title_content_mode) {
		progressbar = elm_progressbar_add(obj);
		elm_object_style_set(progressbar, "process_small");
		elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
		elm_progressbar_pulse(progressbar, EINA_TRUE);
		evas_object_show(progressbar);
	}

	__FUNC_EXIT__;
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
	__FUNC_ENTER__;

	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
	char *ssid;

	DBG(LOG_INFO, "%s", part);

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp("elm.text", part)) {
		if (strlen(peer->ssid) != 0) {
			ssid = elm_entry_utf8_to_markup(peer->ssid);
			if (NULL == ssid) {
				DBG(LOG_ERROR, "elm_entry_utf8_to_markup failed.\n");
				__FUNC_EXIT__;
				return NULL;
			}
			__FUNC_EXIT__;
			return ssid;
		}
	} else if (!strcmp("elm.text.sub", part)) {
		switch (peer->conn_status) {
		case PEER_CONN_STATUS_DISCONNECTED:
			g_strlcpy(buf, _("IDS_CHATON_BODY_AVAILABLE"), WFD_GLOBALIZATION_STR_LENGTH);
			break;
		case PEER_CONN_STATUS_CONNECTING:
			g_strlcpy(buf, _("IDS_WIFI_BODY_CONNECTING_ING"), WFD_GLOBALIZATION_STR_LENGTH);
			break;
		case PEER_CONN_STATUS_CONNECTED:
			if (peer->is_group_owner == FALSE) {
				g_strlcpy(buf, _("IDS_COM_BODY_CONNECTED_M_STATUS"), WFD_GLOBALIZATION_STR_LENGTH);
			} else {
				g_strlcpy(buf, _("IDS_CHATON_BODY_AVAILABLE"), WFD_GLOBALIZATION_STR_LENGTH);
			}
			break;
		case PEER_CONN_STATUS_FAILED_TO_CONNECT:
			g_strlcpy(buf, _("IDS_CHATON_HEADER_CONNECTION_FAILED_ABB2"), WFD_GLOBALIZATION_STR_LENGTH);
			break;
		case PEER_CONN_STATUS_WAIT_FOR_CONNECT:
			g_strlcpy(buf, _("IDS_WIFI_BODY_WAITING_FOR_CONNECTION_M_STATUS_ABB"), WFD_GLOBALIZATION_STR_LENGTH);
			break;
		default:
			g_strlcpy(buf, _("IDS_CHATON_BODY_AVAILABLE"), WFD_GLOBALIZATION_STR_LENGTH);
			break;
		}
	} else {
		__FUNC_EXIT__;
		return NULL;
	}

	__FUNC_EXIT__;
	return g_strdup(buf);
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
	__FUNC_ENTER__;

	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	Evas_Object *icon = NULL;
	Evas_Object *layout = NULL;

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}
	DBG(LOG_INFO, "part = %s", part);

	if (!strcmp("elm.swallow.icon", part)) {
		DBG(LOG_INFO, "elm.swallow.icon - category [%d]\n", peer->category);
		char *img_name = NULL;
		layout = elm_layout_add(obj);
		elm_layout_theme_set(layout, "layout", "list/B/type.3", "default");
		/*
		* the icon of connected device is
		* different from available and busy device
		*/
		switch (peer->category) {
		case WFD_DEVICE_TYPE_COMPUTER:
			img_name = WFD_ICON_DEVICE_COMPUTER;
			break;
		case WFD_DEVICE_TYPE_INPUT_DEVICE:
			img_name = WFD_ICON_DEVICE_INPUT_DEVICE;
			break;
		case WFD_DEVICE_TYPE_PRINTER:
			img_name = WFD_ICON_DEVICE_PRINTER;
			break;
		case WFD_DEVICE_TYPE_CAMERA:
			img_name = WFD_ICON_DEVICE_CAMERA;
			break;
		case WFD_DEVICE_TYPE_STORAGE:
			img_name = WFD_ICON_DEVICE_STORAGE;
			break;
		case WFD_DEVICE_TYPE_NW_INFRA:
			img_name = WFD_ICON_DEVICE_NETWORK_INFRA;
			break;
		case WFD_DEVICE_TYPE_DISPLAYS:
			img_name = WFD_ICON_DEVICE_DISPLAY;
			break;
		case WFD_DEVICE_TYPE_MM_DEVICES:
			if (peer->sub_category == WIFI_DIRECT_SECONDARY_DEVICE_TYPE_MULTIMEDIA_STB) {
				img_name = WFD_ICON_DEVICE_STB;
			} else if (peer->sub_category == WIFI_DIRECT_SECONDARY_DEVICE_TYPE_MULTIMEDIA_MS_MA_ME) {
				img_name = WFD_ICON_DEVICE_DONGLE;
			} else if (peer->sub_category == WIFI_DIRECT_SECONDARY_DEVICE_TYPE_MULTIMEDIA_PVP) {
				img_name = WFD_ICON_DEVICE_BD;
			} else {
				img_name = WFD_ICON_DEVICE_MULTIMEDIA;
			}
			break;
		case WFD_DEVICE_TYPE_GAME_DEVICES:
			img_name = WFD_ICON_DEVICE_GAMING;
			break;
		case WFD_DEVICE_TYPE_TELEPHONE:
			img_name = WFD_ICON_DEVICE_TELEPHONE;
			break;
		case WFD_DEVICE_TYPE_AUDIO:
			if (peer->sub_category == WIFI_DIRECT_SECONDARY_DEVICE_TYPE_AUDIO_TUNER) {
				img_name = WFD_ICON_DEVICE_HOME_THEATER;
			} else {
				img_name = WFD_ICON_DEVICE_HEADSET;
			}
			break;
		default:
			img_name = WFD_ICON_DEVICE_UNKNOWN;
			break;
		}

		icon = elm_image_add(layout);
		elm_image_file_set(icon, WFD_UG_EDJ_PATH, img_name);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_color_set(icon, 2, 61, 132, 204);
		evas_object_show(icon);
		evas_object_propagate_events_set(icon, EINA_FALSE);
		elm_layout_content_set(layout, "elm.swallow.content", icon);
	} else if (!strcmp("elm.swallow.end", part)) {
		DBG(LOG_INFO, "elm.icon.2 - connection status [%d]\n", peer->conn_status);
		if (peer->conn_status == PEER_CONN_STATUS_CONNECTING) {
			layout = elm_layout_add(obj);
			elm_layout_theme_set(layout, "layout", "list/C/type.2", "default");
			icon = elm_progressbar_add(layout);
			elm_object_style_set(icon, "process_medium");
			elm_progressbar_pulse(icon, EINA_TRUE);
		} else if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
			return NULL;
		}
		evas_object_show(icon);
		elm_layout_content_set(layout, "elm.swallow.content", icon);
	}

	if (layout)
		evas_object_show(layout);

	__FUNC_EXIT__;
	return layout;
}

/**
 *	This function let the ug get the icon of peer item
 *	@return   the icon of peer item
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static Evas_Object *_gl_conn_peer_icon_get(void *data, Evas_Object *obj, const char *part)
{
	__FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	Evas_Object *icon = NULL;
	Evas_Object *layout = NULL;

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	DBG(LOG_INFO, "part[%s]\n", part);
	if (!strcmp("elm.swallow.icon", part)) {
		DBG(LOG_INFO, "elm.swallow.icon - category [%d]\n", peer->category);
		char *img_name = NULL;
		layout = elm_layout_add(obj);
		elm_layout_theme_set(layout, "layout", "list/B/type.3", "default");
		/*
		* the icon of connected device is
		* different from available and busy device
		*/
		switch (peer->category) {
		case WFD_DEVICE_TYPE_COMPUTER:
			img_name = WFD_ICON_DEVICE_COMPUTER;
			break;
		case WFD_DEVICE_TYPE_INPUT_DEVICE:
			img_name = WFD_ICON_DEVICE_INPUT_DEVICE;
			break;
		case WFD_DEVICE_TYPE_PRINTER:
			img_name = WFD_ICON_DEVICE_PRINTER;
			break;
		case WFD_DEVICE_TYPE_CAMERA:
			img_name = WFD_ICON_DEVICE_CAMERA;
			break;
		case WFD_DEVICE_TYPE_STORAGE:
			img_name = WFD_ICON_DEVICE_STORAGE;
			break;
		case WFD_DEVICE_TYPE_NW_INFRA:
			img_name = WFD_ICON_DEVICE_NETWORK_INFRA;
			break;
		case WFD_DEVICE_TYPE_DISPLAYS:
			img_name = WFD_ICON_DEVICE_DISPLAY;
			break;
		case WFD_DEVICE_TYPE_MM_DEVICES:
			if (peer->sub_category == WIFI_DIRECT_SECONDARY_DEVICE_TYPE_MULTIMEDIA_STB) {
				img_name = WFD_ICON_DEVICE_STB;
			} else if (peer->sub_category == WIFI_DIRECT_SECONDARY_DEVICE_TYPE_MULTIMEDIA_MS_MA_ME) {
				img_name = WFD_ICON_DEVICE_DONGLE;
			} else if (peer->sub_category == WIFI_DIRECT_SECONDARY_DEVICE_TYPE_MULTIMEDIA_PVP) {
				img_name = WFD_ICON_DEVICE_BD;
			} else {
				img_name = WFD_ICON_DEVICE_MULTIMEDIA;
			}
			break;
		case WFD_DEVICE_TYPE_GAME_DEVICES:
			img_name = WFD_ICON_DEVICE_GAMING;
			break;
		case WFD_DEVICE_TYPE_TELEPHONE:
			img_name = WFD_ICON_DEVICE_TELEPHONE;
			break;
		case WFD_DEVICE_TYPE_AUDIO:
			if (peer->sub_category == WIFI_DIRECT_SECONDARY_DEVICE_TYPE_AUDIO_TUNER) {
				img_name = WFD_ICON_DEVICE_HOME_THEATER;
			} else {
				img_name = WFD_ICON_DEVICE_HEADSET;
			}
			break;
		default:
			img_name = WFD_ICON_DEVICE_UNKNOWN;
			break;
		}

		icon = elm_image_add(layout);
		elm_image_file_set(icon, WFD_UG_EDJ_PATH, img_name);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_color_set(icon, 2, 61, 132, 204);
		evas_object_show(icon);
		evas_object_propagate_events_set(icon, EINA_FALSE);
		elm_layout_content_set(layout, "elm.swallow.content", icon);
	}

	if (layout)
		evas_object_show(layout);

	__FUNC_EXIT__;
	return layout;
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
	__FUNC_ENTER__;

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	DBG(LOG_INFO,"part = %s",part);
	if (!strcmp("elm.text", part)) {
		return g_strdup(_("IDS_BT_BODY_NO_DEVICES_FOUND_ABB"));
	}
	__FUNC_EXIT__;
	return NULL;
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
	__FUNC_ENTER__;

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}
	DBG(LOG_INFO,"part = %s",part);
	if (!strcmp("elm.text", part)) {
		return g_strdup(_("IDS_ST_HEADER_CONNECTED_DEVICES"));
	}

	__FUNC_EXIT__;
	return NULL;
}

char* ConvertRGBAtoHex(int r, int g, int b, int a)
{
	int hexcolor = 0;
	char *string = NULL;
	string = g_try_malloc0(MAX_HEX_COLOR_LENGTH);
	if (string == NULL) {
		return string;
	}
	hexcolor = (r << 24) + (g << 16) + (b << 8) + a;
	snprintf(string, MAX_HEX_COLOR_LENGTH, "%08x", hexcolor );
	return string;
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
	__FUNC_ENTER__;

	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	DBG(LOG_INFO, "%s", part);
	char *det = NULL;
	char *buf = NULL;
	char *ssid;

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp("elm.text", part)) {
		if (strlen(peer->ssid) != 0) {
			ssid = elm_entry_utf8_to_markup(peer->ssid);
			if (NULL == ssid) {
				DBG(LOG_ERROR, "elm_entry_utf8_to_markup failed.\n");
				__FUNC_EXIT__;
				return NULL;
			}
			__FUNC_EXIT__;
			return ssid;
		}
	} else if (!strcmp("elm.text.sub", part)) {
		det = elm_entry_utf8_to_markup(_("IDS_COM_BODY_CONNECTED_M_STATUS"));
		buf = g_strdup_printf("<color=#%s>%s</color>",
			ConvertRGBAtoHex(2, 61, 132, 255), det);
		if (det != NULL) {
			g_free(det);
		}
		__FUNC_EXIT__;
		return buf;
	}
	__FUNC_EXIT__;
	return NULL;
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
	__FUNC_ENTER__;

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp("elm.text", part)) {
		return g_strdup(_("IDS_WIFI_SBODY_NOT_CONNECTED_M_STATUS_ABB"));
	}

	__FUNC_EXIT__;
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
	__FUNC_ENTER__;

	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
	char *ssid;
	DBG(LOG_INFO, "%s", part);

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp("elm.text", part)) {
		if (strlen(peer->ssid) != 0) {
			ssid = elm_entry_utf8_to_markup(peer->ssid);
			if (NULL == ssid) {
				DBG(LOG_ERROR, "elm_entry_utf8_to_markup failed.\n");
				__FUNC_EXIT__;
				return NULL;
			}
			__FUNC_EXIT__;
			return ssid;
		}
	} else if (!strcmp("elm.text.sub", part)) {
		g_strlcpy(buf, _("IDS_CHATON_HEADER_CONNECTION_FAILED_ABB2"),
			WFD_GLOBALIZATION_STR_LENGTH);
		__FUNC_EXIT__;
		return g_strdup(buf);
	}
	return NULL;
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
	__FUNC_ENTER__;
	struct ug_data *ugd = wfd_get_ug_data();

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}
	DBG(LOG_INFO, "%s", part);

	if (!strcmp("elm.text", part)) {
		if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS) {
			return g_strdup(_("IDS_WIFI_HEADER_AVAILABLE_DEVICES_ABB"));
		} else if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_COMPLETED) {
			return g_strdup(_("IDS_WIFI_SBODY_NOT_CONNECTED_M_STATUS_ABB"));
		}
	}

	__FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the ug get the title label of select all in multi connect
 *	@return   the label of select all string
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_gl_multi_connect_select_all_title_label_get(void *data, Evas_Object *obj, const char *part)
{
	__FUNC_ENTER__;

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}
	DBG(LOG_INFO, "%s", part);

	if (!strcmp("elm.text", part)) {
		__FUNC_EXIT__;
		return g_strdup(_("IDS_WIFI_BODY_SELECT_ALL"));
	}
	__FUNC_EXIT__;
	return NULL;
}

/**
 *	This function let the ug get the icon of select all genlist
 *	@return   the icon of select all genlist
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static Evas_Object *_wfd_gl_select_all_icon_get(void *data, Evas_Object *obj, const char *part)
{
	__FUNC_ENTER__;

	struct ug_data *ugd = (struct ug_data *) data;
	Evas_Object *check = NULL;
	Evas_Object *icon_layout = NULL;

	DBG(LOG_INFO, "Part %s", part);

	if (!strcmp("elm.swallow.end", part)) {
		icon_layout = elm_layout_add(obj);
		elm_layout_theme_set(icon_layout, "layout", "list/C/type.2", "default");
		DBG(LOG_INFO, "Part %s", part);
		check = elm_check_add(icon_layout);
		ugd->select_all_icon = check;
		if (ugd->is_select_all_checked == EINA_TRUE) {
			elm_check_state_set(ugd->select_all_icon, TRUE);
		}
		elm_object_style_set(check, "default/genlist");
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_smart_callback_add(check, "changed",
			wfd_genlist_select_all_check_changed_cb, (void *)data);
		elm_layout_content_set(icon_layout, "elm.swallow.content", check);
	}
	__FUNC_EXIT__;
	return icon_layout;
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
	__FUNC_ENTER__;

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}
	DBG(LOG_INFO, "%s", part);

	if (!strcmp("elm.text", part)) {
		return g_strdup(_("IDS_ST_HEADER_BUSY_DEVICES"));
	}

	__FUNC_EXIT__;
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
	__FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s *) data;
	char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
	char *ssid;
	DBG(LOG_INFO, "%s", part);

	if (data == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	DBG(LOG_INFO, "peer->ssid = %s", peer->ssid);

	if (!strcmp("elm.text", part)) {
		if (strlen(peer->ssid) != 0) {
			ssid = elm_entry_utf8_to_markup(peer->ssid);
			if (NULL == ssid) {
				DBG(LOG_ERROR, "elm_entry_utf8_to_markup failed.\n");
				__FUNC_EXIT__;
				return NULL;
			}
			__FUNC_EXIT__;
			return ssid;
		}
	} else if (!strcmp("elm.text.sub", part)) {
		g_strlcpy(buf, _("IDS_ST_BODY_CONNECTED_WITH_ANOTHER_DEVICE_ABB"),
			WFD_GLOBALIZATION_STR_LENGTH);
		__FUNC_EXIT__;
		return g_strdup(buf);
	}

	return NULL;
}

/**
 *	This function let the ug delete the peer item
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 */
static void _gl_peer_del(void *data, Evas_Object *obj)
{
	__FUNC_ENTER__;

	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == data, "NULL!!");

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug initialize the items of genlist
 *	@return   void
 */
void initialize_gen_item_class()
{
	elm_theme_extension_add(NULL, WFD_UG_EDJ_PATH);

	device_name_itc.item_style = WFD_GENLIST_MULTILINE_TEXT_STYLE;
	device_name_itc.func.text_get = _gl_device_name_label_get;
	device_name_itc.func.content_get = NULL;
	device_name_itc.func.state_get = NULL;

#ifdef WFD_ON_OFF_GENLIST
	wfd_onoff_itc.item_style = WFD_GENLIST_2LINE_BOTTOM_TEXT_ICON_STYLE;
	wfd_onoff_itc.func.text_get = _gl_wfd_onoff_text_get;
	wfd_onoff_itc.func.content_get = _gl_wfd_onoff_content_get;
	wfd_onoff_itc.func.state_get = NULL;
	wfd_onoff_itc.func.del = NULL;
#endif

	title_itc.item_style = WFD_GENLIST_GROUP_INDEX_STYLE;
	title_itc.func.text_get = _gl_title_label_get;
	title_itc.func.content_get = _gl_title_content_get;
	title_itc.func.state_get = NULL;
	title_itc.func.del = NULL;

	multi_view_title_itc.item_style = WFD_GENLIST_GROUP_INDEX_STYLE;
	multi_view_title_itc.func.text_get = _gl_multi_title_label_get;
	multi_view_title_itc.func.content_get = _gl_multi_title_content_get;
	multi_view_title_itc.func.state_get = NULL;
	multi_view_title_itc.func.del = NULL;

	title_no_device_itc.item_style = WFD_GENLIST_GROUP_INDEX_STYLE;
	title_no_device_itc.func.text_get = _gl_title_no_device_label_get;
	title_no_device_itc.func.content_get = NULL;
	title_no_device_itc.func.state_get = NULL;
	title_no_device_itc.func.del = NULL;

	title_available_itc.item_style = WFD_GENLIST_GROUP_INDEX_STYLE;
	title_available_itc.func.text_get = _gl_available_title_label_get;
	title_available_itc.func.content_get = _gl_title_content_get;
	title_available_itc.func.state_get = NULL;
	title_available_itc.func.del = NULL;

	peer_itc.item_style = WFD_GENLIST_2LINE_TOP_TEXT_ICON_STYLE;
	peer_itc.func.text_get = _gl_peer_label_get;
	peer_itc.func.content_get = _gl_peer_icon_get;
	peer_itc.func.state_get = NULL;
	peer_itc.func.del = _gl_peer_del;

	noitem_itc.item_style = WFD_GENLIST_1LINE_TEXT_STYLE;
	noitem_itc.func.text_get = _gl_noitem_text_get;
	noitem_itc.func.content_get = NULL;
	noitem_itc.func.state_get = NULL;
	noitem_itc.func.del = NULL;

	title_conn_itc.item_style = WFD_GENLIST_GROUP_INDEX_STYLE;
	title_conn_itc.func.text_get = _gl_conn_dev_title_label_get;
	title_conn_itc.func.content_get = NULL;
	title_conn_itc.func.state_get = NULL;
	title_conn_itc.func.del = NULL;

	peer_conn_itc.item_style = WFD_GENLIST_2LINE_TOP_TEXT_ICON_STYLE;
	peer_conn_itc.func.text_get = _gl_peer_conn_dev_label_get;
	peer_conn_itc.func.content_get = _gl_conn_peer_icon_get;
	peer_conn_itc.func.state_get = NULL;
	peer_conn_itc.func.del = _gl_peer_del;

	title_conn_failed_itc.item_style = WFD_GENLIST_1LINE_TEXT_STYLE;
	title_conn_failed_itc.func.text_get = _gl_conn_failed_dev_title_label_get;
	title_conn_failed_itc.func.content_get = NULL;
	title_conn_failed_itc.func.state_get = NULL;
	title_conn_failed_itc.func.del = NULL;

	peer_conn_failed_itc.item_style = WFD_GENLIST_2LINE_TOP_TEXT_ICON_STYLE;
	peer_conn_failed_itc.func.text_get = _gl_peer_conn_failed_dev_label_get;
	peer_conn_failed_itc.func.content_get = _gl_peer_icon_get;
	peer_conn_failed_itc.func.state_get = NULL;
	peer_conn_failed_itc.func.del = _gl_peer_del;

	title_busy_itc.item_style = WFD_GENLIST_GROUP_INDEX_STYLE;
	title_busy_itc.func.text_get = _gl_busy_dev_title_label_get;
	title_busy_itc.func.content_get = NULL;
	title_busy_itc.func.state_get = NULL;
	title_busy_itc.func.del = NULL;

	peer_busy_itc.item_style = WFD_GENLIST_2LINE_TOP_TEXT_ICON_STYLE;
	peer_busy_itc.func.text_get = _gl_peer_busy_dev_label_get;
	peer_busy_itc.func.content_get = _gl_peer_icon_get;
	peer_busy_itc.func.state_get = NULL;
	peer_busy_itc.func.del = _gl_peer_del;

	title_multi_connect_itc.item_style = WFD_GENLIST_GROUP_INDEX_STYLE;
	title_multi_connect_itc.func.text_get = _gl_multi_connect_dev_title_label_get;
	title_multi_connect_itc.func.content_get = NULL;
	title_multi_connect_itc.func.state_get = NULL;
	title_multi_connect_itc.func.del = NULL;

	select_all_multi_connect_itc.item_style = WFD_GENLIST_1LINE_TEXT_ICON_STYLE;
	select_all_multi_connect_itc.func.text_get =
		_gl_multi_connect_select_all_title_label_get;
	select_all_multi_connect_itc.func.content_get = _wfd_gl_select_all_icon_get;
	select_all_multi_connect_itc.func.state_get = NULL;
	select_all_multi_connect_itc.func.del = NULL;
}
