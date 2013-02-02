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

/**
 *	This function let the ug make a callback for click the button in popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void __popup_resp_cb(void *data, Evas_Object * obj, void *event_info)
{
	__WDPOP_LOG_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();
	int result = -1;
	int resp = (int) data;
	char msg[WFD_POP_STR_MAX_LEN] = {0};

	WDPOP_LOGI( "popup resp : %d\n", resp);

	switch (resp) {
	case /* MT */ WFD_POP_RESP_APRV_CONNECT_PBC_YES:
	{
		WDPOP_LOGI(
				"WFD_POP_RESP_APRV_CONNECT_PBC_YES\n");
		wfd_destroy_popup();

		result = wifi_direct_accept_connection(ad->peer_mac);
		WDPOP_LOGD(
				"wifi_direct_accept_connection() result=[%d]\n",
				result);
		if (result == WIFI_DIRECT_ERROR_NONE) {
			/* tickernoti popup */
			wfd_tickernoti_popup(_("IDS_WFD_POP_CONNECTING"));
		} else {
			WDPOP_LOGE(
					"wifi_direct_accept_connection() FAILED!!\n");
			evas_object_hide(ad->win);

			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, IDS_WFD_POP_CONNECT_FAILED, ad->peer_name);
			wfd_tickernoti_popup(msg);
		}
	}
	break;

	case WFD_POP_RESP_APRV_CONNECT_INVITATION_YES:
		WDPOP_LOGI(
				"WFD_POP_RESP_APRV_CONNECT_INVITATION_YES [" MACSTR "]\n", MAC2STR(ad->peer_mac));
		wfd_destroy_popup();

		result = wifi_direct_connect(ad->peer_mac);
		WDPOP_LOGD(
				"wifi_direct_connect() result=[%d]\n",
				result);
		if (result == WIFI_DIRECT_ERROR_NONE) {
			/* tickernoti popup */
			wfd_tickernoti_popup(_("IDS_WFD_POP_CONNECTING"));
		} else {
			WDPOP_LOGE(
					"wifi_direct_connect() FAILED!!\n");
			evas_object_hide(ad->win);

			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, IDS_WFD_POP_CONNECT_FAILED, ad->peer_name);
			wfd_tickernoti_popup(msg);
		}

	break;

	case /* MT */ WFD_POP_RESP_APRV_CONNECT_DISPLAY_OK:
	{
		char *pin = NULL;

		WDPOP_LOGI(
				"WFD_POP_RESP_APRV_CONNECT_DISPLAY_OK\n");
		wfd_destroy_popup();

		if (wifi_direct_generate_wps_pin() != WIFI_DIRECT_ERROR_NONE) {
			WDPOP_LOGD( "wifi_direct_generate_wps_pin() is failed\n");
			return;
		}

		if (wifi_direct_get_wps_pin(&pin) != WIFI_DIRECT_ERROR_NONE) {
			WDPOP_LOGD( "wifi_direct_generate_wps_pin() is failed\n");
			return;
		}

		strncpy(ad->pin_number, pin, 64);
		free(pin);
		pin = NULL;
		WDPOP_LOGD( "button ok: pin [%s]", ad->pin_number);

		result = wifi_direct_accept_connection(ad->peer_mac);
		if (result == WIFI_DIRECT_ERROR_NONE) {
			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_PIN, NULL);
		} else {
			WDPOP_LOGD(
				"wifi_direct_accept_connection() failed. result=[%d]\n", result);
			/* tickernoti popup */
			wfd_tickernoti_popup(IDS_WFD_POP_CONNECT_FAILED);
		}
	}
	break;

	case /* MO */ WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK:
	{
		WDPOP_LOGI(
				"WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK\n");

		wfd_destroy_popup();

		int len = strlen(ad->pin_number);
		WDPOP_LOGD( "button ok: pin [%s]", ad->pin_number);

		if (len != 8) {
			if (len > 8)
				wfd_tickernoti_popup(_("IDS_CST_BODY_PASSWORD_TOO_LONG"));
			else
				wfd_tickernoti_popup(_("IDS_ST_BODY_PASSWORD_TOO_SHORT"));
			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_KEYPAD, (void *) NULL);
			return;
		}

		int result = 0;
		WDPOP_LOGD( "pin=[%s]\n", ad->pin_number);

		result = wifi_direct_set_wps_pin(ad->pin_number);
		if (result != WIFI_DIRECT_ERROR_NONE) {
			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, IDS_WFD_POP_CONNECT_FAILED, ad->peer_name);
			wfd_tickernoti_popup(msg);
			return;
		}

		//result = wifi_direct_activate_pushbutton();
		result = wifi_direct_accept_connection(ad->peer_mac);
		WDPOP_LOGD(
				"wifi_direct_accept_connection(%s) result=[%d]\n",
				ad->peer_mac, result);
		if (result != WIFI_DIRECT_ERROR_NONE) {
			WDPOP_LOGE(
					"wifi_direct_accept_connection() FAILED!!\n");
			evas_object_hide(ad->win);

			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, IDS_WFD_POP_CONNECT_FAILED, ad->peer_name);
			wfd_tickernoti_popup(msg);
		}
	}
	break;

	case /* MT */ WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES:
	{
		WDPOP_LOGI(
				"WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES\n");
		wfd_destroy_popup();
		if (pb_timer) {
			ecore_timer_del(pb_timer);
			pb_timer = NULL;
		}

		wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_KEYPAD, (void *) NULL);
	}
	break;

	case /* MT */ WFD_POP_RESP_APRV_CONNECT_NO:
	{
		WDPOP_LOGI(
				"WFD_POP_RESP_APRV_CONNECT_NO: destroy_popup...\n");

		wfd_destroy_popup();
		if (pb_timer) {
			ecore_timer_del(pb_timer);
			pb_timer = NULL;
		}

		result = wifi_direct_disconnect(ad->peer_mac);
		WDPOP_LOGD(
				"wifi_direct_disconnect[%s] result=[%d]\n",
				ad->peer_mac, result);
	}
	break;

	default:
	{
		WDPOP_LOGE( "Unknown respone\n");
		evas_object_hide(ad->win);
	}
	break;
	}

	__WDPOP_LOG_FUNC_EXIT__;
}

