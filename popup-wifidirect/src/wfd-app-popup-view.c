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

/**
 * This file implements wifi direct system popup view  functions.
 *
 * @file    wfd-app-popup-view.c
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */

#include <libintl.h>
#include <glib.h>

#include "wifi-direct.h"
#include "wfd-app.h"
#include "wfd-app-strings.h"
#include "wfd-app-util.h"

extern wfd_appdata_t *g_wfd_ad;
extern wfd_popup_t *g_wfd_pop;
extern unsigned char g_wfd_peer_mac[6];
extern unsigned char g_wfd_peer_name[32];
static Ecore_Timer *pb_timer = NULL;

void wfd_tickernoti_popup(char *msg);

static void __popup_resp_cb(void *data, Evas_Object * obj, void *event_info)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = wfd_get_appdata();
	int result = -1;
	int resp = (int) data;
	Evas_Object *btn = obj;
	char msg[WFD_POP_STR_MAX_LEN] = {0};

	WFD_APP_LOG(WFD_APP_LOG_HIGH, "popup resp : %d\n", resp);

	switch (resp)
	{
	case /* MT */ WFD_POP_RESP_APRV_CONNECT_PBC_YES:
	{
		WFD_APP_LOG(WFD_APP_LOG_HIGH,
				"WFD_POP_RESP_APRV_CONNECT_PBC_YES\n");
		wfd_destroy_popup();

		result = wifi_direct_accept_connection(ad->peer_mac);
		WFD_APP_LOG(WFD_APP_LOG_LOW,
				"wifi_direct_accept_connection() result=[%d]\n",
				result);
		if (result == WIFI_DIRECT_ERROR_NONE)
		{
			/* tickernoti popup */
			wfd_tickernoti_popup(_("IDS_WFD_POP_CONNECTING"));
		}
		else
		{
			WFD_APP_LOG(WFD_APP_LOG_ERROR,
					"wifi_direct_accept_connection() FAILED!!\n");
			evas_object_hide(ad->win);

			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WFD_POP_CONNECT_FAILED"), ad->peer_name);
			wfd_tickernoti_popup(msg);
		}
	}
	break;

	case /* MT */ WFD_POP_RESP_APRV_CONNECT_DISPLAY_YES:
	{
		char *pin = NULL;

		WFD_APP_LOG(WFD_APP_LOG_HIGH,
				"WFD_POP_RESP_APRV_CONNECT_DISPLAY_YES\n");
		wfd_destroy_popup();
		if (pb_timer) {
			ecore_timer_del(pb_timer);
			pb_timer = NULL;
		}

		if (wifi_direct_generate_wps_pin() != WIFI_DIRECT_ERROR_NONE)
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wifi_direct_generate_wps_pin() is failed\n");
			return;
		}

		if (wifi_direct_get_wps_pin(&pin) != WIFI_DIRECT_ERROR_NONE)
		{
			WFD_APP_LOG(WFD_APP_LOG_LOW, "wifi_direct_generate_wps_pin() is failed\n");
			return;
		}
		strncpy(ad->pin_number, pin, 32);
		free(pin);
		pin = NULL;

		WFD_APP_LOG(WFD_APP_LOG_LOW, "button ok: pin [%s]", ad->pin_number);

		result = wifi_direct_accept_connection(ad->peer_mac);
		WFD_APP_LOG(WFD_APP_LOG_LOW,
				"wifi_direct_accept_connection() failed. result=[%d]\n",
				result);
		if (result == WIFI_DIRECT_ERROR_NONE)
		{
			evas_object_hide(ad->win);
			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_PIN, NULL);
		}
		else
		{
			/* tickernoti popup */
			wfd_tickernoti_popup(_("IDS_WFD_POP_CONNECT_FAILED"));
		}
	}
	break;

	case /* MO */ WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK:
	case /* MT */ WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES:
	{
		WFD_APP_LOG(WFD_APP_LOG_HIGH,
				"WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES\n");
		wfd_destroy_popup();

		int len = strlen(ad->pin_number);
		WFD_APP_LOG(WFD_APP_LOG_LOW, "button ok: pin [%s]", ad->pin_number);

		if (len > 7 && len < 64)
		{
			int result = 0;
			WFD_APP_LOG(WFD_APP_LOG_LOW, "pin=[%s]\n", ad->pin_number);

			result = wifi_direct_set_wps_pin(ad->pin_number);

			if (result != WIFI_DIRECT_ERROR_NONE)
			{
				/* tickernoti popup */
				snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WFD_POP_CONNECT_FAILED"), ad->peer_name);
				wfd_tickernoti_popup(msg);
				return;
			}

			//result = wifi_direct_activate_pushbutton();
			result = wifi_direct_accept_connection(ad->peer_mac);
			WFD_APP_LOG(WFD_APP_LOG_LOW,
					"wifi_direct_accept_connection(%s) result=[%d]\n",
					ad->peer_mac, result);
			if (result != WIFI_DIRECT_ERROR_NONE)
			{
				WFD_APP_LOG(WFD_APP_LOG_ERROR,
						"wifi_direct_accept_connection() FAILED!!\n");
				evas_object_hide(ad->win);

				/* tickernoti popup */
				snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WFD_POP_CONNECT_FAILED"), ad->peer_name);
				wfd_tickernoti_popup(msg);
			}
		}
		else
		{
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Error, Incorrect PIN!!\n");

			/* tickernoti popup */
			wfd_tickernoti_popup(_("IDS_WFD_POP_PIN_INVALID"));

			/* redraw the popup */
			if (WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK == resp) {
				wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_KEYPAD, (void *) NULL);
			} else {
				wfd_prepare_popup(WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ, (void *) NULL);
			}

			return;
		}
	}
	break;

	case /* MT */ WFD_POP_RESP_APRV_CONNECT_NO:
	{
		WFD_APP_LOG(WFD_APP_LOG_HIGH,
				"WFD_POP_RESP_APRV_CONNECT_NO: destroy_popup...\n");

		wfd_destroy_popup();
		if (pb_timer) {
			ecore_timer_del(pb_timer);
			pb_timer = NULL;
		}

		result = wifi_direct_disconnect(ad->peer_mac);
		WFD_APP_LOG(WFD_APP_LOG_LOW,
				"wifi_direct_disconnect[%s] result=[%d]\n",
				ad->peer_mac, result);
	}
	break;

	case WFD_POP_RESP_PROG_CONNECT_CANCEL:
	{
		WFD_APP_LOG(WFD_APP_LOG_HIGH, "WFD_POP_RESP_PROG_CONNECT_CANCEL\n");
		wfd_destroy_popup();

		result = wifi_direct_disconnect(ad->peer_mac);
		WFD_APP_LOG(WFD_APP_LOG_LOW,
				"wifi_direct_disconnect[%s] result=[%d]\n",
				ad->peer_mac, result);

		if (result == WIFI_DIRECT_ERROR_NONE)
		{
			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WFD_POP_DISCONNECTED"), ad->peer_name);
			wfd_tickernoti_popup(msg);
		}
		else
		{
			WFD_APP_LOG(WFD_APP_LOG_ERROR,
					"wifi_direct_disconnect() FAILED!!\n");

			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WFD_POP_DISCONNECT_FAILED"), ad->peer_name);
			wfd_tickernoti_popup(msg);
		}
	}
	break;

	case WFD_POP_RESP_AUTOMATIC_TURNOFF_OK:
	{
		WFD_APP_LOG(WFD_APP_LOG_HIGH, "WFD_POP_RESP_AUTOMATIC_TURNOFF_OK\n");

		/* turn off the Wi-Fi Direct */
		result = wifi_direct_get_state(&ad->wfd_status);
		if (result != WIFI_DIRECT_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "Failed to get link status. [%d]\n", result);
			return;
		}

		if (ad->wfd_status < WIFI_DIRECT_STATE_ACTIVATING) {
			WFD_APP_LOG(WFD_APP_LOG_LOW, "Wi-Fi Direct is already deactivated\n");
		} else {
			ad->wfd_status = WIFI_DIRECT_STATE_DEACTIVATING;
			wifi_direct_deactivate();
		}

		wfd_destroy_popup();
	}
	break;

	default:
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Unknown respone\n");
		evas_object_hide(ad->win);
	}
	break;
	}

	__WFD_APP_FUNC_EXIT__;
}

