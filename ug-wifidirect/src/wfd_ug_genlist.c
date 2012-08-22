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

#include <assert.h>
#include <glib.h>

#include <Elementary.h>
#include <vconf.h>
#include <ui-gadget-module.h>
#include <wifi-direct.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

Elm_Gen_Item_Class sep_itc;
Elm_Gen_Item_Class sep_itc_end;
Elm_Gen_Item_Class head_itc;
Elm_Gen_Item_Class name_itc;
Elm_Gen_Item_Class title_itc;
Elm_Gen_Item_Class peer_itc;
Elm_Gen_Item_Class noitem_itc;
Elm_Gen_Item_Class help_itc;
Elm_Gen_Item_Class button_itc;

Elm_Gen_Item_Class title_conn_itc;
Elm_Gen_Item_Class peer_conn_itc;

Elm_Gen_Item_Class title_busy_itc;
Elm_Gen_Item_Class peer_busy_itc;

Elm_Gen_Item_Class title_multi_connect_itc;
Elm_Gen_Item_Class peer_multi_connect_itc;

Elm_Gen_Item_Class title_conn_failed_itc;
Elm_Gen_Item_Class peer_conn_failed_itc;


static char *_gl_header_label_get(void *data, Evas_Object * obj, const char *part)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    DBG(LOG_VERBOSE, "%s", part);

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if(!strcmp(part, "elm.text.1"))
    {
        DBG(LOG_VERBOSE, "Current text mode [%d]\n", ugd->head_text_mode);
        switch(ugd->head_text_mode)
        {
            case HEAD_TEXT_TYPE_DIRECT:
	    case HEAD_TEXT_TYPE_ACTIVATED:
	    case HEAD_TEXT_TYPE_SCANING:
                return strdup(dgettext("sys_string", "IDS_COM_OPT1_WI_FI_DIRECT"));
                break;
            case HEAD_TEXT_TYPE_DEACTIVATING:
                return strdup(_("IDS_WFD_BODY_DEACTIVATING")); // "Deactivating Wi-Fi Direct..."
                break;
            case HEAD_TEXT_TYPE_ACTIVATING:
                return strdup(_("IDS_WFD_BODY_ACTIVATING")); //"Activating Wi-Fi Direct..."
                break;
            default:
                break;
        }
    }
    else if(!strcmp(part, "elm.text.1"))
    {
	return strdup(dgettext("sys_string", "IDS_COM_OPT1_WI_FI_DIRECT"));
    } else if(!strcmp(part, "elm.text.2"))
    {
        return strdup(ugd->dev_name);
    }

    __FUNC_EXIT__;
    return NULL;
}

static Evas_Object *_gl_header_icon_get(void *data, Evas_Object * obj, const char *part)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    Evas_Object *onoff = NULL;

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if(ugd->head_text_mode == HEAD_TEXT_TYPE_ACTIVATING ||
        ugd->head_text_mode == HEAD_TEXT_TYPE_DEACTIVATING)
        return NULL;

    DBG(LOG_VERBOSE, "%s", part);
    onoff = elm_check_add(obj);
    elm_object_style_set(onoff, "on&off");
    elm_check_state_set(onoff, ugd->wfd_onoff);
    evas_object_smart_callback_add(onoff, "changed", _wfd_onoff_btn_cb, ugd);
    evas_object_show(onoff);

    __FUNC_EXIT__;

    return onoff;
}


static char *_gl_name_label_get(void *data, Evas_Object *obj, const char *part)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    DBG(LOG_VERBOSE, "%s", part);

    if(!strcmp(part, "elm.text"))
    {
        return strdup(IDS_WFD_TITLE_ABOUT_WIFI); // "Device name"
    }
    else if(!strcmp(part, "elm.text.2"))
    {
        return strdup(ugd->dev_name);
    }

    __FUNC_EXIT__;

    return NULL;
}


static char *_gl_title_label_get(void *data, Evas_Object *obj,
		const char *part)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (!strcmp(part, "elm.text"))
    {
    	if (ugd->multiconn_view_genlist != NULL)
    	{
    		// It's called at Multi connect view...
			if(ugd->gl_available_dev_cnt_at_multiconn_view > 0)
				return strdup(_("IDS_WFD_BODY_AVAILABLE_DEVICES")); // "Available devices"
			else
				return strdup(_("IDS_WFD_BODY_WIFI_DIRECT_DEVICES")); // "Wi-Fi Direct devices"
    	}
    	else
    	{
    		// It's called at Main View
			if(ugd->gl_available_peer_cnt > 0)
				return strdup(_("IDS_WFD_BODY_AVAILABLE_DEVICES")); // "Available devices"
			else
				return strdup(_("IDS_WFD_BODY_WIFI_DIRECT_DEVICES")); // "Wi-Fi Direct devices"
    	}
    }

    __FUNC_EXIT__;

    return NULL;
}