/**
 *	This function let the app destroy the popup
 *	@return   void
 *	@param[in] null
 */
void wfd_destroy_popup()
{
	__WDPOP_LOG_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();

	if (ad == NULL) {
		WDPOP_LOGE( "ad is NULL\n");
		return;
	}

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	if (ad->popup_timeout_handle > 0) {
		g_source_remove(ad->popup_timeout_handle);
		ad->popup_timeout_handle = 0;
	}

	evas_object_hide(ad->win);

	__WDPOP_LOG_FUNC_EXIT__;
	return;
}

/**
 *	This function let the app create a popup which includes no button
 *	@return   popup
 *	@param[in] win the window object
 *	@param[in] pop the pointer to the prepared popup
 */
static Evas_Object *wfd_draw_pop_type_a(Evas_Object * win, wfd_popup_t * pop)
{
	__WDPOP_LOG_FUNC_ENTER__;
	Evas_Object *popup;

	popup = elm_popup_add(win);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, pop->text);
	elm_popup_timeout_set(popup, pop->timeout);
	evas_object_show(popup);
	evas_object_show(win);

	__WDPOP_LOG_FUNC_EXIT__;
	return popup;
}

/**
 *	This function let the app create a popup which includes one button
 *	@return   popup
 *	@param[in] win the window object
 *	@param[in] pop the pointer to the prepared popup
 */
static Evas_Object *wfd_draw_pop_type_b(Evas_Object * win, wfd_popup_t * pop)
{
	__WDPOP_LOG_FUNC_ENTER__;
	Evas_Object *popup = NULL;
	Evas_Object *btn = NULL;

	popup = elm_popup_add(win);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, pop->text);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup_button/default");
	elm_object_text_set(btn, pop->label1);
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", __popup_resp_cb, (void *) pop->resp_data1);

	evas_object_show(popup);
	evas_object_show(win);

	__WDPOP_LOG_FUNC_EXIT__;
	return popup;
}

/**
 *	This function let the app create a popup which includes two buttons
 *	@return   popup
 *	@param[in] win the window object
 *	@param[in] pop the pointer to the prepared popup
 */