static Evas_Object *__create_progress_layout(Evas_Object * parent,
                                             const char *text)
{
    __WFD_APP_FUNC_ENTER__;

    if (parent == NULL || text == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "param is NULL\n");
        return NULL;
    }

    Evas_Object *progressbar = NULL, *layout = NULL;
    Evas_Object *label = NULL;
    int w = 0, h = 0;

    layout = elm_layout_add(parent);
    if (layout == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "layout is NULL\n");
        return NULL;
    }

    elm_layout_file_set(layout, EDJ_NAME, "progress_popup");
    evas_object_size_hint_weight_set(layout,
                                     EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    progressbar = elm_progressbar_add(layout);
    if (progressbar == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "progressbar is NULL\n");
        evas_object_del(layout);
        return NULL;
    }
    elm_object_style_set(progressbar, "list_process");
    evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
    evas_object_size_hint_weight_set(progressbar,
                                     EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(layout, "popup_pb", progressbar);
    elm_progressbar_pulse(progressbar, EINA_TRUE);

    label = elm_label_add(layout);
    if (label == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "label is NULL\n");
        evas_object_del(layout);
        evas_object_del(progressbar);
        return NULL;
    }
    elm_object_style_set(label, "popup_description/default");
    elm_object_part_content_set(layout, "popup_progress_text", label);
    edje_object_part_geometry_get(layout, "popup_progress_text", NULL, NULL, &w,
                                  &h);
    elm_label_line_wrap_set(label, ELM_WRAP_WORD);
    elm_label_wrap_width_set(label, w);
    elm_object_text_set(label, text);

    evas_object_show(layout);

    __WFD_APP_FUNC_EXIT__;

    return layout;
}