static Evas_Object *_gl_title_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *progressbar = NULL;
	struct ug_data *ugd = (struct ug_data*) data;

	if (data == NULL) {
	    DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
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

static char *_gl_peer_label_get(void *data, Evas_Object * obj, const char *part)
{
    __FUNC_ENTER__;
    assertm_if(NULL == obj, "NULL!!");
    assertm_if(NULL == part, "NULL!!");

   device_type_s *peer = (device_type_s*) data;
   char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
   DBG(LOG_VERBOSE, "%s", part);
    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (!strcmp(part, "elm.text.1"))
    {
    	__FUNC_EXIT__;
        return strdup(peer->ssid);
    }
    else if (!strcmp(part, "elm.text.2"))
    {
        switch (peer->conn_status) {
        case PEER_CONN_STATUS_DISCONNECTED:
            g_strlcpy(buf, IDS_WFD_TAP_TO_CONNECT,
                            WFD_GLOBALIZATION_STR_LENGTH);
        break;

        case PEER_CONN_STATUS_CONNECTING:
            g_strlcpy(buf, IDS_WFD_CONNECTING,
                    WFD_GLOBALIZATION_STR_LENGTH);
        break;

        case PEER_CONN_STATUS_CONNECTED:
            g_strlcpy(buf, IDS_WFD_CONNECTED,
                WFD_GLOBALIZATION_STR_LENGTH);
        break;

        case PEER_CONN_STATUS_FAILED_TO_CONNECT:
            g_strlcpy(buf, IDS_WFD_FAILED_TO_CONNECT,
                WFD_GLOBALIZATION_STR_LENGTH);
        break;

        case PEER_CONN_STATUS_WAIT_FOR_CONNECT:
            g_strlcpy(buf, IDS_WFD_WAITING_FOR_CONNECT,
                WFD_GLOBALIZATION_STR_LENGTH);
        break;

        default:
            g_strlcpy(buf, IDS_WFD_TAP_TO_CONNECT,
                WFD_GLOBALIZATION_STR_LENGTH);
        break;
        }
    }
    else
    {
    	__FUNC_EXIT__;
    	return NULL;
    }

    __FUNC_EXIT__;
    return strdup(buf);
}

static Evas_Object *_gl_peer_icon_get(void *data, Evas_Object * obj, const char *part)
{
    __FUNC_ENTER__;
    assertm_if(NULL == obj, "NULL!!");
    assertm_if(NULL == part, "NULL!!");

    device_type_s *peer = (device_type_s*) data;
    Evas_Object *icon = NULL;

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if(!strcmp(part, "elm.icon.2"))
    {
        DBG(LOG_VERBOSE, "elm.icon.2 - connection status [%d]\n", peer->conn_status);
        if(peer->conn_status == PEER_CONN_STATUS_CONNECTING)
        {
            icon = elm_progressbar_add(obj);
            elm_object_style_set(icon, "list_process");
            elm_progressbar_pulse(icon, EINA_TRUE);
        }
        else if(peer->conn_status == PEER_CONN_STATUS_CONNECTED)
        {
#if 0
            icon = elm_icon_add(obj);
            elm_icon_file_set(icon, WFD_ICON_CONNECTED, NULL);
#endif
        	return NULL;
        }

        evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
        elm_icon_resizable_set(icon, 1, 1);
        evas_object_show(icon);
    }
    else if(!strcmp(part, "elm.icon.1"))
    {
        DBG(LOG_VERBOSE, "elm.icon.1 - category [%d]\n", peer->category);
        char *img_path = NULL;
        switch(peer->category)
        {
		case WFD_DEVICE_TYPE_COMPUTER:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_COMPUTER_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_COMPUTER;
			}
			break;
		case WFD_DEVICE_TYPE_INPUT_DEVICE:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_INPUT_DEVICE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_INPUT_DEVICE;
			}
			break;
		case WFD_DEVICE_TYPE_PRINTER:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_PRINTER_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_PRINTER;
			}
			break;
		case WFD_DEVICE_TYPE_CAMERA:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_CAMERA_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_CAMERA;
			}
			break;
		case WFD_DEVICE_TYPE_STORAGE:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_STORAGE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_STORAGE;
			}
			break;
		case WFD_DEVICE_TYPE_NW_INFRA:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_NETWORK_INFRA_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_NETWORK_INFRA;
			}
			break;
		case WFD_DEVICE_TYPE_DISPLAYS:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_DISPLAY_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_DISPLAY;
			}
			break;
		case WFD_DEVICE_TYPE_MM_DEVICES:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_MULTIMEDIA_DEVICE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_MULTIMEDIA_DEVICE;
			}
			break;
		case WFD_DEVICE_TYPE_GAME_DEVICES:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_GAMING_DEVICE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_GAMING_DEVICE;
			}
			break;
		case WFD_DEVICE_TYPE_TELEPHONE:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_TELEPHONE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_TELEPHONE;
			}
			break;
		case WFD_DEVICE_TYPE_AUDIO:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_AUDIO_DEVICE_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_AUDIO_DEVICE;
			}
			break;
		default:
			if (peer->conn_status == PEER_CONN_STATUS_CONNECTED) {
				img_path = WFD_ICON_DEVICE_COMPUTER_CONNECT;
			} else {
				img_path = WFD_ICON_DEVICE_COMPUTER;
			}
			break;
        }

        if(img_path != NULL)
        {
            icon = elm_icon_add(obj);
            elm_icon_file_set(icon, img_path, NULL);
            evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
            elm_icon_resizable_set(icon, 1, 1);
            evas_object_show(icon);
        }
        else
        {
        	return NULL;
        }
    }

    __FUNC_EXIT__;
    return icon;
}