static Evas_Object *wfd_draw_pop_type_c(Evas_Object * win, wfd_popup_t * pop)
{
	__WDPOP_LOG_FUNC_ENTER__;
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

	__WDPOP_LOG_FUNC_EXIT__;
	return popup;
}

/**
 *	This function let the ug make a change callback for password input
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _smart_ime_cb(void *data, Evas_Object * obj, void *event_info)
{
	__WDPOP_LOG_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();

	Ecore_IMF_Context *imf_context = NULL;
	imf_context = (Ecore_IMF_Context *) ad->pin_entry;

	if (NULL == imf_context) {
		WDPOP_LOGE( "Error!!! Ecore_IMF_Context is NULL!!");
		return;
	}

	const char *txt = elm_entry_markup_to_utf8(elm_entry_entry_get((const Evas_Object *) imf_context));
	if (NULL != txt) {
		WDPOP_LOGD( "* text [%s], len=[%d]", txt, strlen(txt));
		strncpy(ad->pin_number, txt, sizeof(ad->pin_number));
	} else {
		WDPOP_LOGD( "Err!");
	}

	__WDPOP_LOG_FUNC_EXIT__;
}

/**
 *	This function let the app make a change callback for password checkbox
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _check_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	wfd_appdata_t *ad = wfd_get_appdata();

	if (obj == NULL) {
		return;
	}

	Eina_Bool state = elm_check_state_get(obj);
	elm_entry_password_set(ad->pin_entry, !state);
}

/**
 *	This function let the app make a callback for progressbar timer
 *	@return   if stop the timer, return ECORE_CALLBACK_CANCEL, else return ECORE_CALLBACK_RENEW
 *	@param[in] data the pointer to the wps structure
 */