void wfd_destroy_popup()
{
    __WFD_APP_FUNC_ENTER__;

    wfd_appdata_t *ad = wfd_get_appdata();

    if (ad == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "ad is NULL\n");
        return;
    }

    if (ad->popup)
    {
        evas_object_del(ad->popup);
        ad->popup = NULL;
    }

    if (ad->popup_timeout_handle > 0)
    {
        g_source_remove(ad->popup_timeout_handle);
        ad->popup_timeout_handle = 0;
    }

    evas_object_hide(ad->win);

    __WFD_APP_FUNC_EXIT__;
    return;
}

static Evas_Object *wfd_draw_pop_type_a(Evas_Object * win, wfd_popup_t * pop)
{                               //no button with spin
    __WFD_APP_FUNC_ENTER__;

    Evas_Object *popup;

    popup = elm_popup_add(win);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, pop->text);
    elm_popup_timeout_set(popup, pop->timeout);
    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}

static Evas_Object *wfd_draw_pop_type_b(Evas_Object * win, wfd_popup_t * pop)
{                               //one button with spin
    __WFD_APP_FUNC_ENTER__;

    Evas_Object *popup = NULL;
    Evas_Object *btn = NULL;

    popup = elm_popup_add(win);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, pop->text);

    btn = elm_button_add(popup);
    elm_object_style_set(btn, "popup_button/default");
    elm_object_text_set(btn, pop->label1);
    elm_object_part_content_set(popup, "button1", btn);
    evas_object_smart_callback_add(btn, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data1);

    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}