static char *_gl_noitem_text_get(void *data, Evas_Object * obj,
					  const char *part)
{
    __FUNC_ENTER__;

	if(data == NULL)
	{
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	__FUNC_EXIT__;
	return strdup(IDS_WFD_NOCONTENT);
}


static char *_gl_help_label_get(void *data, Evas_Object * obj, const char *part)
{
    __FUNC_ENTER__;
    DBG(LOG_VERBOSE, "%s", part);
    __FUNC_EXIT__;
    return strdup("Help");
}


static Evas_Object *_gl_button_get(void *data, Evas_Object * obj, const char *part)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;


    DBG(LOG_VERBOSE, "%s", part);

    ugd->multi_btn = elm_button_add(obj);

	wfd_refresh_wifi_direct_state(ugd);

	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_COMPLETED)
	{
		elm_object_text_set(ugd->multi_btn, IDS_WFD_BUTTON_DISCONNECT);
		DBG(LOG_VERBOSE, "button: Disconnect\n");

		// Don't connect "clicked" callback.
		return ugd->multi_btn;
	}

	if (ugd->wfd_status == WFD_LINK_STATUS_CONNECTING)
	{
		elm_object_text_set(ugd->multi_btn, IDS_WFD_BUTTON_CANCEL);
		DBG(LOG_VERBOSE, "button: Cancel connect\n");
	}
	else if (ugd->wfd_status >= WFD_LINK_STATUS_CONNECTED)
	{
		if (ugd->gl_connected_peer_cnt > 1)
		{
			elm_object_text_set(ugd->multi_btn, IDS_WFD_BUTTON_DISCONNECT_ALL);
			DBG(LOG_VERBOSE, "button: Disconnect All\n");
		}
		else
		{
			elm_object_text_set(ugd->multi_btn, IDS_WFD_BUTTON_DISCONNECT);
			DBG(LOG_VERBOSE, "button: Disconnect\n");
		}
	}
	else
	{
		elm_object_text_set(ugd->multi_btn, IDS_WFD_BUTTON_MULTI);
		DBG(LOG_VERBOSE, "button: Multi connect\n");
	}

    evas_object_smart_callback_add(ugd->multi_btn, "clicked", _wifid_create_multibutton_cb, ugd);
    evas_object_show(ugd->multi_btn);

    __FUNC_EXIT__;

    return ugd->multi_btn;
}


static char *_gl_conn_dev_title_label_get(void *data, Evas_Object *obj,
		const char *part)
{
    __FUNC_ENTER__;

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (!strcmp(part, "elm.text"))
    {
            return strdup(IDS_WFD_BODY_CONNECTED_DEVICES);
    }

    __FUNC_EXIT__;

    return NULL;
}