static Eina_Bool _fn_pb_timer(void *data)
{
	int step = 0;
	double value = 0.0;
	char time_label[32] = {0};
	wfd_wps_display_popup_t *wps_display_popup = (wfd_wps_display_popup_t *) data;

	if (NULL == wps_display_popup) {
		WDPOP_LOGE( "Param is NULL.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	Evas_Object *progressbar = NULL;
	Evas_Object *time = NULL;

	progressbar = wps_display_popup->progressbar;
	time = wps_display_popup->time;
	value = elm_progressbar_value_get(progressbar);

	if (value >= 1.0) {
		WDPOP_LOGE( "Progress end.\n");
		if (pb_timer) {
			ecore_timer_del(pb_timer);
			pb_timer = NULL;
		}
		wfd_destroy_popup();
		return ECORE_CALLBACK_CANCEL;
	}

	wps_display_popup->step++;
	step = wps_display_popup->step;
	value = ((double)step) / WFD_POP_TIMER_120;
	elm_progressbar_value_set(progressbar, value);
	WDPOP_LOGD( "step: %d, value: %f\n", wps_display_popup->step, value);

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

/**
 *	This function let the app create a edit filed
 *	@return   edit filed
 *	@param[in] parent the parent object
 *	@param[in] title the pointer to the title of edit field
 *	@param[in] guide the pointer to the text of guide
 *	@param[in] single_line whether it can support single line
 *	@param[in] is_editable whether it is avaliable to edit
 */
static Evas_Object *_add_edit_field(Evas_Object *parent, const char *title, const char *guide, Eina_Bool single_line, Eina_Bool is_editable)
{
	assertm_if(NULL == parent, "parent is NULL!!");

	Evas_Object *layout = elm_layout_add(parent);
	assertm_if(NULL == layout, "layout is NULL!!");

	if (title && title[0] != '\0') {
		elm_layout_theme_set(layout, "layout", "editfield", "title");
		elm_object_part_text_set(layout, "elm.text", title);
	} else {
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

/**
 *	This function let the app create a display popup
 *	@return   display popup
 *	@param[in] win the window object
 *	@param[in] pop the pointer to the prepared popup
 */
Evas_Object *wfd_draw_pop_type_display(Evas_Object * win, wfd_popup_t * pop)
{
	__WDPOP_LOG_FUNC_ENTER__;

	Evas_Object *popup = NULL;
	Evas_Object *label = NULL;
	Evas_Object *progressbar = NULL;
	Evas_Object *time = NULL;
	static wfd_wps_display_popup_t wps_display_popup;

	popup = elm_popup_add(win);
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
	wps_display_popup.progressbar = progressbar;
	wps_display_popup.time = time;
	if(pb_timer)
		ecore_timer_del(pb_timer);
	pb_timer = ecore_timer_add(1, _fn_pb_timer, &wps_display_popup);

	/* add buttons */
	if (pop->resp_data1 == WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES ||
		pop->resp_data1 == WFD_POP_RESP_APRV_CONNECT_NO) {
		Evas_Object *btn1 = NULL;
		btn1 = elm_button_add(popup);
		elm_object_style_set(btn1, "popup_button/default");
		elm_object_text_set(btn1, pop->label1);
		elm_object_part_content_set(popup, "button1", btn1);
		evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb,
			(void *) pop->resp_data1);
	}

	if (pop->resp_data2 == WFD_POP_RESP_APRV_CONNECT_NO) {
		Evas_Object *btn2 = NULL;
		btn2 = elm_button_add(popup);
		elm_object_style_set(btn2, "popup_button/default");
		elm_object_text_set(btn2, pop->label2);
		elm_object_part_content_set(popup, "button2", btn2);
		evas_object_smart_callback_add(btn2, "clicked", __popup_resp_cb,
			(void *) pop->resp_data2);
	}

	elm_object_content_set(popup, box);
	evas_object_show(popup);
	evas_object_show(win);

	__WDPOP_LOG_FUNC_EXIT__;
	return popup;
}

/**
 *	This function let the app create a keypad popup
 *	@return   keypad popup
 *	@param[in] win the window object
 *	@param[in] pop the pointer to the prepared popup
 */
Evas_Object *wfd_draw_pop_type_keypad(Evas_Object * win, wfd_popup_t * pop)
{
	__WDPOP_LOG_FUNC_ENTER__;
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

	elm_entry_password_set(ad->pin_entry, TRUE);

	Evas_Object *check = elm_check_add(box);
	elm_object_text_set(check, _("Show password"));
	elm_object_focus_allow_set(check, EINA_FALSE);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, 0.0);
	evas_object_smart_callback_add(check, "changed", _check_changed_cb, NULL);
	evas_object_show(check);
	elm_box_pack_end(box, check);

	/* add buttons */
	btn1 = elm_button_add(pinpopup);
	elm_object_style_set(btn1, "popup_button/default");
	elm_object_text_set(btn1, pop->label1);
	elm_object_part_content_set(pinpopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb, (void *)pop->resp_data1);

	btn2 = elm_button_add(pinpopup);
	elm_object_style_set(btn2, "popup_button/default");
	elm_object_text_set(btn2, pop->label2);
	elm_object_part_content_set(pinpopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", __popup_resp_cb, (void *)pop->resp_data2);

	elm_object_content_set(pinpopup, box);
	evas_object_show(pinpopup);
	evas_object_show(win);
	elm_object_focus_set(ad->pin_entry, EINA_TRUE);

	__WDPOP_LOG_FUNC_EXIT__;
	return pinpopup;
}

/**
 *	This function let the app create a popup
 *	@return   void
 *	@param[in] type the type of popup
 *	@param[in] userdata the pointer to the data which will be used
 */
void wfd_prepare_popup(int type, void *userdata)
{
	__WDPOP_LOG_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();
	wfd_popup_t *pop = ad->popup_data;

	wfd_destroy_popup();

	memset(pop, 0, sizeof(wfd_popup_t));

	pop->type = type;

	switch (pop->type) {
	case /* MT */ WFD_POP_APRV_CONNECTION_WPS_PUSHBUTTON_REQ:
		snprintf(pop->text, sizeof(pop->text), IDS_WFD_POP_CONNECT_Q,
				ad->peer_name);
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_YES"));
		snprintf(pop->label2, sizeof(pop->label2), "%s", dgettext("sys_string", "IDS_COM_SK_NO"));
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_PBC_YES;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_c(ad->win, pop);
		break;

	case WFD_POP_APRV_CONNECTION_INVITATION_REQ:
		snprintf(pop->text, sizeof(pop->text), IDS_WFD_POP_CONNECT_Q,
				ad->peer_name);
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_YES"));
		snprintf(pop->label2, sizeof(pop->label2), "%s", dgettext("sys_string", "IDS_COM_SK_NO"));
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_INVITATION_YES;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_c(ad->win, pop);
 		break;

	case /* MT */ WFD_POP_APRV_CONNECTION_WPS_DISPLAY_REQ:
		snprintf(pop->text, sizeof(pop->text), IDS_WFD_POP_ENTER_PIN_WITH_KEYPAD,
				ad->peer_name);
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_OK"));
		snprintf(pop->label2, sizeof(pop->label2), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_DISPLAY_OK;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_c(ad->win, pop);
 		break;

	case /* MT */ WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ:
		snprintf(pop->text, sizeof(pop->text), IDS_WFD_POP_ENTER_PIN,
				ad->peer_name, WFD_POP_TIMER_120);
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_OK"));
		snprintf(pop->label2, sizeof(pop->label2), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_c(ad->win, pop);
 		break;

	case /* MT */ WFD_POP_PROG_CONNECT:
		snprintf(pop->text, sizeof(pop->text), "%s", _("IDS_WFD_POP_CONNECTING"));
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_b(ad->win, pop);
 		break;

	case /* MO */ WFD_POP_PROG_CONNECT_WITH_KEYPAD:
		snprintf(pop->text, sizeof(pop->text), IDS_WFD_POP_CONNECTING_WITH_KEYPAD,
				ad->peer_name, WFD_POP_TIMER_120, ad->peer_name);
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_SK_OK"));
		snprintf(pop->label2, sizeof(pop->label2), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_keypad(ad->win, pop);
 		break;

	case /* MO/MT */ WFD_POP_PROG_CONNECT_WITH_PIN:
		snprintf(pop->text, sizeof(pop->text), IDS_WFD_POP_CONNECTING_WITH_PIN,
			ad->peer_name, WFD_POP_TIMER_120, ad->pin_number);
		snprintf(pop->label1, sizeof(pop->label1), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_display(ad->win, pop);
 		break;

	case WFD_POP_PROG_CONNECT_CANCEL:
		snprintf(pop->text, sizeof(pop->text), "%s", dgettext("sys_string", "IDS_COM_POP_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		ad->popup = wfd_draw_pop_type_a(ad->win, pop);
 		break;

	default:
		break;
	}

	__WDPOP_LOG_FUNC_EXIT__;
	return;
}

/**
 *	This function let the app create a tickernoti syspopup
 *	@return   void
 *	@param[in] msg the pointer to message of tickernoti
 */
void wfd_tickernoti_popup(char *msg)
{
	__WDPOP_LOG_FUNC_ENTER__;

	int ret = -1;
	bundle *b = NULL;

	b = bundle_create();
	if (!b) {
		WDPOP_LOGD( "FAIL: bundle_create()\n");
		return;
	}

	/* tickernoti style */
	ret = bundle_add(b, "0", "info");
	if (ret) {
		WDPOP_LOGD( "Fail to add tickernoti style\n");
		bundle_free(b);
		return;
	}

	/* popup text */
	ret = bundle_add(b, "1", msg);
	if (ret) {
		WDPOP_LOGD( "Fail to add popup text\n");
		bundle_free(b);
		return;
	}

	/* orientation of tickernoti */
	ret = bundle_add(b, "2", "0");
	if (ret) {
		WDPOP_LOGD( "Fail to add orientation of tickernoti\n");
		bundle_free(b);
		return;
	}

	/* timeout(second) of tickernoti */
	ret = bundle_add(b, "3", "3");
	if (ret) {
		WDPOP_LOGD( "Fail to add timeout of tickernoti\n");
		bundle_free(b);
		return;
	}

	/* launch tickernoti */
	ret = syspopup_launch(TICKERNOTI_SYSPOPUP, b);
	if (ret) {
		WDPOP_LOGD( "Fail to launch syspopup\n");
	}

	bundle_free(b);
	__WDPOP_LOG_FUNC_EXIT__;
}