static Evas_Object *wfd_draw_pop_type_c(Evas_Object * win, wfd_popup_t * pop)
{                               //two button
    __WFD_APP_FUNC_ENTER__;

    Evas_Object *popup = NULL;
    Evas_Object *btn1 = NULL, *btn2 = NULL;

    popup = elm_popup_add(win);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, pop->text);

    btn1 = elm_button_add(popup);
    elm_object_style_set(btn1, "popup_button/default");
    elm_object_text_set(btn1, pop->label1);
    elm_object_part_content_set(popup, "button1", btn1);
    evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data1);

    btn2 = elm_button_add(popup);
    elm_object_style_set(btn2, "popup_button/default");
    elm_object_text_set(btn2, pop->label2);
    elm_object_part_content_set(popup, "button2", btn2);
    evas_object_smart_callback_add(btn2, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data2);

    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}

static Evas_Object *wfd_draw_pop_type_d(Evas_Object * win, wfd_popup_t * pop)
{                               //text with spin
    __WFD_APP_FUNC_ENTER__;

    Evas_Object *popup;
    Evas_Object *layout;

    popup = elm_popup_add(win);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_style_set(popup, "customstyle");
    layout = __create_progress_layout(popup, pop->text);
    elm_popup_timeout_set(popup, pop->timeout);
    elm_object_content_set(popup, layout);
    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}

static void __popup_block_clicked_cb(void *data, Evas_Object * obj,
                                     void *event_info)
{
    wfd_appdata_t *ad = wfd_get_appdata();
    wfd_popup_t *pop = NULL;

    if (ad != NULL)
        pop = ad->popup;

    if (ad->win != NULL)
        evas_object_hide(ad->win);

    if (ad->popup_timeout_handle > 0)
    {
        g_source_remove(ad->popup_timeout_handle);
        ad->popup_timeout_handle = 0;
    }
}

gboolean __popup_remove_timeout_cb(gpointer user_data)
{
    wfd_appdata_t *ad = wfd_get_appdata();
    wfd_popup_t *pop = NULL;

    if (ad != NULL)
        pop = ad->popup;
    else
        return false;

    if (pop != user_data)
        return false;

    if (ad->win != NULL)
        evas_object_hide(ad->win);

    ad->popup_timeout_handle = 0;
    return false;
}

static Evas_Object *wfd_draw_pop_type_e(Evas_Object * win, wfd_popup_t * pop)
{
    __WFD_APP_FUNC_ENTER__;

    wfd_appdata_t *ad = wfd_get_appdata();
    Evas_Object *popup;

    if (ad == NULL)
        return NULL;

    popup = elm_popup_add(win);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, pop->text);
    evas_object_smart_callback_add(popup, "block,clicked",
                                   __popup_block_clicked_cb, NULL);
    ad->popup_timeout_handle =
        g_timeout_add(3000, __popup_remove_timeout_cb, popup);
    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}


static void _smart_ime_cb(void *data, Evas_Object * obj, void *event_info)
{
    __WFD_APP_FUNC_ENTER__;
    wfd_appdata_t *ad = wfd_get_appdata();

    Ecore_IMF_Context *imf_context = NULL;
    imf_context = (Ecore_IMF_Context *) ad->pin_entry;

    if (NULL == imf_context)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "Error!!! Ecore_IMF_Context is NULL!!");
        return;
    }

    const char *txt =
        elm_entry_markup_to_utf8(elm_entry_entry_get
                                 ((const Evas_Object *) imf_context));

    if (NULL != txt)
    {
        WFD_APP_LOG(WFD_APP_LOG_LOW, "* text [%s], len=[%d]", txt, strlen(txt));
        strncpy(ad->pin_number, txt, sizeof(ad->pin_number));
    }
    else
    {
        WFD_APP_LOG(WFD_APP_LOG_LOW, "Err!");
    }

    __WFD_APP_FUNC_EXIT__;
}


//#ifdef HIDE_PIN_NUMBER
#if 1
static void _check_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
    wfd_appdata_t *ad = wfd_get_appdata();

    if (obj == NULL)
        return;

    Eina_Bool state = elm_check_state_get(obj);
    elm_entry_password_set(ad->pin_entry, !state);
}
#endif