static char *_gl_peer_conn_dev_label_get(void *data, Evas_Object * obj, const char *part)
{
	__FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");

	device_type_s *peer = (device_type_s*) data;

	char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
	DBG(LOG_VERBOSE, "%s", part);
	if(data == NULL)
	{
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text.1"))
	{
		return strdup(peer->ssid);
	}
	else
	{
		g_strlcpy(buf, IDS_WFD_CONNECTED,
						WFD_GLOBALIZATION_STR_LENGTH);
		__FUNC_EXIT__;
		return strdup(buf);
	}
}

static char *_gl_conn_failed_dev_title_label_get(void *data, Evas_Object *obj,
		const char *part)
{
    __FUNC_ENTER__;

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (!strcmp(part, "elm.text"))
    {
            return strdup(IDS_WFD_BODY_FAILED_DEVICES);
    }

    __FUNC_EXIT__;

    return NULL;
}


static char *_gl_peer_conn_failed_dev_label_get(void *data, Evas_Object * obj, const char *part)
{
	__FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");

	device_type_s *peer = (device_type_s*) data;

	char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
	DBG(LOG_VERBOSE, "%s", part);
	if(data == NULL)
	{
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text.1"))
	{
		return strdup(peer->ssid);
	}
	else
	{
		g_strlcpy(buf, IDS_WFD_FAILED_TO_CONNECT,
						WFD_GLOBALIZATION_STR_LENGTH);
		__FUNC_EXIT__;
		return strdup(buf);
	}
}

static char *_gl_multi_connect_dev_title_label_get(void *data, Evas_Object *obj,
		const char *part)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = wfd_get_ug_data();

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (!strcmp(part, "elm.text"))
    {
    	if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_IN_PROGRESS)
    		return strdup(_("IDS_WFD_BODY_AVAILABLE_DEVICES")); // "Available devices"
    	else if (ugd->multi_connect_mode == WFD_MULTI_CONNECT_MODE_COMPLETED)
    		return strdup(IDS_WFD_BODY_FAILED_DEVICES); // "Available devices"
    }

    __FUNC_EXIT__;

    return NULL;
}


static char *_gl_busy_dev_title_label_get(void *data, Evas_Object *obj,
		const char *part)
{
    __FUNC_ENTER__;

    if(data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (!strcmp(part, "elm.text"))
    {
            return strdup(IDS_WFD_BODY_BUSY_DEVICES);
    }

    __FUNC_EXIT__;

    return NULL;
}


static char *_gl_peer_busy_dev_label_get(void *data, Evas_Object * obj, const char *part)
{
	__FUNC_ENTER__;
	assertm_if(NULL == obj, "NULL!!");
	assertm_if(NULL == part, "NULL!!");
	device_type_s *peer = (device_type_s*) data;
	char buf[WFD_GLOBALIZATION_STR_LENGTH] = { 0, };
	DBG(LOG_VERBOSE, "%s", part);
	if(data == NULL)
	{
		DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
		return NULL;
	}
	DBG(LOG_VERBOSE, "peer->ssid = %s", peer->ssid);
	if (!strcmp(part, "elm.text.1"))
	{
		return strdup(peer->ssid);
	}
	else
	{
		g_strlcpy(buf, IDS_WFD_CONNECTED_WITH_OTHER_DEVICE,
						WFD_GLOBALIZATION_STR_LENGTH);
		__FUNC_EXIT__;
		return strdup(buf);
	}
}

static void _gl_peer_del(void *data, Evas_Object * obj)
{
    __FUNC_ENTER__;
    assertm_if(NULL == obj, "NULL!!");
    assertm_if(NULL == data, "NULL!!");

    __FUNC_EXIT__;
    return;
}

void initialize_gen_item_class()
{
    sep_itc.item_style = "dialogue/separator";
    sep_itc.func.text_get = NULL;
    sep_itc.func.content_get = NULL;
    sep_itc.func.state_get = NULL;
    sep_itc.func.del = NULL;

    sep_itc_end.item_style = "dialogue/separator/end";
    sep_itc_end.func.text_get = NULL;
    sep_itc_end.func.content_get = NULL;
    sep_itc_end.func.state_get = NULL;
    sep_itc_end.func.del = NULL;

    head_itc.item_style = "dialogue/2text.1icon.6";
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

    noitem_itc.item_style ="dialogue/1text";
    noitem_itc.func.text_get = _gl_noitem_text_get;
    noitem_itc.func.content_get = NULL;
    noitem_itc.func.state_get = NULL;
    noitem_itc.func.del = NULL;

    help_itc.item_style = "dialogue/1text";
    help_itc.func.text_get = _gl_help_label_get;
    help_itc.func.content_get = NULL;
    help_itc.func.state_get = NULL;
    help_itc.func.del = NULL;

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

#if 0
    peer_multi_connect_itc.item_style = "dialogue/2text.2icon.3";
    peer_multi_connect_itc.func.text_get = _gl_peer_label_get;
    peer_multi_connect_itc.func.content_get = _gl_peer_icon_get;
    peer_multi_connect_itc.func.state_get = NULL;
    peer_multi_connect_itc.func.del = _gl_peer_del;
#endif
}