static Eina_Bool _fn_pb_timer(void *data)
{
	int step = 0;
	double value = 0.0;
	char time_label[32] = {0};
	wfd_wps_display_popup_t *wps_display_popup = (wfd_wps_display_popup_t*) data;

	if (NULL == wps_display_popup) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Param is NULL.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	Evas_Object *progressbar = NULL;
	Evas_Object *time = NULL;

	progressbar = wps_display_popup->progressbar;
	time = wps_display_popup->time;
	value = elm_progressbar_value_get(progressbar);

	if (value >= 1.0) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Progress end.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	wps_display_popup->step++;
	step = wps_display_popup->step;
	value = ((double)step) / WFD_POP_TIMER_120;
	elm_progressbar_value_set(progressbar, value);
	WFD_APP_LOG(WFD_APP_LOG_LOW, "step: %d, value: %f\n", wps_display_popup->step, value);

	/* show the time label */
	if (step < 60) {
		if (step < 10) {
			snprintf(time_label, 32, "00:0%d", step);
		} else {
			snprintf(time_label, 32, "00:%d", step);
		}
	} else {
		if (step%60 < 10) {
			snprintf(time_label, 32, "0%d:0%d", step/60, step%60);
		} else {
			snprintf(time_label, 32, "0%d:%d", step/60, step%60);
		}
	}

	elm_object_text_set(time, time_label);

	return ECORE_CALLBACK_RENEW;
}

static Evas_Object * _add_edit_field(Evas_Object *parent, const char *title, const char *guide, Eina_Bool single_line, Eina_Bool is_editable)
{
	assertm_if(NULL == parent, "parent is NULL!!");

	Evas_Object *layout = elm_layout_add(parent);
	assertm_if(NULL == layout, "layout is NULL!!");

	if (title && title[0] != '\0') {
		elm_layout_theme_set(layout, "layout", "editfield", "title");
		elm_object_part_text_set(layout, "elm.text", title);
	}
	else {
		elm_layout_theme_set(layout, "layout", "editfield", "default");
	}

	Evas_Object *entry = elm_entry_add(parent);
	assertm_if(NULL == entry, "entry is NULL!!");

	elm_object_part_content_set(layout, "elm.swallow.content", entry);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (guide && guide[0] != '\0') {
		elm_object_part_text_set(layout, "elm.guidetext", guide);
	}

	elm_entry_single_line_set(entry, single_line);
	elm_entry_scrollable_set(entry, single_line);
	elm_entry_editable_set(entry, is_editable);
	elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
	evas_object_show(layout);

	return layout;
}

Evas_Object *wfd_draw_pop_type_display(Evas_Object * win, wfd_popup_t * pop)
{
    __WFD_APP_FUNC_ENTER__;

    Evas_Object *popup = NULL;
    Evas_Object *label = NULL;
    Evas_Object *layout = NULL;
    Evas_Object *progressbar = NULL;
    Evas_Object *time = NULL;
    Evas_Object *btn1 = NULL;
    Evas_Object *btn2 = NULL;
    static wfd_wps_display_popup_t wps_display_popup;

    popup = elm_popup_add(win);
    elm_object_style_set(popup, "customstyle");
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, 0.0);

    Evas_Object *box = elm_box_add(popup);
    if (!box) {
	evas_object_del(popup);
	popup = NULL;
	return NULL;
    }

    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);
    elm_object_part_content_set(popup, NULL, box);
    evas_object_show(box);

    /* add label */
    label = elm_label_add(box);
    elm_object_style_set(label, "popup/default");
    elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
    elm_object_text_set(label, pop->text);
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, 0.0);
    elm_box_pack_end(box, label);
    evas_object_show(label);

    /* add progressbar */
    progressbar = elm_progressbar_add(box);
    elm_object_style_set(progressbar, "list_progress");
    elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
    evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.0);
    evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, 0.0);
    elm_progressbar_value_set(progressbar, 0.0);
    elm_box_pack_end(box, progressbar);
    evas_object_show(progressbar);

    /* add time */
    time = elm_label_add(box);
    elm_object_style_set(time, "popup/default");
    elm_object_text_set(time, "00:00");
    evas_object_size_hint_weight_set(time, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(time, EVAS_HINT_FILL, 0.0);
    elm_box_pack_end(box, time);
    evas_object_show(time);

    /* start progressbar timer */
    wps_display_popup.step = 0;
    wps_display_popup.progressbar= progressbar;
    wps_display_popup.time = time;
    pb_timer = ecore_timer_add(1, _fn_pb_timer, &wps_display_popup);

    /* add cancel buttons */
    btn1 = elm_button_add(popup);
    elm_object_style_set(btn1, "popup_button/default");
    elm_object_text_set(btn1, pop->label1);
    elm_object_part_content_set(popup, "button1", btn1);
    evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data1);

    btn2 = elm_button_add(popup);
    elm_object_style_set(btn2, "popup_button/default");
    elm_object_text_set(btn2, pop->label2);
    elm_object_part_content_set(popup, "button2", btn2);
    evas_object_smart_callback_add(btn2, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data2);

    elm_object_content_set(popup, box);
    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}

Evas_Object *wfd_draw_pop_type_keypad(Evas_Object * win, wfd_popup_t * pop)
{
    __WFD_APP_FUNC_ENTER__;
    wfd_appdata_t *ad = wfd_get_appdata();

    Evas_Object *conformant = NULL;
    Evas_Object *layout = NULL;
    Evas_Object *pinpopup = NULL;
    Evas_Object *label = NULL;
    Evas_Object *btn1 = NULL, *btn2 = NULL;

    conformant = elm_conformant_add(win);
    assertm_if(NULL == conformant, "conformant is NULL!!");
    elm_win_conformant_set(win, EINA_TRUE);
    elm_win_resize_object_add(win, conformant);
    evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(conformant, EVAS_HINT_FILL, 0.0);
    evas_object_show(conformant);

    layout = elm_layout_add(conformant);
    elm_object_content_set(conformant, layout);

    pinpopup = elm_popup_add(layout);
    assertm_if(NULL == pinpopup, "pinpopup is NULL!!");
    elm_object_style_set(pinpopup, "customstyle");
    evas_object_size_hint_weight_set(pinpopup, EVAS_HINT_EXPAND, 0.0);
    elm_object_part_text_set(pinpopup, "title,text", _("IDS_WFD_POP_TITILE_CONNECTION"));

    Evas_Object *box = elm_box_add(pinpopup);
    if (!box) {
	evas_object_del(pinpopup);
	pinpopup = NULL;
	return NULL;
    }

    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);
    elm_object_part_content_set(pinpopup, NULL, box);
    evas_object_show(box);

    /* add label */
    label = elm_label_add(box);
    elm_object_style_set(label, "popup/default");
    elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
    elm_object_text_set(label, pop->text);
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_box_pack_end(box, label);
    evas_object_show(label);

    /* add password */
    Evas_Object *body = elm_layout_add(box);
    if (!body) {
	evas_object_del(pinpopup);
	pinpopup = NULL;
	return NULL;
    }

    elm_layout_theme_set(body, "layout", "dialogue", "1icon");
    evas_object_size_hint_weight_set(body, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(body, EVAS_HINT_FILL, 0.0);

    Evas_Object *editfield_pin = _add_edit_field(body, NULL, NULL, EINA_TRUE, EINA_TRUE);
    Evas_Object *entry_pin = elm_object_part_content_get(editfield_pin, "elm.swallow.content");
    ad->pin_entry = entry_pin;
    evas_object_smart_callback_add(ad->pin_entry, "changed", _smart_ime_cb, NULL);
    elm_object_part_content_set(body, "elm.icon", editfield_pin);
    elm_box_pack_end(box, body);
    evas_object_show(body);

//#ifdef HIDE_PIN_NUMBER
#if 1
    elm_entry_password_set(ad->pin_entry, TRUE);

    Evas_Object *check = elm_check_add(box);
    elm_object_text_set(check, _("Show password"));
    elm_object_focus_allow_set(check, EINA_FALSE);
    evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(check, EVAS_HINT_FILL, 0.0);
    evas_object_smart_callback_add(check, "changed", _check_changed_cb, NULL);
    evas_object_show(check);
    elm_box_pack_end(box, check);
#endif

    /* add buttons */
    btn1 = elm_button_add(pinpopup);
    elm_object_style_set(btn1, "popup_button/default");
    elm_object_text_set(btn1, pop->label1);
    elm_object_part_content_set(pinpopup, "button1", btn1);
    evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data1);

    btn2 = elm_button_add(pinpopup);
    elm_object_style_set(btn2, "popup_button/default");
    elm_object_text_set(btn2, pop->label2);
    elm_object_part_content_set(pinpopup, "button2", btn2);
    evas_object_smart_callback_add(btn2, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data2);

    elm_object_content_set(pinpopup, box);
    evas_object_show(pinpopup);
    evas_object_show(win);
    elm_object_focus_set(ad->pin_entry, EINA_TRUE);

    __WFD_APP_FUNC_EXIT__;

    return pinpopup;
}

void wfd_prepare_popup(int type, void *userdata)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();
	wfd_popup_t *pop = ad->popup_data;

	wfd_destroy_popup();

	memset(pop, 0, sizeof(wfd_popup_t));

	pop->type = type;

	switch (pop->type)
	{

	case /* MT */ WFD_POP_APRV_CONNECTION_WPS_PUSHBUTTON_REQ:
	{
		snprintf(pop->text, sizeof(pop->text), IDS_WFD_POP_CONNECT_Q, // WFD_STR_POP_APRV_CONNECT4
				ad->peer_name);
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_YES")); // IDS_WFD_BUTN_YES
		snprintf(pop->label2, sizeof(pop->label2), "%s", dgettext("sys_string", "IDS_COM_SK_NO")); // WFD_STR_BUTN_NO
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_PBC_YES;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_c(ad->win, pop);
	}
	break;

	case /* MT */ WFD_POP_APRV_CONNECTION_WPS_DISPLAY_REQ:
	{
		char *pin = (char *) userdata;
		snprintf(pop->text, sizeof(pop->text), IDS_WFD_POP_CONNECT_Q, //WFD_STR_POP_APRV_CONNECT4"<br>PIN:%s"
				ad->peer_name, WFD_POP_TIMER_120, ad->pin_number);
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_YES")); // IDS_WFD_BUTN_YES
		snprintf(pop->label2, sizeof(pop->label2), "%s", dgettext("sys_string", "IDS_COM_SK_NO")); // WFD_STR_BUTN_NO
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_DISPLAY_YES;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_display(ad->win, pop);
	}
	break;

	case /*MO/MT */ WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ:
	{
		snprintf(pop->text, sizeof(pop->text), IDS_WFD_POP_ENTER_PIN, // WFD_STR_POP_APRV_CONNECT4
				ad->peer_name, WFD_POP_TIMER_120, ad->peer_name);
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_YES")); // IDS_WFD_BUTN_YES
		snprintf(pop->label2, sizeof(pop->label2), "%s", dgettext("sys_string", "IDS_COM_SK_NO")); // WFD_STR_BUTN_NO
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_keypad(ad->win, pop);
	}
	break;

	case /* MT */ WFD_POP_PROG_CONNECT:
	{
		snprintf(pop->text, sizeof(pop->text), "%s", _("IDS_WFD_POP_CONNECTING")); // WFD_STR_POP_PROG_CONNECT
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL")); // WFD_STR_BTN_CANCEL
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_PROG_CONNECT_CANCEL;

		ad->popup = wfd_draw_pop_type_b(ad->win, pop);
	}
	break;

	case /* MO */ WFD_POP_PROG_CONNECT_WITH_KEYPAD:
	{
		snprintf(pop->text, sizeof(pop->text), IDS_WFD_POP_ENTER_PIN, // WFD_STR_POP_ENTER_PIN
				ad->peer_name, WFD_POP_TIMER_120, ad->peer_name);
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_OK")); // WFD_STR_BTN_OK
		snprintf(pop->label2, sizeof(pop->label2), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL")); // WFD_STR_BTN_CANCEL
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_keypad(ad->win, pop);
	}
	break;

	case /* MO/MT */ WFD_POP_PROG_CONNECT_WITH_PIN:
		snprintf(pop->text, sizeof(pop->text), "%s %s", IDS_WFD_POP_CONNECTING_WITH_PIN, ad->pin_number); // WFD_STR_POP_PROG_CONNECT_WITH_PIN
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL")); // WFD_STR_BTN_CANCEL
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_PROG_CONNECT_CANCEL;

		ad->popup = wfd_draw_pop_type_b(ad->win, pop);
		break;

	case WFD_POP_PROG_CONNECT_CANCEL:
	{
		snprintf(pop->text, sizeof(pop->text), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL")); // WFD_STR_POP_PROG_CANCEL
		pop->timeout = WFD_POP_TIMER_120;
		ad->popup = wfd_draw_pop_type_a(ad->win, pop);
	}
	break;

	case WFD_POP_INCORRECT_PIN:
		snprintf(pop->text, sizeof(pop->text), "%s", _("IDS_WFD_POP_PIN_INVALID")); // WFD_STR_POP_INVALID_PIN
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_OK")); // WFD_STR_BTN_OK
		pop->timeout = WFD_POP_TIMER_3;
		pop->resp_data1 = WFD_POP_RESP_OK;
		break;

	case WFD_POP_NOTI_CONNECTED:
		snprintf(pop->text, sizeof(pop->text), "%s", _("IDS_WFD_POP_CONNECTED")); // WFD_STR_POP_NOTI_CONNECTED
		pop->timeout = WFD_POP_TIMER_3;

		ad->popup = wfd_draw_pop_type_e(ad->win, pop);
		break;

	case WFD_POP_FAIL_CONNECT:
		snprintf(pop->text, sizeof(pop->text), "%s", _("IDS_WFD_POP_DISCONNECT_FAILED")); // IDS_WFD_POP_DISCONNECT_FAILED
		pop->timeout = WFD_POP_TIMER_3;

		ad->popup = wfd_draw_pop_type_e(ad->win, pop);
		break;
	case WFD_POP_AUTOMATIC_TURN_OFF:
		snprintf(pop->text, sizeof(pop->text), "%s", _("IDS_WFD_POP_AUTOMATIC_TURN_OFF"));
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_OK"));
		snprintf(pop->label2, sizeof(pop->label2), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_AUTOMATIC_TURNOFF_OK;

		ad->popup = wfd_draw_pop_type_b(ad->win, pop);

		break;
	default:
		break;
	}

	__WFD_APP_FUNC_EXIT__;
	return;
}

void wfd_tickernoti_popup(char *msg)
{
	__WFD_APP_FUNC_ENTER__;

	int ret = -1;
	bundle *b = NULL;

	b = bundle_create();
	if (!b) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "FAIL: bundle_create()\n");
		return;
	}

	/* tickernoti style */
	ret = bundle_add(b, "0", "info");
	if (ret) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to add tickernoti style\n");
		bundle_free(b);
		return;
	}

	/* popup text */
	ret = bundle_add(b, "1", msg);
	if (ret) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to add popup text\n");
		bundle_free(b);
		return;
	}

	/* orientation of tickernoti */
	ret = bundle_add(b, "2", "0");
	if (ret) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to add orientation of tickernoti\n");
		bundle_free(b);
		return;
	}

	/* timeout(second) of tickernoti */
	ret = bundle_add(b, "3", "3");
	if (ret) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to add timeout of tickernoti\n");
		bundle_free(b);
		return;
	}

	/* launch tickernoti */
	ret = syspopup_launch(TICKERNOTI_SYSPOPUP, b);
	if (ret) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Fail to launch syspopup\n");
	}

	bundle_free(b);
	__WFD_APP_FUNC_EXIT__;
}

